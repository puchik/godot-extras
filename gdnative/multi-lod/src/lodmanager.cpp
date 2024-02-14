#include "lod.h"
#include <string>
#include <ctime>
#define MAX_ARRAY_SIZE 30000

using namespace godot;

void LODManager::_register_methods() {
    register_property<LODManager, bool>("use_multithreading", &LODManager::use_multithreading, true);
    register_property<LODManager, int>("objectsPerFrame", &LODManager::objects_per_frame, 10000);

    register_property<LODManager, float>("fov", &LODManager::set_fov, &LODManager::get_fov, 70.f);

    register_property<LODManager, bool>("update_fov_every_loop", &LODManager::update_fov_every_loop, false);
    register_property<LODManager, bool>("update_AABB_every_loop", &LODManager::update_AABB_every_loop, false);
    register_property<LODManager, float>("tick_speed", &LODManager::tick_speed, 0.2f);

    // Multipliers, should be visible to other nodes so they may update.
    register_property<LODManager, bool>("update_multipliers_every_loop", &LODManager::update_multipliers_every_loop, false);
    // However, if modifying directly, you must call updateLodMultipliersInObjects to take effect in objects
    register_property<LODManager, float>("global_distance_multiplier", &LODManager::global_distance_multiplier, 1.0f);
    register_property<LODManager, float>("lod1_distance_multiplier", &LODManager::lod1_distance_multiplier, 1.0f);
    register_property<LODManager, float>("lod2_distance_multiplier", &LODManager::lod2_distance_multiplier, 1.0f);
    register_property<LODManager, float>("lod3_distance_multiplier", &LODManager::lod3_distance_multiplier, 1.0f);
    register_property<LODManager, float>("hide_distance_multiplier", &LODManager::hide_distance_multiplier, 1.0f);
    register_property<LODManager, float>("unload_distance_multiplier", &LODManager::unload_distance_multiplier, 1.0f);
    register_property<LODManager, float>("shadow_distance_multiplier", &LODManager::shadow_distance_multiplier, 1.0f);

    // 0 = off, 1 = print the steps in each thread loop, 2 = print the object names as we go through them.
    register_property<LODManager, int>("debug_level", &LODManager::debug_level, 0, GODOT_METHOD_RPC_MODE_DISABLED, 
        GODOT_PROPERTY_USAGE_DEFAULT, GODOT_PROPERTY_HINT_ENUM, "Off,Print thread actions,Print thread actions and object names");

    // Exposed methods
    register_method("stop_loop", &LODManager::stop_loop);
    register_method("update_lod_multipliers_from_settings", &LODManager::update_lod_multipliers_from_settings);
    register_method("update_lod_multipliers_in_objects", &LODManager::update_lod_multipliers_in_objects);
    register_method("update_lod_AABBs", &LODManager::update_lod_AABBs);
    register_method("update_fov", &LODManager::update_fov);
    register_method("set_up_camera", &LODManager::set_up_camera);
    register_method("set_camera", &LODManager::set_camera);

    register_method("_ready", &LODManager::_ready);
    register_method("_process", &LODManager::_process);
    register_method("_exit_tree", &LODManager::_exit_tree);

    // Thread function
    register_method("main_loop", &LODManager::main_loop);
}

LODManager::LODManager() {
}

LODManager::~LODManager() {
    // add your cleanup here
}

void LODManager::_init() {
    // initialize any variables here
}

void LODManager::_exit_tree() {
    // Stop loop thread if exiting
    if (use_multithreading) {
        stop_loop();
    }
}

void LODManager::_ready() {
    // Control threading with semaphore
    if (use_multithreading) {
        debug_level_print(1, "LODManager: Creating the Semaphore for the LOD thread.");
        // Do it before the update LOD mults and FOV call because it's used there
        lod_objects_semaphore = (Ref<Semaphore>) Semaphore::_new();
        lod_objects_semaphore->post();
    }

    debug_level_print(1, "LODManager: Setting up the camera.");
    set_up_camera();

    debug_level_print(1, "LODManager: Getting the LOD project settings.");
    project_settings = ProjectSettings::get_singleton();
    update_lod_multipliers_from_settings();
    // We'll be checking the FPS
    perf = Performance::get_singleton();
    last_run = clock();

    // Start main loop thread
    if (use_multithreading) {
        // Start thread
        if (debug_level > 0)
        {
            Godot::print("Starting the LOD thread.");
        }
        lod_loop_thread = (Ref<Thread>) Thread::_new();
        lod_loop_thread->start(this, "main_loop");
    }
}

void LODManager::_process(float delta) {
    if (!camera) {
        set_up_camera();
    }
    if (!use_multithreading && !manager_removed) {
        lod_function();
    }
}

