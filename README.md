CUDA Path Tracer
================

**University of Pennsylvania, CIS 565: GPU Programming and Architecture, Project 2**

* Nathan Chortek
  * [LinkedIn](https://www.linkedin.com/in/nathan-chortek/), [personal website](https://www.nathanchortek.com/)
* Tested on: Windows 11, AMD Ryzen AI 9 HX 370, NVIDIA GeForce RTX 5090 Laptop GPU

<img src="img/cover.png">

## Build Instructions

NCHORTEK TODO
- Detail CMake Changes
- Describe required environment variables and third party libraries / downloads
- Describe how to run with different scenes (without editing the CMake file)

## Features Implemented

### Vulkan Hardware-Accelerated Ray Tracing (RT)

NCHORTEK TODO
- Overview write-up of the feature --> TLAS, BLAS, SBT, plus the core shaders: Raygen, Closest Hit, Miss
- Performance impact of the feature.
- If you did something to accelerate the feature, what did you do and why? --> Push Constants, Buffer Device Address
- Compare your GPU version of the feature to a HYPOTHETICAL CPU version (you don't have to implement it!). Does it benefit or suffer from being implemented on the GPU? --> Yes of course, the HW was literally made for this.
- How might this feature be optimized beyond your current implementation? --> SER

### Refraction & Dielectric Materials

Cornell Box (Open), 5000 Iterations, Max Depth 16

<img src="img/cornellOpen.png">

Cornell Box w/ Glass Sphere and Mirror Wall (Open), 5000 Iterations, Max Depth 16

<img src="img/cornellGlassSphereOpen.png">

NCHORTEK TODO
- Overview write-up of the feature

### Physically-Based Depth-of-Field (DOF)

Funhouse (Open) w/o DOF, 5000 Iterations, Max Depth 16

<img src="img/funhouseOpen.png">

Funhouse (Open) w/ DOF, 5000 Iterations, Max Depth 16

<img src="img/funhouseOpenDOF.png">

NCHORTEK TODO
- Overview write-up of the feature

### Direct Lighting & Multiple Importance Sampling (MIS)

Crystal Table (Open), Naive Integration, 5000 Iterations, Max Depth 16

<img src="img/crystalTableOpenNaive.png">

Crystal Table (Open), MIS Integration, 5000 Iterations, Max Depth 16

<img src="img/crystalTableOpenMIS.png">

NCHORTEK TODO
- Overview write-up of the feature --> Describe the light CDF/PDF, describe what MIS does at a high-level and why we want to combine bsdf and snap-to-light sampling methods rather than just picking one for direct light contribution, describe why direct light sampling is beneficial over naive path tracing where the only light contribution comes from a ray terminating after intersecting a light.

## Performance Analysis

### Naive Path Tracing

Crystal Table (Closed), Naive Integration, 5000 Iterations, Max Depth 16

<img src="img/crystalTableClosedNaive.png">

Crystal Table (Open), Naive Integration, 5000 Iterations, Max Depth 16

<img src="img/crystalTableOpenNaive.png">

Funhouse (Closed), Naive Integration, 5000 Iterations, Max Depth 16

<img src="img/funhouseClosedNaive.png">

Funhouse (Open), Naive Integration, 5000 Iterations, Max Depth 16

<img src="img/funhouseOpenNaive.png">

NCHORTEK TODO
- Describe the causes for noise differences in closed vs open scenes
- Describe the causes for FPS differences in closed vs open scenes

### Direct Lighting & Multiple Importance Sampling (MIS)

Crystal Table (Closed), MIS Integration, 5000 Iterations, Max Depth 16

<img src="img/crystalTableClosedMIS.png">

Crystal Table (Open), MIS Integration, 5000 Iterations, Max Depth 16

<img src="img/crystalTableOpenMIS.png">

Funhouse (Closed), MIS Integration, 5000 Iterations, Max Depth 16

<img src="img/funhouseClosedMIS.png">

Funhouse (Open), MIS Integration, 5000 Iterations, Max Depth 16

<img src="img/funhouseOpenMIS.png">

NCHORTEK TODO
- Describe the causes for noise differences in closed vs open scenes
- Describe the causes for FPS differences in closed vs open scenes

## References

### Articles

https://www.willusher.io/graphics/2019/11/20/the-sbt-three-ways/

https://developer.nvidia.com/blog/vulkan-raytracing/

### Tutorials

https://vulkan-tutorial.com/

https://nvpro-samples.github.io/vk_raytracing_tutorial_KHR/tutorial/

### Other CIS Courses

#### CIS 5660 Procedural Graphics

cube.h, icosphere.h - Primitive mesh generators adapted from *HW 0: Intro to Javascript and WebGL*

https://github.com/nchortek/hw00-intro-base/blob/main/src/geometry/Cube.ts
https://github.com/nchortek/hw00-intro-base/blob/main/src/geometry/Icosphere.ts

#### CIS 5610 Advanced Rendering

bsdf.glsl, math.glsl, raygen.rgen, sampleWarping.glsl - BSDF, sampling, and integrator logic adapted from *HW7: Global Illumination*
