**University of Pennsylvania, CIS 565: GPU Programming and Architecture,
Project 5 - DirectX Procedural Raytracing**

* SOMANSHU AGARWAL
  * [LinkedIn](https://www.linkedin.com/in/somanshu25)
* Tested on: Windows 10, i7-6700 @ 3.4GHz 16GB, Quadro P1000 4GB (Moore 100B Lab)

### Table of Contents

1. [Introduction](#Introduction)
2. [RayTracing](#RayTracing)
3. [Code Section](#Code-Section)
4. [Performace Analysis](#Performace-Analysis)
5. [Bloopers](#Bloopers)

### Introduction
The project aims at doing ray tracing rendering of a scene using DXR API. The steps in the project helped to learn how the pipeline is generated by using CPU code and how the logic for rendering is written using GPU kernels and parallel computation is used for processing. The readme gives the brief understanding about the raytracing, about the CPU and GPU codes and finally a performace analysis of the DXR rattrcing with vrying the maximum depth of a ray. I have included some bloopers in the end to mention some of my mistakes I was ding while coding in the project.

### RayTracing

Raytracing is the technique used to render scenes and created the images. In raytracing, when a ray from the camera through the pixel co-rodinate of the screen space to the object, which is `radiance ray`,, we find the position and the normal corresponding to the intersection between the object geomentry and the ray. To render the object at the position, we create a light ray which goes to the light source (it's a point source in raytracing) and check whether it is blocked by some other geometry on the way to the light source. If it is blocked, then it's called a `shadow ray` and we render the shadow(black) color at that position. If it goes directly to the light source, we render the color of the obejct. We render the backgorund color if it misses our place surface(made of two triangles) and geometries, it is rendered as a background color. If the object is reflective, then we generate two rays, one which goes to the light source and the other as reflective ray which follows the same logic as radiance ray above. The ray is meant ot be dead if it bounces more than the maximum recursion depth of the ray. 

<p align="center"><img src="https://github.com/somanshu25/Project5-DirectX-Procedural-Raytracing/blob/master/images/raytrace.jpg" width="500"/></p>

### Code Section:

While coding, the parts are divided into tow main parts: CPU and GPU section codes.

a. In CPU section, we create the pipeline which could be later on used by the GPU section to render. Note that in raytracing, we need to make sure that all the data is available when the GPU is doing the rendering as compared to rasterization. So, we store the geometries in CPU section of the code and for GPU to access the GPU section, we create dynamic allocation of the memory in the heap and define the dscriptors which gives us the mapping of the CPU and GPU pointers. Here, to create a scene, we first create a transformation of all the objects in their local space (each of then can have their own coordinate space) to the bottom level space which has the same reference coordinate system. After getting the bottom level, we then create the transformation to the scene by creating the instance of the bottom level geometries (also called as Bottom Level Accelearted Structure here) and creating a scene as the Top-Level Accelerated Structure. In our case, we are dealing with two types of geometries, geometries defined by vertices and indices and the other as procedural geometires defined through Axis Aligned Geometry Box(AABB).

<p align="center"><img src="https://github.com/somanshu25/Project5-DirectX-Procedural-Raytracing/blob/master/images/pipeline.png" width="500"/></p>

b. In GPU section, we perform the ray generation from the camera which goes from the camera to the world space with the given information of the pixel co-ordinate of the sreeen space and the camera to workd projection transformation. 

