#include "lod.h"
#include <string>
#include <ctime>
#define MAX_ARRAY_SIZE 30000

using namespace godot;

void LODManager::_register_methods() {
    register_property<LODManager, bool>("use_multithreading", &LODManager::use_multithreading, true);
    register_property<LODManager, int>("objectsPerFrame", &LODManager::objects_per_frame, 10000);
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

    register_property<LODManager, float>("fov", &LODManager::fov, 70.0f);
    register_property<LODManager, bool>("update_fov_every_loop", &LODManager::update_fov_every_loop, false);
    register_property<LODManager, bool>("update_AABB_every_loop", &LODManager::update_AABB_every_loop, false);
    register_property<LODManager, float>("tick_speed", &LODManager::tick_speed, 0.2f);

    // Called by LOD objects with themselves as the argument when entering the tree
    register_method("add_object", &LODManager::add_object);
    register_method("remove_object", &LODManager::remove_object);

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
        debug_level_print(1, "Creating the Semaphore for the LOD thread.\n");
        // Do it before the update LOD mults and FOV call because it's used there
        lod_objects_semaphore = (Ref<Semaphore>) Semaphore::_new();
        lod_objects_semaphore->post();
    }

    debug_level_print(1, "Setting up the camera for LOD.\n");
    set_up_camera();

    debug_level_print(1, "Getting the project settings for LOD");
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
            printf("Starting the LOD thread.\n");
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

        debug_level_print(1, "Checking if the camera we want to use in the LOD thread is valid.\n");
        if (camera && camera->is_inside_tree()) {
            debug_level_print(1, "Getting the camera location in the LOD thread.\n");
            camera_location = camera->get_global_transform().origin;        
        } else {
            debug_level_print(1, "Camera used by the LOD Manager is not in the scene tree.\n");
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
                i = CLAMP((int)floor((double)current_loop_index / (double)MAX_ARRAY_SIZE), 0, lod_object_arrays.size() - 1);
            }
            if (!use_multithreading && (next_loop_end_index < i * MAX_ARRAY_SIZE)) {
                break;
            }

            Array lod_objects = lod_object_arrays[i];
            debug_level_print(1, "Starting to go through our list of LOD objects.\n");
            for (int j = 0; j < lod_objects.size(); j++) {
                if (!use_multithreading && (j == 0)) {
                    j = CLAMP(current_loop_index - i * MAX_ARRAY_SIZE, 0, lod_objects.size() - 1);
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
                debug_level_print(2, "Checking if a LOD object list entry is valid and inside the scene tree in the LOD thread.\n");
                if (lod_objects[j] && Object::cast_to<Node>(lod_objects[j])->is_inside_tree()) {
                    debug_level_print(2, "LOD object is valid.\n");

                    debug_level_print(2, "Getting the Node associated with this LOD object in the LOD thread.\n");
                    Node* lod_object_node = Object::cast_to<Node>(lod_objects[j]);
                    debug_level_print(2, "Object name: ");
                    debug_level_print(2, lod_object_node->get_name().alloc_c_string());
                    debug_level_print(2, "\n");
                    // Update multiplier if needed
                    if (update_multipliers_flag && lod_object_node->has_method("update_lod_multipliers_from_manager")) {
                        debug_level_print(2, "Telling the LOD object to update its multipliers in the LOD thread.\n");
                        // Tell it to update. It will fetch the distances from our public values
                        lod_object_node->call("update_lod_multipliers_from_manager");
                    }
                    // If we are seeing it for the first time, give it our FOV and update its AABBs
                    debug_level_print(2, "Checking if this LOD object has interacted with the LOD manager yet in the LOD thread.\n");
                    bool interacted_with_manager = lod_object_node->get("interacted_with_manager");
                    // Update FOV if needed
                    if ((update_fovs_flag || !interacted_with_manager)) {
                        debug_level_print(2, "Setting the FOV in this LOD Object.\n");
                        lod_object_node->set("fov", fov);
                    }
                    // Update AABB if needed
                    if ((update_AABBs_flag || !interacted_with_manager) && lod_object_node->get("use_screen_percentage")) {
                        debug_level_print(2, "Telling this LOD object to update its AABBs.\n");
                        lod_object_node->call("update_lod_AABB");
                    }
                    if (!interacted_with_manager) {
                        debug_level_print(2, "Setting this LOD object's interaction with the manager flag to true.\n");
                        lod_object_node->set("interacted_with_manager", true);
                    }
                    // Pass camera location and do calculations on LOD object
                    debug_level_print(2, "Checking if the LOD object has a valid process_data function.\n");
                    if (lod_object_node->has_method("process_data")) {
                        debug_level_print(2, "It does. Calling the process_data function with the camera found.\n");
                        lod_object_node->call("process_data", camera_location);
                    } else {
                        debug_level_print(2, "A valid process_data function was not found.\n");
                    }
                } else {
                    debug_level_print(1, "LOD Object is NOT valid.\n");
                }
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
        if (update_fovs_flag) {
            update_fovs_flag = false;
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
    debug_level_print(1, "Stopping the LOD Manager loop/main thread.\n");
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

void LODManager::add_object(Node* obj) {
    // TODO: Print the object name.
    debug_level_print(1, "Adding a new object to the LOD Manager's list.\n");

    // Go through array list to find one that has space
    if (use_multithreading) {
        lod_objects_semaphore->wait();
    }

    bool added = false; // Keep track in case we need to add a new list
    for (int i = 0; i < lod_object_arrays.size(); i++) {
        Array lod_objects = lod_object_arrays[i];
        if (lod_objects.size() < MAX_ARRAY_SIZE) {
            lod_objects.push_back(obj);
            lod_object_count++;
            added = true;
            break;
        }
    }

    if (!added) {
        Array newLODObjectArray;
        newLODObjectArray.push_back(obj);
        lod_object_arrays.push_back(newLODObjectArray);
    }

    if (use_multithreading) {
        lod_objects_semaphore->post();
    }
}

void LODManager::remove_object(Node* obj) {
    // If we're closing/have closed the LOD manager, don't bother doing expensive finds and removes.
    // It might take a while and is unnecessary.
    if (manager_removed) {
        // Also array should be freed by Godot
        return;
    }

    // TODO: Print the object name.
    debug_level_print(1, "Removing an object from the LOD Manager's list.\n");
    if (use_multithreading) {
        lod_objects_semaphore->wait();
    }

    // Find which array has this object
    for (int i = 0; i < lod_object_arrays.size(); i++) {
        Array lod_objects = lod_object_arrays[i];
        int index = lod_objects.find(obj);
        if (index > -1) {
            lod_objects.remove(index);
            lod_object_count--;
            break;
        }
    }

    if (use_multithreading) {
        lod_objects_semaphore->post();
    }
}


void LODManager::update_lod_multipliers_from_settings() {
    debug_level_print(1, "Loading LOD multipliers from the project settings.\n");
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
    debug_level_print(1, "Setting the flag to update LOD multipliers in the LOD thread to true.\n");

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

void LODManager::update_fov() {
    debug_level_print(1, "Updating the LOD manager camera FOV.\n");

    if (!camera) {
        // No camera to get FOV from
        return;
    }
    fov = camera->get_fov();

    debug_level_print(1, "Setting the flag to update FOV in LOD objects to true.\n");
    // Enable the flag to update FOV in the thread loop
    // Make sure we're not partway through the loop
    if (use_multithreading) {
        lod_objects_semaphore->wait();
    }
    update_fovs_flag = true;
    // If we're updating the FOV, the AABB screen percentage will need to be updated, too.
    update_AABBs_flag = true;
    if (use_multithreading) {
        lod_objects_semaphore->post();
    }
}

void LODManager::update_lod_AABBs() {
    debug_level_print(1, "Setting the flag to update LOD AABBs in the LOD thread to true.\n");
    // Enable the flag to update FOV in the thread loop
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
    debug_level_print(1, "Setting up the camera info in the LOD manager.\n");
    if (get_viewport()->get_camera()) {
        camera = get_viewport()->get_camera();
        update_fov();
        return true;
    }
    return false;
}

bool LODManager::set_camera(Node* given_node) {
    debug_level_print(1, "Setting the camera used by the LOD Manager.\n");
    Camera* camera_node;
    camera_node = Object::cast_to<Camera>(given_node);
    if (camera_node) {
        camera = camera_node;
        return true;
    } else {
        printf("Camera provided in setCamera of LODManager was not valid.\n");
        return false;
    }
}

void LODManager::debug_level_print(int min_debug_level, const char* message) {
    if (debug_level >= min_debug_level)
    {
        printf(message);
    }
}