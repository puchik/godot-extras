#include "lod.h"
#include <string>
#include <cmath>

using namespace godot;

void LOD::_register_methods() {
    // Godot and thread functions
    register_method("_ready", &LOD::_ready);
    register_method("_exit_tree", &LOD::_exit_tree);
    register_method("processData", &LOD::processData);
    // Exposed methods
    register_method("updateLodAABB", &LOD::updateLodAABB);
    register_method("updateLodMultipliersFromManager", &LOD::updateLodMultipliersFromManager);
    // Vars for distance-based (in metres)
    // These will be set by the ratios if useScreenPercentage is true
    register_property<LOD, float>("lod1dist", &LOD::lod1dist, 7.0f); // put any of these to -1 if you don't have the lod for it, don't want to hide/unload etc
    register_property<LOD, float>("lod2dist", &LOD::lod2dist, 12.0f);
    register_property<LOD, float>("lod3dist", &LOD::lod3dist, 30.0f);
    register_property<LOD, float>("hideDist", &LOD::hideDist, 100.0f);
    register_property<LOD, float>("unloadDist", &LOD::unloadDist, -1.0f);

    // Screen percentage ratios (and if applicable)
    register_property<LOD, bool>("useScreenPercentage", &LOD::useScreenPercentage, true);
    register_property<LOD, float>("lod1ratio", &LOD::lod1ratio, 25.0f); // put any of these to -1 if you don't have the lod for it, don't want to hide/unload etc
    register_property<LOD, float>("lod2ratio", &LOD::lod2ratio, 10.0f);
    register_property<LOD, float>("lod3ratio", &LOD::lod3ratio, 5.5f);
    register_property<LOD, float>("hideRatio", &LOD::hideRatio, 1.0f);
    register_property<LOD, float>("unloadRatio", &LOD::unloadRatio, -1.0f);
    register_property<LOD, float>("FOV", &LOD::FOV, 70.0f);
    
    // Whether to use distance multipliers from project settings
    register_property<LOD, bool>("affectedByDistanceMultipliers", &LOD::affectedByDistanceMultipliers, true);

    register_property<LOD, bool>("disableProcessing", &LOD::disableProcessing, false); // Hide the node as well as disabling its _process and _physics_process

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

void LOD::_exit_tree() {
    // Leave LOD manager's list
    get_node("/root/LodManager")->call("removeObject", (Node*) this);
}

void LOD::_ready() {
    /// Get the VisualInstance objects for LOD nodes
    // If there's no path, search for any children with "LOD + n" in its name

    // So we don't have to fetch list of children once for every object
    Array childNodes;
    int64_t childCount;
    if (!has_node(lod0path)|| !has_node(lod1path) || !has_node(lod2path) || !has_node(lod3path)) {
        childNodes = get_children();
        childCount = get_child_count();
    }

    if (has_node(lod0path)) {
        lod0 = Object::cast_to<Spatial>(get_node(lod0path));
    } else {
        for (int i = 0; i < childCount; i++) {
            Node *child = Object::cast_to<Node>(childNodes[i]);
            if (child->get_name().find("LOD0") >= 0) {
                lod0 = Object::cast_to<Spatial>(child);
                if (lod0) {
                    break;
                }
            }
        }
    }

    if (has_node(lod1path)) {
        lod1 = Object::cast_to<Spatial>(get_node(lod1path));
    } else {
        for (int i = 0; i < childCount; i++) {
            Node *child = Object::cast_to<Node>(childNodes[i]);
            if (child->get_name().find("LOD1") >= 0) {
                lod1 = Object::cast_to<Spatial>(child);
                if (lod1) {
                    break;
                }
            }
        }
    }

    if (has_node(lod2path)) {
        lod2 = Object::cast_to<Spatial>(get_node(lod2path));
    } else {
        for (int i = 0; i < childCount; i++) {
            Node *child = Object::cast_to<Node>(childNodes[i]);
            if (child->get_name().find("LOD2") >= 0) {
                lod2 = Object::cast_to<Spatial>(child);
                if (lod2) {
                    break;
                }
            }
        }
    }

    if (has_node(lod3path)) {
        lod3 = Object::cast_to<Spatial>(get_node(lod3path));
    } else {
        for (int i = 0; i < childCount; i++) {
            Node *child = Object::cast_to<Node>(childNodes[i]);
            if (child->get_name().find("LOD3") >= 0) {
                lod3 = Object::cast_to<Spatial>(child);
                if (lod3) {
                    break;
                }
            }
        }
    }

    // Initial FOV setup
    FOV = get_viewport()->get_camera()->get_fov();
    if (useScreenPercentage) {
        updateLodAABB();
    }

    // Tell the LOD manager that we want to be part of the LOD list
    get_node("/root/LodManager")->call("addObject", (Node*) this);
    updateLodMultipliersFromManager();
}

void LOD::processData(Vector3 cameraLoc) {
    // Double check for this node being in the scene tree
    // (Otherwise you get errors when ending the thread)
    // Get the distance from the node to the camera (and subtract AABB offset, if applicable)
    Vector3 objLoc;
    if (is_inside_tree()) {
        objLoc = get_global_transform().origin;
    } else {
        return;
    }
 
    if (useScreenPercentage) {
        objLoc -= transformOffsetAABB;
    }
    float distance = cameraLoc.distance_to(objLoc);

    if (unloadDist * globalDistMult * unloadDistMult > 0.0f && distance > unloadDist * globalDistMult * unloadDistMult) {
        queue_free();
    } else if (hideDist * globalDistMult * hideDistMult > 0.0f && distance > hideDist * globalDistMult * hideDistMult) {
        if (lastState == 4) {
            return;
        }
        lastState = 4;
        if (lod0 && lod0->is_inside_tree() && lod0->is_visible()) {
            lod0->hide();
            if (disableProcessing) {
                call_deferred("setNodeProcessing", lod0, false);
            }
        }
        if (lod1 && lod1->is_inside_tree() && lod1->is_visible()) {
            lod1->hide();
            if (disableProcessing) {
                call_deferred("setNodeProcessing", lod1, false);
            }
        }
        if (lod2 && lod2->is_inside_tree() && lod2->is_visible()) {
            lod2->hide();
            if (disableProcessing) {
                call_deferred("setNodeProcessing", lod2, false);
            }
        }
        if (lod3 && lod3->is_inside_tree() && lod3->is_visible()) {
            lod3->hide();
            if (disableProcessing) {
                call_deferred("setNodeProcessing", lod3, false);
            }
        }
    } else if (lod3 && lod3->is_inside_tree() && lod3dist * globalDistMult * lod3DistMult > 0.0f && distance > lod3dist * globalDistMult * lod2DistMult) {
        if (lastState == 3) {
            return;
        }
        lastState = 3;
        lod3->show();
        if (disableProcessing) {
            call_deferred("setNodeProcessing", lod3, true);
        }
        if (lod0 && lod0->is_inside_tree() && lod0->is_visible()) {
            lod0->hide();
            if (disableProcessing) {
                call_deferred("setNodeProcessing", lod0, false);
            }
        }
        if (lod1 && lod1->is_inside_tree() && lod1->is_visible()) {
            lod1->hide();
            if (disableProcessing) {
                call_deferred("setNodeProcessing", lod1, false);
            }
        }
        if (lod2 && lod2->is_inside_tree() && lod2->is_visible()) {
            lod2->hide();
            if (disableProcessing) {
                call_deferred("setNodeProcessing", lod2, false);
            }
        }
    } else if (lod2 && lod2->is_inside_tree() && lod2dist * globalDistMult * lod2DistMult > 0.0f && distance > lod2dist * globalDistMult * lod2DistMult) {
        if (lastState == 2) {
            return;
        }
        lastState = 2;
        lod2->show();
        if (disableProcessing) {
            call_deferred("setNodeProcessing", lod2, true);
        }
        if (lod0 && lod0->is_inside_tree() && lod0->is_visible()) {
            lod0->hide();
            if (disableProcessing) {
                call_deferred("setNodeProcessing", lod0, false);
            }
        }
        if (lod1 && lod1->is_inside_tree() && lod1->is_visible()) {
            lod1->hide();
            if (disableProcessing) {
                call_deferred("setNodeProcessing", lod1, false);
            }
        }
        if (lod3 && lod3->is_inside_tree() && lod3->is_visible()) {
            lod3->hide();
            if (disableProcessing) {
                call_deferred("setNodeProcessing", lod3, false);
            }
        }
    } else if (lod1 && lod1->is_inside_tree() && lod1dist * globalDistMult * lod1DistMult > 0.0f && distance > lod1dist * globalDistMult * lod1DistMult) {
        if (lastState == 1) {
            return;
        }
        lastState = 1;
        lod1->show();
        if (disableProcessing) {
            call_deferred("setNodeProcessing", lod1, true);
        }
        if (lod0 && lod0->is_inside_tree() && lod0->is_visible()) {
            lod0->hide();
            if (disableProcessing) {
                call_deferred("setNodeProcessing", lod0, false);
            }
        }
        if (lod2 && lod2->is_inside_tree() && lod2->is_visible()) {
            lod2->hide();
            if (disableProcessing) {
                call_deferred("setNodeProcessing", lod2, false);
            }
        }
        if (lod3 && lod3->is_inside_tree() && lod3->is_visible()) {
            lod3->hide();
            if (disableProcessing) {
                call_deferred("setNodeProcessing", lod3, false);
            }
        }
    } else if (lod0 && lod0->is_inside_tree()) {
        if (lastState == 0) {
            return;
        }
        lastState = 0;
        lod0->show();
        if (disableProcessing) {
            call_deferred("setNodeProcessing", lod0, true);
        }
        if (lod1 && lod1->is_inside_tree() && lod1->is_visible()) {
            lod1->hide();
            if (disableProcessing) {
                call_deferred("setNodeProcessing", lod1, false);
            }
        }
        if (lod2 && lod2->is_inside_tree() && lod2->is_visible()) {
            lod2->hide();
            if (disableProcessing) {
                call_deferred("setNodeProcessing", lod2, false);
            }
        }
        if (lod3 && lod3->is_inside_tree() && lod3->is_visible()) {
            lod3->hide();
            if (disableProcessing) {
                call_deferred("setNodeProcessing", lod3, false);
            }
        }
    }
}

void LOD::setNodeProcessing(Spatial* node, bool state) {
    node->set_physics_process(state);
    node->set_process(state);
}

// Update the distances based on the AABB
void LOD::updateLodAABB() {
    // Get an AABB for all objects
    // Spatials don't have AABB apparently, and it's convenient to have
    // an "empty" 3D object for the LOD objects' parent.
    // So make an AABB for the objects manually.
    
    // Check for at least LOD0
    if (!lod0) {
        printf("%s: ", get_name().alloc_c_string());
        printf("You need to have a valid LOD0 for screen percentage based LOD.\n");
    }

    // Try casting the LOD objects to VisualInstance (that's the only way we can get an AABB!)
    VisualInstance *lod0VisInst = Object::cast_to<VisualInstance>(lod0);
    if (!lod0VisInst) {
        printf("%s: ", get_name().alloc_c_string());
        printf("LOD0 could not be cast to VisualInstance for the AABB calculation (check the Node type)\n");
    }

    // Get base AABB using LOD0
    AABB objAABB = lod0VisInst->get_transformed_aabb();

    if (objAABB.has_no_area()) {
        printf("%s: ", get_name().alloc_c_string());
        printf("Invalid AABB for LOD0!\n");
        return;
    }

    // Merge others if available
    if (lod1) {
        VisualInstance *lod1VisInst = Object::cast_to<VisualInstance>(lod1);
        if (lod1VisInst) {
            objAABB = objAABB.merge(lod1VisInst->get_transformed_aabb());
        }   
    }
    if (lod2) {
        VisualInstance *lod2VisInst = Object::cast_to<VisualInstance>(lod2);
        if (lod2VisInst) {
            objAABB = objAABB.merge(lod2VisInst->get_transformed_aabb());
        }
    }
    if (lod3) {
        VisualInstance *lod3VisInst = Object::cast_to<VisualInstance>(lod3);
        if (lod3VisInst) {
            objAABB = objAABB.merge(lod3VisInst->get_transformed_aabb());
        }
    }

    // Get the longest axis (conservative estimate of the object size vs screen)
    float longestAxis = objAABB.get_longest_axis_size();

    // Use an isosceles triangle to get a worst-case estimate of the distances
    // Don't forget the degrees to radians conversion
    float tanTheta = tan((FOV * 3.14f / 180.0f) / 2.0f);

    // Get the distances at which we have the LOD ratios of the screen
    lod1dist = ((longestAxis / (lod1ratio / 100.0f)) / (2.0f * tanTheta));
    lod2dist = ((longestAxis / (lod2ratio / 100.0f)) / (2.0f * tanTheta));
    lod3dist = ((longestAxis / (lod3ratio / 100.0f)) / (2.0f * tanTheta));
    hideDist = ((longestAxis / (hideRatio / 100.0f)) / (2.0f * tanTheta));

    // Get the offset of the parent position and the overall AABB
    transformOffsetAABB = get_global_transform().origin - (objAABB.get_endpoint(0) + objAABB.size / 2.0f);
}

void LOD::updateLodMultipliersFromManager() {
    if (affectedByDistanceMultipliers) {
        Node* LODManagerNode = get_node("/root/LodManager");
        globalDistMult = LODManagerNode->get("globalDistMult");
        lod1DistMult = LODManagerNode->get("lod1DistMult");
        lod2DistMult = LODManagerNode->get("lod2DistMult");
        lod3DistMult = LODManagerNode->get("lod3DistMult");
        hideDistMult = LODManagerNode->get("hideDistMult");
        unloadDistMult = LODManagerNode->get("unloadDistMult");
    } else {
        globalDistMult = 1.0f;
        lod1DistMult = 1.0f;
        lod2DistMult = 1.0f;
        lod3DistMult = 1.0f;
        hideDistMult = 1.0f;
        unloadDistMult = 1.0f;
    }
}