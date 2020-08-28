# GIProbe blending
This git patch allows up to 4 GIProbes to be blended across a single object. This is useful for my [LOD system's](https://github.com/puchik/godot-extras/tree/master/gdnative/multi-lod) GIProbe functionality.

It also makes GIProbes compensate the ambient light when fading out its energy (if "interior" is not enabled).

Before and after:
![](https://puchik.now.sh/images/lod-post/gifade-before.gif)
![](https://puchik.now.sh/images/lod-post/gifade-after.gif)
