#ifndef GDEXAMPLE_H
#define GDEXAMPLE_H

#ifndef CLAMP
#define CLAMP(m_a, m_min, m_max) (((m_a) < (m_min)) ? (m_min) : (((m_a) > (m_max)) ? m_max : m_a))
#endif

#include <Godot.hpp>
#include <Viewport.hpp>
#include <Camera.hpp>
#include <Transform.hpp>

// Objects
#include <NodePath.hpp>
#include <Node.hpp>
#include <Spatial.hpp>

// Lights
#include <Color.hpp>
#include <Light.hpp>

// GIProbe
#include <GIProbe.hpp>

// MultiMesh
#include <MultiMeshInstance.hpp>
#include <MultiMesh.hpp>

namespace godot {

//// Object based LOD ----------------------------------------------------------------------------------
class LOD : public Spatial {
    GODOT_CLASS(LOD, Node)

private:
    float tickSpeed = 0.2f;
    float timePassed = 0.0f;

    float lod1dist = 30.0f; // put any of these to -1 if you don't have a lod or don't want to unload etc
    float lod2dist = 60.0f;
    float hideDist = 130.0f;
    float unloadDist = -1.0f;

    bool disableProcessing = true;

    NodePath lod0path;
    NodePath lod1path;
    NodePath lod2path;

    Spatial* lod0 = NULL;
    Spatial* lod1 = NULL;
    Spatial* lod2 = NULL;

    Camera* camera;

public:
    static void _register_methods();

    LOD();
    ~LOD();

    void _init(); // our initializer called by Godot

    void _ready();
    void _process(float delta);

    void setNodeProcessing(Spatial* node, bool state);
};

//// Light detail (shadow and light itself) LOD ------------------------------------------------------------
class LightLOD : public Light {
    GODOT_CLASS(LightLOD, Light)

private:
    float tickSpeed = 0.5f;
    float timePassed = 0.0f;

    // Don't set these to stupid values because I won't be checking them
    float shadowDist = 20.0f;
    float hideDist = 80.0f; // -1 to never hide
    float fadeRange = 5.0f; // For ex, the intensity of the shadow will adjust from 0 to 1 between [shadowDist - fadeRange, shadowDist]

    float fadeSpeed = 1.0f;
    Color white; // So we don't have to keep making a new Color... and for readability

    real_t lightBaseEnergy;
    Color shadowBaseColor;

    real_t lightTargetEnergy;
    Color shadowTargetColor;

    Camera* camera;

    void fadeLight(float delta);
    void fadeShadow(float delta);

public:
    static void _register_methods();

    LightLOD();
    ~LightLOD();

    void _init(); // our initializer called by Godot

    void _ready();
    void _process(float delta);
};

//// GIProbe LOD -------------------------------------------------------------------
class GIProbeLOD : public GIProbe  {
    GODOT_CLASS(GIProbeLOD, GIProbe)

private:
    float tickSpeed = 0.1f;
    float timePassed = 0.0f;

    // Don't set these to stupid values because I won't be checking them
    float hideDist = 80.0f;
    float fadeRange = 5.0f; // The energy of the probe will adjust from 0 to 1 between [unloadDist - fadeRange, unloadDist]

    float fadeSpeed = 1.0f;

    real_t probeTargetEnergy;

    real_t probeBaseEnergy;

    Camera* camera;

public:
    static void _register_methods();

    GIProbeLOD();
    ~GIProbeLOD();

    void _init(); // our initializer called by Godot

    void _ready();
    void _process(float delta);
};

//// MultiMeshInstance LOD -------------------------------------------------------------------
class MultiMeshLOD : public MultiMeshInstance  {
    GODOT_CLASS(MultiMeshLOD, MultiMeshInstance)

private:
    float tickSpeed = 0.5f;
    float timePassed = 0.0f;

    // Don't set these to stupid values because I won't be checking them
    float minDist = 5.0f; // At this distance, or below, we see max number of multimesh count
    float maxDist = 80.0f; // At this distance, or above, we see min (or none) number of multimesh count

    int64_t minCount = 0;
    int64_t maxCount = -1; // -1 means the number generated in the multimesh we find

    int64_t targetCount = 50;

    float fadeSpeed = 1.0f;
    float fadeExponent = 1.0f;  // Exponent of the [0, 1] curve that reduces count. At 1, we fade linearly.

    Camera* camera;

    MultiMesh* multiMesh;

public:
    static void _register_methods();

    MultiMeshLOD();
    ~MultiMeshLOD();

    void _init(); // our initializer called by Godot

    void _ready();
    void _process(float delta);
};

}

#endif