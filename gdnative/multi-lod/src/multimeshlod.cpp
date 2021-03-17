#include "lod.h"
#include <cmath>

using namespace godot;

void MultiMeshLOD::_register_methods() {
    register_method("_process", &MultiMeshLOD::_process);
    register_method("_ready", &MultiMeshLOD::_ready);
    register_method("_exit_tree", &MultiMeshLOD::_exit_tree);
    register_method("processData", &MultiMeshLOD::processData);

    // Exposed methods
    register_method("updateLodAABB", &MultiMeshLOD::updateLodAABB);
    register_method("updateLodMultipliersFromManager", &MultiMeshLOD::updateLodMultipliersFromManager);

    // Vars for distance-based (in metres)
    // These will be set by the ratios if useScreenPercentage is true
    register_property<MultiMeshLOD, float>("minDist", &MultiMeshLOD::minDist, 5.0f); 
    register_property<MultiMeshLOD, float>("maxDist", &MultiMeshLOD::maxDist, 80.0f); 

    // Screen percentage ratios (and if applicable)
    register_property<MultiMeshLOD, bool>("useScreenPercentage", &MultiMeshLOD::useScreenPercentage, true);
    register_property<MultiMeshLOD, float>("minRatio", &MultiMeshLOD::minRatio, 2.0f);
    register_property<MultiMeshLOD, float>("maxRatio", &MultiMeshLOD::maxRatio, 5.0f);
    register_property<MultiMeshLOD, float>("FOV", &MultiMeshLOD::FOV, 70.0f);
    register_property<MultiMeshLOD, bool>("interactedWithManager", &MultiMeshLOD::interactedWithManager, false, GODOT_METHOD_RPC_MODE_DISABLED, GODOT_PROPERTY_USAGE_NOEDITOR);

    // Whether to use distance multipliers from project settings
    register_property<MultiMeshLOD, bool>("affectedByDistanceMultipliers", &MultiMeshLOD::affectedByDistanceMultipliers, true);

    register_property<MultiMeshLOD, int64_t>("minCount", &MultiMeshLOD::minCount, 0); 
    register_property<MultiMeshLOD, int64_t>("maxCount", &MultiMeshLOD::maxCount, -1); 

    register_property<MultiMeshLOD, float>("fadeSpeed", &MultiMeshLOD::fadeSpeed, 1.0f);
    register_property<MultiMeshLOD, float>("fadeExponent", &MultiMeshLOD::fadeExponent, 1.0f);

    register_property<MultiMeshLOD, float>("tickSpeed", &MultiMeshLOD::tickSpeed, 0.1f);
}

MultiMeshLOD::MultiMeshLOD() {
}

MultiMeshLOD::~MultiMeshLOD() {
    // add your cleanup here
}

void MultiMeshLOD::_init() {
}

void MultiMeshLOD::_exit_tree() {
    // Leave LOD manager's list
    enabled = false;
    attemptRegister(false);
}

void MultiMeshLOD::_ready() {
    multiMesh = *get_multimesh();
    if (maxCount < 0) {
        maxCount = multiMesh->get_instance_count();
    }
    targetCount = maxCount;

    // FOV and AABB initial set up is done by the manager

    // Tell the LOD manager that we want to be part of the LOD list
    attemptRegister(true);
}

void MultiMeshLOD::processData(Vector3 cameraLoc) {
    // Double check for this node being in the scene tree
    // (Otherwise you get errors when ending the thread)
    // Get the distance from the node to the camera
    Vector3 objLoc;
    if (is_inside_tree()) {
        objLoc = get_global_transform().origin;
    } else {
        return;
    }

    float distance = cameraLoc.distance_to(objLoc);

    // Get our target value
    // ((max - current) / (max - min))^fadeExponent will give us the ratio of where we want to set our values
    // Multiply by how many we are interpolating then add the minimum
    targetCount = int64_t(floor(pow(CLAMP((maxDist * globalDistMult - distance) / (maxDist * globalDistMult  - minDist * globalDistMult), 0.0, 1.0), fadeExponent) * (maxCount - minCount))) + minCount;
}

void MultiMeshLOD::_process(float delta) {
    // Enter manager's list if not already done so (possibly due to timing issues upon game load)
    if (!registered && enabled) {
        attemptRegister(true);
    }

    // Lerp visible instance count if needed
    int64_t instanceCount = multiMesh->get_visible_instance_count();
    if (instanceCount != targetCount) {
        /// Lerp
        // We normally floor the value, but in case of high FPS, our lerp might get stuck.
        // Let's get the equation result first. Then, if the difference is positive and above 0.001,
        // make sure we raise the count by at least 1.
        float nextValue = CLAMP(instanceCount + (targetCount - instanceCount) * delta * fadeSpeed, 0.1f, maxCount);
        int nextInstanceCount = int64_t(floor(nextValue));
        if (nextValue - (float)instanceCount > 0.001f && instanceCount == nextInstanceCount) {
            nextInstanceCount++;
        }
        instanceCount = nextInstanceCount;
        multiMesh->set_visible_instance_count(instanceCount);

        if (instanceCount == 0 && is_visible()) {
            hide();
        } else if (instanceCount > 0 && !is_visible()) {
            show();
        }
    }
}

// Update the distances based on the AABB
void MultiMeshLOD::updateLodAABB() {
    AABB objAABB = get_transformed_aabb();

    if (objAABB.has_no_area()) {
        printf("%s: ", get_name().alloc_c_string());
        printf("Invalid AABB for this MultiMeshInstance!\n");
        return;
    }

    // Get the longest axis (conservative estimate of the object size vs screen)
    float longestAxis = objAABB.get_longest_axis_size();

    // Use an isosceles triangle to get a worst-case estimate of the distances
    float tanTheta = tan((FOV * 3.14f / 180.0f));

    // Get the distances at which we have the LOD ratios of the screen
    minDist = ((longestAxis / (maxRatio / 100.0f)) / (2.0f * tanTheta));
    maxDist = ((longestAxis / (minRatio / 100.0f)) / (2.0f * tanTheta));
}

void MultiMeshLOD::updateLodMultipliersFromManager() {
    if (affectedByDistanceMultipliers) {
        Node* LODManagerNode = get_node("/root/LodManager");
        globalDistMult = LODManagerNode->get("globalDistMult");
    } else {
        globalDistMult = 1.0f;
    }
}

bool MultiMeshLOD::attemptRegister(bool state) {
    if (get_node("/root/LodManager")) {
        if (state) {
            get_node("/root/LodManager")->call("addObject", (Node*) this);
            updateLodMultipliersFromManager();
            registered = true;
        } else {
            get_node("/root/LodManager")->call("removeObject", (Node*) this);
            registered = false;
            interactedWithManager = false;
        }
        return true;
    }
    return false;
}