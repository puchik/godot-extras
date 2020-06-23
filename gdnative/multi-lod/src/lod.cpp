#include "lod.h"
#include <string>

using namespace godot;

void LOD::_register_methods() {
    register_method("_process", &LOD::_process);
    register_method("_ready", &LOD::_ready);
    register_property<LOD, float>("lod1dist", &LOD::lod1dist, 30.0f);
    register_property<LOD, float>("lod2dist", &LOD::lod2dist, 60.0f);
    register_property<LOD, float>("unloadDist", &LOD::unloadDist, 130.0f); // -1 to never unload

    register_property<LOD, float>("tickSpeed", &LOD::tickSpeed, 0.2f);

    register_property<LOD, NodePath>("lod0path", &LOD::lod0path, NodePath());
    register_property<LOD, NodePath>("lod1path", &LOD::lod1path, NodePath());
    register_property<LOD, NodePath>("lod2path", &LOD::lod2path, NodePath());
}

LOD::LOD() {
}

LOD::~LOD() {
    // add your cleanup here
}

void LOD::_init() {
    // initialize any variables here
}

void LOD::_ready() {
    camera = get_viewport()->get_camera();

    /// Get the Spatial objects for LOD nodes
    // If there's no path, search for any children with "LOD + n" in its name

    // So we don't have to fetch list of children once for every object
    Array childNodes;
    int64_t childCount;
    if (!has_node(lod0path)|| !has_node(lod1path) || !has_node(lod2path)) {
        childNodes = get_children();
        childCount = get_child_count();
    }

    if (has_node(lod0path)) {
        lod0 = Object::cast_to<Spatial>(get_node(lod0path));
    } else {
        for (int i = 0; i < childCount; i++) {
            Spatial *child = Object::cast_to<Spatial>(childNodes[i]);
            if (child->get_name().find("LOD0") >= 0) {
                lod0 = child;
                break;
            }
        }
    }

    if (has_node(lod1path)) {
        lod1 = Object::cast_to<Spatial>(get_node(lod1path));
    } else {
        for (int i = 0; i < childCount; i++) {
            Spatial *child = Object::cast_to<Spatial>(childNodes[i]);
            if (child->get_name().find("LOD1") >= 0) {
                lod1 = child;
                break;
            }
        }
    }

    if (has_node(lod2path)) {
        lod2 = Object::cast_to<Spatial>(get_node(lod2path));
    } else {
        for (int i = 0; i < childCount; i++) {
            Spatial *child = Object::cast_to<Spatial>(childNodes[i]);
            if (child->get_name().find("LOD2") >= 0) {
                lod2 = child;
                break;
            }
        }
    }
    
}

void LOD::_process(float delta) {
    timePassed += delta;

    // Don't have to check every frame
    if (timePassed < tickSpeed) {
        return;
    } else {
        timePassed = 0;
    }

    // Get the distance from the node to the camera
    float distance = camera->get_global_transform().origin.distance_to(get_global_transform().origin);

    if (unloadDist > 0.0f && distance > unloadDist && lod0) {
        if (lod0) {
            lod0->hide();
        }
        if (lod1) {
            lod1->hide();
        }
        if (lod2) {
            lod2->hide();
        }
    } else if (distance > lod2dist && lod1) {
        lod2->show();
        if (lod0) {
            lod0->hide();
        }
        if (lod1) {
            lod1->hide();
        }
    } else if (distance > lod1dist && lod2) {
        lod1->show();
        if (lod0) {
            lod0->hide();
        }
        if (lod2) {
            lod2->hide();
        }
    } else {
        lod0->show();
        if (lod1) {
            lod1->hide();
        }
        if (lod2) {
            lod2->hide();
        }
    }
}