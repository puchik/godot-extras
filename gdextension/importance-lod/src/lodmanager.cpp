#include "importance_lod.h"
#include <string>
#include <ctime>

#define MAX_ARRAY_SIZE 30000

#define DEBUG_PRINT_NONE 0
#define DEBUG_PRINT_ACTIONS 1
#define DEBUG_PRINT_ACTIONS_AND_NAMES 2

// Somewhat hacky. Some low value below which we do not run the LOD thread to avoid overloading the CPU with more work during heavy workloads.
#define MINIMUM_LOOP_FPS 7.0

using namespace godot;

void LODManager::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_use_multithreading"), &LODManager::get_use_multithreading);
    ClassDB::bind_method(D_METHOD("set_use_multithreading", "p_use_multithreading"), &LODManager::set_use_multithreading);
    ClassDB::add_property("LODManager", PropertyInfo(Variant::BOOL, "use_multithreading"), "set_use_multithreading", "get_use_multithreading");

    ClassDB::bind_method(D_METHOD("get_objects_per_frame"), &LODManager::get_objects_per_frame);
    ClassDB::bind_method(D_METHOD("set_objects_per_frame", "p_objects_per_frame"), &LODManager::set_objects_per_frame);
    ClassDB::add_property("LODManager", PropertyInfo(Variant::INT, "objects_per_frame"), "set_objects_per_frame", "get_objects_per_frame");

    ClassDB::bind_method(D_METHOD("get_fov"), &LODManager::get_fov);
    ClassDB::bind_method(D_METHOD("set_fov", "p_fov"), &LODManager::set_fov);
    ClassDB::add_property("LODManager", PropertyInfo(Variant::FLOAT, "fov"), "set_fov", "get_fov");

    ClassDB::bind_method(D_METHOD("get_update_fov_every_loop"), &LODManager::get_update_fov_every_loop);
    ClassDB::bind_method(D_METHOD("set_update_fov_every_loop", "p_update_fov_every_loop"), &LODManager::set_update_fov_every_loop);
    ClassDB::add_property("LODManager", PropertyInfo(Variant::BOOL, "update_fov_every_loop"), "set_update_fov_every_loop", "get_update_fov_every_loop");

    ClassDB::bind_method(D_METHOD("get_update_AABB_every_loop"), &LODManager::get_update_AABB_every_loop);
    ClassDB::bind_method(D_METHOD("set_update_AABB_every_loop", "p_update_AABB_every_loop"), &LODManager::set_update_AABB_every_loop);
    ClassDB::add_property("LODManager", PropertyInfo(Variant::BOOL, "update_AABB_every_loop"), "set_update_AABB_every_loop", "get_update_AABB_every_loop");

    ClassDB::bind_method(D_METHOD("get_tick_speed"), &LODManager::get_tick_speed);
    ClassDB::bind_method(D_METHOD("set_tick_speed", "p_tick_speed"), &LODManager::set_tick_speed);
    ClassDB::add_property("LODManager", PropertyInfo(Variant::FLOAT, "tick_speed"), "set_tick_speed", "get_tick_speed");

    // Multipliers, should be visible to other nodes so they may update.
    // However, if modifying directly, you must call update_lod_multipliers_in_objects to take effect in objects
    ClassDB::bind_method(D_METHOD("get_update_multipliers_every_loop"), &LODManager::get_update_multipliers_every_loop);
    ClassDB::bind_method(D_METHOD("set_update_multipliers_every_loop", "p_update_multipliers_every_loop"), &LODManager::set_update_multipliers_every_loop);
    ClassDB::add_property("LODManager", PropertyInfo(Variant::BOOL, "update_multipliers_every_loop"), "set_update_multipliers_every_loop", "get_update_multipliers_every_loop");

    ClassDB::bind_method(D_METHOD("get_global_distance_multiplier"), &LODManager::get_global_distance_multiplier);
    ClassDB::bind_method(D_METHOD("set_global_distance_multiplier", "p_global_distance_multiplier"), &LODManager::set_global_distance_multiplier);
    ClassDB::add_property("LODManager", PropertyInfo(Variant::FLOAT, "global_distance_multiplier"), "set_global_distance_multiplier", "get_global_distance_multiplier");

    ClassDB::bind_method(D_METHOD("get_lod1_distance_multiplier"), &LODManager::get_lod1_distance_multiplier);
    ClassDB::bind_method(D_METHOD("set_lod1_distance_multiplier", "p_lod1_distance_multiplier"), &LODManager::set_lod1_distance_multiplier);
    ClassDB::add_property("LODManager", PropertyInfo(Variant::FLOAT, "lod1_distance_multiplier"), "set_lod1_distance_multiplier", "get_lod1_distance_multiplier");

    ClassDB::bind_method(D_METHOD("get_lod2_distance_multiplier"), &LODManager::get_lod2_distance_multiplier);
    ClassDB::bind_method(D_METHOD("set_lod2_distance_multiplier", "p_lod2_distance_multiplier"), &LODManager::set_lod2_distance_multiplier);
    ClassDB::add_property("LODManager", PropertyInfo(Variant::FLOAT, "lod2_distance_multiplier"), "set_lod2_distance_multiplier", "get_lod2_distance_multiplier");

    ClassDB::bind_method(D_METHOD("get_lod3_distance_multiplier"), &LODManager::get_lod3_distance_multiplier);
    ClassDB::bind_method(D_METHOD("set_lod3_distance_multiplier", "p_lod3_distance_multiplier"), &LODManager::set_lod3_distance_multiplier);
    ClassDB::add_property("LODManager", PropertyInfo(Variant::FLOAT, "lod3_distance_multiplier"), "set_lod3_distance_multiplier", "get_lod3_distance_multiplier");

    ClassDB::bind_method(D_METHOD("get_hide_distance_multiplier"), &LODManager::get_hide_distance_multiplier);
    ClassDB::bind_method(D_METHOD("set_hide_distance_multiplier", "p_hide_distance_multiplier"), &LODManager::set_hide_distance_multiplier);
    ClassDB::add_property("LODManager", PropertyInfo(Variant::FLOAT, "hide_distance_multiplier"), "set_hide_distance_multiplier", "get_hide_distance_multiplier");

    ClassDB::bind_method(D_METHOD("get_unload_distance_multiplier"), &LODManager::get_use_multithreading);
    ClassDB::bind_method(D_METHOD("set_unload_distance_multiplier", "p_unload_distance_multiplier"), &LODManager::set_unload_distance_multiplier);
    ClassDB::add_property("LODManager", PropertyInfo(Variant::FLOAT, "unload_distance_multiplier"), "set_unload_distance_multiplier", "get_unload_distance_multiplier");

    // 0 = off, 1 = print the steps in each thread loop, 2 = print the object names as we go through them.
    ClassDB::bind_method(D_METHOD("get_debug_level"), &LODManager::get_debug_level);
    ClassDB::bind_method(D_METHOD("set_debug_level", "p_debug_level"), &LODManager::set_debug_level);
    ClassDB::add_property("LODManager", PropertyInfo(Variant::INT, "debug_level", PROPERTY_HINT_ENUM, "Off,Print thread actions,Print thread actions and object names"), 
        "set_debug_level", "get_debug_level");

    // Exposed methods
    ClassDB::bind_method(D_METHOD("stop_loop"), &LODManager::stop_loop);
    ClassDB::bind_method(D_METHOD("update_lod_multipliers_from_settings"), &LODManager::update_lod_multipliers_from_settings);
    ClassDB::bind_method(D_METHOD("update_lod_multipliers_in_objects"), &LODManager::update_lod_multipliers_in_objects);
    ClassDB::bind_method(D_METHOD("update_lod_AABBs"), &LODManager::update_lod_AABBs);
    ClassDB::bind_method(D_METHOD("update_fov"), &LODManager::update_fov);
    ClassDB::bind_method(D_METHOD("set_up_camera"), &LODManager::set_up_camera);
    ClassDB::bind_method(D_METHOD("set_camera", "p_camera"), &LODManager::set_camera);

    // Thread function
    ClassDB::bind_method(D_METHOD("main_loop"), &LODManager::main_loop);
}

