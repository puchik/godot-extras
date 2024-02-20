#ifndef IMPORTANCE_LOD_H
#define IMPORTANCE_LOD_H

#include "godot_cpp/core/class_db.hpp"



#include <godot_cpp/godot.hpp>


#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
//#include <AABB.hpp>
#include <godot_cpp/classes/thread.hpp>
//#include <Ref.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/mutex.hpp>
#include <godot_cpp/classes/semaphore.hpp>
#include <godot_cpp/classes/performance.hpp>
#include <godot_cpp/classes/engine.hpp>

#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/camera3d.hpp>
//#include <Transform.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
//#include <Vector3.hpp>

#include <ctime>
#include <vector>

// Objects

//#include <NodePath.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/visual_instance3d.hpp>

// MultiMesh
#include <godot_cpp/classes/multi_mesh_instance3d.hpp>
#include <godot_cpp/classes/multi_mesh.hpp>

// Doesn't currently support more than 4 LODs for meshes due to other hardcoded variables.
#define LOD_COUNT 4

#define PI 3.14159f

#ifndef CLAMP
#define CLAMP(m_a, m_min, m_max) (((m_a) < (m_min)) ? (m_min) : (((m_a) > (m_max)) ? m_max : m_a))
#endif

//using namespace godot;

namespace godot {

class LODBaseComponent;

//// Manager of all LOD objects ----------------------------------------------------------------------------------
class LODManager : public Node {
    GDCLASS(LODManager, Node)

private:
    float tick_speed = 0.2f;

    // Array of arrays of LOD objects
    std::vector<std::vector<LODBaseComponent*>> lod_object_arrays;

    Camera3D* camera = nullptr;
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

protected:
    static void _bind_methods();

public:
    static void _register_methods();

    LODManager();
    ~LODManager();

    void _init(); // our initializer called by Godot

    void _ready();
    void _process(float delta);
    void _exit_tree();

    // Returns false if operation failed.
    bool add_object(LODBaseComponent* lbc);
    void remove_object(LODBaseComponent* lbc);
    
    // Reading project settings is pretty expensive... set up to only update manually by default
    // but we have the option to update every loop, too
    bool update_multipliers_every_loop = false;
    void update_lod_multipliers_from_settings();
    void update_lod_multipliers_in_objects(); // Tell LOD objects it's time to update
    bool set_up_camera();
    bool set_camera(Camera3D* p_camera);
    bool update_AABB_every_loop = false;
    void update_lod_AABBs(); // Need AABB for getting screen percentages
    bool update_fov_every_loop = false;
    void update_fov(); // Need FOV for getting screen percentages
    void set_fov(float p_fov);
    inline float get_fov() { return fov; }
    inline float get_tan_theta() { return tan_theta; }

    void stop_loop(); // Stops the main thread

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
    
    /// Basic (mandatory) setters and getters
    void set_use_multithreading(const bool p_use_multithreading) { use_multithreading = p_use_multithreading; };
    bool get_use_multithreading() const { return use_multithreading; };

    void set_objects_per_frame(const int p_objects_per_frame) { objects_per_frame = p_objects_per_frame; };
    int get_objects_per_frame() const { return objects_per_frame; };

    void set_update_fov_every_loop(const bool p_update_fov_every_loop) { update_fov_every_loop = p_update_fov_every_loop; };
    bool get_update_fov_every_loop() const { return update_fov_every_loop; };
    void set_update_AABB_every_loop(const bool p_update_AABB_every_loop) { update_AABB_every_loop = p_update_AABB_every_loop; };
    bool get_update_AABB_every_loop() const { return update_AABB_every_loop; };

    void set_tick_speed(const float p_tick_speed) { tick_speed = p_tick_speed; };
    float get_tick_speed() const { return tick_speed; };

    void set_debug_level(const float p_debug_level) { debug_level = p_debug_level; };
    int get_debug_level() const { return debug_level; };

    void set_update_multipliers_every_loop(const bool p_update_multipliers_every_loop) { update_multipliers_every_loop = p_update_multipliers_every_loop; };
    bool get_update_multipliers_every_loop() const { return update_multipliers_every_loop; };
    void set_global_distance_multiplier(const float p_global_distance_multiplier) { global_distance_multiplier = p_global_distance_multiplier; };
    float get_global_distance_multiplier() const { return global_distance_multiplier; };
    void set_lod1_distance_multiplier(const float p_lod1_distance_multiplier) { lod1_distance_multiplier = p_lod1_distance_multiplier; };
    float get_lod1_distance_multiplier() const { return lod1_distance_multiplier; };
    void set_lod2_distance_multiplier(const float p_lod2_distance_multiplier) { lod2_distance_multiplier = p_lod2_distance_multiplier; };
    float get_lod2_distance_multiplier() const { return lod2_distance_multiplier; };
    void set_lod3_distance_multiplier(const float p_lod3_distance_multiplier) { lod3_distance_multiplier = p_lod3_distance_multiplier; };
    float get_lod3_distance_multiplier() const { return lod3_distance_multiplier; };
    void set_hide_distance_multiplier(const float p_hide_distance_multiplier) { hide_distance_multiplier = p_hide_distance_multiplier; };
    float get_hide_distance_multiplier() const { return hide_distance_multiplier; };
    void set_unload_distance_multiplier(const float p_unload_distance_multiplier) { unload_distance_multiplier = p_unload_distance_multiplier; };
    float get_unload_distance_multiplier() const { return unload_distance_multiplier; };
};

//// Base parent for LOD types.
class LODBaseComponent : public Object {
    GDCLASS(LODBaseComponent, Object)

protected:
    Node3D* lod_object; // Object that owns us
    static void _bind_methods() { } ;

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

