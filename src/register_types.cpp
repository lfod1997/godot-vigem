#include "register_types.h"

#include <gdextension_interface.h>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>

#include "vigem_server.h"
#include "example_class.h"

using namespace godot;

static ViGEmServer *server = nullptr;

void initialize_gdextension_types(ModuleInitializationLevel p_level) {
	switch (p_level) {
	case MODULE_INITIALIZATION_LEVEL_SERVERS: {
		GDREGISTER_CLASS(ViGEmServer);
		server = memnew(ViGEmServer);
		const Error err = server->init();
		ERR_FAIL_COND_MSG(err != OK, vformat("ViGEmServer initialization failed: %s.", UtilityFunctions::error_string(err)));
		break;
	}
	case MODULE_INITIALIZATION_LEVEL_SCENE: {
		GDREGISTER_CLASS(ExampleClass);
		break;
	}
	default: break;
	}
}

void uninitialize_gdextension_types(ModuleInitializationLevel p_level) {
	switch (p_level) {
	case MODULE_INITIALIZATION_LEVEL_SERVERS: {
		if (server) {
			server->finish();
			memdelete(server);
		}
		break;
	}
	default: break;
	}
}

extern "C" {
// Initialization
GDExtensionBool GDE_EXPORT godot_vigem_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
	GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);
	init_obj.register_initializer(initialize_gdextension_types);
	init_obj.register_terminator(uninitialize_gdextension_types);
	init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

	return init_obj.init();
}
}
