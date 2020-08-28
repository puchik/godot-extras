#include "lod.h"
#include <string>

using namespace godot;

void LOD::_register_methods() {
    register_method("_process", &LOD::_process);
    register_method("_ready", &LOD::_ready);
    register_property<LOD, float>("lod1dist", &LOD::lod1dist, 7.0f); // put any of these to -1 if you don't have the lod for it, don't want to hide/unload etc
    register_property<LOD, float>("lod2dist", &LOD::lod2dist, 12.0f);
    register_property<LOD, float>("lod3dist", &LOD::lod3dist, 30.0f);
    register_property<LOD, float>("hideDist", &LOD::hideDist, 100.0f);
    register_property<LOD, float>("unloadDist", &LOD::unloadDist, -1.0f);
    
    // Whether to use distance multipliers from project settings
    register_property<LOD, bool>("affectedByDistanceMultipliers", &LOD::affectedByDistanceMultipliers, true);

    register_property<LOD, bool>("disableProcessing", &LOD::disableProcessing, true); // Hide the node as well as disabling its _process and _physics_process

    register_property<LOD, float>("tickSpeed", &LOD::tickSpeed, 0.2f);

    register_property<LOD, NodePath>("lod0path", &LOD::lod0path, NodePath());
    register_property<LOD, NodePath>("lod1path", &LOD::lod1path, NodePath());
    register_property<LOD, NodePath>("lod2path", &LOD::lod2path, NodePath());
    register_property<LOD, NodePath>("lod3path", &LOD::lod3path, NodePath());
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

    projectSettings = ProjectSettings::get_singleton();
    updateLodMultipliers();

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

    if (has_node(lod3path)) {
        lod3 = Object::cast_to<Spatial>(get_node(lod3path));
    } else {
        for (int i = 0; i < childCount; i++) {
            Spatial *child = Object::cast_to<Spatial>(childNodes[i]);
            if (child->get_name().find("LOD3") >= 0) {
                lod3 = child;
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

    if (unloadDist * globalDistMult * unloadDistMult > 0.0f && distance > unloadDist * globalDistMult * unloadDistMult) {
        queue_free();
    } else if (hideDist * globalDistMult * hideDistMult > 0.0f && distance > hideDist * globalDistMult * hideDistMult) {
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
        if (lod3) {
            lod3->hide();
            if (disableProcessing) {
                setNodeProcessing(lod3, false);
            }
        }
    } else if (lod3dist * globalDistMult * lod3DistMult > 0.0f && distance > lod3dist * globalDistMult * lod2DistMult && lod3) {
        lod3->show();
        if (disableProcessing) {
            setNodeProcessing(lod3, true);
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
        if (lod2) {
            lod2->hide();
            if (disableProcessing) {
                setNodeProcessing(lod2, false);
            }
        }
    } else if (lod2dist * globalDistMult * lod2DistMult > 0.0f && distance > lod2dist * globalDistMult * lod2DistMult && lod2) {
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
        if (lod3) {
            lod3->hide();
            if (disableProcessing) {
                setNodeProcessing(lod3, false);
            }
        }
    } else if (lod1dist * globalDistMult * lod1DistMult > 0.0f && distance > lod1dist * globalDistMult * lod1DistMult && lod1) {
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
        if (lod3) {
            lod3->hide();
            if (disableProcessing) {
                setNodeProcessing(lod3, false);
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
        if (lod3) {
            lod3->hide();
            if (disableProcessing) {
                setNodeProcessing(lod3, false);
            }
        }
    }
}

void LOD::setNodeProcessing(Spatial* node, bool state) {
    node->set_process(state);
    node->set_physics_process(state);
}

void LOD::updateLodMultipliers() {
    if (affectedByDistanceMultipliers) {

        // In case the setting (plugin/patch) is missing, make sure multipliers aren't set to 0
        float newGlobalDist = projectSettings->get_setting("rendering/quality/lod/global_multiplier");
        if (newGlobalDist > 0.0) {
            globalDistMult = newGlobalDist;    
        }

        float newLod1Dist = projectSettings->get_setting("rendering/quality/lod/lod1_multiplier");
        if (newLod1Dist > 0.0) {
            lod1DistMult = lod1DistMult;    
        }

        float newLod2Dist = projectSettings->get_setting("rendering/quality/lod/lod2_multiplier");
        if (newLod1Dist > 0.0) {
            lod2DistMult = newLod2Dist;
        }

        float newLod3Dist = projectSettings->get_setting("rendering/quality/lod/lod3_multiplier");
        if (newLod3Dist > 0.0) {
            lod3DistMult = newLod3Dist;
        }

        float newHideDist = projectSettings->get_setting("rendering/quality/lod/hide_multiplier");
        if (newHideDist > 0.0) {
            hideDistMult = newHideDist;
        }

        float newUnloadDist = projectSettings->get_setting("rendering/quality/lod/unload_multiplier");
        if (newUnloadDist > 0.0) {
            unloadDistMult = newUnloadDist;
        }

    } else {
        globalDistMult = 1.0f;
        lod1DistMult = 1.0f;
        lod2DistMult = 1.0f;
        lod3DistMult = 1.0f;
        hideDistMult = 1.0f;
        unloadDistMult = 1.0f;
    }
}