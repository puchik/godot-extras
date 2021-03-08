#include "lod.h"
#include <string>
#include <ctime>
#define MAX_ARRAY_SIZE 30000

using namespace godot;

void LODManager::_register_methods() {
    register_property<LODManager, bool>("useMultithreading", &LODManager::useMultithreading, true);
    register_property<LODManager, int>("objectsPerFrame", &LODManager::objectsPerFrame, 10000);
    // Exposed methods
    register_method("stopLoop", &LODManager::stopLoop);
    register_method("updateLodMultipliersFromSettings", &LODManager::updateLodMultipliersFromSettings);
    register_method("updateLodMultipliersInObjects", &LODManager::updateLodMultipliersInObjects);
    register_method("updateLodAABBs", &LODManager::updateLodAABBs);
    register_method("updateFOV", &LODManager::updateFOV);

    register_method("_ready", &LODManager::_ready);
    register_method("_process", &LODManager::_process);
    register_method("_exit_tree", &LODManager::_exit_tree);

    // Thread function
    register_method("mainLoop", &LODManager::mainLoop);

    register_property<LODManager, float>("FOV", &LODManager::FOV, 70.0f);
    register_property<LODManager, bool>("updateFOVEveryLoop", &LODManager::updateFOVEveryLoop, false);
    register_property<LODManager, bool>("updateAABBEveryLoop", &LODManager::updateAABBEveryLoop, false);
    register_property<LODManager, float>("tickSpeed", &LODManager::tickSpeed, 0.2f);

    // Called by LOD objects with themselves as the argument when entering the tree
    register_method("addObject", &LODManager::addObject);
    register_method("removeObject", &LODManager::removeObject);

    // Multipliers, should be visible to other nodes so they may update.
    register_property<LODManager, bool>("updateMultsEveryLoop", &LODManager::updateMultsEveryLoop, false);
    // However, if modifying directly, you must call updateLodMultipliersInObjects to take effect in objects
    register_property<LODManager, float>("globalDistMult", &LODManager::globalDistMult, 1.0f);
    register_property<LODManager, float>("lod1DistMult", &LODManager::lod1DistMult, 1.0f);
    register_property<LODManager, float>("lod2DistMult", &LODManager::lod2DistMult, 1.0f);
    register_property<LODManager, float>("lod3DistMult", &LODManager::lod3DistMult, 1.0f);
    register_property<LODManager, float>("hideDistMult", &LODManager::hideDistMult, 1.0f);
    register_property<LODManager, float>("unloadDistMult", &LODManager::unloadDistMult, 1.0f);
    register_property<LODManager, float>("shadowDistMult", &LODManager::shadowDistMult, 1.0f);
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
    if (useMultithreading) {
        stopLoop();
    }
}


void LODManager::_ready() {
    camera = get_viewport()->get_camera();

    // Control threading with semaphore
    if (useMultithreading) {
        // Do it before the update LOD mults and FOV call because it's used there
        LODObjectsSemaphore = (Ref<Semaphore>) Semaphore::_new();
        LODObjectsSemaphore->post();
    }

    updateFOV();

    projectSettings = ProjectSettings::get_singleton();
    updateLodMultipliersFromSettings();
    // We'll be checking the FPS
    perf = Performance::get_singleton();
    last_run = clock();

    // Start main loop thread
    if (useMultithreading) {
        // Start thread
        LODLoopThread = (Ref<Thread>) Thread::_new();
        LODLoopThread->start(this, "mainLoop");
    }
}

void LODManager::_process(float delta) {
    if (!useMultithreading && !managerRemoved) {
        LODFunction();
    }
}

void LODManager::LODFunction() {
        currentFPS = perf->get_monitor(0); // gets the FPS
        // A bit hacky, but we'll check if the main thread is frozen/doing something heavy.
        // We don't need LOD when nothing is really moving nor do we want to potentially overload
        // the main thread with deferred calls.
        if (currentFPS < 7.0f) {
            return;
        }

        // Could use OS.delay_msec, but loop may take a long time so keep track of time manually instead
        if ((double)(clock() - last_run) / CLOCKS_PER_SEC < tickSpeed) {
            return;
        }

        if (useMultithreading) {
            LODObjectsSemaphore->wait();
        }

        last_run = clock();

        if (camera->is_inside_tree()) {
            cameraLoc = camera->get_global_transform().origin;        
        }

        // --------- TODO: Make a dynamic number of objects checked per frame --------------------
        // Probably do a calculation here, use FPS
        // Get next "chunk" of objects to check if not multithreaded
        int nextLoopEndIndex;
        if (!useMultithreading) {
            nextLoopEndIndex = currentLoopIndex + 8000;
        }

        // Go through all array lists
        for (int i = 0; i < LODObjectArrays.size(); i++) {
            if (!useMultithreading && i == 0) {
                i = CLAMP((int)floor((double)currentLoopIndex / (double)MAX_ARRAY_SIZE), 0, LODObjectArrays.size() - 1);
            }
            if (!useMultithreading && nextLoopEndIndex < i * MAX_ARRAY_SIZE) {
                break;
            }

            Array LODObjects = LODObjectArrays[i];
            for (int j = 0; j < LODObjects.size(); j++) {
                if (!useMultithreading && j == 0) {
                    j = CLAMP(currentLoopIndex - i * MAX_ARRAY_SIZE, 0, LODObjects.size() - 1);
                }
                if (!useMultithreading && nextLoopEndIndex < j + i * MAX_ARRAY_SIZE) {
                    break;
                }

                // LOD thread might be still going through the list when LOD exits tree, check
                if (managerRemoved) {
                    break;
                }

                // Make sure object exists *and* is in the tree.
                // Exiting without stopping the LOD thread causes errors otherwise.
                if (LODObjects[j] && Object::cast_to<Node>(LODObjects[j])->is_inside_tree()) {
                    Node* LODObjectNode = Object::cast_to<Node>(LODObjects[j]);
                    // Update multiplier if needed
                    if (updateMultsFlag && LODObjectNode->has_method("updateLodMultipliersFromManager")) {
                        // Tell it to update. It will fetch the distances from our public values
                        LODObjectNode->call("updateLodMultipliersFromManager");
                    }
                    // Update FOV if needed
                    if (updateFOVsFlag && LODObjectNode->get("FOV")) {
                        LODObjectNode->set("FOV", FOV);
                    }
                    // Update AABB if needed
                    if (updateAABBsFlag && LODObjectNode->get("useScreenPercentage")) {
                        LODObjectNode->call("updateLodAABB");
                    }
                    // Pass camera location and do calculations on LOD object
                    if (LODObjectNode->has_method("processData")) {
                        LODObjectNode->call("processData", cameraLoc);
                    }
                }
            }
            if (managerRemoved) {
                break;
            }
        }
        if (useMultithreading) {
            LODObjectsSemaphore->post();
        }

        if (!useMultithreading) {
            if (nextLoopEndIndex >= LODObjectCount) {
                currentLoopIndex = 0;
            } else {
                currentLoopIndex = nextLoopEndIndex;
            }
        }

        if (updateMultsFlag) {
            updateMultsFlag = false;
        }
        if (updateFOVsFlag) {
            updateFOVsFlag = false;
        }
        if (updateAABBsFlag) {
            updateAABBsFlag = false;
        }

        // If necessary, update multipliers and/or FOV
        if (updateMultsEveryLoop) {
            updateLodMultipliersFromSettings();
        }
        if (updateFOVEveryLoop) {
            updateFOV();
        }
        if (updateAABBEveryLoop) {
            updateLodAABBs();
        }

        if (managerRemoved) {
            return;
        }
}

