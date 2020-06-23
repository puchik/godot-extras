# Multi LOD
This is a collection of LOD systems for various built-in Godot types.

*For any node*, you can attach the regular LOD script to switch its children. If you do not specify the LOD0, LOD1, LOD2 objects, the
system will look for children whose names contain LOD0/LOD1/LOD2, so it's partially automatic.

*For light nodes*, you can specify a distance where shadows fade out and get disabled, a distance where the light itself fades out and gets disabled,
and a range where the fading happens (max distance - fade range is when it starts to fade out).

*For GIProbes*, you need to be careful with their placement (as usual with GIProbes). The idea is to have a grid of overlapping GIProbes. Play around
with the subdivison number, size of GIPRobe etc to get the effect you want without making the grid obvious. NOTE: for 4 GIProbes to be able
to blend and affect a single object at the same time, you should get the GIProbe blending patch (also in this repository) and apply to your fork of
Godot and recompile the engine. Otherwise, this "LOD" system is sort of useless.

*For MultiMeshInstance*, you define the max and min number of visible instance (max visible instances = -1 will set it to the number of instances itself), the min and max fade distance, and an exponent for the fade. The number of visible instances will fade from the min to max range according to a curve
defined by the exponent. By default, the exponent = 1, meaning the fade will be linear. (Maybe I'll eventually add the actual Godot curve type!)


You need the GDNative library to point to the library (e.g. .dll) correctly, and then the have the NativeScripts correctly point to the library. Have a read of the GDNative documentation in the docs if you're having trouble.
