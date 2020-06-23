#include "lod.h"
#include <cmath>

using namespace godot;

void MultiMeshLOD::_register_methods() {
    register_method("_process", &MultiMeshLOD::_process);
    register_method("_ready", &MultiMeshLOD::_ready);
    register_property<MultiMeshLOD, float>("minDist", &MultiMeshLOD::minDist, 5.0f); 
    register_property<MultiMeshLOD, float>("maxDist", &MultiMeshLOD::maxDist, 80.0f); 

    register_property<MultiMeshLOD, int64_t>("minCount", &MultiMeshLOD::minCount, 0); 
    register_property<MultiMeshLOD, int64_t>("maxCount", &MultiMeshLOD::maxCount, -1); 

    register_property<MultiMeshLOD, float>("fadeSpeed", &MultiMeshLOD::fadeSpeed, 1.0f);
    register_property<MultiMeshLOD, float>("fadeExponent", &MultiMeshLOD::fadeExponent, 1.0f);

    register_property<MultiMeshLOD, float>("tickSpeed", &MultiMeshLOD::tickSpeed, 0.1f);
}

MultiMeshLOD::MultiMeshLOD() {
}

MultiMeshLOD::~MultiMeshLOD() {
    // add your cleanup here
}

void MultiMeshLOD::_init() {
}

void MultiMeshLOD::_ready() {
    camera = get_viewport()->get_camera();

    multiMesh = *get_multimesh();
    if (maxCount < 0) {
        maxCount = multiMesh->get_instance_count();
    }
    targetCount = maxCount;
}

void MultiMeshLOD::_process(float delta) {
    timePassed += delta;

    // Lerp visible instance count if needed
    int64_t instanceCount = multiMesh->get_visible_instance_count();
    if (instanceCount != targetCount) {
        // -------- Actually, I don't think there's much of a benefit of disabling the object if none are visible?
        // -------- Maybe just leave it for now.
        // // If there were 0 instances, we need to re-enable the multimesh
        // if (probeEnergy == 0.0) {
        //     show();
        // }

        /// Lerp
        instanceCount = int64_t(floor(instanceCount + (targetCount - instanceCount) * delta * fadeSpeed));
        multiMesh->set_visible_instance_count(instanceCount);

        // ------- See comment at beginning of if statement
        // // If we just set it to 0, it means we turned it off
        // if (instanceCount == 0) {
        //     hide();
        // }
    }

    // Don't have to check every frame
    if (timePassed < tickSpeed) {
        return;
    } else {
        timePassed = 0;
    }

    // Get the distance from the node to the camera
    real_t distance = camera->get_global_transform().origin.distance_to(get_global_transform().origin);

    // Get our target value
    // ((max - current) / (max - min))^fadeExponent will give us the ratio of where we want to set our values
    // then we multiple by the range of instances we can have
    targetCount = int64_t(floor(pow(CLAMP((maxDist - distance) / (maxDist  - minDist), 0.0, 1.0), fadeExponent) * (maxCount - minCount)));
}