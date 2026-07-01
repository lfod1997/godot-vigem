#include "gamepad_emulator.h"

#include <godot_cpp/classes/engine.hpp>

using namespace godot;

void GamepadEmulator::_bind_methods() {
	ClassDB::bind_method(D_METHOD("send_event", "event"), &GamepadEmulator::send_event);
	ClassDB::bind_method(D_METHOD("reset_state"), &GamepadEmulator::reset_state);
	ClassDB::bind_method(D_METHOD("get_vibration_weak"), &GamepadEmulator::get_vibration_weak);
	ClassDB::bind_method(D_METHOD("get_vibration_strong"), &GamepadEmulator::get_vibration_strong);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "vibration_weak"), "", "get_vibration_weak");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "vibration_strong"), "", "get_vibration_strong");
	ADD_SIGNAL(MethodInfo("vibration_changed", PropertyInfo(Variant::FLOAT, "weak_magnitude"), PropertyInfo(Variant::FLOAT, "strong_magnitude")));
}

void GamepadEmulator::emit_vibration_changed(float p_weak, float p_strong) {
	// This is called from another thread (that ViGEmClient spawns for each target)
	call_deferred("emit_signal", "vibration_changed", p_weak, p_strong);
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

void XBox360ControllerEmulator::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_led_number"), &XBox360ControllerEmulator::get_led_number);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "led_number"), "", "get_led_number");
	ADD_SIGNAL(MethodInfo("led_number_changed", PropertyInfo(Variant::INT, "number")));
}

void XBox360ControllerEmulator::create_device() {
	_device = ViGEmServer::get_singleton()->create_device<ViGEmDeviceX360>();
	if (_device.is_valid()) {
		ViGEmServer::get_singleton()->register_notification_handler(
			_device,
			+[](PVIGEM_CLIENT, PVIGEM_TARGET target, UCHAR motor_l, UCHAR motor_s, UCHAR led_num, void *ctx) {
				if (auto *self = static_cast<XBox360ControllerEmulator*>(ctx)) {
					if (motor_l != self->_cached_motor_l || motor_s != self->_cached_motor_s) {
						self->_cached_motor_l = motor_l;
						self->_cached_motor_s = motor_s;
						self->emit_vibration_changed(
							static_cast<float>(motor_s) / 255.0f,
							static_cast<float>(motor_l) / 255.0f
						);
					}
					if (led_num != self->_cached_led_num) {
						self->_cached_led_num = led_num;
						self->emit_led_number_changed(led_num);
					}
				}
				else { print_error(vformat("Could not recover object for ViGEm target at 0x%x.", (size_t)target)); }
			},
			this
		);
	}
}

void XBox360ControllerEmulator::emit_led_number_changed(int p_led_number) {
	// This is called from another thread (that ViGEmClient spawns for each target)
	call_deferred("emit_signal", "led_number_changed", p_led_number);
}

void DualShock4Emulator::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_led_color"), &DualShock4Emulator::get_led_color);
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "led_color"), "", "get_led_color");
	ADD_SIGNAL(MethodInfo("led_color_changed", PropertyInfo(Variant::COLOR, "color")));
}

void DualShock4Emulator::create_device() {
	_device = ViGEmServer::get_singleton()->create_device<ViGEmDeviceDS4>();
	if (_device.is_valid()) {
		ViGEmServer::get_singleton()->register_notification_handler(
			_device,
			+[](PVIGEM_CLIENT, PVIGEM_TARGET target, UCHAR motor_l, UCHAR motor_s, DS4_LIGHTBAR_COLOR led_rgb, void *ctx) {
				if (auto *self = static_cast<DualShock4Emulator*>(ctx)) {
					if (motor_l != self->_cached_motor_l || motor_s != self->_cached_motor_s) {
						self->_cached_motor_l = motor_l;
						self->_cached_motor_s = motor_s;
						self->emit_vibration_changed(
							static_cast<float>(motor_s) / 255.0f,
							static_cast<float>(motor_l) / 255.0f
						);
					}
					if (std::memcmp(&led_rgb, &self->_cached_led_rgb, sizeof(DS4_LIGHTBAR_COLOR)) != 0) {
						std::memcpy(&self->_cached_led_rgb, &led_rgb, sizeof(DS4_LIGHTBAR_COLOR));
						self->emit_led_color_changed(Color::from_rgba8(led_rgb.Red, led_rgb.Green, led_rgb.Blue));
					}
				}
				else { print_error(vformat("Could not recover object for ViGEm target at 0x%x.", (size_t)target)); }
			},
			this
		);
	}
}

void DualShock4Emulator::emit_led_color_changed(Color p_led_color) {
	// This is called from another thread (that ViGEmClient spawns for each target)
	call_deferred("emit_signal", "led_color_changed", p_led_color);
}
