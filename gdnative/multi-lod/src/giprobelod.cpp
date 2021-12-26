#include "lod.h"

using namespace godot;

void GIProbeLOD::_register_methods() {
    register_method("_process", &GIProbeLOD::_process);
    register_method("_ready", &GIProbeLOD::_ready);
    register_method("_exit_tree", &GIProbeLOD::_exit_tree);
    register_method("process_data", &GIProbeLOD::process_data);

    // Exposed methods
    register_method("update_lod_AABB", &GIProbeLOD::update_lod_AABB);
    register_method("update_lod_multipliers_from_manager", &GIProbeLOD::update_lod_multipliers_from_manager);

    // Vars for distance-based (in metres)
    // This will be set by the ratio if useScreenPercentage is true
    // Distance and ratio exposed names and variable names do not match to avoid massive compatability breakage with an older version of the addon.
    register_property<GIProbeLOD, float>("hideDist", &GIProbeLOD::hide_distance, 80.0f); 
    register_property<GIProbeLOD, float>("fade_range", &GIProbeLOD::fade_range, 5.0f);

    // Screen percentage ratios (and if applicable)
    register_property<GIProbeLOD, bool>("use_screen_percentage", &GIProbeLOD::use_screen_percentage, true);
    register_property<GIProbeLOD, float>("hideRatio", &GIProbeLOD::hide_ratio, 2.0f);
    register_property<GIProbeLOD, float>("fov", &GIProbeLOD::fov, 70.0f, GODOT_METHOD_RPC_MODE_DISABLED, GODOT_PROPERTY_USAGE_NOEDITOR);
    register_property<GIProbeLOD, bool>("registered", &GIProbeLOD::registered, false, GODOT_METHOD_RPC_MODE_DISABLED, GODOT_PROPERTY_USAGE_NOEDITOR);
    register_property<GIProbeLOD, bool>("interacted_with_manager", &GIProbeLOD::interacted_with_manager, false, GODOT_METHOD_RPC_MODE_DISABLED, GODOT_PROPERTY_USAGE_NOEDITOR);

    // Whether to use distance multipliers from project settings
    register_property<GIProbeLOD, bool>("affected_by_distance_multipliers", &GIProbeLOD::affected_by_distance_multipliers, true);

    register_property<GIProbeLOD, float>("fade_speed", &GIProbeLOD::fade_speed, 1.0f);

    register_property<GIProbeLOD, bool>("enabled", &GIProbeLOD::enabled, true);
}

GIProbeLOD::GIProbeLOD() {
}

GIProbeLOD::~GIProbeLOD() {
}

void GIProbeLOD::_init() {
}

void GIProbeLOD::_exit_tree() {
    // Leave LOD manager's list.
    enabled = false;
    LODCommonFunctions::try_register(Object::cast_to<Node>(this), false);
}

void GIProbeLOD::_ready() {
    if (get_class() != "GIProbe") {
        printf("%s: ", get_name().alloc_c_string());
        printf("A GIProbeLOD script is attached, but this is not a GIProbe!\n");
        enabled = false;
        return;
    }

    // Save original probe energy
    probe_base_energy = get_energy();
    probe_target_energy = probe_base_energy;

    LODCommonFunctions::try_register(Object::cast_to<Node>(this), true);
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
    if (!registered && enabled) {
        LODCommonFunctions::try_register(Object::cast_to<Node>(this), true);
    }

    // Fade GIProbe if needed
    real_t probe_energy = get_energy();
    if (registered && enabled && (probe_energy != probe_target_energy)) {
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
    AABB objAABB = get_transformed_aabb();

    if (objAABB.has_no_area()) {
        printf("%s: ", get_name().alloc_c_string());
        printf("Invalid AABB for this GIProbe!\n");
        return;
    }

    // Get the longest axis (conservative estimate of the object size vs screen)
    float longest_axis = objAABB.get_longest_axis_size();

    // Use an isosceles triangle to get a worst-case estimate of the distances
    float tan_theta = LODCommonFunctions::lod_calculate_AABB_distance_tan_theta(fov);

    // Get the distances at which we have the LOD ratios of the screen
    hide_distance = ((longest_axis / (hide_ratio / 100.0f)) / (2.0f * tan_theta));
}

void GIProbeLOD::update_lod_multipliers_from_manager() {
    if (affected_by_distance_multipliers && get_node("/root/LodManager")) {
        Node* lod_manager_node = get_node("/root/LodManager");
        global_distance_multiplier = lod_manager_node->get("global_distance_multiplier");
    } else {
        global_distance_multiplier = 1.0f;
    }
}