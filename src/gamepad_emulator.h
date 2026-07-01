#ifndef GODOT_VIGEM_GAMEPAD_EMULATOR_H
#define GODOT_VIGEM_GAMEPAD_EMULATOR_H

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/wrapped.hpp>

#include "vigem_server.h"
#include "vigem_device.h"

class GamepadEmulator : public godot::Node {
	GDCLASS(GamepadEmulator, godot::Node);

protected:
	godot::RID _device;

	UCHAR _cached_motor_l = 0;
	UCHAR _cached_motor_s = 0;

	static void _bind_methods();
	virtual void create_device() = 0;
	void emit_vibration_changed(float p_weak, float p_strong);

public:
	GamepadEmulator() = default;
	~GamepadEmulator() override = default;

	void _enter_tree() override;
	void _exit_tree() override;

	godot::Error send_event(const godot::Ref<godot::InputEvent> &p_evt);

	void reset_state();

	[[nodiscard]] float get_vibration_weak() const { return _cached_motor_s; }

	[[nodiscard]] float get_vibration_strong() const { return _cached_motor_l; }
};

class XBox360ControllerEmulator : public GamepadEmulator {
	GDCLASS(XBox360ControllerEmulator, GamepadEmulator);

	UCHAR _cached_led_num = 255;

protected:
	static void _bind_methods();
	void create_device() override;
	void emit_led_number_changed(int p_led_number);

public:
	[[nodiscard]] int get_led_number() const { return _cached_led_num; }
};

class DualShock4Emulator : public GamepadEmulator {
	GDCLASS(DualShock4Emulator, GamepadEmulator);

	DS4_LIGHTBAR_COLOR _cached_led_rgb{ 0 };

protected:
	static void _bind_methods();
	void create_device() override;
	void emit_led_color_changed(godot::Color p_led_color);

public:
	[[nodiscard]] godot::Color get_led_color() const { return godot::Color::from_rgba8(_cached_led_rgb.Red, _cached_led_rgb.Green, _cached_led_rgb.Blue); }
};

#endif //GODOT_VIGEM_GAMEPAD_EMULATOR_H
