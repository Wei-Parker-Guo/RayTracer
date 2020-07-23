# CS 184 Fall 2016
Instructor: James O'Brien

# Implementer of This Project
Wei Guo: wei.parker.guo.sg@gmail.com

# References (Books & Used Libs Mainly)
1. GLFW Version 3.3.2
2. GLEW
3. ASSIMP Version 5.0.1
4. CUDA
5. dirent.h for directory/file manipulations
6. linmath.h under GLFW Dependencies
8. tooljpeg jpeg encoder from https://create.stephan-brumme.com/toojpeg/
7. Code references are enclosed as inline comments.

# Dummy Program Logic Flow

<p align="center">
  <img src="docs&images/dummy_logic_flow.jpg">
</p>

# Render Results
1. All the logs are recorded in logs.txt in raytracer.exe's directory.

2. Render result of the default stanford bunny scene with basic lambert materials and shadow rays with two directional lights:

<p align="center">
  <img src="docs&images/render_result_standford_rabbit_lambert_softshadow.jpg">
</p>

<p align="center">
	Samples per pixel: 4<br/>
	Samples per ray: 4<br/>
	Epsilon: 0.000100<br/>
	Ray Jitter Epsilon: 0.100000<br/>
	Max bounces: 3<br/>
	Field of view: 60.00<br/>
	Resolution: 1920 X 1080<br/>
</p>

3. Render result of the default standford bunny scene with rabbit as lambert material and other objects as phong reflective:

<p align="center">
  <img src="docs&images/render_result_standford_rabbit_lambert_softshadow_softreflection.jpg">
</p>

<p align="center">
	Samples per pixel: 4<br/>
	Samples per ray: 4<br/>
	Epsilon: 0.001000<br/>
	Ray Jitter Epsilon: 0.100000<br/>
	Max bounces: 2<br/>
	Field of view: 60.00<br/>
	Resolution: 1920 X 1080<br/>
</p>

4. Render result of the a bunch of spheres in a mirrored room:

<p align="center">
  <img src="docs&images/render_result_mirror_sphere_room.jpg">
</p>

<p align="center">
	Samples per pixel: 4<br/>
	Samples per ray: 4<br/>
	Epsilon: 0.001000<br/>
	Ray Jitter Epsilon: 0.010000<br/>
	Max bounces: 3<br/>
	Field of view: 60.00<br/>
	Resolution: 1827 X 1077<br/>
</p>

5. Render result of three reflective cubes under a point light:

<p align="center">
  <img src="docs&images/render_result_pointlight_cubes.jpg">
</p>

<p align="center">
	Samples per pixel: 4<br/>
	Samples per ray: 4<br/>
	Epsilon: 0.001000<br/>
	Ray Jitter Epsilon: 0.100000<br/>
	Max bounces: 3<br/>
	Field of view: 54.5<br/>
	Resolution: 1920 X 1080<br/>
</p>

5. Render result of three glass balls under a point light (the white artifacts are due to full reflectivity set by the material):

<p align="center">
  <img src="docs&images/render_result_glass_balls.jpg">
</p>

<p align="center">
	Samples per pixel: 4<br/>
	Samples per ray: 5<br/>
	Epsilon: 0.001000<br/>
	Ray Jitter Epsilon: 0.100000<br/>
	Max bounces: 3<br/>
	Max refractive bounces: 5<br/>
	Field of view: 54.50<br/>
	Resolution: 1920 X 1080<br/>
</p>

6. Render result of a bunch of bunnies with different materials:

<p align="center">
  <img src="docs&images/render_result_bunnies.jpg">
</p>

<p align="center">
	Samples per pixel: 4<br/>
	Samples per ray: 5<br/>
	Epsilon: 0.001000<br/>
	Ray Jitter Epsilon: 0.100000<br/>
	Max bounces: 3<br/>
	Max refractive bounces: 3<br/>
	Field of view: 54.50<br/>
	Resolution: 1920 X 1080<br/>
</p>

# USAGE
This project is platform independant and uses cmake to build. You do need an **Nvidia GTX GPU** if you plan to build the Cuda version.

## [General Build Steps]
Should work for any IDEs as long as you have cmake version >= 2.8.

1. ‘cd’ into your clone directory, type into the Terminal the following step by step
2. mkdir build
3. cd build
4. cmake ..
5. make
6. run the raytracer.exe in build
7. Supply the root folder of your scene to load, otherwise default to existent one called testScene (make sure your textures and materials are named properly!)
8. if you wish to use custom setup for options, type the options line by line in a txt file in build and specify it when program opens.
9. there is a template options.txt inside src, you can copy, rename and modify it to start quicker.

