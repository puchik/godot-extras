# Importance LOD for Godot in a multithreaded C++ GDExtension
## Description
Godot 4 comes with useful LOD functionality built in many of its features, but there are cases where additional fine-tuning may be helpful. As a result, my [LOD plugin for Godot 3](https://github.com/puchik/godot-extras/tree/3.x/gdnative/multi-lod) has been updated to Godot 4 in a slightly different form; it focuses on replacement of nodes and signal dispatching along with a continuous "Importance" value for judging how many resources should be dedicated to that given node. 

You use these values and signals to modify your object's workload or appearance based on distance away from the player, such as reducing AI logic for objects that are further away. You could also store individual nodes in a LOD0 parent, and then use lower LOD nodes as parent nodes for merged versions of those nodes, like an HLOD.

It's inspired partially by Unreal's [Significance Manager](https://dev.epicgames.com/documentation/en-us/unreal-engine/significance-manager-in-unreal-engine), which doesn't improve performance on its own but relies on user-specified optimizations. However, with my plugin you can use it for the bare minimum of hiding and/or disabling objects and nodes as a quick optimization.

![Importance LOD GIF](https://puchik.vercel.app/images/importance-lod/importance-lod.gif)

The car animation stops when you get far enough. Notice that the headlight colours do not animate when the car is yellow.

## Usage
### Installation
Download and add the "importance_lod" folder to your addons folder and in Godot enable the Importance LOD plugin. This will add some new quality settings to your project and register the ImportanceLOD Autoload, which will exist globally in your game. An example project is available that outlines all of its features.

### Exposed functions and variables
You can run functions on the Importance LOD Manager Autoload from anywhere, just like any Autoload. You have access to:

#### LOD Manager functions

* `ImportanceLODManager.set_camera(camera)`: If the manager fails to find your camera automatically, or you need
    to set a specific camera, or you are switching levels, etc., you can set the camera manually.
* `ImportanceLODManager.set_up_camera()`: Like `set_camera()`, but automatic.
* `ImportanceLODManager.update_fov()`: The FOV of the camera is cached on the Autoload start. Use this to fetch and update the new value.
* `ImportanceLODManager.update_lod_AABBs()`: The AABBs of each object are used for determining distance for screen-percentage LOD. If you need, you may update them manually.
* `ImportanceLODManager.update_lod_multipliers_from_settings()`: When you change the project settings at runtime, you must update the LOD Manager's values for changes to take effect.
* Setters and getters for its variables.

Like other Autoloads, the LOD Manager is just another scene file (`lod_manager.tscn`). It has some adjustable parameters. The manual update functions can be unnecessary if you set variables to update every frame (see section below), but this is disabled by default for optimization:

#### LOD Manager variables

* Use Multithreading: Whether multithreading is used for the main loop. Keep in mind, updates to nodes are done through deferred calls i.e. on the main thread.
* Objects per Frame: If multithreading is off, the manager will go through its list of nodes in chunks to avoid lag caused by excessive object updates per frame.
* FOV: Used for screen-percentage LOD. Set automatically on manager startup.
* Update FOV Every Loop: Fetches the current FOV on every loop. Off by default for optimization.
* Update AABB Every Loop: Fetches each object's AABB on every loop. Off by default for optimization.
* Update Multipliers Every Loop: Fetches the project setting and multipliers on every loop. Off by default for optimization.
* Tick speed: The rate at which LODs are updated.
* Various multiplier settings: Set through project settings
* Debug level: If you have problems, this setting will tell the manager to print each one of its steps as it runs.

#### LOD Object functions
LODObjects also have their set of functions and variables.

* `get_current_lod()`: Returns the current discrete LOD level.
* `get_importance()`: Returns a continuous "Importance" value from 0 to 1 based on the min and max LOD levels.
* Setters and getters for their variables.

#### LOD Object variables
* Enabled: Whether the LOD manager affects this LODObject.
* Affected by Distance Multipliers: Whether the project settings affect this LODObject's distances.
* Use Screen Percentage: By default, the screen percentage is used for determining LOD distances automatically. This relies on child nodes (LOD0, at least), having a node type that has an AABBB.
* Distance settings: If screen percentage is off, these distances are used for each LOD level.
* Ratio settings: If screen percentage is on, these screen percentages/ratios are used for each LOD level's distance calculation.
* LOD Paths: Either set these manually, or the manager will determine them based on the child names, which should be "LOD0", "LOD1", "LOD2", and "LOD3". A LOD 0 level is mandatory.

### Project settings
To accomodate for different quality settings in your game (or quick debugging), the plugin supports LOD multipliers for each LOD level or a global value. This will proportionally reduce or increase the distances used for your LODs. They will appear in your project settings, if advanced settings are enabled, as:

* `performance/importance_lod/global_multiplier`
* `performance/importance_lod/lod1_multiplier`
* `performance/importance_lod/lod2_multiplier`
* `performance/importance_lod/lod3_multiplier`
* `performance/importance_lod/hide_multiplier`
* `performance/importance_lod/unload_multiplier`

These are modified in the same way as native Godot project settings, using the same functions and menus.

For example:

`ProjectSettings.set_setting("performance/importance_lod/global_multiplier", 0.5)`

### Signals
One of the main features of Importance LOD are the signals. You can bind to them in the UI like normal signals, or through script like so:

`$YourNode.connect("lod_changed", Callable(self, "_on_lod_changed"))`

## Important Notes
* **Do not attach a script to the LODObject node.** Godot handles GDExtension as a native node and will replace the functions in that node with your GDScript. Changes to GDScript in 4.0 resulted in parent functions not being called
unless they are called explicitly, but GDScript does not fully support inheritance, so here we are... See the AnimatedCar example in the TestProject.

* Development occurs on Windows. Linux builds are provided for your convenience but are *not* tested. You will have to compile yourself for other platforms.



Licensed under MIT. 