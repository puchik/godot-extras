#include "lod.h"
#include <string>
#include <cmath>

using namespace godot;

void LOD::_register_methods() {
    // Godot and thread functions
    register_method("_process", &LOD::_process);
    register_method("_ready", &LOD::_ready);
    register_method("_enter_tree", &LOD::_enter_tree);
    register_method("_exit_tree", &LOD::_exit_tree);
    register_method("process_data", &LOD::process_data);
    // Exposed methods
    register_method("update_lod_AABB", &LOD::update_lod_AABB);
    register_method("update_lod_multipliers_from_manager", &LOD::update_lod_multipliers_from_manager);
    register_method("get_current_lod", &LOD::get_current_lod);

    // Inspector properties
    register_property<LOD, bool>("enabled", &LOD::enabled, true);

    // Vars for distance-based (in metres)
    // These will be set by the ratios if use_screen_percentage is true
    // Distance and ratio exposed names and variable names do not match to avoid massive compatability breakage with an older version of the addon.
    register_property<LOD, float>("lod1dist", &LOD::lod1_distance, 7.0f); // put any of these to -1 if you don't have the lod for it, don't want to hide/unload etc
    register_property<LOD, float>("lod2dist", &LOD::lod2_distance, 12.0f);
    register_property<LOD, float>("lod3dist", &LOD::lod3_distance, 30.0f);
    register_property<LOD, float>("hideDist", &LOD::hide_distance, 100.0f);
    register_property<LOD, float>("unloadDist", &LOD::unload_distance, -1.0f);

    // Screen percentage ratios (and if applicable)
    register_property<LOD, bool>("use_screen_percentage", &LOD::use_screen_percentage, true);
    register_property<LOD, float>("lod1ratio", &LOD::lod1_ratio, 25.0f); // put any of these to -1 if you don't have the lod for it, don't want to hide/unload etc
    register_property<LOD, float>("lod2ratio", &LOD::lod2_ratio, 10.0f);
    register_property<LOD, float>("lod3ratio", &LOD::lod3_ratio, 5.5f);
    register_property<LOD, float>("hideRatio", &LOD::hide_ratio, 1.0f);
    register_property<LOD, float>("unloadRatio", &LOD::unload_ratio, -1.0f);
    register_property<LOD, float>("fov", &LOD::fov, 70.0f, GODOT_METHOD_RPC_MODE_DISABLED, GODOT_PROPERTY_USAGE_NOEDITOR);
    register_property<LOD, bool>("registered", &LOD::registered, false, GODOT_METHOD_RPC_MODE_DISABLED, GODOT_PROPERTY_USAGE_NOEDITOR);
    register_property<LOD, bool>("ready_finished", &LOD::ready_finished, false, GODOT_METHOD_RPC_MODE_DISABLED, GODOT_PROPERTY_USAGE_NOEDITOR);
    register_property<LOD, bool>("interacted_with_manager", &LOD::interacted_with_manager, false, GODOT_METHOD_RPC_MODE_DISABLED, GODOT_PROPERTY_USAGE_NOEDITOR);
    
    // Whether to use distance multipliers from project settings
    register_property<LOD, bool>("affected_by_distance_multipliers", &LOD::affected_by_distance_multipliers, true);

    register_property<LOD, int>("max_shadow_caster", &LOD::max_shadow_caster, 0,
        GODOT_METHOD_RPC_MODE_DISABLED,
        GODOT_PROPERTY_USAGE_DEFAULT,
        GODOT_PROPERTY_HINT_ENUM,
        "LOD0,LOD1,LOD2,LOD3"
        );

    register_property<LOD, NodePath>("lod0_path", &LOD::lod0_path, NodePath());
    register_property<LOD, NodePath>("lod1_path", &LOD::lod1_path, NodePath());
    register_property<LOD, NodePath>("lod2_path", &LOD::lod2_path, NodePath());
    register_property<LOD, NodePath>("lod3_path", &LOD::lod3_path, NodePath());

    register_signal<LOD>("lod_changed", "lod", GODOT_VARIANT_TYPE_INT);
    register_signal<LOD>("freed");
}

LOD::LOD() {
}

LOD::~LOD() {
    // add your cleanup here
}

void LOD::_init() {
}

void LOD::_exit_tree() {
    // Leave LOD manager's list.
    enabled = false;
    LODCommonFunctions::try_register(Object::cast_to<Node>(this), false);
}

void LOD::_enter_tree() {
    // Ready and not registered? Probably re-entered the tree and need to re-regster.
    if (!registered && ready_finished) {
        enabled = true;
        LODCommonFunctions::try_register(Object::cast_to<Node>(this), false);    
    }
}

