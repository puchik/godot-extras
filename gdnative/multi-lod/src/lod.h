#ifndef MULTI_LOD_H
#define MULTI_LOD_H

#ifndef CLAMP
#define CLAMP(m_a, m_min, m_max) (((m_a) < (m_min)) ? (m_min) : (((m_a) > (m_max)) ? m_max : m_a))
#endif

#include <Godot.hpp>
#include <ProjectSettings.hpp>
#include <SceneTree.hpp>
#include <AABB.hpp>
#include <Thread.hpp>
#include <Ref.hpp>
#include <OS.hpp>
#include <Mutex.hpp>
#include <Semaphore.hpp>
#include <Performance.hpp>

#include <Viewport.hpp>
#include <Camera.hpp>
#include <Transform.hpp>
#include <Spatial.hpp>
#include <Vector3.hpp>
#include <ctime>

// Objects
#include <NodePath.hpp>
#include <Node.hpp>
#include <VisualInstance.hpp>

// Lights
#include <Color.hpp>
#include <Light.hpp>

// GIProbe
#include <GIProbe.hpp>

// MultiMesh
#include <MultiMeshInstance.hpp>
#include <MultiMesh.hpp>

namespace godot {

//// Manager of all LOD objects ----------------------------------------------------------------------------------
class LODManager : public Node {
    GODOT_CLASS(LODManager, Node)

private:
    float tickSpeed = 0.2f;

    // Array of arrays of LOD objects
    Array LODObjectArrays;

    Camera* camera = NULL;
    float FOV; // Need FOV for getting screen percentages

    ProjectSettings* projectSettings;

    // Used to stop the management thread
    bool managerRemoved = false;

    // Flag to let LOD objects know to update their multipliers (if necessary)
    bool updateMultsFlag = false;
    // Flag to let LOD objects know to update their FOVs (AABB calculation for screen size based LOD)
    bool updateFOVsFlag = false;
    // Flag to let LOD objects know to update their AABB (use after settinf FOV if necessary)
    bool updateAABBsFlag = false;

    // Debug to console detail level. 0 = off, 1 = print the steps in each thread loop, 2 = print the object names as we go through them.
    int debugLevel = 0;

    // Threading
    Ref<Thread> LODLoopThread;
    Ref<Semaphore> LODObjectsSemaphore;
    Ref<Semaphore> managerRemovedSemaphore;

    void mainLoop();
    void LODFunction();

    // Variables used for the LOD function
    clock_t last_run;
    Vector3 cameraLoc;
    Performance* perf;
    float currentFPS = 0.0f;

public:
    static void _register_methods();

    LODManager();
    ~LODManager();

    void _init(); // our initializer called by Godot

    void _ready();
    void _process(float delta);
    void _exit_tree();

    void addObject(Node* obj);
    void removeObject(Node* obj);
    
    // Reading project settings is pretty expensive... set up to only update manually by default
    // but we have the option to update every loop, too
    bool updateMultsEveryLoop = false;
    void updateLodMultipliersFromSettings();
    void updateLodMultipliersInObjects(); // Tell LOD objects it's time to update
    bool setUpCamera();
    bool setCamera(Node* givenNode);
    bool updateFOVEveryLoop = false;
    void updateFOV(); // Need FOV for getting screen percentages
    bool updateAABBEveryLoop = false;
    void updateLodAABBs(); // Need AABB for getting screen percentages
    void stopLoop(); // Stops the main thread
    void debugLevelPrint(int minDebugLevel, const char* text); // Use this 

    // Distance multipliers, available to access by other LOD objects
    float globalDistMult = 1.0f;
    float lod1DistMult = 1.0f;
    float lod2DistMult = 1.0f;
    float lod3DistMult = 1.0f;
    float hideDistMult = 1.0f;
    float unloadDistMult = 1.0f;
    float shadowDistMult = 1.0f;

