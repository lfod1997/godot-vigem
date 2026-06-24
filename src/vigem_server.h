#ifndef GDVIGEM_VIGEM_SERVER
#define GDVIGEM_VIGEM_SERVER

#include <type_traits>

#include <windows.h>
#include <ViGEm/Client.h>

#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/core/mutex_lock.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/thread.hpp>
#include <godot_cpp/classes/mutex.hpp>
#include <godot_cpp/templates/rid_owner.hpp>

#include "vigem_device.h"

namespace aux {
template <typename>
inline constexpr bool always_false_v = false;
}

class ViGEmServer : public godot::Object {
	GDCLASS(ViGEmServer, godot::Object);

	friend void initialize_gdextension_types(godot::ModuleInitializationLevel p_level);
	friend void uninitialize_gdextension_types(godot::ModuleInitializationLevel p_level);

	static ViGEmServer *_inst;
	void _thread_func();

	godot::Ref<godot::Thread> _thread;
	godot::Ref<godot::Mutex> _mutex;
	godot::SafeFlag _exit_thread;

	PVIGEM_CLIENT _client = nullptr;
	mutable godot::RID_PtrOwner<ViGEmDevice> _device_owner;

protected:
	static void _bind_methods();

	ViGEmServer() { _inst = this; }

	godot::Error init();
	void finish();

public:
	_FORCE_INLINE_ static ViGEmServer* get_singleton() { return _inst; }

	template <typename T>
	godot::RID create_device() {
		ERR_FAIL_COND_V_MSG(!_client || !_mutex.is_valid(), {}, "ViGEmServer not initialized.");
		godot::MutexLock _{ *_mutex.ptr() };

		ViGEmDevice *d = nullptr;
		if constexpr (std::is_same_v<std::decay_t<T>, ViGEmDeviceX360>) { d = memnew(ViGEmDeviceX360); }
		else if constexpr (std::is_same_v<std::decay_t<T>, ViGEmDeviceDS4>) { d = memnew(ViGEmDeviceDS4); }
		else { static_assert(aux::always_false_v<T>); }
		ERR_FAIL_COND_V(!d, {});

		if (const godot::Error err = d->init(_client); err == godot::OK) {
			return _device_owner.make_rid(d);
		}
		else {
			godot::memdelete(d);
			ERR_FAIL_V({});
		}
	}

	godot::Error destroy_device(const godot::RID &p_device);
	godot::Error send_event_to_device(const godot::RID &p_device, const godot::Ref<godot::InputEvent> &p_evt);
	void reset_device(const godot::RID &p_device);
};

#endif // GDVIGEM_VIGEM_SERVER
