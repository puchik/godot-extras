#include "importance_lod.h"
#include <string>
#include <cmath>

using namespace godot;

void LOD::_bind_methods() {
    // Inspector properties
    ClassDB::bind_method(D_METHOD("get_enabled"), &LOD::get_enabled);
    ClassDB::bind_method(D_METHOD("set_enabled", "p_enabled"), &LOD::set_enabled);
    ClassDB::add_property("LOD", PropertyInfo(Variant::BOOL, "enabled"), "set_enabled", "get_enabled");

    // Whether to use distance multipliers from project settings
    ClassDB::bind_method(D_METHOD("get_affected_by_distance_multipliers"), &LODBaseComponent::get_affected_by_distance_multipliers);
    ClassDB::bind_method(D_METHOD("set_affected_by_distance_multipliers", "p_affected_by_distance_multipliers"), &LODBaseComponent::set_affected_by_distance_multipliers);
    ClassDB::add_property("LODBaseComponent", PropertyInfo(Variant::BOOL, "affected_by_distance_multipliers"), "set_affected_by_distance_multipliers", "get_affected_by_distance_multipliers");

    // Screen percentage ratios (and if applicable)
    ClassDB::bind_method(D_METHOD("get_use_screen_percentage"), &LOD::get_use_screen_percentage);
    ClassDB::bind_method(D_METHOD("set_use_screen_percentage", "p_use_screen_percentage"), &LOD::set_use_screen_percentage);
    ClassDB::add_property("LOD", PropertyInfo(Variant::BOOL, "use_screen_percentage"), "set_use_screen_percentage", "get_use_screen_percentage");

    // Vars for distance-based (in metres)
    // These will be set by the ratios if use_screen_percentage is true
    ClassDB::bind_method(D_METHOD("get_lod1_distance"), &LOD::get_enabled);
    ClassDB::bind_method(D_METHOD("set_lod1_distance", "p_lod1_distance"), &LOD::set_lod1_distance);
    ClassDB::add_property("LOD", PropertyInfo(Variant::FLOAT, "lod1_distance"), "set_lod1_distance", "get_lod1_distance");

    ClassDB::bind_method(D_METHOD("get_lod2_distance"), &LOD::get_enabled);
    ClassDB::bind_method(D_METHOD("set_lod2_distance", "p_lod2_distance"), &LOD::set_lod2_distance);
    ClassDB::add_property("LOD", PropertyInfo(Variant::FLOAT, "lod2_distance"), "set_lod2_distance", "get_lod2_distance");

    ClassDB::bind_method(D_METHOD("get_lod3_distance"), &LOD::get_enabled);
    ClassDB::bind_method(D_METHOD("set_lod3_distance", "p_lod3_distance"), &LOD::set_lod3_distance);
    ClassDB::add_property("LOD", PropertyInfo(Variant::FLOAT, "lod3_distance"), "set_lod3_distance", "get_lod3_distance");

    ClassDB::bind_method(D_METHOD("get_hide_distance"), &LOD::get_enabled);
    ClassDB::bind_method(D_METHOD("set_hide_distance", "p_hide_distance"), &LOD::set_hide_distance);
    ClassDB::add_property("LOD", PropertyInfo(Variant::FLOAT, "hide_distance"), "set_hide_distance", "get_hide_distance");

    ClassDB::bind_method(D_METHOD("get_unload_distance"), &LOD::get_enabled);
    ClassDB::bind_method(D_METHOD("set_unload_distance", "p_unload_distance"), &LOD::set_unload_distance);
    ClassDB::add_property("LOD", PropertyInfo(Variant::FLOAT, "unload_distance"), "set_unload_distance", "get_unload_distance");

    // Screen percentage ratios (and if applicable)
    ClassDB::bind_method(D_METHOD("get_lod1_ratio"), &LOD::get_enabled);
    ClassDB::bind_method(D_METHOD("set_lod1_ratio", "p_lod1_ratio"), &LOD::set_lod1_ratio);
    ClassDB::add_property("LOD", PropertyInfo(Variant::FLOAT, "lod1_ratio"), "set_lod1_ratio", "get_lod1_ratio");

    ClassDB::bind_method(D_METHOD("get_lod2_ratio"), &LOD::get_enabled);
    ClassDB::bind_method(D_METHOD("set_lod2_ratio", "p_lod2_ratio"), &LOD::set_lod2_ratio);
    ClassDB::add_property("LOD", PropertyInfo(Variant::FLOAT, "lod2_ratio"), "set_lod2_ratio", "get_lod2_ratio");

    ClassDB::bind_method(D_METHOD("get_lod3_ratio"), &LOD::get_enabled);
    ClassDB::bind_method(D_METHOD("set_lod3_ratio", "p_lod3_ratio"), &LOD::set_lod3_ratio);
    ClassDB::add_property("LOD", PropertyInfo(Variant::FLOAT, "lod3_ratio"), "set_lod3_ratio", "get_lod3_ratio");

    ClassDB::bind_method(D_METHOD("get_hide_ratio"), &LOD::get_enabled);
    ClassDB::bind_method(D_METHOD("set_hide_ratio", "p_hide_ratio"), &LOD::set_hide_ratio);
    ClassDB::add_property("LOD", PropertyInfo(Variant::FLOAT, "hide_ratio"), "set_hide_ratio", "get_hide_ratio");

    ClassDB::bind_method(D_METHOD("get_unload_ratio"), &LOD::get_enabled);
    ClassDB::bind_method(D_METHOD("set_unload_ratio", "p_unload_ratio"), &LOD::set_unload_ratio);
    ClassDB::add_property("LOD", PropertyInfo(Variant::FLOAT, "unload_ratio"), "set_unload_ratio", "get_unload_ratio");

    ClassDB::bind_method(D_METHOD("get_lod0_path"), &LOD::get_lod0_path);
    ClassDB::bind_method(D_METHOD("set_lod0_path", "p_lod0_path"), &LOD::set_lod0_path);
    ClassDB::add_property("LOD", PropertyInfo(Variant::NODE_PATH, "lod0_path"), "set_lod0_path", "get_lod0_path");

    ClassDB::bind_method(D_METHOD("get_lod1_path"), &LOD::get_lod1_path);
    ClassDB::bind_method(D_METHOD("set_lod1_path", "p_lod1_path"), &LOD::set_lod1_path);
    ClassDB::add_property("LOD", PropertyInfo(Variant::NODE_PATH, "lod1_path"), "set_lod1_path", "get_lod1_path");

    ClassDB::bind_method(D_METHOD("get_lod2_path"), &LOD::get_lod2_path);
    ClassDB::bind_method(D_METHOD("set_lod2_path", "p_lod2_path"), &LOD::set_lod2_path);
    ClassDB::add_property("LOD", PropertyInfo(Variant::NODE_PATH, "lod2_path"), "set_lod2_path", "get_lod2_path");

    ClassDB::bind_method(D_METHOD("get_lod3_path"), &LOD::get_lod3_path);
    ClassDB::bind_method(D_METHOD("set_lod3_path", "p_lod3_path"), &LOD::set_lod3_path);
    ClassDB::add_property("LOD", PropertyInfo(Variant::NODE_PATH, "lod3_path"), "set_lod3_path", "get_lod3_path");

    // Exposed methods
    ClassDB::bind_method(D_METHOD("update_lod_AABB"), &LOD::update_lod_AABB);
    ClassDB::bind_method(D_METHOD("update_lod_multipliers_from_manager"), &LOD::update_lod_multipliers_from_manager);
    ClassDB::bind_method(D_METHOD("get_current_lod"), &LOD::get_current_lod);

    // Signals
    ADD_SIGNAL(MethodInfo("lod_changed", PropertyInfo(Variant::INT, "new_lod")));
    ADD_SIGNAL(MethodInfo("freed"));
    
}

