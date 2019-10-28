**University of Pennsylvania, CIS 565: GPU Programming and Architecture,
Project 5 - DirectX Procedural Raytracing**

Peyman Norouzi
* [LinkedIn](https://www.linkedin.com/in/peymannorouzi)
* Tested on: Windows 10, Xeon E5 @ 3.70GHz 32GB, GTX 1070 8192MB (SIG LAB Second Computer from Left)

## Ray Tracing:

<p align="center">
  <img src="images/top2.gif">
</p>

In computer graphics, ray tracing is a rendering technique for generating photo realistic images. In this approach, we trace paths of light as they leave from a camera as pixels in an image plane and simulating the effects of them encountering with virtual objects. In my one of previous projects, I did implement Path tracer on cuda which is a subset of ray tracing method. You can look the previous project [here](https://github.com/pnorouzi/Project3-CUDA-Path-Tracer).


## Table of Contents:

- [CUDA Path Tracing Implementation](#cuda-path-tracing-implementation)
  * [Core Implementation](#core-implementation)
  * [Core Implementation + Anti-Aliasing](#core-implementation-+-anti-aliasing)
  * [Core Implementation + Anti-Aliasing + Motion Blur](#core-implementation-+-anti-aliasing-+-motion-blur)
- [Perfomance Implementation and Analysis](#perfomance-implementation-and-analysis)
  * [Stream Compaction](#stream-compaction)
  * [First bounce intersections Caching](#first-bounce-intersections-caching)
  * [Material Sort](#material-sort)
- [Cool Renders](#cool-renders)
- [Bloopers](#bloopers)


## DirectX Procedural Ray Tracing:

As explained earlier, Ray Tracing, is a process in which we can produce/render photo realistic images by firing rays through a imaginary camera and then following the illumination of objects in the scene with a set of pre-defined rules. The process is simillar to path tracing except the fact that it is deterministiv and the tracing process only needs a single pass over the scene. The basic idea of the implementation can be seen below:

<p align="center">
  <img src="images/raytrace.jpg">
</p>


### DXR Implementation:

I used the DirectX 12 Raytracing (DXR) API for the implementation. The basic idea and pipeline of the implementation can be understood using the following diagram:

<p align="center">
  <img src="images/pipeline.png">
</p>

In this project, we use a *minimum depth of 3*, which means we will be calling `TraceRay()` roughly 3 times. We will be tracing the following:

1. a **primary (radiance) ray** generated from the camera
2. a **shadow ray** just in case the ray hits a geometry on its way to the light source
3. a **reflection** ray in case the material of the object is reflective

Therefore, the lifecycle of a single ray can be thought of as follows:

1. generate a ray, see if it hits something
2. if it hits something, then attempt to *light/color* it
    * attempting to color that hit point is equivalent to **tracing that ray towards the light source**. 
    * if that ray hits *another* object on its way to the light, then the region is effectively shadowed
    * if not, then we successfully colored that point
3. if at any point we hit a reflective material, then trace another ray in the reflected direction and repeat the process

To be able to implement such algrithm on the GPU (to make it fast and performant) we need to make all of the following data available in the GPU before any tracing happens:

    * all geometries must be positioned within an acceleration structure (KD-Tree, Bounding Volume Hierarchy, or whatever your choice is..)
    * the camera must be set up
    * the light sources must be configured
    * the shading logic must be configured
    * and the output buffer must be ready
    
In essence, the entire *ray tracing pipeline* must be ready on the GPU. In rasterization, this does not need to hold: you can render shadows *after* you render diffuse colors for example. So a good chunk of DXR is spent setting that up from the CPU. Once the GPU knows about all the details on the pipeline, it can execute the ray tracing algorithm. **This is where the DXR API would be very helpful since it would allow us to set up all of these things easier**.

## Perfomance Implementation and Analysis:

In the naive approach, we track each rays motion and bounce, throughout its journey until our depth requirement is met. But this is not the best and most efficient way to approach this since many rays will be terminating their journey earlier by either hitting the light source or a diffusing surface.


## Cool Renders:

Here are some cool renders that shows how powerfull DXR ray tracing can be. For all of the renders bellow, the maximum Recurssion Depth is 3.

In the following the light source is dynamically rotating slowly across the scene creating realistic shadows of the objects in the scene:

<p align="center">
  <img src="images/light.gif">
</p>

In addition to moving the light source, in the following render, the geometries are dynamically moving across the scene:

<p align="center">
  <img src="images/move.gif">
</p>



## Bloopers:

Here are some bloopers showing that everyone can and will make mistakes along the way. Especially in this project, there can be so many places that you can make a mistake. :) 

<img src="img/blooper_MB.png" width="280"> <img src="img/blooper_refract.PNG" width="280"> <img src="img/blooper_refract2.PNG" width="280">

## Sources:

https://docs.microsoft.com/en-us/windows/win32/directx