LODManager::LODManager() {
}

LODManager::~LODManager() {
    // add your cleanup here
}

void LODManager::_init() {
    // TODO: GDExtensions run as tools at the time of writing. Remove this when Godot supports disabling this behaviour.
    if (Engine::get_singleton()->is_editor_hint()) {
        return;
    }
}

void LODManager::_exit_tree() {
    // TODO: GDExtensions run as tools at the time of writing. Remove this when Godot supports disabling this behaviour.
    if (Engine::get_singleton()->is_editor_hint()) {
        return;
    }

    // Stop loop thread if exiting
    if (use_multithreading) {
        stop_loop();
    }
}

void LODManager::_ready() {
    // TODO: GDExtensions run as tools at the time of writing. Remove this when Godot supports disabling this behaviour.
    if (Engine::get_singleton()->is_editor_hint()) {
        return;
    }

    // Control threading with semaphore
    if (use_multithreading) {
        debug_level_print(DEBUG_PRINT_ACTIONS, "LODManager: Creating the Semaphore for the LOD thread.");
        // Do it before the update LOD mults and FOV call because it's used there
        lod_objects_semaphore = Ref<Semaphore>(new Semaphore());
        lod_objects_semaphore->post();
    }
    debug_level_print(DEBUG_PRINT_ACTIONS, "LODManager: Setting up the camera.");
    set_up_camera();

    debug_level_print(DEBUG_PRINT_ACTIONS, "LODManager: Getting the LOD project settings.");
    project_settings = ProjectSettings::get_singleton();
    update_lod_multipliers_from_settings();
    // We'll be checking the FPS
    perf = Performance::get_singleton();
    last_run = clock();

    // Start main loop thread
    if (use_multithreading) {
        // Start thread
        debug_level_print(DEBUG_PRINT_ACTIONS, "Starting the LOD thread.");
        lod_loop_thread = Ref<Thread>(new Thread());
        Callable main_loop_callable = Callable(this, "main_loop").bind();
        lod_loop_thread->start(main_loop_callable);
    }
}