LOD::LOD() {
}

LOD::~LOD() {
    // add your cleanup here
}

void LOD::_init() {
    // TODO: GDExtensions run as tools at the time of writing. Remove this when Godot supports disabling this behaviour.
    if (Engine::get_singleton()->is_editor_hint()) {
        return;
    }
}

void LOD::_exit_tree() {
    // TODO: GDExtensions run as tools at the time of writing. Remove this when Godot supports disabling this behaviour.
    if (Engine::get_singleton()->is_editor_hint()) {
        return;
    }

    // Leave LOD manager's list.
    lod_component.unregister();
}

void LOD::_enter_tree() {
    // TODO: GDExtensions run as tools at the time of writing. Remove this when Godot supports disabling this behaviour.
    if (Engine::get_singleton()->is_editor_hint()) {
        return;
    }

    // Ready and not registered? Probably re-entered the tree and need to re-regster.
    if (!lod_component.registered && lod_component.ready_finished) {
        lod_component.unregister();
        set_process(true);
    }
}

void LOD::_process(float delta) {
    // TODO: GDExtensions run as tools at the time of writing. Remove this when Godot supports disabling this behaviour.
    if (Engine::get_singleton()->is_editor_hint()) {
        return;
    }

    // Enter manager's list if not already done so (possibly due to timing issues upon game load)
    if (!lod_component.registered) {
        lod_component.try_register();
        if (lod_component.registered) {
            set_process(false);
        }
    }
}

