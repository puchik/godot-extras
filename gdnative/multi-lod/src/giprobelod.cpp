#include "lod.h"

using namespace godot;

void GIProbeLOD::_register_methods() {
    register_method("_process", &GIProbeLOD::_process);
    register_method("_ready", &GIProbeLOD::_ready);
    register_method("_exit_tree", &GIProbeLOD::_exit_tree);
    register_method("processData", &GIProbeLOD::processData);

    // Exposed methods
    register_method("updateLodAABB", &GIProbeLOD::updateLodAABB);
    register_method("updateLodMultipliersFromManager", &GIProbeLOD::updateLodMultipliersFromManager);

    // Vars for distance-based (in metres)
    // This will be set by the ratio if useScreenPercentage is true
    register_property<GIProbeLOD, float>("hideDist", &GIProbeLOD::hideDist, 80.0f); 
    register_property<GIProbeLOD, float>("fadeRange", &GIProbeLOD::fadeRange, 5.0f);

    // Screen percentage ratios (and if applicable)
    register_property<GIProbeLOD, bool>("useScreenPercentage", &GIProbeLOD::useScreenPercentage, true);
    register_property<GIProbeLOD, float>("hideRatio", &GIProbeLOD::hideRatio, 2.0f);
    register_property<GIProbeLOD, float>("FOV", &GIProbeLOD::FOV, 70.0f);
    register_property<GIProbeLOD, bool>("interactedWithManager", &GIProbeLOD::interactedWithManager, false, GODOT_METHOD_RPC_MODE_DISABLED, GODOT_PROPERTY_USAGE_NOEDITOR);

    // Whether to use distance multipliers from project settings
    register_property<GIProbeLOD, bool>("affectedByDistanceMultipliers", &GIProbeLOD::affectedByDistanceMultipliers, true);

    register_property<GIProbeLOD, float>("fadeSpeed", &GIProbeLOD::fadeSpeed, 1.0f);

    register_property<GIProbeLOD, float>("tickSpeed", &GIProbeLOD::tickSpeed, 0.5f);

    register_property<GIProbeLOD, bool>("enabled", &GIProbeLOD::enabled, true);
}

GIProbeLOD::GIProbeLOD() {
}

GIProbeLOD::~GIProbeLOD() {
    // add your cleanup here
}

void GIProbeLOD::_init() {
}

void GIProbeLOD::_exit_tree() {
    // Leave LOD manager's list
    attemptRegister(false);
}

void GIProbeLOD::_ready() {
    if (get_class() != "GIProbe") {
        printf("%s: ", get_name().alloc_c_string());
        printf("A GIProbeLOD script is attached, but this is not a GIProbe!\n");
        enabled = false;
        return;
    }

    // Save original probe energy
    probeBaseEnergy = get_energy();
    probeTargetEnergy = probeBaseEnergy;

    // FOV and AABB initial set up is done by the manager

    // Tell the LOD manager that we want to be part of the LOD list
    attemptRegister(true);
}

void GIProbeLOD::processData(Vector3 cameraLoc) {
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
    // (max - current) / (max - min) will give us the ratio of where we want to set our values
    probeTargetEnergy = CLAMP((hideDist * globalDistMult - distance) / (hideDist * globalDistMult - (hideDist * globalDistMult - fadeRange)), 0.0f, 1.0f);

}

void GIProbeLOD::_process(float delta) {
    // Enter manager's list if not already done so (possibly due to timing issues upon game load)
    if (!registered && enabled) {
        attemptRegister(true);
    }

    // Fade GIProbe if needed
    real_t probeEnergy = get_energy();
    if (registered && enabled && probeEnergy != probeTargetEnergy) {
        /// Lerp
        // If the probe energy wasn't 1, then the fade might be slower or faster
        // We don't want the speed to be dependent on energy, so multiply speed by base
        probeEnergy = probeEnergy + (probeTargetEnergy - probeEnergy) * delta * (fadeSpeed * probeBaseEnergy);
        set_energy(probeEnergy);

        if (probeEnergy < 0.05 && is_visible()) {
            hide();
        } else if (probeEnergy >= 0.05 && !is_visible()) {
            show();
        }
    }
}

// Update the distances based on the AABB
void GIProbeLOD::updateLodAABB() {
    AABB objAABB = get_transformed_aabb();

    if (objAABB.has_no_area()) {
        printf("%s: ", get_name().alloc_c_string());
        printf("Invalid AABB for this GIProbe!\n");
        return;
    }

    // Get the longest axis (conservative estimate of the object size vs screen)
    float longestAxis = objAABB.get_longest_axis_size();

    // Use an isosceles triangle to get a worst-case estimate of the distances
    float tanTheta = tan((FOV * 3.14f / 180.0f));

    // Get the distances at which we have the LOD ratios of the screen
    hideDist = ((longestAxis / (hideRatio / 100.0f)) / (2.0f * tanTheta));
}

void GIProbeLOD::updateLodMultipliersFromManager() {
    if (affectedByDistanceMultipliers) {
        Node* LODManagerNode = get_node("/root/LodManager");
        globalDistMult = LODManagerNode->get("globalDistMult");
    } else {
        globalDistMult = 1.0f;
    }
}

bool GIProbeLOD::attemptRegister(bool state) {
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