# godot-extras
My collection of various plugins, modules, or patches for Godot.

Everything is licensed under MIT.

## [LOD in C++](https://github.com/puchik/godot-extras/tree/master/gdnative/multi-lod)
A LOD system that can be applied directly to any Spatial. There are also more specialized variants for Lights, GIProbes, and MultiMeshInstances.

## [Terrain/Vertex colour mixing shader](https://github.com/puchik/godot-extras/tree/master/visual/terrain-vertex-shader)
A shader that was originally intended for terrains, but can be applied to any mesh to add material mixing, detail, and more.

## [Snow shader](https://github.com/puchik/godot-extras/tree/master/tutorials/snow-shader)
This was supposed to be for a tutorial video, but I kept never getting to it. I decided to, at least, release the tutorial files (including the full shader).

## [4 GIProbe blending patch](https://github.com/puchik/godot-extras/tree/master/patches/giprobe-blending)
A minor (only tested on my system, not sure if there will be problems on others!) that allows 4 GIProbes (up from 2) to affect and blend on a single object. This is important
for the [GIProbe LOD](https://github.com/puchik/godot-extras/tree/master/gdnative/multi-lod). It also compensates non-interior environment lighting if you're lowering/the GIProbe doesn't cover an object fully. Normally, the areas not covered by the GIProbe (or if you lower energy) aren't affected by ambient/environment lighting at all (think enabling the "Interior" option).