    void setup(Node3D* p_object);
    inline Node3D* get_object() { return lod_object; }

    inline float get_fov();
    float get_tan_theta();

    virtual void process_data(Vector3 p_camera_location) {}
    virtual void update_lod_AABB() {}
    virtual void update_lod_multipliers_from_manager() {}

    void try_register();  // Register is a C++ keyword, so try_
    void unregister();
    
    /// Basic (mandatory) setters and getters
    /*
    void set_enabled(bool input) { enabled = input; };
    bool get_enabled() { return enabled; };
    void set_use_screen_percentage(bool input) { use_screen_percentage = input; };
    bool get_use_screen_percentage() { return use_screen_percentage; };
    */
    void set_affected_by_distance_multipliers(bool p_affected_by_distance_multipliers) { affected_by_distance_multipliers = p_affected_by_distance_multipliers; };
    bool get_affected_by_distance_multipliers() { return affected_by_distance_multipliers; };
};

template <class T>
class LODComponent : public LODBaseComponent {
    GDCLASS(LODComponent, LODBaseComponent)
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
class LOD : public VisualInstance3D {
    GDCLASS(LOD, VisualInstance3D)

protected:
    LODComponent<LOD> lod_component;

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
    Node3D* lods[LOD_COUNT] = { };

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

    Vector3 cached_scale;

    static void _bind_methods();

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

    inline int get_current_lod() const { return current_lod; }

    inline void set_enabled(const bool p_enabled) { lod_component.enabled = p_enabled; }
    inline bool get_enabled() const { return lod_component.enabled; }

    inline void set_affected_by_distance(const bool p_affected_by_distance_multipliers) { lod_component.affected_by_distance_multipliers = p_affected_by_distance_multipliers; }
    inline bool get_affected_by_distance() const { return lod_component.affected_by_distance_multipliers; }

    inline void set_use_screen_percentage(const bool p_use_screen_percentage) { lod_component.use_screen_percentage = p_use_screen_percentage; }
    inline bool get_use_screen_percentage() const { return lod_component.use_screen_percentage; }

    /// Basic (mandatory) setters and getters
    void set_lod1_distance(const float p_lod1_distance) { lod1_distance = p_lod1_distance; };
    float get_lod1_distance() const { return lod1_distance; };
    void set_lod2_distance(const float p_lod2_distance) { lod2_distance = p_lod2_distance; };
    float get_lod2_distance() const { return lod2_distance; };
    void set_lod3_distance(const float p_lod3_distance) { lod3_distance = p_lod3_distance; };
    float get_lod3_distance() const { return lod3_distance; };
    void set_hide_distance(const float p_hide_distance) { hide_distance = p_hide_distance; };
    float get_hide_distance() const { return hide_distance; };
    void set_unload_distance(const float p_unload_distance) { unload_distance = p_unload_distance; };
    float get_unload_distance() const { return unload_distance; };

    void set_lod1_ratio(const float p_lod1_ratio) { lod1_ratio = p_lod1_ratio; };
    float get_lod1_ratio() const { return lod1_ratio; };
    void set_lod2_ratio(const float p_lod2_ratio) { lod2_ratio = p_lod2_ratio; };
    float get_lod2_ratio() const { return lod2_ratio; };
    void set_lod3_ratio(const float p_lod3_ratio) { lod3_ratio = p_lod3_ratio; };
    float get_lod3_ratio() const { return lod3_ratio; };
    void set_hide_ratio(const float p_hide_ratio) { hide_ratio = p_hide_ratio; };
    float get_hide_ratio() const { return hide_ratio; };
    void set_unload_ratio(const float p_unload_ratio) { unload_ratio = p_unload_ratio; };
    float get_unload_ratio() const { return unload_ratio; };

    void set_lod0_path(const NodePath p_lod0_path) { lod0_path = p_lod0_path; };
    NodePath get_lod0_path() const { return lod0_path; };
    void set_lod1_path(const NodePath p_lod1_path) { lod1_path = p_lod1_path; };
    NodePath get_lod1_path() const { return lod1_path; };
    void set_lod2_path(const NodePath p_lod2_path) { lod2_path = p_lod2_path; };
    NodePath get_lod2_path() const { return lod2_path; };
    void set_lod3_path(const NodePath p_lod3_path) { lod3_path = p_lod3_path; };
    NodePath get_lod3_path() const { return lod3_path; };
};

/*

//// MultiMeshInstance LOD -------------------------------------------------------------------
class MultiMeshLOD : public MultiMeshInstance {
    GDCLASS(MultiMeshLOD, MultiMeshInstance)

private:
    LODComponent<MultiMeshLOD> lod_component;

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

    inline void set_enabled(bool value) { lod_component.enabled = value; }
    inline bool get_enabled() { return lod_component.enabled; }

    inline void set_affected_by_distance(bool value) { lod_component.affected_by_distance_multipliers = value; }
    inline bool get_affected_by_distance() { return lod_component.affected_by_distance_multipliers; }

    inline void set_use_screen_percentage(bool value) { lod_component.use_screen_percentage = value; }
    inline bool get_use_screen_percentage() { return lod_component.use_screen_percentage; }
};
*/

}

#endif
