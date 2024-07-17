<h1 align="center"><b>Guiding Light</b></h3>

<p align="center">
  <img src="readme_files/game.webp" />
</p>

## What is it?
Guiding Light is a game developed along it's own game engine in 4 months, for courses conducted by the [Lodz University of Technology](https://p.lodz.pl/).

## Story
You're a lighthouse keeper and a courier… at once. In "Guiding Light", a casual time-management game, you operate alone in the Pole of Cold. And you’re the only one who can feed the penguins! Guide the ships, pick-up the food rations, and upgrade your lighthouse. Get ready to prevent shipwrecks and master the polar night!

## How to play?
Go to: https://oelj.itch.io/guiding-light

## How do I build and run this?
Run the following command to generate the build system:
```
cmake -B build
```
or use the `build.bat` which does the same.

To compile and run, C++ compiler with C++23 support is required. MSVC 14.40 has been tested.

`.sln` file is located in the generated `/build` directory.
You can open it in Visual Studio, choose the desired build configuration (`Debug` is the default), and simply run it.

## Engine
Our Engine is written in modern C++23; its architecture is based on **Object-Component model used by Unity**. We use **DirectX 11** as our graphics API (initially it was OpenGL, but we decided to port it). Some notable systems:
- 2D physics engine (collision detection and resolution)
- Resource Manager (for sharing resources like models and textures)
- UI
- Runtime prefab loading
- Input and Event systems

## Editor
<p align="center">
  <img src="readme_files/engine.gif" />
</p>
We developed our own editor along with countless tools and functionalities:

* Scene and prefab loading
* Gizmos
* Curve editors
* Debug drawings
* Debug logging
* Shader hot-swapping
* Custom inspectors for specific Components

## EngineHeaderTool
To speed up our work, we wrote a Python script that generates (de)serialization code
for all Components by parsing the C++ header files, similarly to [UnrealHeaderTool](https://docs.unrealengine.com/4.27/en-US/ProductionPipelines/BuildTools/UnrealHeaderTool/). (We are very proud of that.)

## Rendering
We are using deferred rendering, with exceptions for transparent objects and UI that use forward rendering.
We managed to implement a couple of rendering algorithms:
* Screen Space Reflection
* Screen Space Refraction
* Volumetric light scattering
* Screen Space Ambient Occlusion (SSAO)
* Particles (and particle emitters, done on CPU)
* Fast Approximate Anti-Aliasing (FXAA)
* Gerstner waves (for water geometry)
* Shadow mapping (including point lights)
* Percentage-Closer Soft Shadows

## Thirdparty
Editor's UI is made with **[ImGui](https://github.com/ocornut/imgui)** and it's extensions: **[ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo), [ImPlot](https://github.com/epezent/implot)**. Audio is handled by **[miniaudio](https://github.com/mackron/miniaudio)**. **[glm](https://github.com/g-truc/glm)** for additional math operations. (De)serializing yaml: **[yaml-cpp](https://github.com/jbeder/yaml-cpp)**. Window abstraction **[glfw](https://github.com/glfw/glfw)**. Helper loading libraries used: **[DDSTextureLoader](https://github.com/Microsoft/DirectXTK/wiki/DDSTextureLoader), [stb_image](https://github.com/nothings/stb/blob/master/stb_image.h), [assimp](https://github.com/assimp/assimp)**.

## Awards
🏆 **[ZTGK 2024](https://gry.it.p.lodz.pl/main/index.php/pl/) Game Development category winner**

🏆 **[ZTGK 2024](https://gry.it.p.lodz.pl/main/index.php/pl/) Activision special award winner**

## Credits
| Name | Link | Role |
|------|--------|--------|
| Mikołaj Przybylski | https://github.com/0GreenClover0| Programming Lead |
| Michał Galiński | https://github.com/MikeMG-PL| Production Lead |
| Jakub Januszewicz  | https://github.com/boniffacy | 2D & Branding Lead |
| Miłosz Kawczyński  | https://github.com/MiloszKawczynski | Design Lead |
| Nadia Kowalska  | https://github.com/nadkow | 3D Art Lead |
| Hubert Olejnik | https://github.com/umbc1ok | Rendering Lead |
| Michał Świstak | | Sound Design |
| Julian Rakowski |https://www.linkedin.com/in/julian-rakowski/ |  Voice Acting |
