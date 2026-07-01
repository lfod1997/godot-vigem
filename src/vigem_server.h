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

template <typename>
struct vigem_device_of;

template <>
struct vigem_device_of<PFN_VIGEM_X360_NOTIFICATION> {
	// Uses: void(*)(PVIGEM_CLIENT client, PVIGEM_TARGET target, UCHAR motor_l, UCHAR motor_s, UCHAR led_num, void *ctx)
	using type = ViGEmDeviceX360;
};

template <>
struct vigem_device_of<PFN_VIGEM_DS4_NOTIFICATION> {
	// Uses: void(*)(PVIGEM_CLIENT client, PVIGEM_TARGET target, UCHAR motor_l, UCHAR motor_s, DS4_LIGHTBAR_COLOR led_rgb, void *ctx)
	using type = ViGEmDeviceDS4;
};

template <typename F>
using vigem_device_of_t = typename vigem_device_of<F>::type;
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

	template <typename D>
	godot::RID create_device() {
		ERR_FAIL_COND_V_MSG(!_client || !_mutex.is_valid(), {}, "ViGEmServer not initialized.");
		godot::MutexLock _{ *_mutex.ptr() };

		ViGEmDevice *device = nullptr;
		if constexpr (std::is_same_v<std::decay_t<D>, ViGEmDeviceX360>) { device = memnew(ViGEmDeviceX360); }
		else if constexpr (std::is_same_v<std::decay_t<D>, ViGEmDeviceDS4>) { device = memnew(ViGEmDeviceDS4); }
		else { static_assert(aux::always_false_v<D>); }
		ERR_FAIL_NULL_V_MSG(device, {}, "Failed to create device; check memory usage.");

		if (const godot::Error err = device->init(_client); err == godot::OK) {
			return _device_owner.make_rid(device);
		}
		else {
			godot::memdelete(device);
			ERR_FAIL_V_MSG({}, godot::vformat("Device initialization failed: %s.", godot::UtilityFunctions::error_string(err)));
		}
	}

	template <typename F>
	// ReSharper disable once CppNotAllPathsReturnValue
	godot::Error register_notification_handler(const godot::RID &p_device, F p_handler, void *p_ctx) {
		ERR_FAIL_COND_V_MSG(!_client || !_mutex.is_valid(), {}, "ViGEmServer not initialized.");
		godot::MutexLock _{ *_mutex.ptr() };

		ViGEmDevice *device = _device_owner.get_or_null(p_device);
		ERR_FAIL_NULL_V_MSG(device, godot::ERR_INVALID_PARAMETER, "Attempting to register notification handler for an invalid device.");
		if constexpr (std::is_same_v<aux::vigem_device_of_t<std::remove_cv_t<F>>, ViGEmDeviceX360>) {
			ViGEmDeviceX360 *x360 = dynamic_cast<ViGEmDeviceX360*>(device);
			ERR_FAIL_NULL_V(x360, godot::ERR_BUG);
			return x360->register_notification_handler(p_handler, p_ctx);
		}
		else if constexpr (std::is_same_v<aux::vigem_device_of_t<std::remove_cv_t<F>>, ViGEmDeviceDS4>) {
			ViGEmDeviceDS4 *ds4 = dynamic_cast<ViGEmDeviceDS4*>(device);
			ERR_FAIL_NULL_V(ds4, godot::ERR_BUG);
			return ds4->register_notification_handler(p_handler, p_ctx);
		}
		else { static_assert(aux::always_false_v<F>); }
	}

	godot::Error destroy_device(const godot::RID &p_device);
	godot::Error send_event_to_device(const godot::RID &p_device, const godot::Ref<godot::InputEvent> &p_evt);
	void reset_device(const godot::RID &p_device);
};

#endif // GDVIGEM_VIGEM_SERVER
