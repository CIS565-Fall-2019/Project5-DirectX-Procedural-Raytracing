**University of Pennsylvania, CIS 565: GPU Programming and Architecture,
Project 5 - DirectX Procedural Raytracing**

* Disha Jindal: [Linkedin](https://www.linkedin.com/in/disha-jindal/)
* Tested on: Windows 10, Intel(R) Core(TM) i7-6700 CPU @ 3.40GHz 16GB, GTX 1080 (SIGLAB)

## DirectX Procedural Raytracing
<p align="center"><img src="https://github.com/DishaJindal/Project5-DirectX-Procedural-Raytracing/blob/master/images/camera_geo_light_anime.gif"></p>
 
## Introduction
The aim of this project is to raytrace procedural geometries using the DXR rendering pipeline. DirectX Raytracing (DXR) is basically a DirectX API added for real-time raytracing. 

**Ray Tracing**

Ray tracing is a process similar to path tracing, except that it is deterministic without any probabilities and that we only do a single pass over the entire scene, unlike multiple iterations in path tracing. There are three types of rays: Radiance Ray (Rays generated from camera), Shadow Ray (To check if the ray from the intersection point to the light source hits some object on the way or not) and Reflection Ray (It is generated if the material is reflective). This image summarizes what goes on in ray tracing:

<p align="center"><img src="https://github.com/DishaJindal/Project5-DirectX-Procedural-Raytracing/blob/master/images/raytrace.jpg" width="400"></p>

**DXR Pipeline**

The DXR execution pipeline mimics all the interactions depicted below. Since raytracing is an expensive process, the most prevalent way to boost performance is to skip unnecessary geometry that the ray will never intersect with. DXR has a built in support for acceleration structures. This acceleration structures are created around the geometry to be rendered and it defines a set of rules to be followed by the ray. This diagram summarizes the DXR execution pipeline:

<p align="center"><img src="https://github.com/DishaJindal/Project5-DirectX-Procedural-Raytracing/blob/master/images/hitgroup.png" width="400"></p>

## Animations

The gif on the top of the page has three animations in it: geometry, light and camera. Without any animation, the scene looks like below:

<p align="center"><img src="https://github.com/DishaJindal/Project5-DirectX-Procedural-Raytracing/blob/master/images/no_anime.PNG" width="600"></p>

Following are the images with three different animations on the above image. The first one shows geometric animation where the objects are moving. The one is the middle has fixed camera and objects but the light position is moving. Finally, in the last one object and lights are fixed but the camera viewpoint is moving.


Geometric Animation        |  Light Animation          |  Camera Animation 
:-------------------------:|:-------------------------:|:-------------------------:
![](https://github.com/DishaJindal/Project5-DirectX-Procedural-Raytracing/blob/master/images/geo_anime.gif)  |  ![](https://github.com/DishaJindal/Project5-DirectX-Procedural-Raytracing/blob/master/images/light_anime.gif)  |  ![](https://github.com/DishaJindal/Project5-DirectX-Procedural-Raytracing/blob/master/images/camera_anime.gif)

## Implementation
There are two main implementation components: the first one is to set up the graphics pipeline in CPU and the second one is GPU side code for raytracing. The first part of pipeline setup included creating the scene data which included a camera, lights, colors for objects, as well as transforms for the objects to render and then, mapping this data onto the CPU in a region that the GPU can read from. The next part was to create the global and local [root signatures](https://docs.microsoft.com/en-us/windows/win32/direct3d12/root-signatures). Then, we created the hitgroups which consists of a `Closest Hit Shader`, an `Intersection Shader` and an optional `Any Hit Shader`. Then, we glued everything created so far into a Ray Tracing Pipeline State Object and then we do the most important part creating Top Level and Bottom Level `Acceleratiion Structures`. Finally, we conclude the pipeline creation by fill in the shader tables and the command lists.

After the pipeline creation, the next part to raytrace in the GPU code. The first part to generate rays (radiance rays) and then write functions to trace radiance and shadow rays. Then we implement the miss shaders which are required to handle the case when a ray hits absolutely nothing in the acceleration structure and then the intersection shaders for spheres, boxes, and metaballs to compute the hit point and the normal. Then, we implement various lighting effects: [Phong Lighting Model Shading](https://en.wikipedia.org/wiki/Phong_reflection_model), [Schlick's Approximation](https://en.wikipedia.org/wiki/Schlick%27s_approximation). Then, we implement the closest hit shader to select the winning object out of all intersecting ones. Following is the illustration of some of these effects:

Without Closed Hit Shaders |  With Closed Hit Shaders      
:-------------------------:|:-------------------------:
![](https://github.com/DishaJindal/Project5-DirectX-Procedural-Raytracing/blob/master/images/before_3.6.PNG)  |  ![](https://github.com/DishaJindal/Project5-DirectX-Procedural-Raytracing/blob/master/images/no_anime.PNG) 

Without Distance falloff function  |  With Distance falloff function   
:---------------------------------:|:---------------------------------:
![](https://github.com/DishaJindal/Project5-DirectX-Procedural-Raytracing/blob/master/images/without_fog_after3.6.PNG)  |  ![](https://github.com/DishaJindal/Project5-DirectX-Procedural-Raytracing/blob/master/images/no_anime.PNG) 

## Performance Analysis
Following plot shows the performance of ray tracing with the ray depth. The x-axis denotes the maximum ray depth which goes from 3-10 and the y-axis shows the frames per second. The results are pretty straightforward as the performance is dropping as we increase the ray depth. 
<p align="center"><img src="https://github.com/DishaJindal/Project5-DirectX-Procedural-Raytracing/blob/master/images/fps_ray_depth.png" width="600"></p>

## Bloopers
Following bloopers are a result of some of the bugs introduced while working on this:

**Upside Down** 
This is the result of my incorrect ray generation logic. Initially in pixel coordinates, (0,0) is the top left corner of the canvas. I normalized it to bring it to bring every pixel from (-1,1) range but missed that for DXR (-1,-1) should be bottom left not top left. 

<p align="center"><img src="https://github.com/DishaJindal/Project5-DirectX-Procedural-Raytracing/blob/master/images/blooper1_axis.PNG" width="400"></p>

**Visibility Falloff For AABB**
Incorrect configuration for visibility falloff for AABB's, led to this:
<p align="center"><img src="https://github.com/DishaJindal/Project5-DirectX-Procedural-Raytracing/blob/master/images/blooper3_visibility_falloff.PNG" width="400"></p>

**Background Not Far Enough For Triangles**
We implemented distance falloff function inside the triangle closest hit shader so that faraway triangles are slightly blurred. By not properly configuring the far away function, the background overshadowed the ground.

<p align="center"><img src="https://github.com/DishaJindal/Project5-DirectX-Procedural-Raytracing/blob/master/images/blooper2_background_not_far_enough.PNG" width="400"></p>
