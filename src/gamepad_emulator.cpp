#include "gamepad_emulator.h"

#include <godot_cpp/classes/engine.hpp>

using namespace godot;

void GamepadEmulator::_bind_methods() {
	ClassDB::bind_method(D_METHOD("send_event", "event"), &GamepadEmulator::send_event);
	ClassDB::bind_method(D_METHOD("reset_state"), &GamepadEmulator::reset_state);
}

void GamepadEmulator::_enter_tree() {
	if (!Engine::get_singleton()->is_editor_hint()) {
		ERR_FAIL_COND_MSG(_device.is_valid(), "Previous ViGEm device not freed properly, please report to developers of this add-on.");
		create_device();
	}
}

void GamepadEmulator::_exit_tree() {
	if (!Engine::get_singleton()->is_editor_hint()) {
		ViGEmServer::get_singleton()->destroy_device(_device);
		_device = RID();
	}
}

Error GamepadEmulator::send_event(const Ref<InputEvent> &p_evt) {
	if (!is_inside_tree()) { return ERR_UNAVAILABLE; }
	return ViGEmServer::get_singleton()->send_event_to_device(_device, p_evt);
}

void GamepadEmulator::reset_state() {
	if (!is_inside_tree()) { return; }
	ViGEmServer::get_singleton()->reset_device(_device);
}

void XBox360ControllerEmulator::_bind_methods() {}

void XBox360ControllerEmulator::create_device() {
	_device = ViGEmServer::get_singleton()->create_device<ViGEmDeviceX360>();
}

void DualShock4Emulator::_bind_methods() {}

void DualShock4Emulator::create_device() {
	_device = ViGEmServer::get_singleton()->create_device<ViGEmDeviceDS4>();
}
