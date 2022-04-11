#ifndef MULTI_LOD_H
#define MULTI_LOD_H

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
#include <vector>

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

// Doesn't currently support more than 4 LODs for meshes due to other hardcoded variables.
#define LOD_COUNT 4

#define PI 3.14159f

#ifndef CLAMP
#define CLAMP(m_a, m_min, m_max) (((m_a) < (m_min)) ? (m_min) : (((m_a) > (m_max)) ? m_max : m_a))
#endif

namespace godot {

class LODBaseComponent;

//// Manager of all LOD objects ----------------------------------------------------------------------------------
class LODManager : public Node {
    GODOT_CLASS(LODManager, Node)

private:
    float tick_speed = 0.2f;

    // Array of arrays of LOD objects
    std::vector<std::vector<LODBaseComponent*>> lod_object_arrays;

    Camera* camera = nullptr;
    float fov = 70.0f; // Need FOV for getting screen percentages
    float tan_theta = 0.7002f; // Calc for 70.0f

    ProjectSettings* project_settings;

    // Used to stop the management thread
    bool manager_removed = false;

    // Flag to let LOD objects know to update their multipliers (if necessary)
    bool update_multipliers_flag = false;
    // Flag to let LOD objects know to update their AABB (use after setting FOV if necessary)
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

    void add_object(LODBaseComponent* lbc);
    void remove_object(LODBaseComponent* lbc);
    
    // Reading project settings is pretty expensive... set up to only update manually by default
    // but we have the option to update every loop, too
    bool update_multipliers_every_loop = false;
    void update_lod_multipliers_from_settings();
    void update_lod_multipliers_in_objects(); // Tell LOD objects it's time to update
    bool set_up_camera();
    bool set_camera(Camera* p_camera);
    bool update_AABB_every_loop = false;
    void update_lod_AABBs(); // Need AABB for getting screen percentages
    void stop_loop(); // Stops the main thread
    bool update_fov_every_loop = false;
    void update_fov(); // Need FOV for getting screen percentages
    void set_fov(float p_fov);
    inline float get_fov() { return fov; }
    inline float get_tan_theta() { return tan_theta; }

    void debug_level_print(int min_debug_level, const String &message);

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

class LODBaseComponent : public Reference {
    GODOT_CLASS(LODBaseComponent, Reference)
protected:
    Spatial* lod_object; // Object that owns us

public:
    LODManager* lod_manager;
    bool enabled = true; // Switch to false if we want to turn off LOD functionality
    bool registered = false; // Whether the manager knows we exist
    bool ready_finished = false;

    // Distance by screen percentage
    // Use a conservative/worst-case method for getting the size of the object
    // relative to the screen (largest AABB axis on both viewport axes)
    bool use_screen_percentage = true;
    bool affected_by_distance_multipliers = true;

    void _init() {}
    LODBaseComponent() {}
    ~LODBaseComponent() {}

    void setup(Spatial* p_object);
    inline Spatial* get_object() { return lod_object; }

    inline float get_fov();
    float get_tan_theta();

    virtual void process_data(Vector3 p_camera_location) {}
    virtual void update_lod_AABB() {}
    virtual void update_lod_multipliers_from_manager() {}

    void try_register();   // register is a C++ keyword, so try_
    void unregister();
};

template <class T>
class LODComponent : public LODBaseComponent {
    GODOT_CLASS(LODComponent, LODBaseComponent)
public:
    void process_data(Vector3 p_camera_location) override {
        Object::cast_to<T>(lod_object)->process_data(p_camera_location);
    }
    void update_lod_AABB() override {
        Object::cast_to<T>(lod_object)->update_lod_AABB();
    }
    void update_lod_multipliers_from_manager()override {
        Object::cast_to<T>(lod_object)->update_lod_multipliers_from_manager();
    }
};

//// Object based LOD ----------------------------------------------------------------------------------
class LOD : public VisualInstance {
    GODOT_CLASS(LOD, VisualInstance)

private:
    LODComponent<LOD> lc;

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
    float lod1_ratio = 25.0f;
    float lod2_ratio = 10.0f;
    float lod3_ratio = 5.5f;
    float hide_ratio = 1.0f;
    float unload_ratio = -1.0f;

