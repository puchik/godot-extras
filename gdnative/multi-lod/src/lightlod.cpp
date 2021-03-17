#include "lod.h"

using namespace godot;

void LightLOD::_register_methods() {
    register_method("_process", &LightLOD::_process);
    register_method("_ready", &LightLOD::_ready);
    register_method("_exit_tree", &LightLOD::_exit_tree);
    register_method("processData", &LightLOD::processData);

    // Exposed methods
    register_method("updateLodAABB", &LightLOD::updateLodAABB);
    register_method("updateLodMultipliersFromManager", &LightLOD::updateLodMultipliersFromManager);

    // Vars for distance-based (in metres)
    // These will be set by the ratios if useScreenPercentage is true
    register_property<LightLOD, float>("shadowDist", &LightLOD::shadowDist, 50.0f);
    register_property<LightLOD, float>("hideDist", &LightLOD::hideDist, 150.0f); // -1 to never unload
    register_property<LightLOD, float>("fadeRange", &LightLOD::fadeRange, 5.0f); 

    // Screen percentage ratios (and if applicable)
    register_property<LightLOD, bool>("useScreenPercentage", &LightLOD::useScreenPercentage, true);
    register_property<LightLOD, float>("shadowRatio", &LightLOD::shadowRatio, 6.0f);
    register_property<LightLOD, float>("hideRatio", &LightLOD::hideRatio, 2.0f);
    register_property<LightLOD, float>("FOV", &LightLOD::FOV, 70.0f);
    register_property<LightLOD, bool>("interactedWithManager", &LightLOD::interactedWithManager, false, GODOT_METHOD_RPC_MODE_DISABLED, GODOT_PROPERTY_USAGE_NOEDITOR);

    // Whether to use distance multipliers from project settings
    register_property<LightLOD, bool>("affectedByDistanceMultipliers", &LightLOD::affectedByDistanceMultipliers, true);

    register_property<LightLOD, float>("fadeSpeed", &LightLOD::fadeSpeed, 2.0f);

    register_property<LightLOD, float>("tickSpeed", &LightLOD::tickSpeed, 0.5f);
}

LightLOD::LightLOD() {
}

LightLOD::~LightLOD() {
    // add your cleanup here
}

void LightLOD::_init() {
}

void LightLOD::_exit_tree() {
    // Leave LOD manager's list
    enabled = false;
    attemptRegister(false);
}

void LightLOD::_ready() {
    // Save original light and shadow colours
    lightBaseEnergy = get_param(PARAM_ENERGY);
    lightTargetEnergy = lightBaseEnergy;
    shadowTargetColor = get_shadow_color();

    // FOV and AABB initial set up is done by the manager

    // Tell the LOD manager that we want to be part of the LOD list
    attemptRegister(true);
}

void LightLOD::processData(Vector3 cameraLoc) {
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

    // Get our target values for light and shadow
    // (max - current) / (max - min) will give us the ratio of where we want to set our values
    if (hideDist >= 0) {
        lightTargetEnergy = CLAMP((hideDist * globalDistMult - distance) / 
            (hideDist * globalDistMult - (hideDist * globalDistMult - fadeRange)), 0.0f, lightBaseEnergy);
    }

    if (shadowDist >= 0) {
        float shadowRatio = CLAMP((shadowDist * shadowDistMult * globalDistMult - distance) / 
            (shadowDist * shadowDistMult * globalDistMult - (shadowDist * shadowDistMult * globalDistMult - fadeRange)), 0.0f, 1.0f);
        shadowTargetColor = Color(1.0f - shadowRatio, 1.0f - shadowRatio, 1.0f - shadowRatio, 1.0f);
    }

}

void LightLOD::_process(float delta) {
    // Enter manager's list if not already done so (possibly due to timing issues upon game load)
    if (!registered && enabled) {
        attemptRegister(true);
    }

    if (registered && enabled) {
        // Fade light
        fadeLight(delta);
        // Fade shadows
        fadeShadow(delta);
    }
}

void LightLOD::fadeLight(float delta) {
    real_t lightEnergy = get_param(PARAM_ENERGY);
    if (lightEnergy != lightTargetEnergy) {
        /// Lerp
        // If the light energy wasn't 1, then the fade might be slower or faster
        // We don't want the speed to be dependent on light energy, so multiply speed by base
        lightEnergy = lightEnergy + (lightTargetEnergy - lightEnergy) * delta * (fadeSpeed * lightBaseEnergy);
        set_param(PARAM_ENERGY, lightEnergy);

        if (lightEnergy < 0.05 && is_visible()) {
            hide();
        } else if (lightEnergy >= 0.05 && !is_visible()) {
            show();
        }
    }
}

void LightLOD::fadeShadow(float delta) {
    Color shadowColor = get_shadow_color();
    if (shadowColor != shadowTargetColor) {
        if (shadowColor.r > 0.95 && has_shadow()) {
            set_shadow(false);
        } else if (shadowColor.r < 0.95 && !has_shadow()) {
            set_shadow(true);
        }

        // Lerp
        shadowColor = shadowColor.linear_interpolate(shadowTargetColor, fadeSpeed * delta);
        set_shadow_color(shadowColor);
    }
}

// Update the distances based on the AABB
void LightLOD::updateLodAABB() {
    AABB objAABB = get_transformed_aabb();

    if (objAABB.has_no_area()) {
        printf("%s: ", get_name().alloc_c_string());
        printf("Invalid AABB for this light!\n");
        return;
    }

    // Get the longest axis (conservative estimate of the object size vs screen)
    float longestAxis = objAABB.get_longest_axis_size();

    // Use an isosceles triangle to get a worst-case estimate of the distances
    float tanTheta = tan((FOV * 3.14f / 180.0f));

    // Get the distances at which we have the LOD ratios of the screen
    shadowDist = ((longestAxis / (shadowRatio / 100.0f)) / (2.0f * tanTheta));
    hideDist = ((longestAxis / (hideRatio / 100.0f)) / (2.0f * tanTheta));
}

void LightLOD::updateLodMultipliersFromManager() {
    if (affectedByDistanceMultipliers) {
        Node* LODManagerNode = get_node("/root/LodManager");
        globalDistMult = LODManagerNode->get("globalDistMult");
        shadowDistMult = LODManagerNode->get("shadowDistMult");
    } else {
        globalDistMult = 1.0f;
        shadowDistMult = 1.0f;
    }
}

bool LightLOD::attemptRegister(bool state) {
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