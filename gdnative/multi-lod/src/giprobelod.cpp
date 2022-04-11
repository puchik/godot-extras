#include "lod.h"

using namespace godot;

void GIProbeLOD::_register_methods() {
    register_method("_process", &GIProbeLOD::_process);
    register_method("_ready", &GIProbeLOD::_ready);
    register_method("_enter_tree", &GIProbeLOD::_enter_tree);
    register_method("_exit_tree", &GIProbeLOD::_exit_tree);
    register_method("process_data", &GIProbeLOD::process_data);
    // Inspector properties
    register_property<GIProbeLOD, bool>("enabled", &GIProbeLOD::set_enabled, &GIProbeLOD::get_enabled, true);

    // Whether to use distance multipliers from project settings
    register_property<GIProbeLOD, bool>("affectedByDistanceMultipliers", &GIProbeLOD::set_affected_by_distance, &GIProbeLOD::get_affected_by_distance, true);

    // Exposed methods
    register_method("update_lod_AABB", &GIProbeLOD::update_lod_AABB);
    register_method("update_lod_multipliers_from_manager", &GIProbeLOD::update_lod_multipliers_from_manager);
    // Screen percentage ratios (and if applicable)
    register_property<GIProbeLOD, bool>("use_screen_percentage", &GIProbeLOD::set_use_screen_percentage, &GIProbeLOD::get_use_screen_percentage, true);

    // Vars for distance-based (in metres)
    // This will be set by the ratio if useScreenPercentage is true
    // Distance and ratio exposed names and variable names do not match to avoid massive compatability breakage with an older version of the addon.
    register_property<GIProbeLOD, float>("hideDist", &GIProbeLOD::hide_distance, 80.0f); 
    register_property<GIProbeLOD, float>("fade_range", &GIProbeLOD::fade_range, 5.0f);

    register_property<GIProbeLOD, float>("hideRatio", &GIProbeLOD::hide_ratio, 2.0f);
    register_property<GIProbeLOD, float>("fade_speed", &GIProbeLOD::fade_speed, 1.0f);

}

GIProbeLOD::GIProbeLOD() {
}

GIProbeLOD::~GIProbeLOD() {
}

void GIProbeLOD::_init() {
}

void GIProbeLOD::_exit_tree() {
    // Leave LOD manager's list.
    lc.unregister();

}

void GIProbeLOD::_enter_tree() {
    // Ready and not registered? Probably re-entered the tree and need to re-regster.
    if (!lc.registered && lc.ready_finished) {
        lc.unregister();
        set_process(true);
    }
}

void GIProbeLOD::_ready() {
    lc.setup(Object::cast_to<Spatial>(this));
    lc.lod_manager->debug_level_print(1, get_name() + String(": Initializing GIProbeLOD."));

    if (get_class() != "GIProbe") {
        ERR_PRINT(get_name() + ": A GIProbeLOD script is attached, but this is not a GIProbe!");
        lc.enabled = false;
        return;
    }

    // Save original probe energy
    probe_base_energy = get_energy();
    probe_target_energy = probe_base_energy;

    update_lod_AABB();
    update_lod_multipliers_from_manager();
    lc.try_register();
    lc.ready_finished = true;
}

void GIProbeLOD::process_data(Vector3 camera_location) {
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

    // Get our target value
    // (max - current) / (max - min) will give us the ratio of where we want to set our values
    float actual_hide_distance = hide_distance * global_distance_multiplier;
    probe_target_energy = CLAMP((actual_hide_distance - distance) / (actual_hide_distance - (actual_hide_distance - fade_range)), 
                                0.0f, 
                                1.0f);

}

void GIProbeLOD::_process(float delta) {
    // Enter manager's list if not already done so (possibly due to timing issues upon game load)
    if (!lc.registered) {
        lc.try_register();
        set_process(false);
    }

    // Fade GIProbe if needed
    real_t probe_energy = get_energy();
    if (lc.registered && (probe_energy != probe_target_energy)) {
        /// Lerp
        // If the probe energy wasn't 1, then the fade might be slower or faster
        // We don't want the speed to be dependent on energy, so multiply speed by base
        probe_energy = probe_energy + ((probe_target_energy - probe_energy) * (fade_speed * probe_base_energy * delta));
        set_energy(probe_energy);

        if ((probe_energy < 0.05) && is_visible()) {
            hide();
        } else if ((probe_energy >= 0.05) && !is_visible()) {
            show();
        }
    }
}

// Update the distances based on the AABB
void GIProbeLOD::update_lod_AABB() {
    if (lc.use_screen_percentage) {
        AABB objAABB = get_transformed_aabb();

        if (objAABB.has_no_area()) {
            ERR_PRINT(get_name() + ": Invalid AABB for this GIProbe!");
            return;
        }

        // Get the longest axis (conservative estimate of the object size vs screen)
        float longest_axis = objAABB.get_longest_axis_size();

        // Get the distances at which we have the LOD ratios of the screen
        hide_distance = ((longest_axis / (hide_ratio / 100.0f)) / (2.0f * lc.get_tan_theta()));
    }
}

void GIProbeLOD::update_lod_multipliers_from_manager() {
    if (lc.affected_by_distance_multipliers && lc.lod_manager) {
        global_distance_multiplier = lc.lod_manager->global_distance_multiplier;
    } else {
        global_distance_multiplier = 1.0f;
    }
}