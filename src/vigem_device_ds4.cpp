// ReSharper disable CppDFANullDereference
#include "vigem_device.h"

#include <frozen/unordered_map.h>

using namespace godot;

static constexpr frozen::unordered_map<JoyButton, DS4_BUTTONS, 10> BUTTONS_DS4_COMMON{
	{ JOY_BUTTON_A, DS4_BUTTON_CROSS },
	{ JOY_BUTTON_B, DS4_BUTTON_CIRCLE },
	{ JOY_BUTTON_X, DS4_BUTTON_SQUARE },
	{ JOY_BUTTON_Y, DS4_BUTTON_TRIANGLE },
	{ JOY_BUTTON_BACK, DS4_BUTTON_SHARE },
	{ JOY_BUTTON_START, DS4_BUTTON_OPTIONS },
	{ JOY_BUTTON_LEFT_STICK, DS4_BUTTON_THUMB_LEFT },
	{ JOY_BUTTON_RIGHT_STICK, DS4_BUTTON_THUMB_RIGHT },
	{ JOY_BUTTON_LEFT_SHOULDER, DS4_BUTTON_SHOULDER_LEFT },
	{ JOY_BUTTON_RIGHT_SHOULDER, DS4_BUTTON_SHOULDER_RIGHT },
};

static constexpr frozen::unordered_map<JoyButton, DS4_SPECIAL_BUTTONS, 2> BUTTONS_DS4_SPECIAL{
	{ JOY_BUTTON_GUIDE, DS4_SPECIAL_BUTTON_PS },
	{ JOY_BUTTON_TOUCHPAD, DS4_SPECIAL_BUTTON_TOUCHPAD },
};

static constexpr frozen::unordered_map<JoyButton, ViGEmDeviceDS4::DpadButtonBit, 4> BITS_DS4_DPAD{
	{ JOY_BUTTON_DPAD_UP, ViGEmDeviceDS4::DpadButtonBit::DS4_DPAD_UP_BIT },
	{ JOY_BUTTON_DPAD_DOWN, ViGEmDeviceDS4::DpadButtonBit::DS4_DPAD_DOWN_BIT },
	{ JOY_BUTTON_DPAD_LEFT, ViGEmDeviceDS4::DpadButtonBit::DS4_DPAD_LEFT_BIT },
	{ JOY_BUTTON_DPAD_RIGHT, ViGEmDeviceDS4::DpadButtonBit::DS4_DPAD_RIGHT_BIT },
};

static constexpr frozen::unordered_map<uint8_t, DS4_DPAD_DIRECTIONS, 16> DIRS_DS4_DPAD{
	{ 0b0000, DS4_BUTTON_DPAD_NONE },
	{ 0b0101, DS4_BUTTON_DPAD_NORTHWEST },
	{ 0b0100, DS4_BUTTON_DPAD_WEST },
	{ 0b0110, DS4_BUTTON_DPAD_SOUTHWEST },
	{ 0b0010, DS4_BUTTON_DPAD_SOUTH },
	{ 0b1010, DS4_BUTTON_DPAD_SOUTHEAST },
	{ 0b1000, DS4_BUTTON_DPAD_EAST },
	{ 0b1001, DS4_BUTTON_DPAD_NORTHEAST },
	{ 0b0001, DS4_BUTTON_DPAD_NORTH },
	// Impossibles:
	{ 0b0011, DS4_BUTTON_DPAD_NONE },
	{ 0b1100, DS4_BUTTON_DPAD_NONE },
	{ 0b1110, DS4_BUTTON_DPAD_NONE },
	{ 0b1101, DS4_BUTTON_DPAD_NONE },
	{ 0b1011, DS4_BUTTON_DPAD_NONE },
	{ 0b0111, DS4_BUTTON_DPAD_NONE },
	{ 0b1111, DS4_BUTTON_DPAD_NONE },
};

enum ButtonCategory { NONEXISTENT = -1, COMMON, DPAD, SPECIAL };

_FORCE_INLINE_ static constexpr ButtonCategory categorize(const JoyButton p_btn) {
	if (p_btn > JOY_BUTTON_INVALID) { // likely
		if (p_btn <= JOY_BUTTON_RIGHT_SHOULDER) { // likely
			if (p_btn != JOY_BUTTON_GUIDE) { return COMMON; }
			else { return SPECIAL; }
		}
		else if (p_btn <= JOY_BUTTON_DPAD_RIGHT) { return DPAD; }
		else if (p_btn == JOY_BUTTON_TOUCHPAD) { return SPECIAL; }
		else { return NONEXISTENT; }
	}
	else { return NONEXISTENT; } // better used else here because MSVC treats an else branch as "unlikely"
}

VIGEM_ERROR ViGEmDeviceDS4::update_button(const Ref<InputEventJoypadButton> &p_evt) {
	const JoyButton evt_btn = p_evt->get_button_index();
	switch (categorize(evt_btn)) {
	case COMMON: {
		const DS4_BUTTONS ds4_btn = BUTTONS_DS4_COMMON.at(evt_btn);
		if (p_evt->is_pressed()) { _state.wButtons |= ds4_btn; }
		else { _state.wButtons &= ~ds4_btn; }
		break;
	}
	case DPAD: {
		const DpadButtonBit dpad_bit = BITS_DS4_DPAD.at(evt_btn);
		if (p_evt->is_pressed()) { _dpad_bits |= dpad_bit; }
		else { _dpad_bits &= ~dpad_bit; }
		DS4_SET_DPAD(&_state, DIRS_DS4_DPAD.at(_dpad_bits));
		break;
	}
	case SPECIAL: {
		const DS4_SPECIAL_BUTTONS ds4_btn = BUTTONS_DS4_SPECIAL.at(evt_btn);
		if (p_evt->is_pressed()) { _state.bSpecial |= ds4_btn; }
		else { _state.bSpecial &= ~ds4_btn; }
		break;
	}
	default: return VIGEM_ERROR_NONE;
	}
	return vigem_target_ds4_update(_client, _target, _state);
}

VIGEM_ERROR ViGEmDeviceDS4::update_axis(const Ref<InputEventJoypadMotion> &p_evt) {
	switch (p_evt->get_axis()) {
	case JOY_AXIS_LEFT_X: {
		_state.bThumbLX = 128 + static_cast<BYTE>(p_evt->get_axis_value() * 127.0f);
		break;
	}
	case JOY_AXIS_LEFT_Y: {
		_state.bThumbLY = 128 + static_cast<BYTE>(p_evt->get_axis_value() * 127.0f);
		break;
	}
	case JOY_AXIS_RIGHT_X: {
		_state.bThumbRX = 128 + static_cast<BYTE>(p_evt->get_axis_value() * 127.0f);
		break;
	}
	case JOY_AXIS_RIGHT_Y: {
		_state.bThumbRY = 128 + static_cast<BYTE>(p_evt->get_axis_value() * 127.0f);
		break;
	}
	case JOY_AXIS_TRIGGER_LEFT: {
		_state.bTriggerL = static_cast<BYTE>(p_evt->get_axis_value() * 255.0f);
		break;
	}
	case JOY_AXIS_TRIGGER_RIGHT: {
		_state.bTriggerR = static_cast<BYTE>(p_evt->get_axis_value() * 255.0f);
		break;
	}
	default: return VIGEM_ERROR_NONE;
	}
	return vigem_target_ds4_update(_client, _target, _state);
}

void ViGEmDeviceDS4::reset_state() {
	DS4_REPORT_INIT(&_state);
	_dpad_bits = 0;
}
