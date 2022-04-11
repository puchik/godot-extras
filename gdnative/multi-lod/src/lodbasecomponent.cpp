#include "lod.h"

using namespace godot;

void LODBaseComponent::setup(Spatial* p_object) {
    if (p_object) {
        lod_object = p_object;
        Node* lm = lod_object->get_node_or_null(NodePath("/root/LodManager"));
        if (lm) {
            lod_manager = Object::cast_to<LODManager>(lm);
        }
        if (!lod_manager) {
            ERR_PRINT("Error, can't find /root/LodManager. Make sure plugin is enabled.");
        }
        return;
    } else {
        ERR_PRINT("Error, LODBaseComponent initialized with a null object.");
    }
}

float LODBaseComponent::get_fov() {
    if (lod_manager) {
        return lod_manager->get_fov();
    } else {
        return 70.f;
    }
}

// Use an isosceles triangle to get a worst-case estimate of the distances
float LODBaseComponent::get_tan_theta() {
    if (lod_manager) {
        return lod_manager->get_tan_theta();
    } else {
        return 0.7002f; // Calc for 70.0f
    }
}

void LODBaseComponent::try_register() {
    if (registered || !enabled) {
        return;
    }
    if (lod_manager) {
        lod_manager->add_object(this);
        registered = true;
    } else {
        ERR_PRINT(lod_object->get_name() + ": Error, LODManager is not set during registration.");
    }
}

void LODBaseComponent::unregister() {
    if (!registered) {
        return;
    }
    if (lod_manager) {
        lod_manager->remove_object(this);
        registered = false;
    } else {
        ERR_PRINT(lod_object->get_name() + ": Error, LODManager is not set during unregistration.");
    }
}