void LOD::_ready() {
    lc.setup(Object::cast_to<Spatial>(this));
    lc.lod_manager->debug_level_print(1, get_name() + String(": Initializing Mesh LOD."));

    /// Get the VisualInstance objects for LOD nodes
    // If there's no path, search for any children with "LOD + n" in its name
    // So we don't have to fetch list of children once for every object
    Array child_nodes;
    int64_t child_count;
    if (!has_node(lod0_path)|| !has_node(lod1_path) || !has_node(lod2_path) || !has_node(lod3_path)) {
        child_nodes = get_children();
        child_count = get_child_count();
    }

    if (has_node(lod0_path)) {
        lods[0] = Object::cast_to<Spatial>(get_node(lod0_path));
        last_lod = 0;
    } else {
        for (int i = 0; i < child_count; i++) {
            Node *child = Object::cast_to<Node>(child_nodes[i]);
            if (child->get_name().find("LOD0") >= 0) {
                lods[0] = Object::cast_to<Spatial>(child);
                if (lods[0]) {
                    last_lod = 0;
                    break;
                }
            }
        }
    }

    if (has_node(lod1_path)) {
        lods[1] = Object::cast_to<Spatial>(get_node(lod1_path));
        last_lod = 1;
    } else {
        for (int i = 0; i < child_count; i++) {
            Node *child = Object::cast_to<Node>(child_nodes[i]);
            if (child->get_name().find("LOD1") >= 0) {
                lods[1] = Object::cast_to<Spatial>(child);
                if (lods[1]) {
                    last_lod = 1;
                    break;
                }
            }
        }
    }

    if (has_node(lod2_path)) {
        lods[2] = Object::cast_to<Spatial>(get_node(lod2_path));
        last_lod = 2;
    } else {
        for (int i = 0; i < child_count; i++) {
            Node *child = Object::cast_to<Node>(child_nodes[i]);
            if (child->get_name().find("LOD2") >= 0) {
                lods[2] = Object::cast_to<Spatial>(child);
                if (lods[2]) {
                    last_lod = 2;
                    break;
                }
            }
        }
    }

    if (has_node(lod3_path)) {
        lods[3] = Object::cast_to<Spatial>(get_node(lod3_path));
        last_lod = 3;
    } else {
        for (int i = 0; i < child_count; i++) {
            Node *child = Object::cast_to<Node>(child_nodes[i]);
            if (child->get_name().find("LOD3") >= 0) {
                lods[3] = Object::cast_to<Spatial>(child);
                if (lods[3]) {
                    last_lod = 3;
                    break;
                }
            }
        }
    }

    // Set up initial shadow casting
    for (int i = 0; i < max_shadow_caster; i++) {
        if (lods[i]) {
            lods[i]->set("cast_shadow", GeometryInstance::SHADOW_CASTING_SETTING_OFF);
        }
    }

    lc.try_register();
    lc.ready_finished = true;
}

void LOD::process_data(Vector3 camera_location) {
    // Double check for this node being in the scene tree
    // (Otherwise you get errors when ending the thread)
    // Get the distance from the node to the camera (and subtract AABB offset, if applicable)
    Vector3 object_location;
    if (is_inside_tree()) {
        object_location = get_global_transform().origin;
    } else {
        return;
    }
 
    if (use_screen_percentage) {
        object_location -= transform_offset_AABB;
    }
    float distance = camera_location.distance_to(object_location);

    // Calculate distances
    float actual_unload_distance = unload_distance * unload_distance_multiplier * global_distance_multiplier;
    float actual_hide_distance = hide_distance * hide_distance_multiplier * global_distance_multiplier;
    float actual_lod3_distance = lod3_distance * lod3_distance_multiplier * global_distance_multiplier;
    float actual_lod2_distance = lod2_distance * lod2_distance_multiplier * global_distance_multiplier;
    float actual_lod1_distance = lod1_distance * lod1_distance_multiplier * global_distance_multiplier;

    if ((actual_unload_distance > 0.0f) &&
        (distance > actual_unload_distance)) {
      emit_signal("freed");
      queue_free();
    } else if ((actual_hide_distance > 0.0f) && (distance > actual_hide_distance)) {
      show_lod(LOD_COUNT); // >= LOD_COUNT results in all LODs hidden
    } else if ((actual_lod3_distance > 0.0f) && (distance > actual_lod3_distance)) {
      show_lod(3);
    } else if ((actual_lod2_distance > 0.0f) && (distance > actual_lod2_distance)) {
      show_lod(2);
    } else if ((actual_lod1_distance > 0.0f) && (distance > actual_lod1_distance)) {
      show_lod(1);
    } else {
      show_lod(0);
    }
}

void LOD::_process(float delta) {
    // Enter manager's list if not already done so (possibly due to timing issues upon game load)
    if (!registered && enabled) {
        LODCommonFunctions::try_register(Object::cast_to<Node>(this), true);
    }
}

