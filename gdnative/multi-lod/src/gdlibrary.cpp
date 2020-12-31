#include "lod.h"

extern "C" void GDN_EXPORT gdn_multi_lod_gdnative_init(godot_gdnative_init_options *o) {
    godot::Godot::gdnative_init(o);
}

extern "C" void GDN_EXPORT gdn_multi_lod_gdnative_terminate(godot_gdnative_terminate_options *o) {
    godot::Godot::gdnative_terminate(o);
}

extern "C" void GDN_EXPORT gdn_multi_lod_nativescript_init(void *handle) {
    godot::Godot::nativescript_init(handle);

    godot::register_class<godot::LOD>();
    godot::register_class<godot::LODManager>();
    godot::register_class<godot::LightLOD>();
    godot::register_class<godot::GIProbeLOD>();
    godot::register_class<godot::MultiMeshLOD>();
}