void LODManager::_process(float delta) {
    // TODO: GDExtensions run as tools at the time of writing. Remove this when Godot supports disabling this behaviour.
    if (Engine::get_singleton()->is_editor_hint()) {
        return;
    }

    if (!camera) {
        set_up_camera();
    }
    if (!use_multithreading && !manager_removed) {
        lod_function();
    }
}

void LODManager::lod_function() {
    ERR_FAIL_NULL(perf);
    current_fps = perf->get_monitor(Performance::Monitor::TIME_FPS); // gets the FPS
    // A bit hacky, but we'll check if the main thread is frozen/doing something heavy.
    // We don't need LOD when nothing is really moving nor do we want to potentially overload
    // the main thread with deferred calls.
    if (current_fps < MINIMUM_LOOP_FPS) {
        return;
    }
    // Could use OS.delay_msec, but loop may take a long time so keep track of time manually instead
    if ((double)(clock() - last_run) / CLOCKS_PER_SEC < tick_speed) {
        return;
    }

    debug_level_print(DEBUG_PRINT_ACTIONS, "LODManager: Checking if the camera is valid.");
    ERR_FAIL_NULL(camera);
    if (camera && camera->is_inside_tree()) {
        debug_level_print(DEBUG_PRINT_ACTIONS, "LODManager: Getting the camera location.");
        camera_location = camera->get_global_transform().origin;        
    } else {
        ERR_PRINT("LODManager: Camera is invalid or not attached to the scene tree.");
        stop_loop();
        return;
    }

    if (use_multithreading) {
        lod_objects_semaphore->wait();
    }

    last_run = clock();

    // --------- TODO: Make a dynamic number of objects checked per frame --------------------
    // Probably do a calculation here, use FPS
    // Get next "chunk" of objects to check if not multithreaded
    int next_loop_end_index;
    if (!use_multithreading) {
        next_loop_end_index = current_loop_index + 8000;
    }

    // Go through all array lists
    for (int i = 0; i < lod_object_arrays.size(); i++) {
        if (!use_multithreading && i == 0) {
            i = CLAMP((int)floor((double)current_loop_index / (double)MAX_ARRAY_SIZE), 0, (int)lod_object_arrays.size() - 1);
        }
        if (!use_multithreading && (next_loop_end_index < i * MAX_ARRAY_SIZE)) {
            break;
        }

        //Array lod_objects = lod_object_arrays[i];
        std::vector<LODBaseComponent*> lod_objects = lod_object_arrays[i];
        debug_level_print(DEBUG_PRINT_ACTIONS, String("LODManager: Processing list of LOD Objects, size: ") + String::num_int64(lod_object_count));
        for (int j = 0; j < lod_objects.size(); j++) {
            if (!use_multithreading && (j == 0)) {
                j = CLAMP(current_loop_index - i * MAX_ARRAY_SIZE, 0, (int)lod_objects.size() - 1);
            }
            if (!use_multithreading && (next_loop_end_index < (j + i * MAX_ARRAY_SIZE))) {
                break;
            }

            // LOD thread might be still going through the list when LOD exits tree, check
            if (manager_removed) {
                break;
            }

            // Make sure object exists *and* is in the tree.
            // Exiting without stopping the LOD thread causes errors otherwise.
            debug_level_print(DEBUG_PRINT_ACTIONS_AND_NAMES, "LODManager: Checking if a LOD object is valid and inside the scene tree.");

            if (!lod_objects[j]) {
                ERR_PRINT("LODManager: Invalid object found in process array.");
                continue;
            }

            LODBaseComponent* lod_object = lod_objects[j];
            Node3D* obj = lod_object->get_object();

            if (!obj->is_inside_tree()) {
                ERR_PRINT(obj->get_name() + String(": LOD Object is not in the scene tree"));
                continue;
            }

            debug_level_print(DEBUG_PRINT_ACTIONS_AND_NAMES, String("LODManager: ") + obj->get_name() + ": LOD object is valid.");

            // Update multiplier if needed. It will fetch the distances from our public values
            if (update_multipliers_flag) {
                debug_level_print(DEBUG_PRINT_ACTIONS_AND_NAMES, "LODManager: Telling the LOD object to update its multipliers in the LOD thread.");
                lod_object->update_lod_multipliers_from_manager();
            }

            // Update AABB if needed
            if (update_AABBs_flag && lod_object->use_screen_percentage) {
                debug_level_print(DEBUG_PRINT_ACTIONS_AND_NAMES, "LODManager: Telling this LOD object to update its AABBs.");
                lod_object->update_lod_AABB();
            }

            // Pass camera location and do calculations on LOD object
            debug_level_print(DEBUG_PRINT_ACTIONS_AND_NAMES, "LODManager: Calling the process_data function with the camera found.");
            lod_object->process_data(camera_location);
        }
        if (manager_removed) {
            break;
        }
    }
    if (use_multithreading) {
        lod_objects_semaphore->post();
    }

    if (!use_multithreading) {
        if (next_loop_end_index >= lod_object_count) {
            current_loop_index = 0;
        } else {
            current_loop_index = next_loop_end_index;
        }
    }

    if (update_multipliers_flag) {
        update_multipliers_flag = false;
    }
    if (update_AABBs_flag) {
        update_AABBs_flag = false;
    }

    // If necessary, update multipliers and/or FOV
    if (update_multipliers_every_loop) {
        update_lod_multipliers_from_settings();
    }
    if (update_fov_every_loop) {
        update_fov();
    }
    if (update_AABB_every_loop) {
        update_lod_AABBs();
    }
}