    int LODObjectCount = 0;
    // Whether to use multithreading or not
    bool useMultithreading = true;
    // If we're not using a thread, looping through all objects at once can cause stutters.
    // So let's break the objects looped into separate frames
    int currentLoopIndex = 0;
    int objectsPerFrame = 10000;
};

//// Object based LOD ----------------------------------------------------------------------------------
class LOD : public VisualInstance {
    GODOT_CLASS(LOD, VisualInstance)

private:
    float tickSpeed = 0.2f;
    bool enabled = true; // Switch to false if we want to turn off LOD functionality
    bool registered = false; // Whether the manager knows we exist
    bool attemptRegister(bool state); // Register or unregister the object. Returns success or fail
    // Dirty bit. Indicates if the LOD manager has had any contact with this object
    bool interactedWithManager = false;

    // Distance by metres
    // These will be set by the ratios below if useScreenPercentage is true
    float lod1dist = 7.0f; // put any of these to -1 if you don't have a lod or don't want to unload etc
    float lod2dist = 12.0f;
    float lod3dist = 30.0f;
    float hideDist = 100.0f;
    float unloadDist = -1.0f;

    // Distance by screen percentage
    // Use a conservative/worst-case method for getting the size of the object
    // relative to the screen (largest AABB axis on both viewport axes)
    bool useScreenPercentage = true;
    float lod1ratio = 25.0f;
    float lod2ratio = 10.0f;
    float lod3ratio = 5.5f;
    float hideRatio = 1.0f;
    float unloadRatio = -1.0f;

    bool disableProcessing = false;

    // Prevents calling the processData function too fast from the main loop
    bool alreadyRunning = false;

    // Keep track of last state to avoid unnecessary show/hide/process toggle calls
    int lastState = -1;

    NodePath lod0path;
    NodePath lod1path;
    NodePath lod2path;
    NodePath lod3path;

    Spatial* lod0 = NULL; // LOD0 MUST exist otherwise screen percentage breaks
    Spatial* lod1 = NULL; // Plus it doesn't make sense for it not to exist
    Spatial* lod2 = NULL;
    Spatial* lod3 = NULL;

    float FOV; // Need FOV for getting screen percentages

    // Let's use the AABB centre for the centre of the object instead of
    // the parent's centre (if applicable). Instead of constantly 
    // accessing AABB, just store an offset.
    Vector3 transformOffsetAABB;

    ProjectSettings* projectSettings;
    bool affectedByDistanceMultipliers = true;
    float globalDistMult = 1.0f;
    float lod1DistMult = 1.0f;
    float lod2DistMult = 1.0f;
    float lod3DistMult = 1.0f;
    float hideDistMult = 1.0f;
    float unloadDistMult = 1.0f;

    void setNodeProcessing(Spatial* node, bool state);

public:
    static void _register_methods();

    LOD();
    ~LOD();

    void _init(); // our initializer called by Godot

    void _ready();
    void _process(float delta);
    void _exit_tree();
    void processData(Vector3 cameraLoc);

    void updateLodMultipliersFromManager(); // Reading project settings is pretty expensive... only update manually
    void updateLodAABB(); // Update AABB only if necessary
    void updateFOV(); // Need FOV for getting screen percentages
};

//// Light detail (shadow and light itself) LOD ------------------------------------------------------------
class LightLOD : public Light {
    GODOT_CLASS(LightLOD, Light)

private:
    float tickSpeed = 0.5f;
    bool enabled = true; // Switch to false if we want to turn off LOD functionality
    bool registered = false; // Whether the manager knows we exist
    bool attemptRegister(bool state); // Register or unregister the object. Returns success or fail
    // Dirty bit. Indicates if the LOD manager has had any contact with this object
    bool interactedWithManager = false;

    float shadowDist = 20.0f;
    float hideDist = 80.0f; // -1 to never hide
    float fadeRange = 5.0f; // For ex, the intensity of the shadow will adjust from 0 to 1 between [shadowDist - fadeRange, shadowDist]

    // Distance by screen percentage
    // Use a conservative/worst-case method for getting the size of the object
    // relative to the screen (largest AABB axis on both viewport axes)
    bool useScreenPercentage = true;
    float shadowRatio = 6.0f;
    float hideRatio = 2.0f;

    float fadeSpeed = 2.0f;

    real_t lightBaseEnergy;

    real_t lightTargetEnergy;
    Color shadowTargetColor;

    float FOV; // Need FOV for getting screen percentages

    void fadeLight(float delta);
    void fadeShadow(float delta);

    ProjectSettings* projectSettings;
    bool affectedByDistanceMultipliers = true;
    float globalDistMult = 1.0f;
    float shadowDistMult = 1.0f;

public:
    static void _register_methods();