// Update the distances based on the AABB
void LOD::update_lod_AABB() {
    // Get an AABB for all objects
    // Spatials don't have AABB apparently, and it's convenient to have
    // an "empty" 3D object for the LOD objects' parent.
    // So make an AABB for the objects manually.
    
    // Check for at least LOD0
    if (!lods[0]) {
        ERR_PRINT(get_name() + ": You need to have a valid LOD0 for screen percentage based LOD.");
    }

    // Try casting the LOD objects to VisualInstance (that's the only way we can get an AABB!)
    VisualInstance *lod0_visual_instance = Object::cast_to<VisualInstance>(lods[0]);
    if (!lod0_visual_instance) {
        ERR_PRINT(get_name() + ": LOD0 could not be cast to VisualInstance for the AABB calculation (check the Node type)");
    }

    // Get base AABB using LOD0
    AABB object_AABB = lod0_visual_instance->get_transformed_aabb();

    if (object_AABB.has_no_area()) {
        ERR_PRINT(get_name() + ": Invalid AABB for LOD0!");
        return;
    }

    // Merge others if available
    if (lods[1]) {
        VisualInstance *lod1_visual_instance = Object::cast_to<VisualInstance>(lods[1]);
        if (lod1_visual_instance) {
            object_AABB = object_AABB.merge(lod1_visual_instance->get_transformed_aabb());
        }   
    }
    if (lods[2]) {
        VisualInstance *lod2_visual_instance = Object::cast_to<VisualInstance>(lods[2]);
        if (lod2_visual_instance) {
            object_AABB = object_AABB.merge(lod2_visual_instance->get_transformed_aabb());
        }
    }
    if (lods[3]) {
        VisualInstance *lod3_visual_instance = Object::cast_to<VisualInstance>(lods[3]);
        if (lod3_visual_instance) {
            object_AABB = object_AABB.merge(lod3_visual_instance->get_transformed_aabb());
        }
    }

    // Get the longest axis (conservative estimate of the object size vs screen)
    float longest_axis = object_AABB.get_longest_axis_size();

    // Use an isosceles triangle to get a worst-case estimate of the distances
    // Don't forget the degrees to radians conversion
    float tan_theta = LODCommonFunctions::lod_calculate_AABB_distance_tan_theta(fov);

    // Get the distances at which we have the LOD ratios of the screen
    lod1_distance = ((longest_axis / (lod1_ratio / 100.0f)) / (2.0f * tan_theta));
    lod2_distance = ((longest_axis / (lod2_ratio / 100.0f)) / (2.0f * tan_theta));
    lod3_distance = ((longest_axis / (lod3_ratio / 100.0f)) / (2.0f * tan_theta));
    hide_distance = ((longest_axis / (hide_ratio / 100.0f)) / (2.0f * tan_theta));

    // Get the offset of the parent position and the overall AABB
    transform_offset_AABB = get_global_transform().origin - (object_AABB.get_endpoint(0) + (object_AABB.size / 2.0f));
}

void LOD::update_lod_multipliers_from_manager() {
    if (affected_by_distance_multipliers && get_node("/root/LodManager")) {
        Node* LOD_manager_node = get_node("/root/LodManager");
        global_distance_multiplier = LOD_manager_node->get("global_distance_multiplier");
        lod1_distance_multiplier = LOD_manager_node->get("lod1_distance_multiplier");
        lod2_distance_multiplier = LOD_manager_node->get("lod2_distance_multiplier");
        lod3_distance_multiplier = LOD_manager_node->get("lod3_distance_multiplier");
        hide_distance_multiplier = LOD_manager_node->get("hide_distance_multiplier");
        unload_distance_multiplier = LOD_manager_node->get("unload_distance_multiplier");
    } else {
        global_distance_multiplier = 1.0f;
        lod1_distance_multiplier = 1.0f;
        lod2_distance_multiplier = 1.0f;
        lod2_distance_multiplier = 1.0f;
        hide_distance_multiplier = 1.0f;
        unload_distance_multiplier = 1.0f;
    }
}

void LOD::show_lod(int lod) {
    // If lod >= LOD_COUNT, all LODS will be hidden
   
    // Do nothing if already on this level
    if (lod == current_lod) {
        return;
    }

    current_lod = lod;
    emit_signal("lod_changed", lod);

    // If lod requested doesn't exist, show last active lod until actual_hide_distance
    if (((lod < LOD_COUNT) && !lods[lod])) {
        lod = last_lod;
    }

    // Count backwards to hit shadow caster first
    for(int i = last_lod; i >= 0 ; i--) {
        if (lods[i] && lods[i]->is_inside_tree()) {
            if (i == lod) {
                lods[i]->show();

                // If shadow casting enabled
                if (lods[max_shadow_caster] && max_shadow_caster > 0) {
                    // If lower LOD, turn on shadow caster, otherwise reset it
                    if (i < max_shadow_caster) {
                        lods[max_shadow_caster]->set("cast_shadow", GeometryInstance::SHADOW_CASTING_SETTING_SHADOWS_ONLY);
                        lods[max_shadow_caster]->show();
                    } else {
                        lods[max_shadow_caster]->set("cast_shadow", GeometryInstance::SHADOW_CASTING_SETTING_ON);
                    }
                }
            } else if (lods[i]->is_visible()) {
                lods[i]->hide();
            }
        }
    }
}
