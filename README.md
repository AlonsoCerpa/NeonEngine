# NeonEngine

Neon Engine is a 3D Game Engine that I implemented from scratch in C++ using OpenGL, glm, ImGui and Assimp.

The currently supported features are:
- PBR shading.
- Loading 3D models in the glTF format.
- Point lights, directional lights and spot lights.
- Load models animations.
- Bloom.
- Editable parameters of the Game Objects and the World via the User Interface.
- Load HDRI maps and show them as skyboxes.
- Rendering basic shapes: Cones, cylinders, cubes, spheres and disks.
- Translate, rotate and scale the game objects via widgets inside the viewport.
- Load materials.
- Change materials of the game objects via the UI.
- List all the current game objects in the UI.
- Etc.

Screenshot of Neon Engine:

![Neon Engine screenshot](https://github.com/AlonsoCerpa/NeonEngine/blob/master/images/neon_engine_image1.png)
<br />
<br />

## Requirements

- OS: Windows, tested with Windows 11
- IDE: Visual Studio, tested with Visual Studio 2022
- Package manager: vcpkg

## Installation
1. Download and install vcpkg
2. Download and install the following libraries using vcpkg (tested with the 64 bit versions):
- glad
- glm
- assimp
- glfw3
- imgui
- stb
3. Download and install Visual Studio 2022
4. Download this repository
5. Using Visual Studio 2022, open the Visual Studio solution file of this repo located in the following path: NeonEngine/NeonEngine.sln

## Demos

Demo doing transformations in Neon Engine:

![Neon Engine short demo](https://github.com/AlonsoCerpa/NeonEngine/blob/master/gifs/neon_engine_gif1.gif)
<br />
<br />


Demo changing the HDRI maps in Neon Engine:

![Neon Engine short demo](https://github.com/AlonsoCerpa/NeonEngine/blob/master/gifs/neon_engine_gif2.gif)
<br />
<br />


YouTube video of a demo of Neon Engine:

[![Neon Engine demo video](https://img.youtube.com/vi/rJXNfAThIbU/maxresdefault.jpg)](https://www.youtube.com/watch?v=rJXNfAThIbU)
