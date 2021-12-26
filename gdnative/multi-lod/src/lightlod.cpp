#include "lod.h"

using namespace godot;

void LightLOD::_register_methods() {
    register_method("_process", &LightLOD::_process);
    register_method("_ready", &LightLOD::_ready);
    register_method("_exit_tree", &LightLOD::_exit_tree);
    register_method("process_data", &LightLOD::process_data);

    // Exposed methods
    register_method("update_lod_AABB", &LightLOD::update_lod_AABB);
    register_method("update_lod_multipliers_from_manager", &LightLOD::update_lod_multipliers_from_manager);

    // Vars for distance-based (in metres)
    // These will be set by the ratios if useScreenPercentage is true
    // Distance and ratio exposed names and variable names do not match to avoid massive compatability breakage with an older version of the addon.
    register_property<LightLOD, float>("shadowDist", &LightLOD::shadow_distance, 50.0f);
    register_property<LightLOD, float>("hideDist", &LightLOD::hide_distance, 150.0f); // -1 to never unload
    register_property<LightLOD, float>("fade_range", &LightLOD::fade_range, 5.0f); 

    // Screen percentage ratios (and if applicable)
    register_property<LightLOD, bool>("use_screen_percentage", &LightLOD::use_screen_percentage, true);
    register_property<LightLOD, float>("shadowRatio", &LightLOD::shadow_ratio, 6.0f);
    register_property<LightLOD, float>("hideRatio", &LightLOD::hide_ratio, 2.0f);
    register_property<LightLOD, float>("fov", &LightLOD::fov, 70.0f, GODOT_METHOD_RPC_MODE_DISABLED, GODOT_PROPERTY_USAGE_NOEDITOR);
    register_property<LightLOD, bool>("registered", &LightLOD::registered, false, GODOT_METHOD_RPC_MODE_DISABLED, GODOT_PROPERTY_USAGE_NOEDITOR);
    register_property<LightLOD, bool>("interacted_with_manager", &LightLOD::interacted_with_manager, false, GODOT_METHOD_RPC_MODE_DISABLED, GODOT_PROPERTY_USAGE_NOEDITOR);

    // Whether to use distance multipliers from project settings
    register_property<LightLOD, bool>("affected_by_distance_multipliers", &LightLOD::affected_by_distance_multipliers, true);

    register_property<LightLOD, float>("fade_speed", &LightLOD::fade_speed, 2.0f);

    register_property<LightLOD, bool>("enabled", &LightLOD::enabled, true);
}

LightLOD::LightLOD() {
}

LightLOD::~LightLOD() {
    // add your cleanup here
}

void LightLOD::_init() {
}

void LightLOD::_exit_tree() {
    // Leave LOD manager's list.
    enabled = false;
    LODCommonFunctions::try_register(Object::cast_to<Node>(this), false);
}

void LightLOD::_ready() {
    if ((get_class() != "OmniLight") && (get_class() != "SpotLight")) {
        printf("%s: ", get_name().alloc_c_string());
        printf("A LightLOD script is attached, but this is not a Light!\n");
        enabled = false;
        return;
    }

    // Save original light and shadow colours
    light_base_energy = get_param(PARAM_ENERGY);
    light_target_energy = light_base_energy;
    shadow_target_color = get_shadow_color();

    LODCommonFunctions::try_register(Object::cast_to<Node>(this), true);
}

void LightLOD::process_data(Vector3 camera_location) {
    // Double check for this node being in the scene tree
    // (Otherwise you get errors when ending the thread)
    // Get the distance from the node to the camera
    Vector3 object_location;
    if (is_inside_tree()) {
        object_location = get_global_transform().origin;
    } else {
        return;
    }

    float distance = camera_location.distance_to(object_location);

    // Get our target values for light and shadow
    // (max - current) / (max - min) will give us the ratio of where we want to set our values
    if (shadow_distance >= 0) {
        float actual_shadow_distance = shadow_distance * shadow_distance_multiplier * global_distance_multiplier;
        float shadow_calculation_ratio = CLAMP((actual_shadow_distance - distance) / (actual_shadow_distance - (actual_shadow_distance - fade_range)),
                                                0.0f, 
                                                1.0f);
        float shadow_calculation_result = 1.0f - shadow_calculation_ratio;
        shadow_target_color = Color(shadow_calculation_result, shadow_calculation_result, shadow_calculation_result, 1.0f);
    }

    if (hide_distance >= 0) {
        float actual_hide_distance = hide_distance * global_distance_multiplier;
        light_target_energy = CLAMP((actual_hide_distance - distance) / (actual_hide_distance - (actual_hide_distance - fade_range)),
                                    0.0f,
                                    light_base_energy);
    }

}

void LightLOD::_process(float delta) {
    // Enter manager's list if not already done so (possibly due to timing issues upon game load)
    if (!registered && enabled) {
        LODCommonFunctions::try_register(Object::cast_to<Node>(this), true);
    }

    if (registered && enabled) {
        // Fade light
        fade_light(delta);
        // Fade shadows
        fade_shadow(delta);
    }
}

void LightLOD::fade_light(float delta) {
    real_t light_energy = get_param(PARAM_ENERGY);
    if (light_energy != light_target_energy) {
        /// Lerp
        // If the light energy wasn't 1, then the fade might be slower or faster
        // We don't want the speed to be dependent on light energy, so multiply speed by base
        light_energy = light_energy + ((light_target_energy - light_energy) * (fade_speed * light_base_energy) * delta);
        set_param(PARAM_ENERGY, light_energy);

        if ((light_energy < 0.05) && is_visible()) {
            hide();
        } else if ((light_energy >= 0.05) && !is_visible()) {
            show();
        }
    }
}

void LightLOD::fade_shadow(float delta) {
    Color shadow_color = get_shadow_color();
    if (shadow_color != shadow_target_color) {
        if ((shadow_color.r > 0.95) && has_shadow()) {
            set_shadow(false);
        } else if ((shadow_color.r < 0.95) && !has_shadow()) {
            set_shadow(true);
        }

        // Lerp
        shadow_color = shadow_color.linear_interpolate(shadow_target_color, fade_speed * delta);
        set_shadow_color(shadow_color);
    }
}

// Update the distances based on the AABB
void LightLOD::update_lod_AABB() {
    AABB object_AABB = get_transformed_aabb();

    if (object_AABB.has_no_area()) {
        printf("%s: ", get_name().alloc_c_string());
        printf("Invalid AABB for this light!\n");
        return;
    }

    // Get the longest axis (conservative estimate of the object size vs screen)
    float longest_axis = object_AABB.get_longest_axis_size();

    // Use an isosceles triangle to get a worst-case estimate of the distances
    float tan_theta = LODCommonFunctions::lod_calculate_AABB_distance_tan_theta(fov);

    // Get the distances at which we have the LOD ratios of the screen
    shadow_distance = ((longest_axis / (shadow_ratio / 100.0f)) / (2.0f * tan_theta));
    hide_distance = ((longest_axis / (hide_ratio / 100.0f)) / (2.0f * tan_theta));
}

void LightLOD::update_lod_multipliers_from_manager() {
    if (affected_by_distance_multipliers && get_node("/root/LodManager")) {
        Node* lod_manager_node = get_node("/root/LodManager");
        global_distance_multiplier = lod_manager_node->get("global_distance_multiplier");
        shadow_distance_multiplier = lod_manager_node->get("shadow_distance_multiplier");
    } else {
        global_distance_multiplier = 1.0f;
        shadow_distance_multiplier = 1.0f;
    }
}