## [For Windows Visual Studio Build Only]
This is the exact repulication of the implementer's build process.

1. ‘cd’ into your clone directory, type into the Terminal the following step by step
2. mkdir build
3. cd build
4. cmake ..
5. Open Assignment2.sin in build folder with Visual Studio
6. Build entire solution
7. Switch to Folder View in the solution explorer
8. Select Debug-x64 and specify target to be raytracer.exe
9. Build

# Features
1. Multiple model file formats supporting.
2. Material editing and creating.
3. Viewport customization.
4. Midpoint Partitioning BVH Tree acceleration for ray hits.
5. Render result saving to jpeg image on program exit.

## Accepted Model Files:
The files accepted by this raytracer will be synced with the accepted file types of Assimp.

**Collada** ( .dae, .xml )<br/>
**Blender** ( .blend )<br/>
**Biovision BVH** ( .bvh )<br/>
**3D Studio Max 3DS** ( .3ds )<br/>
**3D Studio Max ASE** ( .ase )<br/>
**Wavefront Object** ( .obj )<br/>
**Stanford Polygon Library** ( .ply )<br/>
**AutoCAD DXF** ( .dxf )<br/>
**IFC-STEP** ( .ifc )<br/>
**Neutral File Format** ( .nff )<br/>
**Sense8 WorldToolkit** ( .nff )<br/>
**Valve Model** ( .smd, .vta )<br/>
**Quake I** ( .mdl )<br/>
**Quake II** ( .md2 )<br/>
**Quake III** ( .md3 )<br/>
**Quake 3 BSP** ( .pk3 )<br/>
**RtCW** ( .mdc )<br/>
**Doom 3** ( .md5mesh, .md5anim, .md5camera )<br/>
**DirectX X** ( .x )<br/>
**Quick3D** ( .q3o, .q3s )<br/>
**Raw Triangles** ( .raw )<br/>
**AC3D** ( .ac )<br/>
**Stereolithography** ( .stl )<br/>
**Autodesk DXF** ( .dxf )<br/>
**Irrlicht Mesh** ( .irrmesh, .xml )<br/>
**Irrlicht Scene** ( .irr, .xml )<br/>
**Object File Format** ( .off )<br/>
**Terragen Terrain** ( .ter )<br/>
**3D GameStudio Model** ( .mdl )<br/>
**3D GameStudio Terrain** ( .hmp )<br/>
**Ogre** ( .mesh.xml, .skeleton.xml, .material )<br/>
**Milkshape 3D** ( .ms3d )<br/>
**LightWave Model** ( .lwo )<br/>
**LightWave Scene** ( .lws )<br/>
**Modo Model** ( .lxo )<br/>
**CharacterStudio Motion** ( .csm )<br/>
**Stanford Ply** ( .ply )<br/>
**TrueSpace** ( .cob, .scn )

## Keyboard features
1. [Space]                Start a new rendering frame, if rendering then suspend/continue the render process.
2. [F]                    Make the program fullscreen.
4. [Up/Down/Left/Right]   Move the rendered image around.
3. [Esc]/[Q]              Quit the program.

## Command Line Options:
The command line options are taken as a txt file to be specified when program starts for convenience. 
You can check and modify the options.txt file in src to cook your own options.

• **-dispw n**

This sets the display width of the viewport to be n pixels. n is a positive nozero integer.

• **-disph n**

This sets the display height of the viewport to be n pixels. n is a positive nozero integer.

• **-mpbs n**

This sets the display/calculation domain of a multi-threading block, bigger number actually boosts speed since we are rasterizing less often. n is a positive nonzero integer.

• **-hfov f**

This sets the the horizontal field of view angle for the camera in degrees. A recommended value is 54.43 for human eyes. f is a positive nonzero float.

• **-spp n**

This sets the anti-aliasing samples per pixel used by the raytracer, higher means smoother/slower. n is a positive nonzero integer.

• **-bounce n**

This sets the maximum bouncing time of each light reflect, higher means slower/better viusals. n is a positive integer.

• **-spr n**

This sets the samples splitted out from each ray after a hit for anti-aliasing and soft shadows. n is a positive nonzero integer.

• **-eps f**

This sets the epsilon amount of next ray offset to avoid self-collison. f should be a fairly small positive float.

• **-reps f**

This sets the epsilon amount of splitted ray's offset from the parent ray, higher means softer and more random splits. f should be a fairly small positive float.

• **-rbounce n**

This sets the maximum bounce depth of a refractive ray. n is a positive nonzero integer.

# Additonal Features