void LODManager::main_loop() {
    while (!manager_removed) {
        lod_function();
    }
}

void LODManager::stop_loop() {
    debug_level_print(DEBUG_PRINT_ACTIONS, "LODManager: Stopping the LOD Manager loop/main thread.");
    set_process(false);
    if (manager_removed) {
        return;
    } else {
        manager_removed = true;
    }

    if (lod_loop_thread->is_alive()) {
        lod_loop_thread->wait_to_finish();
    }

    //delete lod_objects_semaphore;
}

bool LODManager::add_object(LODBaseComponent* lod_object) {
    // Go through array list to find one that has space
    ERR_FAIL_COND_V_MSG(!lod_objects_semaphore.is_valid(), false, String("LODManager: LOD Objects Semaphore was somehow not initialized when trying to add a LOD object. Skipping."));
    if (use_multithreading) {
        lod_objects_semaphore->wait();
    }

    debug_level_print(DEBUG_PRINT_ACTIONS, lod_object->get_object()->get_name() + String(": Adding a new object to the LOD Manager's list"));

    bool added = false; // Keep track in case we need to add a new list
    for (int i = 0; i < lod_object_arrays.size(); i++) {
        std::vector<LODBaseComponent*>& lod_objects = lod_object_arrays[i];
        if (lod_objects.size() < MAX_ARRAY_SIZE) {
            lod_objects.push_back(lod_object);
            lod_object_count++;
            added = true;
            break;
        }
    }

    if (!added) {
        std::vector<LODBaseComponent*> new_lod_objects_vector;
        new_lod_objects_vector.push_back(lod_object);
        lod_object_arrays.push_back(new_lod_objects_vector);
        lod_object_count++;
    }

    if (use_multithreading) {
        lod_objects_semaphore->post();
    }

    return true;
}