void LODManager::lod_function() {
        current_fps = perf->get_monitor(0); // gets the FPS
        // A bit hacky, but we'll check if the main thread is frozen/doing something heavy.
        // We don't need LOD when nothing is really moving nor do we want to potentially overload
        // the main thread with deferred calls.
        if (current_fps < 7.0f) {
            return;
        }

        // Could use OS.delay_msec, but loop may take a long time so keep track of time manually instead
        if ((double)(clock() - last_run) / CLOCKS_PER_SEC < tick_speed) {
            return;
        }

        debug_level_print(1, "LODManager: Checking if the camera is valid.");
        if (camera && camera->is_inside_tree()) {
            debug_level_print(1, "LODManager: Getting the camera location.");
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
            debug_level_print(1, String("LODManager: Processing list of LOD Objects, size: ") + String::num_int64(lod_object_count));
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
                debug_level_print(2, "LODManager: Checking if a LOD object is valid and inside the scene tree.");

                if (!lod_objects[j]) {
                    ERR_PRINT("LODManager: Invalid object found in process array.");
                    continue;
                }

                LODBaseComponent* lbc = lod_objects[j];
                Spatial* obj = lbc->get_object();

                if (!obj->is_inside_tree()) {
                    ERR_PRINT(obj->get_name() + String(": LOD Object is not in the scene tree"));
                    continue;
                }

                debug_level_print(2, String("LODManager: ") + obj->get_name() + ": LOD object is valid.");

                // Update multiplier if needed. It will fetch the distances from our public values
                if (update_multipliers_flag) {
                    debug_level_print(2, "LODManager: Telling the LOD object to update its multipliers in the LOD thread.");
                    lbc->update_lod_multipliers_from_manager();
                }

                // Update AABB if needed
                if (update_AABBs_flag && lbc->use_screen_percentage) {
                    debug_level_print(2, "LODManager: Telling this LOD object to update its AABBs.");
                    lbc->update_lod_AABB();
                }

                // Pass camera location and do calculations on LOD object
                debug_level_print(2, "LODManager: Calling the process_data function with the camera found.");
                lbc->process_data(camera_location);
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
    debug_level_print(1, "LODManager: Stopping the LOD Manager loop/main thread.");
    set_process(false);
    if (manager_removed) {
        return;
    } else {
        manager_removed = true;
    }

    if (lod_loop_thread->is_active()) {
        lod_loop_thread->wait_to_finish();
    }

    lod_objects_semaphore->free();
}

void LODManager::add_object(LODBaseComponent* lbc) {
    // Go through array list to find one that has space
    if (use_multithreading) {
        lod_objects_semaphore->wait();
    }

    debug_level_print(1, lbc->get_object()->get_name() + String(": Adding a new object to the LOD Manager's list"));

    bool added = false; // Keep track in case we need to add a new list
    for (int i = 0; i < lod_object_arrays.size(); i++) {
        std::vector<LODBaseComponent*>& lod_objects = lod_object_arrays[i];
        if (lod_objects.size() < MAX_ARRAY_SIZE) {
            lod_objects.push_back(lbc);
            lod_object_count++;
            added = true;
            break;
        }
    }

    if (!added) {
        std::vector<LODBaseComponent*> newLODObjectArray;
        newLODObjectArray.push_back(lbc);
        lod_object_arrays.push_back(newLODObjectArray);
        lod_object_count++; // why wasn't this here?
    }

    if (use_multithreading) {
        lod_objects_semaphore->post();
    }
}

void LODManager::remove_object(LODBaseComponent* lbc) {
    // If we're closing/have closed the LOD manager, don't bother doing expensive finds and removes.
    // It might take a while and is unnecessary.
    if (manager_removed) {
        // Also array should be freed by Godot
        return;
    }

    debug_level_print(1, lbc->get_object()->get_name() + String(": Removing an object from the LOD Manager's list"));
    
    if (use_multithreading) {
        lod_objects_semaphore->wait();
    }
    //TOOD Don't call this function for every node on exit_tree, just mark as unregistered
    // Find which array has this object
    bool done = false;
    for (int i = 0; i < lod_object_arrays.size(); i++) {
        std::vector<LODBaseComponent*>& lod_objects = lod_object_arrays[i];
        for (std::vector<LODBaseComponent*>::iterator it = lod_objects.begin(); it != lod_objects.end(); ) {
            if (*it == lbc) {
                it = lod_objects.erase(it);
                lod_object_count--;
                done = true;
                break;
            } else {
                ++it;
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
    debug_level_print(1, "LODManager: Loading LOD multipliers from the project settings.");
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
    debug_level_print(1, "LODManager: Setting the flag to update LOD multipliers in the LOD thread to true.");

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
    debug_level_print(1, "LODManager: Updating the camera FOV, and setting flag to update AABBs.");
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
    debug_level_print(1, "LODManager: Setting the flag to update LOD AABBs in the LOD thread.");
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
    debug_level_print(1, "LODManager: Setting up the camera info.");
    if (get_viewport()->get_camera()) {
        camera = get_viewport()->get_camera();
        update_fov();
        return true;
    }
    return false;
}

bool LODManager::set_camera(Camera* p_camera) {
    debug_level_print(1, "LODManager: Setting the camera.");
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
        Godot::print(message);
    }
}