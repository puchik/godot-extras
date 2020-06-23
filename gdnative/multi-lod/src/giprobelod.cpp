#include "lod.h"

using namespace godot;

void GIProbeLOD::_register_methods() {
    register_method("_process", &GIProbeLOD::_process);
    register_method("_ready", &GIProbeLOD::_ready);
    register_property<GIProbeLOD, float>("hideDist", &GIProbeLOD::hideDist, 80.0f); 
    register_property<GIProbeLOD, float>("fadeRange", &GIProbeLOD::fadeRange, 5.0f); 

    register_property<GIProbeLOD, float>("fadeSpeed", &GIProbeLOD::fadeSpeed, 1.0f);

    register_property<GIProbeLOD, float>("tickSpeed", &GIProbeLOD::tickSpeed, 0.5f);
}

GIProbeLOD::GIProbeLOD() {
}

GIProbeLOD::~GIProbeLOD() {
    // add your cleanup here
}

void GIProbeLOD::_init() {
}

void GIProbeLOD::_ready() {
    // Find camera and save original probe energy
    camera = get_viewport()->get_camera();

    probeBaseEnergy = get_energy();
    probeTargetEnergy = probeBaseEnergy;
}

void GIProbeLOD::_process(float delta) {
    timePassed += delta;

    // Fade GIProbe if needed
    real_t probeEnergy = get_energy();
    if (probeEnergy != probeTargetEnergy) {
        // If we're just starting to enable the probe, make sure it's on
        if (probeEnergy == 0.0) {
            show();
        }

        /// Lerp
        // If the probe energy wasn't 1, then the fade might be slower or faster
        // We don't want the speed to be dependent on energy, so multiply speed by base
        probeEnergy = probeEnergy + (probeTargetEnergy - probeEnergy) * delta * (fadeSpeed * probeBaseEnergy);
        set_energy(probeEnergy);

        // If we just set it to 0, it means we turned it off
        if (probeEnergy == 0.0) {
            hide();
        }
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
    // (max - current) / (max - min) will give us the ratio of where we want to set our values
    probeTargetEnergy = CLAMP((hideDist - distance) / (hideDist - (hideDist - fadeRange)), 0.0f, 1.0f);
}