# Computer Graphics course project - 2021/2022
3D application created using Vulkan in C++. 

Virtual museum in vaporwave style where you can observe paintings and discover some information about them.

## Commands
- Use WASD to move
- Keep pressing SHIFT to run
- Use the directional arrows or mouse for controlling the camera
- Keep pressing SPACE near a painting to visualize its card with some information
- Press M to pause/play the music

## Pipelines
There are 4 main pipelines, each one associated with different shaders:
- `P1` is associated with the main objects (museum and mountains). Ligths consists of a directional light, a spot light and an ambient light. The rendering is perfomed with Lambert diffuse, Phong specular and hemispheric ambient.
- `PMarble` is used for the statues. It is the same as `P1` but it uses Oren diffuse.
- `PC` for the cards UI. The rendering is perfomed with Lambert diffuse and uses a fixed orthographic projection to resemble a UI.
- `skyBoxPipeline` to render the skybox.

## Includes and libraries
- Vulkan SDK
- GLFW
- GLM
- tinyobjloader
- stb
- SDL2
- SDL2 mixer

## La Fabbrica del Vaporwave - Screenshots
![alt text](https://github.com/MarcelloDeSalvo/CG_Project/blob/b11242a7df3ebea1302a1988d65eb23441b4c927/readme_images/screen1.png)
![alt text](https://github.com/MarcelloDeSalvo/CG_Project/blob/0f4bb521f3d91db85618fe1fc7c800c61af9170d/readme_images/screen3.png)
![alt text](https://github.com/MarcelloDeSalvo/CG_Project/blob/0f4bb521f3d91db85618fe1fc7c800c61af9170d/readme_images/screen4.png)
