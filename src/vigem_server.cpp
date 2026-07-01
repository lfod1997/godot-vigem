// ReSharper disable CppDFAUnreachableCode
#include "vigem_server.h"

#include <godot_cpp/classes/os.hpp>

using namespace godot;

ViGEmServer *ViGEmServer::_inst = nullptr;

void ViGEmServer::_thread_func() {
	while (!_exit_thread.is_set()) {
		// TODO: routine
		OS::get_singleton()->delay_msec(1000);
	}
}

void ViGEmServer::_bind_methods() {}

Error ViGEmServer::init() {
	ERR_FAIL_COND_V_MSG(_thread.is_valid() && _thread->is_alive(), ERR_ALREADY_IN_USE, "ViGEmServer already initialized.");

	// Try connecting to driver first
	ERR_FAIL_COND_V_MSG(_client, ERR_ALREADY_IN_USE, "ViGEmClient session already started.");
	_client = vigem_alloc();
	ERR_FAIL_COND_V_MSG(!_client, ERR_OUT_OF_MEMORY, "Could not allocate memory for a new ViGEmClient session.");
	if (const VIGEM_ERROR err = vigem_connect(_client); !VIGEM_SUCCESS(err)) {
		_client = nullptr;
		ERR_FAIL_V_MSG(ERR_CANT_CONNECT, vformat("ViGEm bus connection failed with error code: 0x%x.", (int)err));
	}
	print_line("Connected to ViGEm bus driver.");

	// Ready to start server thread
	_thread.instantiate();
	_mutex.instantiate();
	_exit_thread.clear();
	// ReSharper disable once CppDFANullDereference
	return _thread->start(callable_mp(this, &ViGEmServer::_thread_func));
}

void ViGEmServer::finish() {
	_exit_thread.set();
	// ReSharper disable CppDFANullDereference
	if (_thread.is_valid() && _thread->is_started()) { _thread->wait_to_finish(); }
	// ReSharper restore CppDFANullDereference

	List<RID> devices;
	_device_owner.get_owned_list(&devices);
	for (const RID &it : devices) {
		memdelete(_device_owner.get_or_null(it));
		_device_owner.free(it);
	}

	// Disconnect from driver
	vigem_disconnect(_client);
	vigem_free(_client);
	_client = nullptr;
	print_line("Disconnected from ViGEm bus driver.");
}

Error ViGEmServer::destroy_device(const RID &p_device) {
	ERR_FAIL_COND_V_MSG(!_client || !_mutex.is_valid(), {}, "ViGEmServer not initialized.");
	MutexLock _{ *_mutex.ptr() };

	ViGEmDevice *device = _device_owner.get_or_null(p_device);
	ERR_FAIL_NULL_V_MSG(device, ERR_INVALID_PARAMETER, "Attempting to destroy an invalid device.");
	memdelete(device);
	_device_owner.free(p_device);
	return OK;
}

Error ViGEmServer::send_event_to_device(const RID &p_device, const Ref<InputEvent> &p_evt) {
	// TODO: Further enhance timing accuracy by pushing timestamped command from main thread and sending to device from server thread
	ERR_FAIL_COND_V_MSG(!_client || !_mutex.is_valid(), {}, "ViGEmServer not initialized.");
	MutexLock _{ *_mutex.ptr() };

	ViGEmDevice *device = _device_owner.get_or_null(p_device);
	ERR_FAIL_NULL_V_MSG(device, ERR_INVALID_PARAMETER, "Attempting to send event to an invalid device.");
	return device->send_event(p_evt);
}

void ViGEmServer::reset_device(const RID &p_device) {
	// TODO: Further enhance timing accuracy by pushing timestamped command from main thread and sending to device from server thread
	ERR_FAIL_COND_MSG(!_client || !_mutex.is_valid(), "ViGEmServer not initialized.");
	MutexLock _{ *_mutex.ptr() };

	ViGEmDevice *device = _device_owner.get_or_null(p_device);
	ERR_FAIL_NULL_MSG(device, "Attempting to reset an invalid device.");
	return device->reset_state();
}
