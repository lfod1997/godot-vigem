#ifndef GDVIGEM_VIGEM_SERVER
#define GDVIGEM_VIGEM_SERVER

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/thread.hpp>
#include <godot_cpp/classes/mutex.hpp>

class ViGEmServer : public godot::Object {
	GDCLASS(ViGEmServer, godot::Object);

	static ViGEmServer *_inst;
	void _thread_func();

	godot::Ref<godot::Thread> _thread;
	godot::Ref<godot::Mutex> _mutex;
	godot::SafeFlag _exit_thread;

protected:
	static void _bind_methods();

public:
	// region Threading

	_FORCE_INLINE_ ViGEmServer() { _inst = this; }

	_FORCE_INLINE_ static ViGEmServer* get_singleton() { return _inst; }

	godot::Error init();
	void finish();

	_FORCE_INLINE_ void lock() {
		// ReSharper disable once CppDFANullDereference
		_mutex->lock();
	}

	_FORCE_INLINE_ void unlock() {
		// ReSharper disable once CppDFANullDereference
		_mutex->unlock();
	}

	// endregion
};

#endif // GDVIGEM_VIGEM_SERVER
