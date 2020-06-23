#include "lod.h"
#include <string>

using namespace godot;

void LOD::_register_methods() {
    register_method("_process", &LOD::_process);
    register_method("_ready", &LOD::_ready);
    register_property<LOD, float>("lod1dist", &LOD::lod1dist, 30.0f); // put any of these to -1 if you don't have the lod for it, don't want to hide/unload etc
    register_property<LOD, float>("lod2dist", &LOD::lod2dist, 60.0f);
    register_property<LOD, float>("hideDist", &LOD::hideDist, 130.0f);
    register_property<LOD, float>("unloadDist", &LOD::unloadDist, -1.0f);

    register_property<LOD, bool>("disableProcessing", &LOD::disableProcessing, true); // Hide the node as well as disabling its _process and _physics_process

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

    if (unloadDist > 0.0f && distance > unloadDist) {
        queue_free();
    } else if (hideDist > 0.0f && distance > hideDist) {
        if (lod0) {
            lod0->hide();
            if (disableProcessing) {
                setNodeProcessing(lod0, false);
            }
        }
        if (lod1) {
            lod1->hide();
            if (disableProcessing) {
                setNodeProcessing(lod1, false);
            }
        }
        if (lod2) {
            lod2->hide();
            if (disableProcessing) {
                setNodeProcessing(lod2, false);
            }
        }
    } else if (lod2dist > 0.0f && distance > lod2dist && lod2) {
        lod2->show();
        if (disableProcessing) {
            setNodeProcessing(lod2, true);
        }
        if (lod0) {
            lod0->hide();
            if (disableProcessing) {
                setNodeProcessing(lod0, false);
            }
        }
        if (lod1) {
            lod1->hide();
            if (disableProcessing) {
                setNodeProcessing(lod1, false);
            }
        }
    } else if (lod1dist > 0.0f && distance > lod1dist && lod1) {
        lod1->show();
        if (disableProcessing) {
            setNodeProcessing(lod1, true);
        }
        if (lod0) {
            lod0->hide();
            if (disableProcessing) {
                setNodeProcessing(lod0, false);
            }
        }
        if (lod2) {
            lod2->hide();
            if (disableProcessing) {
                setNodeProcessing(lod2, false);
            }
        }
    } else {
        lod0->show();
        if (disableProcessing) {
            setNodeProcessing(lod0, true);
        }
        if (lod1) {
            lod1->hide();
            if (disableProcessing) {
                setNodeProcessing(lod1, false);
            }
        }
        if (lod2) {
            lod2->hide();
            if (disableProcessing) {
                setNodeProcessing(lod2, false);
            }
        }
    }
}

void LOD::setNodeProcessing(Spatial* node, bool state) {
    node->set_process(state);
    node->set_physics_process(state);
}