    NodePath lod0_path;
    NodePath lod1_path;
    NodePath lod2_path;
    NodePath lod3_path;

    // LOD0 MUST exist otherwise screen percentage breaks
    // Plus it doesn't make sense for it not to exist
    Spatial* lods[LOD_COUNT] = { };

    // The highest level of LOD found
    int last_lod = 0;

    // Keep track of last state to avoid unnecessary show/hide/process toggle calls
    int current_lod = -1;

    // This LOD is the maximum used for shadow casting. If 0 (LOD0), it's effectively disabled.
    int max_shadow_caster = 0;

    // Let's use the AABB centre for the centre of the object instead of
    // the parent's centre (if applicable). Instead of constantly 
    // accessing AABB, just store an offset.
    Vector3 transform_offset_AABB;

    float global_distance_multiplier = 1.0f;
    float lod1_distance_multiplier = 1.0f;
    float lod2_distance_multiplier = 1.0f;
    float lod3_distance_multiplier = 1.0f;
    float hide_distance_multiplier = 1.0f;
    float unload_distance_multiplier = 1.0f;

public:
    static void _register_methods();

    LOD();
    ~LOD();

    void _init(); // our initializer called by Godot

    void _ready();
    void _enter_tree();
    void _process(float delta);
    void _exit_tree();
    void process_data(Vector3 camera_location);

    void update_lod_multipliers_from_manager(); // Reading project settings is pretty expensive... only update manually
    void update_lod_AABB(); // Update AABB only if necessary
    void show_lod(int lod); // Show only this LOD, hide others

    inline int get_current_lod() { return current_lod; }

    inline void set_enabled(bool value) { lc.enabled = value; }
    inline bool get_enabled() { return lc.enabled; }

    inline void set_affected_by_distance(bool value) { lc.affected_by_distance_multipliers = value; }
    inline bool get_affected_by_distance() { return lc.affected_by_distance_multipliers; }

    inline void set_use_screen_percentage(bool value) { lc.use_screen_percentage = value; }
    inline bool get_use_screen_percentage() { return lc.use_screen_percentage; }
};

//// Light detail (shadow and light itself) LOD ------------------------------------------------------------
class LightLOD : public Light {
    GODOT_CLASS(LightLOD, Light)

private:
    LODComponent<LightLOD> lc;

    float shadow_distance = 20.0f;
    float hide_distance = 80.0f; // -1 to never hide
    float fade_range = 5.0f; // For ex, the intensity of the shadow will adjust from 0 to 1 between [shadow_distance - fade_range, shadow_distance]

    // Distance by screen percentage
    // Use a conservative/worst-case method for getting the size of the object
    // relative to the screen (largest AABB axis on both viewport axes)
    float shadow_ratio = 6.0f;
    float hide_ratio = 2.0f;

    float fade_speed = 2.0f;

    real_t light_base_energy;

    real_t light_target_energy;
    Color shadow_target_color;

    void fade_light(float delta);
    void fade_shadow(float delta);

    float global_distance_multiplier = 1.0f;
    float shadow_distance_multiplier = 1.0f;

public:
    static void _register_methods();

    LightLOD();
    ~LightLOD();

    void _init(); // our initializer called by Godot

    void _ready();
    void _enter_tree();
    void _process(float delta);
    void _exit_tree();
    void process_data(Vector3 camera_location);

    void update_lod_multipliers_from_manager(); // Reading project settings is pretty expensive... we have the option to
    void update_lod_AABB(); // Update AABB only if necessary

    inline void set_enabled(bool value) { lc.enabled = value; }
    inline bool get_enabled() { return lc.enabled; }

