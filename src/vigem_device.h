#ifndef GDVIGEM_VIGEM_DEVICE
#define GDVIGEM_VIGEM_DEVICE

#include <windows.h>
#include <ViGEm/Client.h>

#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/input_event_joypad_button.hpp>
#include <godot_cpp/classes/input_event_joypad_motion.hpp>

class ViGEmDevice {
protected:
	PVIGEM_CLIENT _client = nullptr;
	PVIGEM_TARGET _target;

	explicit ViGEmDevice(PVIGEM_TARGET p_target) :
		_target(p_target) {}

	virtual VIGEM_ERROR update_button(const godot::Ref<godot::InputEventJoypadButton> &p_evt) = 0;
	virtual VIGEM_ERROR update_axis(const godot::Ref<godot::InputEventJoypadMotion> &p_evt) = 0;

public:
	virtual ~ViGEmDevice();
	ViGEmDevice(const ViGEmDevice &) = delete;
	ViGEmDevice& operator=(const ViGEmDevice &) = delete;

	godot::Error init(PVIGEM_CLIENT p_client);
	godot::Error send_event(const godot::Ref<godot::InputEvent> &p_evt);

	virtual void reset_state() = 0;
};

class ViGEmDeviceX360 : public ViGEmDevice {
	XUSB_REPORT _state;

protected:
	VIGEM_ERROR update_button(const godot::Ref<godot::InputEventJoypadButton> &p_evt) override;
	VIGEM_ERROR update_axis(const godot::Ref<godot::InputEventJoypadMotion> &p_evt) override;

public:
	ViGEmDeviceX360() : // NOLINT(*-pro-type-member-init)
		ViGEmDevice(vigem_target_x360_alloc()) {
		XUSB_REPORT_INIT(&_state);
	}

	void reset_state() override;
};

class ViGEmDeviceDS4 : public ViGEmDevice {
public:
	enum DpadButtonBit {
		DS4_DPAD_UP_BIT = 0b0001,
		DS4_DPAD_DOWN_BIT = 0b0010,
		DS4_DPAD_LEFT_BIT = 0b0100,
		DS4_DPAD_RIGHT_BIT = 0b1000,
	};

private:
	DS4_REPORT _state;
	uint8_t _dpad_bits = 0;

protected:
	VIGEM_ERROR update_button(const godot::Ref<godot::InputEventJoypadButton> &p_evt) override;
	VIGEM_ERROR update_axis(const godot::Ref<godot::InputEventJoypadMotion> &p_evt) override;

public:
	ViGEmDeviceDS4() : // NOLINT(*-pro-type-member-init)
		ViGEmDevice(vigem_target_ds4_alloc()) {
		DS4_REPORT_INIT(&_state);
	}

	void reset_state() override;
};

#endif // GDVIGEM_VIGEM_DEVICE
