# Optimization, LOD, and rendering work/plugins for Godot Engine 4.x and 3.x.
## The master branch is developed for 4.x and a 3.x branch is available for Godot 3.

## [Multithreaded LOD in C++ with "Importance" value](https://github.com/puchik/godot-importance-lod)
A LOD-like system which contains a LODObject that controls child LOD levels through a LODManager autoload. It calculates a discrete LOD value based on your input parameters, supports switching child nodes when a given LOD is reached, sends signals you can hook into to perform distance optimizations (like AI complexity), and calculates a float "Importance" value. Partially inspired by the [Significance Manager](https://dev.epicgames.com/documentation/en-us/unreal-engine/significance-manager-in-unreal-engine) in Unreal Engine.

## [Terrain/Vertex colour mixing shader](https://github.com/puchik/godot-extras/tree/master/visual/terrain-vertex-shader)
A shader that was originally intended for terrains, but can be applied to any mesh to add material mixing, detail, and more.

## [Snow shader](https://github.com/puchik/godot-extras/tree/master/tutorials/snow-shader)
This was supposed to be for a tutorial video, but I kept never getting to it. I decided to, at least, release the tutorial files (including the full shader).
