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

	static void _bind_methods();
	virtual void create_device() = 0;

public:
	GamepadEmulator() = default;
	~GamepadEmulator() override = default;

	void _enter_tree() override;
	void _exit_tree() override;

	godot::Error send_event(const godot::Ref<godot::InputEvent> &p_evt);

	void reset_state();
};

class XBox360ControllerEmulator : public GamepadEmulator {
	GDCLASS(XBox360ControllerEmulator, GamepadEmulator);

protected:
	static void _bind_methods();
	void create_device() override;
};

class DualShock4Emulator : public GamepadEmulator {
	GDCLASS(DualShock4Emulator, GamepadEmulator);

protected:
	static void _bind_methods();
	void create_device() override;
};

#endif //GODOT_VIGEM_GAMEPAD_EMULATOR_H
