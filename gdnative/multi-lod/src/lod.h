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
    float tick_speed = 0.2f;

    // Array of arrays of LOD objects
    Array lod_object_arrays;

    Camera* camera = NULL;
    float fov; // Need FOV for getting screen percentages

    ProjectSettings* project_settings;

    // Used to stop the management thread
    bool manager_removed = false;

    // Flag to let LOD objects know to update their multipliers (if necessary)
    bool update_multipliers_flag = false;
    // Flag to let LOD objects know to update their FOVs (AABB calculation for screen size based LOD)
    bool update_fovs_flag = false;
    // Flag to let LOD objects know to update their AABB (use after settinf FOV if necessary)
    bool update_AABBs_flag = false;

    // Debug to console detail level. 0 = off, 1 = print the steps in each thread loop, 2 = print the object names as we go through them.
    int debug_level = 0;

    // Threading
    Ref<Thread> lod_loop_thread;
    Ref<Semaphore> lod_objects_semaphore;
    Ref<Semaphore> manager_removed_semaphore;

    void main_loop();
    void lod_function();

    // Variables used for the LOD function
    clock_t last_run;
    Vector3 camera_location;
    Performance* perf;
    float current_fps = 0.0f;

public:
    static void _register_methods();

    LODManager();
    ~LODManager();

    void _init(); // our initializer called by Godot

    void _ready();
    void _process(float delta);
    void _exit_tree();

    void add_object(Node* obj);
    void remove_object(Node* obj);
    
    // Reading project settings is pretty expensive... set up to only update manually by default
    // but we have the option to update every loop, too
    bool update_multipliers_every_loop = false;
    void update_lod_multipliers_from_settings();
    void update_lod_multipliers_in_objects(); // Tell LOD objects it's time to update
    bool set_up_camera();
    bool set_camera(Node* given_node);
    bool update_fov_every_loop = false;
    void update_fov(); // Need FOV for getting screen percentages
    bool update_AABB_every_loop = false;
    void update_lod_AABBs(); // Need AABB for getting screen percentages
    void stop_loop(); // Stops the main thread
    void debug_level_print(int min_debug_level, const char* message);

    // Distance multipliers, available to access by other LOD objects
    float global_distance_multiplier = 1.0f;
    float lod1_distance_multiplier = 1.0f;
    float lod2_distance_multiplier = 1.0f;
    float lod3_distance_multiplier = 1.0f;
    float hide_distance_multiplier = 1.0f;
    float unload_distance_multiplier = 1.0f;
    float shadow_distance_multiplier = 1.0f;

    int lod_object_count = 0;
    // Whether to use multithreading or not
    bool use_multithreading = true;
    // If we're not using a thread, looping through all objects at once can cause stutters.
    // So let's break the objects looped into separate frames
    int current_loop_index = 0;
    int objects_per_frame = 10000;
};

//// Object based LOD ----------------------------------------------------------------------------------
class LOD : public VisualInstance {
    GODOT_CLASS(LOD, VisualInstance)

private:
    float tick_speed = 0.2f;
    bool enabled = true; // Switch to false if we want to turn off LOD functionality
    bool registered = false; // Whether the manager knows we exist
    bool try_register(bool state); // Register or unregister the object. Returns success or fail
    // Dirty bit. Indicates if the LOD manager has had any contact with this object
    bool interacted_with_manager = false;

    // Distance by metres
    // These will be set by the ratios below if use_screen_percentage is true
    float lod1_distance = 7.0f; // put any of these to -1 if you don't have a lod or don't want to unload etc
    float lod2_distance = 12.0f;
    float lod3_distance = 30.0f;
    float hide_distance = 100.0f;
    float unload_distance = -1.0f;

    // Distance by screen percentage
    // Use a conservative/worst-case method for getting the size of the object
    // relative to the screen (largest AABB axis on both viewport axes)
    bool use_screen_percentage = true;
    float lod1_ratio = 25.0f;
    float lod2_ratio = 10.0f;
    float lod3_ratio = 5.5f;
    float hide_ratio = 1.0f;
    float unload_ratio = -1.0f;

    bool disable_processing = false;

    // Prevents calling the process_data function too fast from the main loop
    bool already_running = false;

    // Keep track of last state to avoid unnecessary show/hide/process toggle calls
    int last_state = -1;

    NodePath lod0_path;
    NodePath lod1_path;
    NodePath lod2_path;
    NodePath lod3_path;

    Spatial* lod0 = NULL; // LOD0 MUST exist otherwise screen percentage breaks
    Spatial* lod1 = NULL; // Plus it doesn't make sense for it not to exist
    Spatial* lod2 = NULL;
    Spatial* lod3 = NULL;

    float fov; // Need FOV for getting screen percentages

    // Let's use the AABB centre for the centre of the object instead of
    // the parent's centre (if applicable). Instead of constantly 
    // accessing AABB, just store an offset.
    Vector3 transform_offset_AABB;

    ProjectSettings* project_settings;
    bool affected_by_distance_multipliers = true;
    float global_distance_multiplier = 1.0f;
    float lod1_distance_multiplier = 1.0f;
    float lod2_distance_multiplier = 1.0f;
    float lod3_distance_multiplier = 1.0f;
    float hide_distance_multiplier = 1.0f;
    float unload_distance_multiplier = 1.0f;

    void set_node_processing(Spatial* node, bool state);

public:
    static void _register_methods();

    LOD();
    ~LOD();

    void _init(); // our initializer called by Godot

    void _ready();
    void _process(float delta);
    void _exit_tree();
    void process_data(Vector3 camera_location);

    void update_lod_multipliers_from_manager(); // Reading project settings is pretty expensive... only update manually
    void update_lod_AABB(); // Update AABB only if necessary
    void update_fov(); // Need FOV for getting screen percentages
    void show_lod(Spatial* lod_object, bool show); // Show or hide a LOD Spatial
};

