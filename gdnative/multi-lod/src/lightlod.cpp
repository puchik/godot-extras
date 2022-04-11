#include "lod.h"

using namespace godot;

void LightLOD::_register_methods() {
    // Inspector properties
    register_property<LightLOD, bool>("enabled", &LightLOD::set_enabled, &LightLOD::get_enabled, true);

    // Whether to use distance multipliers from project settings
    register_property<LightLOD, bool>("affectedByDistanceMultipliers", &LightLOD::set_affected_by_distance, &LightLOD::get_affected_by_distance, true);

    // Screen percentage ratios (and if applicable)
    register_property<LightLOD, bool>("use_screen_percentage", &LightLOD::set_use_screen_percentage, &LightLOD::get_use_screen_percentage, true);

    // Vars for distance-based (in metres)
    // These will be set by the ratios if useScreenPercentage is true
    // Distance and ratio exposed names and variable names do not match to avoid massive compatability breakage with an older version of the addon.
    register_property<LightLOD, float>("shadowDist", &LightLOD::shadow_distance, 50.0f);
    register_property<LightLOD, float>("hideDist", &LightLOD::hide_distance, 150.0f); // -1 to never unload
    register_property<LightLOD, float>("fade_range", &LightLOD::fade_range, 5.0f); 
    register_property<LightLOD, float>("fade_speed", &LightLOD::fade_speed, 2.0f);

    // Screen percentage ratios (and if applicable)
    register_property<LightLOD, float>("shadowRatio", &LightLOD::shadow_ratio, 6.0f);
    register_property<LightLOD, float>("hideRatio", &LightLOD::hide_ratio, 2.0f);

    // Exposed methods
    register_method("_process", &LightLOD::_process);
    register_method("_ready", &LightLOD::_ready);
    register_method("_enter_tree", &LightLOD::_enter_tree);
    register_method("_exit_tree", &LightLOD::_exit_tree);
    register_method("update_lod_AABB", &LightLOD::update_lod_AABB);
    register_method("update_lod_multipliers_from_manager", &LightLOD::update_lod_multipliers_from_manager);
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
    lc.unregister();

}

void LightLOD::_enter_tree() {
    // Ready and not registered? Probably re-entered the tree and need to re-regster.
    if (!lc.registered && lc.ready_finished) {
        lc.unregister();
        set_process(true);
    }
}

void LightLOD::_process(float delta) {
    // Enter manager's list if not already done so (possibly due to timing issues upon game load)
    if (!lc.registered) {
        lc.try_register();
        set_process(false);
    }

    if (lc.registered) {
        // Fade light
        fade_light(delta);
        // Fade shadows
        fade_shadow(delta);
    }
}

void LightLOD::_ready() {
    lc.setup(Object::cast_to<Spatial>(this));
    lc.lod_manager->debug_level_print(1, get_name() + String(": Initializing LightLOD."));

    if ((get_class() != "OmniLight") && (get_class() != "SpotLight")) {
        ERR_PRINT(get_name() + ": A LightLOD script is attached, but this is not a Light!");
        lc.enabled = false;
        set_process(false);
        return;
    }

    // Save original light and shadow colours
    light_base_energy = get_param(PARAM_ENERGY);
    light_target_energy = light_base_energy;
    shadow_target_color = get_shadow_color();

    update_lod_AABB();
    update_lod_multipliers_from_manager();
    lc.try_register();
    lc.ready_finished = true;
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
    if (lc.use_screen_percentage) {
        AABB object_AABB = get_transformed_aabb();

        if (object_AABB.has_no_area()) {
            ERR_PRINT(get_name() + ": Invalid AABB for this light!");
            return;
        }

        // Get the longest axis (conservative estimate of the object size vs screen)
        float longest_axis = object_AABB.get_longest_axis_size();

        // Use an isosceles triangle to get a worst-case estimate of the distances
        float tan_theta = lc.get_tan_theta();

        // Get the distances at which we have the LOD ratios of the screen
        shadow_distance = ((longest_axis / (shadow_ratio / 100.0f)) / (2.0f * tan_theta));
        hide_distance = ((longest_axis / (hide_ratio / 100.0f)) / (2.0f * tan_theta));
    }
}

void LightLOD::update_lod_multipliers_from_manager() {
    if (lc.affected_by_distance_multipliers && lc.lod_manager) {
        global_distance_multiplier = lc.lod_manager->global_distance_multiplier;
        shadow_distance_multiplier = lc.lod_manager->shadow_distance_multiplier;
    } else {
        global_distance_multiplier = 1.0f;
        shadow_distance_multiplier = 1.0f;
    }
}