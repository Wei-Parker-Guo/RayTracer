# CS 184 Fall 2016
Instructor: James O'Brien

# Implementer of This Project
Wei Guo: wei.parker.guo.sg@gmail.com

# References (Books & Used Libs Mainly)
1. GLFW Version 3.3.2
2. GLEW
3. ASSIMP Version 5.0.1
4. CUDA
5. linmath.h under GLFW Dependencies
6. Code references are enclosed as inline comments.

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

# Dummy Program Logic Flow

<p align="center">
  <img width="960" height="960" src="docs&images/dummy_logic_flow.jpg">
</p>

# Keyboard features
1. [Space]                Start a new rendering frame
2. [F]                    Make the program fullscreen
4. [Up/Down/Left/Right]   Move the rendered image around
3. [Esc]/[Q]              Quit the program

# Render Results
1. All the logs are recorded in logs.txt in raytracer.exe's directory.

# Features
1. Multiple model file formats supporting.
2. Material editing and creating.
3. Viewport customization.

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

## Command Line Options:
The command line options are taken as a txt file to be specified when program starts for convenience. 
You can check and modify the options.txt file in src to cook your own options.

• **-dispw n**

This sets the display width of the viewport to be n pixels.

• **-disph n**

This sets the display height of the viewport to be n pixels.

# Additonal Features

