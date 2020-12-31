# Multi LOD
This is a collection of LOD systems for various built-in Godot types.

More detailed write-up available here:
[First write-up](https://puchik.now.sh/posts/lod/).
[Second, newest write-up](https://puchik.now.sh/posts/multithreaded-lod/).

For all the sytems, there is a tick speed that specifies how often the distances should be checked. Also, setting any distance to -1 will disable a detail level (e.g. don't use
LOD1, never fade out shadows, etc.)

**Installation**
Drag and drop the MultiLOD folder from the addons folder in the example folder into your addons folder. The source code is available in the src folder.

**Any VisualInstance inheriting object**

You can attach the regular LOD script to switch its children. If you do not specify the LOD0, LOD1, LOD2 objects, the
system will look for children whose names contain LOD0/LOD1/LOD2, so it's partially automatic.

Use "disableProcessing" (off by default) to have the \_process and \_physics_process functions disabled when the LOD is disabled

**Light nodes** 

You can specify a distance where shadows fade out and get disabled, a distance where the light itself fades out and gets disabled,
and a range where the fading happens (max distance - fade range is when it starts to fade out).

**GIProbes**

You need to be careful with GIProbe placement (as usual with GIProbes). The idea is to have a grid of overlapping GIProbes. Play around with the subdivison number, size of GIPRobe etc to get the effect you want without making the grid obvious. NOTE: for 4 GIProbes to be able to blend and affect a single object at the same time, you should get the [GIProbe blending patch](https://github.com/puchik/godot-extras/tree/master/patches/giprobe-blending) (also in this repository) and apply to your fork of Godot and recompile the engine. Otherwise, this "LOD" system is sort of useless.

The idea is to have your "main" world meshes separated (no huge spots of ground), and have overlapping GIProbes. Something like this (blue squares are your map chunks, green squares are GIProbes):

![](https://puchik.now.sh/images/lod-post/giprobe-layout.jpg)

**MultiMeshInstance**

Define the max and min number of visible instance (max visible instances = -1 will set it to the number of instances itself), the min and max fade distance, and an exponent for the fade. The number of visible instances will fade from the min to max range according to a curve
defined by the exponent. By default, the exponent = 1, meaning the fade will be linear. (Maybe I'll eventually add the actual Godot curve type!)


**Notes**
You need the GDNative library to point to the library file (e.g. .dll) correctly, and then the have the NativeScripts correctly point to the library. Have a read of the GDNative documentation in the docs if you're having trouble.
For Windows, this is already set up.

Alternatively, the source C++ files are also in this directory. You'll need them to compile for different platforms + any changes you'd like + see the internals.

**Limitations**

If you have many objects, your message queue will overflow. You will need to increase its size in project settings.

I have put print statements for the beginning and the ending of a thread. I have also tested the lifecycle of the thread and tried doing a very simple threaded application in Godot that is identical to its gdscript equivalent in the threading guide. Yet, when the thread ends, Godot throws an error about losing reference to thread while it is running. I have even looked at the source code and am not quite sure what is going on. I am moderately convinced this is a Godot bug, but I have yet to report it on GitHub. Functionality does not seem to be affected.

If you experience problems, download and use the previous commit/first version for now and let me know.

**Known bugs:**

You tell me :)