    LightLOD();
    ~LightLOD();

    void _init(); // our initializer called by Godot

    void _ready();
    void _process(float delta);
    void _exit_tree();
    void processData(Vector3 cameraLoc);

    void updateLodMultipliersFromManager(); // Reading project settings is pretty expensive... we have the option to
    void updateLodAABB(); // Update AABB only if necessary
    void updateFOV(); // Need FOV for getting screen percentages
};

//// GIProbe LOD -------------------------------------------------------------------
class GIProbeLOD : public GIProbe  {
    GODOT_CLASS(GIProbeLOD, GIProbe)

private:
    float tickSpeed = 0.1f;
    bool enabled = true; // Switch to false if we want to turn off LOD functionality
    bool registered = false; // Whether the manager knows we exist
    bool attemptRegister(bool state); // Register or unregister the object. Returns success or fail
    // Dirty bit. Indicates if the LOD manager has had any contact with this object
    bool interactedWithManager = false;

    float hideDist = 80.0f;
    float fadeRange = 5.0f; // The energy of the probe will adjust from 0 to 1 between [unloadDist - fadeRange, unloadDist]

    // Distance by screen percentage
    // Use a conservative/worst-case method for getting the size of the object
    // relative to the screen (largest AABB axis on both viewport axes)
    bool useScreenPercentage = true;
    float hideRatio = 2.0f;

    float fadeSpeed = 1.0f;

    real_t probeTargetEnergy;

    real_t probeBaseEnergy;

    float FOV; // Need FOV for getting screen percentages

    ProjectSettings* projectSettings;
    bool affectedByDistanceMultipliers = true;
    float globalDistMult = 1.0f;

public:
    static void _register_methods();

    GIProbeLOD();
    ~GIProbeLOD();

    void _init(); // our initializer called by Godot

    void _ready();
    void _process(float delta);
    void _exit_tree();
    void processData(Vector3 cameraLoc);

    void updateLodMultipliersFromManager(); // Reading project settings is pretty expensive... only update manually
    void updateLodAABB(); // Update AABB only if necessary
    void updateFOV(); // Need FOV for getting screen percentages
};

//// MultiMeshInstance LOD -------------------------------------------------------------------
class MultiMeshLOD : public MultiMeshInstance  {
    GODOT_CLASS(MultiMeshLOD, MultiMeshInstance)

private:
    float tickSpeed = 0.5f;
    bool enabled = true; // Switch to false if we want to turn off LOD functionality
    bool registered = false; // Whether the manager knows we exist
    bool attemptRegister(bool state); // Register or unregister the object. Returns success or fail
    // Dirty bit. Indicates if the LOD manager has had any contact with this object
    bool interactedWithManager = false;

    float minDist = 5.0f; // At this distance, or below, we see max number of multimesh count
    float maxDist = 80.0f; // At this distance, or above, we see min (or none) number of multimesh count

    // Distance by screen percentage
    // Use a conservative/worst-case method for getting the size of the object
    // relative to the screen (largest AABB axis on both viewport axes)
    bool useScreenPercentage = true;
    float minRatio = 2.0f;
    float maxRatio = 5.0f;

    int64_t minCount = 0;
    int64_t maxCount = -1; // -1 means the number generated in the multimesh we find

    int64_t targetCount = 50;

    float fadeSpeed = 1.0f;
    float fadeExponent = 1.0f;  // Exponent of the [0, 1] curve that reduces count. At 1, we fade linearly.

    float FOV; // Need FOV for getting screen percentages

    ProjectSettings* projectSettings;
    bool affectedByDistanceMultipliers = true;
    float globalDistMult = 1.0f;

    MultiMesh* multiMesh;

public:
    static void _register_methods();

    MultiMeshLOD();
    ~MultiMeshLOD();

    void _init(); // our initializer called by Godot

    void _ready();
    void _process(float delta);
    void _exit_tree();
    void processData(Vector3 cameraLoc);

    void updateLodMultipliersFromManager(); // Reading project settings is pretty expensive... only update manually
    void updateLodAABB(); // Update AABB only if necessary
    void updateFOV(); // Need FOV for getting screen percentages
};

}

#endif