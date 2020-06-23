#include "lod.h"

using namespace godot;

void LightLOD::_register_methods() {
    register_method("_process", &LightLOD::_process);
    register_method("_ready", &LightLOD::_ready);
    register_property<LightLOD, float>("shadowDist", &LightLOD::shadowDist, 50.0f);
    register_property<LightLOD, float>("hideDist", &LightLOD::hideDist, 150.0f); // -1 to never unload
    register_property<LightLOD, float>("fadeRange", &LightLOD::fadeRange, 5.0f); 

    register_property<LightLOD, float>("fadeSpeed", &LightLOD::fadeSpeed, 1.0f);

    register_property<LightLOD, float>("tickSpeed", &LightLOD::tickSpeed, 0.5f);
}

LightLOD::LightLOD() {
}

LightLOD::~LightLOD() {
    // add your cleanup here
}

void LightLOD::_init() {
    // Save a white colour so we're not constantly making Color objects + better readability
    white = Color(1.0, 1.0, 1.0, 1.0);
}

void LightLOD::_ready() {
    // Find camera and save original light and shadow colours
    camera = get_viewport()->get_camera();

    lightBaseEnergy = get_param(PARAM_ENERGY);
    lightTargetEnergy = lightBaseEnergy;
    shadowBaseColor = get_shadow_color();
    shadowTargetColor = shadowBaseColor;
}

void LightLOD::_process(float delta) {
    timePassed += delta;

    // Fade light
    fadeLight(delta);

    // Fade shadows
    fadeShadow(delta);

    // Don't have to check every frame
    if (timePassed < tickSpeed) {
        return;
    } else {
        timePassed = 0;
    }

    // Get the distance from the node to the camera
    float distance = camera->get_global_transform().origin.distance_to(get_global_transform().origin);

    // Get our target values for light and shadow
    // (max - current) / (max - min) will give us the ratio of where we want to set our values
    lightTargetEnergy = CLAMP((hideDist - distance) / (hideDist - (hideDist - fadeRange)), 0.0f, 1.0f);

    // Feels like creating a new Color every time might not be a great idea
    float shadowRatio = CLAMP((shadowDist - distance) / (shadowDist - (shadowDist - fadeRange)), 0.0f, 1.0f);
    shadowTargetColor = Color(1.0f * shadowRatio, 1.0f * shadowRatio, 1.0f * shadowRatio, 1.0f);

}

void LightLOD::fadeLight(float delta) {
    real_t lightEnergy = get_param(PARAM_ENERGY);
    if (lightEnergy != lightTargetEnergy) {
        // If we're at 0 and not at target, then we're about to turn the light on... so show it in advance
        if (lightEnergy == 0.0) {
            show();
        }

        /// Lerp
        // If the light energy wasn't 1, then the fade might be slower or faster
        // We don't want the speed to be dependent on light energy, so multiply speed by base
        lightEnergy = lightEnergy + (lightTargetEnergy - lightEnergy) * delta * (fadeSpeed * lightBaseEnergy);
        set_param(PARAM_ENERGY, lightEnergy);

        // If after adjusting light we are at zero, it means we just turned it off... so hide light
        if (lightEnergy == 0.0) {
            hide();
        }
    }
}

void LightLOD::fadeShadow(float delta) {
    Color shadowColor = get_shadow_color();
    if (shadowColor != shadowTargetColor) {
        // If we have to move from blank shadows, it means we're turning them back on
        if (shadowColor == white) {
            set_shadow(true);
        }

        // Lerp
        shadowColor = shadowColor.linear_interpolate(shadowTargetColor, fadeSpeed * delta);
        set_shadow_color(shadowColor);

        // If after adjusting, our shadow is blank, then turn shadows off
        if (shadowColor == white) {
            set_shadow(false);
        }
    }
}