void LODManager::mainLoop() {
    while (!managerRemoved) {
        LODFunction();
    }
}

void LODManager::stopLoop() {
    if (managerRemoved) {
        return;
    } else {
        managerRemoved = true;
    }

    if (LODLoopThread->is_active()) {
        LODLoopThread->wait_to_finish();
    }

    LODObjectsSemaphore->free();
}

void LODManager::addObject(Node* obj) {
    // Go through array list to find one that has space
    if (useMultithreading) {
        LODObjectsSemaphore->wait();
    }

    bool added = false; // Keep track in case we need to add a new list
    for (int i = 0; i < LODObjectArrays.size(); i++) {
        Array LODObjects = LODObjectArrays[i];
        if (LODObjects.size() < MAX_ARRAY_SIZE) {
            LODObjects.push_back(obj);
            LODObjectCount++;
            added = true;
            break;
        }
    }

    if (!added) {
        Array newLODObjectArray;
        newLODObjectArray.push_back(obj);
        LODObjectArrays.push_back(newLODObjectArray);
    }

    if (useMultithreading) {
        LODObjectsSemaphore->post();
    }
}

void LODManager::removeObject(Node* obj) {
    // If we're closing/have closed the LOD manager, don't bother doing expensive finds and removes.
    // It might take a while and is unnecessary.
    if (managerRemoved) {
        // Also array should be freed by Godot
        return;
    }

    if (useMultithreading) {
        LODObjectsSemaphore->wait();
    }

    // Find which array has this object
    for (int i = 0; i < LODObjectArrays.size(); i++) {
        Array LODObjects = LODObjectArrays[i];
        int index = LODObjects.find(obj);
        if (index > -1) {
            LODObjects.remove(index);
            LODObjectCount--;
            break;
        }
    }

    if (useMultithreading) {
        LODObjectsSemaphore->post();
    }
}


void LODManager::updateLodMultipliersFromSettings() {
    // Load multipliers from project settings
    globalDistMult = (float)projectSettings->get_setting("rendering/quality/lod/global_multiplier");
    lod1DistMult = (float)projectSettings->get_setting("rendering/quality/lod/lod1_multiplier");
    lod2DistMult = (float)projectSettings->get_setting("rendering/quality/lod/lod2_multiplier");
    lod3DistMult = (float)projectSettings->get_setting("rendering/quality/lod/lod3_multiplier");
    hideDistMult = (float)projectSettings->get_setting("rendering/quality/lod/hide_multiplier");
    unloadDistMult = (float)projectSettings->get_setting("rendering/quality/lod/unload_multiplier");
    shadowDistMult = (float)projectSettings->get_setting("rendering/quality/lod/shadow_disable_multiplier");

    // Update LOD objects' multipliers
    updateLodMultipliersInObjects();
}

void LODManager::updateLodMultipliersInObjects() {
    // Enable the flag to update LOD multipliers in the thread loop
    // Make sure we're not partway through the loop
    if (useMultithreading) {
        LODObjectsSemaphore->wait();
    }
    updateMultsFlag = true;
    if (useMultithreading) {
        LODObjectsSemaphore->post();
    }
}

void LODManager::updateFOV() {
    FOV = camera->get_fov();
    // Enable the flag to update FOV in the thread loop
    // Make sure we're not partway through the loop
    if (useMultithreading) {
        LODObjectsSemaphore->wait();
    }
    updateFOVsFlag = true;
    if (useMultithreading) {
        LODObjectsSemaphore->post();
    }
}

void LODManager::updateLodAABBs() {
    // Enable the flag to update FOV in the thread loop
    // Make sure we're not partway through the loop
    if (useMultithreading) {
        LODObjectsSemaphore->wait();
    }
    updateAABBsFlag = true;
    if (useMultithreading) {
        LODObjectsSemaphore->post();
    }
}