#include "vigem_server.h"

#include <godot_cpp/classes/os.hpp>

using namespace godot;

ViGEmServer *ViGEmServer::_inst = nullptr;

// region Threading

void ViGEmServer::_thread_func() {
	while (!_exit_thread.is_set()) {
		// TODO: routine
		OS::get_singleton()->delay_msec(1000);
		print_line("Hello from thread.");
	}
}

void ViGEmServer::_bind_methods() {
	// TODO: maybe nothing to do here
}

Error ViGEmServer::init() {
	ERR_FAIL_COND_V_MSG(_thread.ptr() && _thread->is_alive(), ERR_ALREADY_IN_USE, "ViGEmServer already initialized.");
	_thread.instantiate();
	_mutex.instantiate();
	_exit_thread.clear();
	// ReSharper disable once CppDFANullDereference
	return _thread->start(callable_mp(this, &ViGEmServer::_thread_func));
}

void ViGEmServer::finish() {
	_exit_thread.set();
	// ReSharper disable CppDFANullDereference
	if (_thread.ptr() && _thread->is_started()) { _thread->wait_to_finish(); }
	// ReSharper restore CppDFANullDereference

	// TODO: release all ViGEmDevice instances
}

// endregion