//// Light detail (shadow and light itself) LOD ------------------------------------------------------------
class LightLOD : public Light {
    GODOT_CLASS(LightLOD, Light)

private:
    float tick_speed = 0.5f;
    bool enabled = true; // Switch to false if we want to turn off LOD functionality
    bool registered = false; // Whether the manager knows we exist
    bool try_register(bool state); // Register or unregister the object. Returns success or fail
    // Dirty bit. Indicates if the LOD manager has had any contact with this object
    bool interacted_with_manager = false;

    float shadow_distance = 20.0f;
    float hide_distance = 80.0f; // -1 to never hide
    float fade_range = 5.0f; // For ex, the intensity of the shadow will adjust from 0 to 1 between [shadow_distance - fade_range, shadow_distance]

    // Distance by screen percentage
    // Use a conservative/worst-case method for getting the size of the object
    // relative to the screen (largest AABB axis on both viewport axes)
    bool use_screen_percentage = true;
    float shadow_ratio = 6.0f;
    float hide_ratio = 2.0f;

    float fade_speed = 2.0f;

    real_t light_base_energy;

    real_t light_target_energy;
    Color shadow_target_color;

    float fov; // Need FOV for getting screen percentages

    void fade_light(float delta);
    void fade_shadow(float delta);

    ProjectSettings* project_settings;
    bool affected_by_distance_multipliers = true;
    float global_distance_multiplier = 1.0f;
    float shadow_distance_multiplier = 1.0f;

public:
    static void _register_methods();

    LightLOD();
    ~LightLOD();

    void _init(); // our initializer called by Godot

    void _ready();
    void _process(float delta);
    void _exit_tree();
    void process_data(Vector3 camera_location);

    void update_lod_multipliers_from_manager(); // Reading project settings is pretty expensive... we have the option to
    void update_lod_AABB(); // Update AABB only if necessary
    void update_fov(); // Need FOV for getting screen percentages
};

//// GIProbe LOD -------------------------------------------------------------------
class GIProbeLOD : public GIProbe  {
    GODOT_CLASS(GIProbeLOD, GIProbe)

private:
    float tick_speed = 0.1f;
    bool enabled = true; // Switch to false if we want to turn off LOD functionality
    bool registered = false; // Whether the manager knows we exist
    bool try_register(bool state); // Register or unregister the object. Returns success or fail
    // Dirty bit. Indicates if the LOD manager has had any contact with this object
    bool interacted_with_manager = false;

    float hide_distance = 80.0f;
    float fade_range = 5.0f; // The energy of the probe will adjust from 0 to 1 between [unload_distance - fade_range, unload_distance]

    // Distance by screen percentage
    // Use a conservative/worst-case method for getting the size of the object
    // relative to the screen (largest AABB axis on both viewport axes)
    bool use_screen_percentage = true;
    float hide_ratio = 2.0f;

    float fade_speed = 1.0f;

    real_t probe_target_energy;

    real_t probe_base_energy;

    float fov; // Need FOV for getting screen percentages

    ProjectSettings* project_settings;
    bool affected_by_distance_multipliers = true;
    float global_distance_multiplier = 1.0f;

public:
    static void _register_methods();

    GIProbeLOD();
    ~GIProbeLOD();

    void _init(); // our initializer called by Godot

    void _ready();
    void _process(float delta);
    void _exit_tree();
    void process_data(Vector3 camera_location);

    void update_lod_multipliers_from_manager(); // Reading project settings is pretty expensive... only update manually
    void update_lod_AABB(); // Update AABB only if necessary
    void update_fov(); // Need FOV for getting screen percentages
};

//// MultiMeshInstance LOD -------------------------------------------------------------------
class MultiMeshLOD : public MultiMeshInstance  {
    GODOT_CLASS(MultiMeshLOD, MultiMeshInstance)

private:
    float tick_speed = 0.5f;
    bool enabled = true; // Switch to false if we want to turn off LOD functionality
    bool registered = false; // Whether the manager knows we exist
    bool try_register(bool state); // Register or unregister the object. Returns success or fail
    // Dirty bit. Indicates if the LOD manager has had any contact with this object
    bool interacted_with_manager = false;

    float min_distance = 5.0f; // At this distance, or below, we see max number of multimesh count
    float max_distance = 80.0f; // At this distance, or above, we see min (or none) number of multimesh count

    // Distance by screen percentage
    // Use a conservative/worst-case method for getting the size of the object
    // relative to the screen (largest AABB axis on both viewport axes)
    bool use_screen_percentage = true;
    float min_ratio = 2.0f;
    float max_ratio = 5.0f;

    int64_t min_count = 0;
    int64_t max_count = -1; // -1 means the number generated in the multimesh we find

    int64_t target_count = 50;

    float fade_speed = 1.0f;
    float fade_exponent = 1.0f;  // Exponent of the [0, 1] curve that reduces count. At 1, we fade linearly.

    float fov; // Need FOV for getting screen percentages

    ProjectSettings* project_settings;
    bool affected_by_distance_multipliers = true;
    float global_distance_multiplier = 1.0f;

    MultiMesh* multimesh;

public:
    static void _register_methods();

    MultiMeshLOD();
    ~MultiMeshLOD();

    void _init(); // our initializer called by Godot

    void _ready();
    void _process(float delta);
    void _exit_tree();
    void process_data(Vector3 camera_location);

    void update_lod_multipliers_from_manager(); // Reading project settings is pretty expensive... only update manually
    void update_lod_AABB(); // Update AABB only if necessary
    void update_fov(); // Need FOV for getting screen percentages
};

}

#endif