void LOD::_ready() {
    // TODO: GDExtensions run as tools at the time of writing. Remove this when Godot supports disabling this behaviour.
    if (Engine::get_singleton()->is_editor_hint()) {
        return;
    }

    lod_component.setup(Object::cast_to<Node3D>(this));
    lod_component.lod_manager->debug_level_print(1, get_name() + String(": Initializing Mesh LOD."));

    /// Get the VisualInstance3D objects for LOD nodes
    // If there's no path, search for any children with "LOD + n" in its name
    // So we don't have to fetch list of children once for every object
    Array child_nodes;
    int64_t child_count;
    if (!has_node(lod0_path)|| !has_node(lod1_path) || !has_node(lod2_path) || !has_node(lod3_path)) {
        child_nodes = get_children();
        child_count = get_child_count();
    }

    if (has_node(lod0_path)) {
        lods[0] = get_node<Node3D>(lod0_path);
        last_lod = 0;
    } else {
        for (int i = 0; i < child_count; i++) {
            Node *child = Object::cast_to<Node>(child_nodes[i]);
            if (child->get_name().find("LOD0") >= 0) {
                lods[0] = Object::cast_to<Node3D>(child);
                if (lods[0]) {
                    last_lod = 0;
                    break;
                }
            }
        }
    }

    if (has_node(lod1_path)) {
        lods[1] = get_node<Node3D>(lod1_path);
        last_lod = 1;
    } else {
        for (int i = 0; i < child_count; i++) {
            Node *child = Object::cast_to<Node>(child_nodes[i]);
            if (child->get_name().find("LOD1") >= 0) {
                lods[1] = Object::cast_to<Node3D>(child);
                if (lods[1]) {
                    last_lod = 1;
                    break;
                }
            }
        }
    }

    if (has_node(lod2_path)) {
        lods[2] = get_node<Node3D>(lod2_path);
        last_lod = 2;
    } else {
        for (int i = 0; i < child_count; i++) {
            Node *child = Object::cast_to<Node>(child_nodes[i]);
            if (child->get_name().find("LOD2") >= 0) {
                lods[2] = Object::cast_to<Node3D>(child);
                if (lods[2]) {
                    last_lod = 2;
                    break;
                }
            }
        }
    }

    if (has_node(lod3_path)) {
        lods[3] = get_node<Node3D>(lod3_path);
        last_lod = 3;
    } else {
        for (int i = 0; i < child_count; i++) {
            Node *child = Object::cast_to<Node>(child_nodes[i]);
            if (child->get_name().find("LOD3") >= 0) {
                lods[3] = Object::cast_to<Node3D>(child);
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
            lods[i]->set("cast_shadow", GeometryInstance3D::SHADOW_CASTING_SETTING_OFF);
        }
    }

    update_lod_AABB();
    update_lod_multipliers_from_manager();
    lod_component.try_register();
    lod_component.ready_finished = true;
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
 
    if (lod_component.use_screen_percentage) {
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

// Update the distances based on the AABB
void LOD::update_lod_AABB() {
    // Get an AABB for all objects
    // Node3Ds don't have AABB apparently, and it's convenient to have
    // an "empty" 3D object for the LOD objects' parent.
    // So make an AABB for the objects manually.
    
    // Check for at least LOD0
    ERR_FAIL_NULL_MSG(lods[0], String(get_name()) + ": Does not have a valid LOD0, required for screen percentage.");

    // Try casting the LOD objects to VisualInstance3D (that's the only way we can get an AABB!)
    VisualInstance3D *lod0_visual_instance = Object::cast_to<VisualInstance3D>(lods[0]);
    ERR_FAIL_NULL_MSG(lod0_visual_instance, String(get_name()) + ": LOD0 could not be cast to VisualInstance3D for the AABB calculation (check the Node type).");

    // Get base AABB using LOD0
    AABB object_AABB = lod0_visual_instance->get_aabb();
    ERR_FAIL_COND_MSG(!object_AABB.has_volume(), String(get_name()) + ": Invalid AABB for LOD0!");

    // Merge others if available
    if (lods[1]) {
        VisualInstance3D *lod1_visual_instance = Object::cast_to<VisualInstance3D>(lods[1]);
        if (lod1_visual_instance) {
            object_AABB = object_AABB.merge(lod1_visual_instance->get_aabb());
        }   
    }
    if (lods[2]) {
        VisualInstance3D *lod2_visual_instance = Object::cast_to<VisualInstance3D>(lods[2]);
        if (lod2_visual_instance) {
            object_AABB = object_AABB.merge(lod2_visual_instance->get_aabb());
        }
    }
    if (lods[3]) {
        VisualInstance3D *lod3_visual_instance = Object::cast_to<VisualInstance3D>(lods[3]);
        if (lod3_visual_instance) {
            object_AABB = object_AABB.merge(lod3_visual_instance->get_aabb());
        }
    }

    // Get the offset of the parent position and the overall AABB
    transform_offset_AABB = get_global_transform().origin - (object_AABB.get_endpoint(0) + (object_AABB.size / 2.0f));

    if (lod_component.use_screen_percentage) {
        // Get the longest axis (conservative estimate of the object size vs screen)
        float longest_axis = object_AABB.get_longest_axis_size();

        // Use an isosceles triangle to get a worst-case estimate of the distances
        // Don't forget the degrees to radians conversion
        float tan_theta = lod_component.get_tan_theta();

        // Get the distances at which we have the LOD ratios of the screen
        lod1_distance = ((longest_axis / (lod1_ratio / 100.0f)) / (2.0f * tan_theta));
        lod2_distance = ((longest_axis / (lod2_ratio / 100.0f)) / (2.0f * tan_theta));
        lod3_distance = ((longest_axis / (lod3_ratio / 100.0f)) / (2.0f * tan_theta));
        hide_distance = ((longest_axis / (hide_ratio / 100.0f)) / (2.0f * tan_theta));
    }
}

void LOD::update_lod_multipliers_from_manager() {
    if (lod_component.affected_by_distance_multipliers && lod_component.lod_manager) { 
        global_distance_multiplier = lod_component.lod_manager->global_distance_multiplier;
        lod1_distance_multiplier = lod_component.lod_manager->lod1_distance_multiplier;
        lod2_distance_multiplier = lod_component.lod_manager->lod2_distance_multiplier;
        lod3_distance_multiplier = lod_component.lod_manager->lod3_distance_multiplier;
        hide_distance_multiplier = lod_component.lod_manager->hide_distance_multiplier;
        unload_distance_multiplier = lod_component.lod_manager->unload_distance_multiplier;
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
                        lods[max_shadow_caster]->set("cast_shadow", GeometryInstance3D::SHADOW_CASTING_SETTING_SHADOWS_ONLY);
                        lods[max_shadow_caster]->show();
                    } else {
                        lods[max_shadow_caster]->set("cast_shadow", GeometryInstance3D::SHADOW_CASTING_SETTING_ON);
                    }
                }
            } else if (lods[i]->is_visible()) {
                lods[i]->hide();
            }
        }
    }
}
