// ReSharper disable CppDFANullDereference
#include "vigem_device.h"

#include <frozen/unordered_map.h>

using namespace godot;

static constexpr frozen::unordered_map<JoyButton, XUSB_BUTTON, 15> BUTTONS_X360{
	{ JOY_BUTTON_A, XUSB_GAMEPAD_A },
	{ JOY_BUTTON_B, XUSB_GAMEPAD_B },
	{ JOY_BUTTON_X, XUSB_GAMEPAD_X },
	{ JOY_BUTTON_Y, XUSB_GAMEPAD_Y },
	{ JOY_BUTTON_BACK, XUSB_GAMEPAD_BACK },
	{ JOY_BUTTON_GUIDE, XUSB_GAMEPAD_GUIDE },
	{ JOY_BUTTON_START, XUSB_GAMEPAD_START },
	{ JOY_BUTTON_LEFT_STICK, XUSB_GAMEPAD_LEFT_THUMB },
	{ JOY_BUTTON_RIGHT_STICK, XUSB_GAMEPAD_RIGHT_THUMB },
	{ JOY_BUTTON_LEFT_SHOULDER, XUSB_GAMEPAD_LEFT_SHOULDER },
	{ JOY_BUTTON_RIGHT_SHOULDER, XUSB_GAMEPAD_RIGHT_SHOULDER },
	{ JOY_BUTTON_DPAD_UP, XUSB_GAMEPAD_DPAD_UP },
	{ JOY_BUTTON_DPAD_DOWN, XUSB_GAMEPAD_DPAD_DOWN },
	{ JOY_BUTTON_DPAD_LEFT, XUSB_GAMEPAD_DPAD_LEFT },
	{ JOY_BUTTON_DPAD_RIGHT, XUSB_GAMEPAD_DPAD_RIGHT },
};

_FORCE_INLINE_ static constexpr bool can_handle_button(const JoyButton p_btn) {
	return p_btn > JOY_BUTTON_INVALID && p_btn <= JOY_BUTTON_DPAD_RIGHT;
}

VIGEM_ERROR ViGEmDeviceX360::update_button(const Ref<InputEventJoypadButton> &p_evt) {
	const JoyButton evt_btn = p_evt->get_button_index();
	if (!can_handle_button(evt_btn)) { return VIGEM_ERROR_NONE; }
	const XUSB_BUTTON x360_btn = BUTTONS_X360.at(evt_btn);
	if (p_evt->is_pressed()) { _state.wButtons |= x360_btn; }
	else { _state.wButtons &= ~x360_btn; }
	return vigem_target_x360_update(_client, _target, _state);
}

VIGEM_ERROR ViGEmDeviceX360::update_axis(const Ref<InputEventJoypadMotion> &p_evt) {
	switch (p_evt->get_axis()) {
	case JOY_AXIS_LEFT_X: {
		_state.sThumbLX = static_cast<BYTE>(p_evt->get_axis_value() * 32767.0f);
		break;
	}
	case JOY_AXIS_LEFT_Y: {
		_state.sThumbLY = static_cast<BYTE>(p_evt->get_axis_value() * 32767.0f);
		break;
	}
	case JOY_AXIS_RIGHT_X: {
		_state.sThumbRX = static_cast<BYTE>(p_evt->get_axis_value() * 32767.0f);
		break;
	}
	case JOY_AXIS_RIGHT_Y: {
		_state.sThumbRY = static_cast<BYTE>(p_evt->get_axis_value() * 32767.0f);
		break;
	}
	case JOY_AXIS_TRIGGER_LEFT: {
		_state.bLeftTrigger = static_cast<BYTE>(p_evt->get_axis_value() * 255.0f);
		break;
	}
	case JOY_AXIS_TRIGGER_RIGHT: {
		_state.bRightTrigger = static_cast<BYTE>(p_evt->get_axis_value() * 255.0f);
		break;
	}
	default: return VIGEM_ERROR_NONE;
	}
	return vigem_target_x360_update(_client, _target, _state);
}

Error ViGEmDeviceX360::register_notification_handler(PFN_VIGEM_X360_NOTIFICATION p_handler, void *p_ctx) {
	if (const VIGEM_ERROR err = vigem_target_x360_register_notification(_client, _target, p_handler, p_ctx); VIGEM_SUCCESS(err)) { return OK; }
	else if (err == VIGEM_ERROR_CALLBACK_ALREADY_REGISTERED) { return ERR_ALREADY_IN_USE; }
	else { return ERR_CONNECTION_ERROR; }
}

void ViGEmDeviceX360::reset_state() { XUSB_REPORT_INIT(&_state); }