    inline void set_affected_by_distance(bool value) { lc.affected_by_distance_multipliers = value; }
    inline bool get_affected_by_distance() { return lc.affected_by_distance_multipliers; }

    inline void set_use_screen_percentage(bool value) { lc.use_screen_percentage = value; }
    inline bool get_use_screen_percentage() { return lc.use_screen_percentage; }
};

//// GIProbe LOD -------------------------------------------------------------------
class GIProbeLOD : public GIProbe {
    GODOT_CLASS(GIProbeLOD, GIProbe)

private:
    LODComponent<GIProbeLOD> lc;

    float hide_distance = 80.0f;
    float fade_range = 5.0f; // The energy of the probe will adjust from 0 to 1 between [unload_distance - fade_range, unload_distance]

    // Distance by screen percentage
    // Use a conservative/worst-case method for getting the size of the object
    // relative to the screen (largest AABB axis on both viewport axes)
    float hide_ratio = 2.0f;

    float fade_speed = 1.0f;

    real_t probe_target_energy;

    real_t probe_base_energy;

    float global_distance_multiplier = 1.0f;

public:
    static void _register_methods();

    GIProbeLOD();
    ~GIProbeLOD();

    void _init(); // our initializer called by Godot

    void _ready();
    void _enter_tree();
    void _process(float delta);
    void _exit_tree();
    void process_data(Vector3 camera_location);

    void update_lod_multipliers_from_manager(); // Reading project settings is pretty expensive... only update manually
    void update_lod_AABB(); // Update AABB only if necessary

    inline void set_enabled(bool value) { lc.enabled = value; }
    inline bool get_enabled() { return lc.enabled; }

    inline void set_affected_by_distance(bool value) { lc.affected_by_distance_multipliers = value; }
    inline bool get_affected_by_distance() { return lc.affected_by_distance_multipliers; }

    inline void set_use_screen_percentage(bool value) { lc.use_screen_percentage = value; }
    inline bool get_use_screen_percentage() { return lc.use_screen_percentage; }
};

//// MultiMeshInstance LOD -------------------------------------------------------------------
class MultiMeshLOD : public MultiMeshInstance {
    GODOT_CLASS(MultiMeshLOD, MultiMeshInstance)

private:
    LODComponent<MultiMeshLOD> lc;

    float min_distance = 5.0f; // At this distance, or below, we see max number of multimesh count
    float max_distance = 80.0f; // At this distance, or above, we see min (or none) number of multimesh count

    // Distance by screen percentage
    // Use a conservative/worst-case method for getting the size of the object
    // relative to the screen (largest AABB axis on both viewport axes)
    float min_ratio = 2.0f;
    float max_ratio = 5.0f;

    int64_t min_count = 0;
    int64_t max_count = -1; // -1 means the number generated in the multimesh we find

    int64_t target_count = 50;

    float fade_speed = 1.0f;
    float fade_exponent = 1.0f;  // Exponent of the [0, 1] curve that reduces count. At 1, we fade linearly.

    float global_distance_multiplier = 1.0f;

    MultiMesh* multimesh;

public:
    static void _register_methods();

    MultiMeshLOD();
    ~MultiMeshLOD();

    void _init(); // our initializer called by Godot

    void _ready();
    void _enter_tree();
    void _process(float delta);
    void _exit_tree();
    void process_data(Vector3 camera_location);

    void update_lod_multipliers_from_manager(); // Reading project settings is pretty expensive... only update manually
    void update_lod_AABB(); // Update AABB only if necessary

    inline void set_enabled(bool value) { lc.enabled = value; }
    inline bool get_enabled() { return lc.enabled; }

    inline void set_affected_by_distance(bool value) { lc.affected_by_distance_multipliers = value; }
    inline bool get_affected_by_distance() { return lc.affected_by_distance_multipliers; }

    inline void set_use_screen_percentage(bool value) { lc.use_screen_percentage = value; }
    inline bool get_use_screen_percentage() { return lc.use_screen_percentage; }
};

}

#endif