void LODManager::remove_object(LODBaseComponent* lod_object) {
    // If we're closing/have closed the LOD manager, don't bother doing expensive finds and removes.
    // It might take a while and is unnecessary.
    if (manager_removed) {
        // Also array should be freed by Godot
        return;
    }

    debug_level_print(DEBUG_PRINT_ACTIONS, lod_object->get_object()->get_name() + String(": Removing an object from the LOD Manager's list"));
    if (use_multithreading) {
        lod_objects_semaphore->wait();
    }
    // TODO: Don't call this function for every node on exit_tree, just mark as unregistered
    // Find which array has this object
    bool done = false;
    for (int i = 0; i < lod_object_arrays.size(); i++) {
        std::vector<LODBaseComponent*>& lod_objects = lod_object_arrays[i];
        for (std::vector<LODBaseComponent*>::iterator lod_iterator = lod_objects.begin(); lod_iterator != lod_objects.end(); ) {
            if (*lod_iterator == lod_object) {
                lod_iterator = lod_objects.erase(lod_iterator);
                lod_object_count--;
                done = true;
                break;
            } else {
                ++lod_iterator;
            }
        }
        if (done) {
            break;
        }
    }

    if (use_multithreading) {
        lod_objects_semaphore->post();
    }
}


void LODManager::update_lod_multipliers_from_settings() {
    debug_level_print(DEBUG_PRINT_ACTIONS, "LODManager: Loading LOD multipliers from the project settings.");
    // Load multipliers from project settings
    global_distance_multiplier = (float)project_settings->get_setting("rendering/quality/lod/global_multiplier");
    lod1_distance_multiplier = (float)project_settings->get_setting("rendering/quality/lod/lod1_multiplier");
    lod2_distance_multiplier = (float)project_settings->get_setting("rendering/quality/lod/lod2_multiplier");
    lod3_distance_multiplier = (float)project_settings->get_setting("rendering/quality/lod/lod3_multiplier");
    hide_distance_multiplier = (float)project_settings->get_setting("rendering/quality/lod/hide_multiplier");
    unload_distance_multiplier = (float)project_settings->get_setting("rendering/quality/lod/unload_multiplier");
    shadow_distance_multiplier = (float)project_settings->get_setting("rendering/quality/lod/shadow_disable_multiplier");

    // Update LOD objects' multipliers
    update_lod_multipliers_in_objects();
}

void LODManager::update_lod_multipliers_in_objects() {
    debug_level_print(DEBUG_PRINT_ACTIONS, "LODManager: Setting the flag to update LOD multipliers in the LOD thread to true.");

    // Enable the flag to update LOD multipliers in the thread loop
    // Make sure we're not partway through the loop
    if (use_multithreading) {
        lod_objects_semaphore->wait();
    }

    update_multipliers_flag = true;

    if (use_multithreading) {
        lod_objects_semaphore->post();
    }
}

void LODManager::set_fov(float p_fov) {
    debug_level_print(DEBUG_PRINT_ACTIONS, "LODManager: Updating the camera FOV, and setting flag to update AABBs.");
    fov = p_fov;
    tan_theta = tan((p_fov * PI / 180.0f) / 2.0f);

    update_lod_AABBs();
}

// Just syntatic sugar
void LODManager::update_fov() {
    if (!camera) {
        ERR_PRINT("LODManager: No camera to get FOV from.");
        return;
    }
    set_fov(camera->get_fov());
}

void LODManager::update_lod_AABBs() {
    debug_level_print(DEBUG_PRINT_ACTIONS, "LODManager: Setting the flag to update LOD AABBs in the LOD thread.");

    // Enable the flag to update AABBs in the thread loop
    // Make sure we're not partway through the loop
    if (use_multithreading) {
        lod_objects_semaphore->wait();
    }

    update_AABBs_flag = true;

    if (use_multithreading) {
        lod_objects_semaphore->post();
    }
}

bool LODManager::set_up_camera() {
    debug_level_print(DEBUG_PRINT_ACTIONS, "LODManager: Setting up the camera info.");
    if (get_viewport()->get_camera_3d()) {
        camera = get_viewport()->get_camera_3d();
        update_fov();
        return true;
    }
    return false;
}

bool LODManager::set_camera(Camera3D* p_camera) {
    debug_level_print(DEBUG_PRINT_ACTIONS, "LODManager: Setting the camera.");
    if (p_camera) {
        camera = p_camera;
        update_fov();
        return true;
    } else {
        ERR_PRINT("LODManager: Camera provided in set_camera() is not valid.");
        return false;
    }
}

void LODManager::debug_level_print(int min_debug_level, const String &message) {
    if (debug_level >= min_debug_level) {
        UtilityFunctions::print(message);
    }
}