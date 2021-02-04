# Terrain/Vertex colour shader
This is a shader that lets you mix 4 sets of textures/materials with support for normal maps, roughness maps, and parallax mapping using vertex colours. It also supports height maps (with alpha channels for cuts/holes
in the terrain), splat maps (instead of vertex colours), distance culling (define a min and max visible distance), anti-tiling (you need to supply a noise texture), and height blending.

The original idea was to use this with terrains, but it's really just a vertex colour shader! Use it to add details to your models such as rust on metal or puddles on the ground.

[Video 1](https://youtu.be/4e_iv0hZssM) and [Video 2](https://youtu.be/C9tCCJoFKZU)

**Usage**
Make a ShaderMaterial, define the .shader file as its shader, and set up the parameters as you wish.

If you want to paint vertex colours within Godot, you can use [VPainter](https://github.com/tomankirilov/VPainter).

**Parameters**
Keep in mind images appear at the end of the ShaderMaterial settings (as of 2021/01/31) despite the order in the shader variables, so the texture slots may look out of place.

You have 4 texture sets/materials you can blend. They come with the "main" features of the SpatialMaterial such normal maps, roughness, etc.
You can adjust these parameters as usual (although some of the min/max ranges are altered). Under those you will also find a scaling factor for those individual materials UV coordinates.

Under those, you'll find other settings:

* Enabling Distance Culling will make the object vertices disappear between Min Dist and Max Dist (e.g. object is invisible between 10m and 20m) with a "fade margin" of Cull Fade Margin (instead of cutting out immediately, vertices will fade out at random intervals within the margin). 

* Prevent tiling will use an algorithm loosely based on Technique 3 on [this](www.iquilezles.org/www/articles/texturerepetition/texturerepetition.htm) page to reduce appearance of texture tiling.
NOTE: You need to provide a noise texture for anti-tiling to work correctly. There is one provided in the example project.

* Height blending will use the displacement maps to blend materials rather than a simple fade.

* Height multipliers and offsets allow you to change how the materials blend together (when blending by height) without modifying the displacement maps (which are also used for parallax mapping).

* Height maps are supported in texture form. The alpha channel is used to put holes in the terrain.

* Splat maps can be used to blend materials instead of vertex colours.

**Limitations**

In some cases, the anti-tiling might cause the appearance of lines across the surface of the mesh.

Due to texture quantity limits, metallic and ambient occlusion textures are not present by default. 
However, you can combine your metallic, roughness, and occlusion textures and use a single texture slot. See comments inside the shader for further explanation.

Performance is good, but there is room for improvement.

**Other**

The project contains a water shader available on the AssetLib which was created by Maujoe.

Textures are taken from CC0 Textures.