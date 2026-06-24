#include "vigem_device.h"

#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/core/print_string.hpp>

using namespace godot;

ViGEmDevice::~ViGEmDevice() {
	if (!_target) { return; }
	if (vigem_target_is_attached(_target)) {
		if (const VIGEM_ERROR err = vigem_target_remove(_client, _target); !VIGEM_SUCCESS(err)) {
			print_error(vformat("ViGEm target unplug failed with error code: 0x%x.", (int)err));
		}
	}
	vigem_target_free(_target);
}

Error ViGEmDevice::init(PVIGEM_CLIENT p_client) {
	ERR_FAIL_COND_V_MSG(_client, ERR_ALREADY_IN_USE, "ViGEmDevice already initialized.");
	ERR_FAIL_COND_V(!_target, ERR_OUT_OF_MEMORY);
	ERR_FAIL_COND_V(!p_client, ERR_INVALID_PARAMETER);
	_client = p_client;
	const VIGEM_ERROR err = vigem_target_add(_client, _target);
	ERR_FAIL_COND_V_MSG(!VIGEM_SUCCESS(err), ERR_CONNECTION_ERROR, vformat("ViGEm target plugin failed with error code: 0x%x.", (int)err));
	return OK;
}

Error ViGEmDevice::send_event(const Ref<InputEvent> &p_evt) {
	ERR_FAIL_COND_V_MSG(!_client, ERR_UNCONFIGURED, "ViGEmDevice not initialized.");
	ERR_FAIL_COND_V(p_evt.is_null(), ERR_INVALID_PARAMETER);
	if (const Ref<InputEventJoypadMotion> axis_evt = p_evt; axis_evt.is_valid()) { // likely
		const VIGEM_ERROR err = update_axis(axis_evt);
		ERR_FAIL_COND_V_MSG(!VIGEM_SUCCESS(err), ERR_CONNECTION_ERROR, vformat("ViGEm target axis update failed with error code: 0x%x.", (int)err));
	}
	else if (const Ref<InputEventJoypadButton> btn_evt = p_evt; btn_evt.is_valid()) {
		const VIGEM_ERROR err = update_button(btn_evt);
		ERR_FAIL_COND_V_MSG(!VIGEM_SUCCESS(err), ERR_CONNECTION_ERROR, vformat("ViGEm target button update failed with error code: 0x%x.", (int)err));
	}
	else { return ERR_SKIP; }
	return OK;
}
