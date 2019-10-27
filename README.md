**University of Pennsylvania, CIS 565: GPU Programming and Architecture,
Project 5 - DirectX Procedural Raytracing**

* Disha Jindal: [Linkedin](https://www.linkedin.com/in/disha-jindal/)
* Tested on: 

## DirectX Procedural Raytracing
<p align="center"><img src="https://github.com/DishaJindal/Project5-DirectX-Procedural-Raytracing/blob/master/images/camera_geo_light_anime.gif"></p>
 
## Introduction
The aim of this project is to raytrace procedural geometries using the DXR rendering pipeline. DirectX Raytracing (DXR) is basically a DirectX API added for real-time raytracing. 

**Ray Tracing**

Ray tracing is a process similar to path tracing, except that it is deterministic without any probabilities and that we only do a single pass over the entire scene, unlike multiple iterations in path tracing. There are three types of rays: Radiance Ray (Rays generated from camera), Shadow Ray (To check if the ray from the intersection point to the light source hits some object on the way or not) and Reflection Ray (It is generated if the material is reflective). This image summarizes what goes on in ray tracing:

<p align="center"><img src="https://github.com/DishaJindal/Project5-DirectX-Procedural-Raytracing/blob/master/images/raytrace.jpg" width="400"></p>

**DXR Pipeline**

Specifically, the DXR execution pipeline mimics all the interactions depicted above. This diagram summarizes the DXR execution pipeline:

<p align="center"><img src="https://github.com/DishaJindal/Project5-DirectX-Procedural-Raytracing/blob/master/images/hitgroup.png" width="400"></p>

## Animations
### No Animation

<p align="center"><img src="https://github.com/DishaJindal/Project5-DirectX-Procedural-Raytracing/blob/master/images/no_anime.PNG" width="600"></p>

### Geometric Animation
<p align="center"><img src="https://github.com/DishaJindal/Project5-DirectX-Procedural-Raytracing/blob/master/images/geo_anime.gif" width="600"></p>

### Light Animation
<p align="center"><img src="https://github.com/DishaJindal/Project5-DirectX-Procedural-Raytracing/blob/master/images/light_anime.gif" width="600"></p>

### Camera Animation
<p align="center"><img src="https://github.com/DishaJindal/Project5-DirectX-Procedural-Raytracing/blob/master/images/camera_anime.gif" width="600"></p>

## Implementation

### Effect of Closest Hit Shaders
<p align="center"><img src="https://github.com/DishaJindal/Project5-DirectX-Procedural-Raytracing/blob/master/images/before_3.6.PNG" width="600"></p>

### Effect of Distance Fog
<p align="center"><img src="https://github.com/DishaJindal/Project5-DirectX-Procedural-Raytracing/blob/master/images/without_fog_after3.6.PNG" width="600"></p>

## Performance Analysis
<p align="center"><img src="https://github.com/DishaJindal/Project5-DirectX-Procedural-Raytracing/blob/master/images/fps_ray_depth.png" width="600"></p>

## Bloopers

### Reversed y-axis
<p align="center"><img src="https://github.com/DishaJindal/Project5-DirectX-Procedural-Raytracing/blob/master/images/blooper1_axis.PNG" width="600"></p>

### Background Not Far Enough
<p align="center"><img src="https://github.com/DishaJindal/Project5-DirectX-Procedural-Raytracing/blob/master/images/blooper2_background_not_far_enough.PNG" width="600"></p>

### Visibility Falloff
<p align="center"><img src="https://github.com/DishaJindal/Project5-DirectX-Procedural-Raytracing/blob/master/images/blooper3_visibility_falloff.PNG" width="600"></p>

