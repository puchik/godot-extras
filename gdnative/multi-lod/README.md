# Multi LOD
This is a collection of LOD systems for various built-in Godot types.

For all the sytems, there is a tick speed that specifies how often the distances should be checked. Also, setting any distance to -1 will disable a detail level (e.g. don't use
LOD1, never fade out shadows, etc.)

**Any spatial/derivative**

You can attach the regular LOD script to switch its children. If you do not specify the LOD0, LOD1, LOD2 objects, the
system will look for children whose names contain LOD0/LOD1/LOD2, so it's partially automatic.

Use "disableProcessing" (on by default) to have the \_process and \_physics_process functions disabled when the LOD is disabled

**Light nodes** 

You can specify a distance where shadows fade out and get disabled, a distance where the light itself fades out and gets disabled,
and a range where the fading happens (max distance - fade range is when it starts to fade out).

**GIProbes**

You need to be careful with GIProbe placement (as usual with GIProbes). The idea is to have a grid of overlapping GIProbes. Play around with the subdivison number, size of GIPRobe etc to get the effect you want without making the grid obvious. NOTE: for 4 GIProbes to be able to blend and affect a single object at the same time, you should get the [GIProbe blending patch](https://github.com/puchik/godot-extras/tree/master/patches/giprobe-blending) (also in this repository) and apply to your fork of Godot and recompile the engine. Otherwise, this "LOD" system is sort of useless.

The idea is to have your "main" world meshes separated (no huge spots of ground), and have overlapping GIProbes. 

**MultiMeshInstance**

Define the max and min number of visible instance (max visible instances = -1 will set it to the number of instances itself), the min and max fade distance, and an exponent for the fade. The number of visible instances will fade from the min to max range according to a curve
defined by the exponent. By default, the exponent = 1, meaning the fade will be linear. (Maybe I'll eventually add the actual Godot curve type!)


**Notes**

You can also recompile Godot with the provided git patch; it is used for adding the multipliers for LOD distances so you can quickly change the settings globally if needed.
I tried to add the multipliers with an addon, but the C++ code would not be able to load them at game launch. I suspect the plugin may be loaded after the GDNative code begins running.

You need the GDNative library to point to the library file (e.g. .dll) correctly, and then the have the NativeScripts correctly point to the library. Have a read of the GDNative documentation in the docs if you're having trouble.

Alternatively, the source C++ files are also in this directory. You'll need them to compile for different platforms + any changes you'd like + see the internals.

**Limitations**

LOD multipliers are not updated at runtime when changes are made.

For multipliers, need to use a git-patch and recompile the engine to get it to work. Addon method doesn't seem to work (GDNative runs before addon initializer?).

LOD is based on distance instead of screen size.

**Known bugs:**

You tell me :)
