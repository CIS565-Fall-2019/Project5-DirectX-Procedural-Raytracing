DirectX Procedural Raytracing
============================

DUE DATES:
- **Sunday, October 19**: Answers to [Conteptual Questions](https://github.com/CIS565-Fall-2019/Project5-DirectX-Procedural-Raytracing/blob/master/INSTRUCTIONS.md#conceptual-questions) are due. [for submission: create a branch called `conceptual-questions` and open a pull request from that branch to submit. make sure to follow usual guidelines for PR formatting]
- **Sunday, October 27th**: The full assignment is due.

THE ASSIGNMENT:

**Summary:** In this project, you will learn how to use the newly released DirectX Raytracing API to ray trace procedural geometries. The project is structured such that you're introduced to DXR concepts in multiple steps, starting with CPU-side code for setting up the rendering pipeline, and finishing up with the actual rendering pipeline execution through GPU-side code.

<p align="center">
  <img src="https://github.com/CIS565-Fall-2019/Project5-DirectX-Procedural-Raytracing/blob/master/images/render.png">
</p>

Note on terminology: `DirectX` is a graphics API developed by Microsoft. `DirectX Raytracing` (DXR) is an API extension that supports GPU raytracing capabilities (mostly NVIDIA high end GPUs), that is to say that the API supports operations closely tied to the concept of raytracing.

# Part 0: DXR-Compatibility & Building the Project & Conceptual Questions
## DXR-Compatibility
Go back to [Project0-Getting-Started](https://github.com/CIS565-Fall-2019/Project0-Getting-Started) and make sure Part 2.3 works correctly.

## Building the Project
1. Open the VS Solution named `DXR-Project`.
2. Make `D3D12RaytracingProceduralGeometry` as `Start-Up Project`
3. Build both `Release` and `Debug`. This will build the `Fallback Layer` with it too.
4. **Important Note**: running the solution won't do anything unless the CPU code works perfectly. We added a flag to `Main.cpp` which you should set to 1 once you complete this entire CPU section. Basically, when you move on to writing shader/GPU code, the flag should always be set to 1.

<p align="center">
  <img src="https://github.com/CIS565-Fall-2019/Project5-DirectX-Procedural-Raytracing/blob/master/images/flag.png">
</p>

## Conceptual Questions
This part is due Sunday, October 19th.

Answer these conceptual questions. They may help you gain a solid understanding of Raytracing and DXR. Include your answers in your README, then make a GitHub pull request highlighting your answers:

1. Ray tracing begins by firing off rays from the camera's perspective, with 1 ray corresponding to 1 pixel. Say the viewport is (1280 by 720), **how would you convert these pixel locations into rays**, with each ray being defined by an `Origin` and a `Direction`, such that `Ray = Origin + t * Direction`? Consult this [intro](https://www.scratchapixel.com/lessons/3d-basic-rendering/computing-pixel-coordinates-of-3d-point/mathematics-computing-2d-coordinates-of-3d-points) to camera transformations and this [explanation](http://webglfactory.blogspot.com/2011/05/how-to-convert-world-to-screen.html) of world-to-screen/screen-to-world space article to formulate an answer in your own words.
2. Each procedural geometry can be defined using 3 things: the `Axis-Aligned Bounding Box` (AABB) (e.g. bottom left corner at (-1,-1,-1) and top right corner at (1,1,1)) that surrounds it, the `Type` (e.g. Sphere) of the procedural geometry contained within the AABB, and an `Equation` describing the procedural geometry (e.g. Sphere: `(x - center)^2 = r^2`). **Using these 3 constructs, conceptually explain how one could go about rendering the procedural geometry**. To be specific, consider how to proceed when a ray enters the AABB of the procedural geometry.
3. **Draw a diagram of the DXR Top-Level/Bottom-Level Acceleration Structures** of the following scene. Refer to section 2.6 below for an explanation of DXR Acceleration Structures. We require that you limit your answer to 1 TLAS. You may use multiple BLASes, but you must define the Geometry contained within each BLAS.

<p align="center">
  <img src="https://github.com/CIS565-Fall-2019/Project5-DirectX-Procedural-Raytracing/blob/master/images/scene.png">
</p>


# Part 1: Project Description
This DXR project will teach you how to **(1)** build a DXR rendering pipeline and **(2)** raytrace procedural geometries using the pipeline.

For the unfamiliar, ray tracing is a process similar to path tracing, except that it is deterministic (no more probabilities!) and that we only do a single pass over the entire scene (no more multiple iterations). This image summarizes what goes on in ray tracing:

<p align="center">
  <img src="https://github.com/CIS565-Fall-2019/Project5-DirectX-Procedural-Raytracing/blob/master/images/raytrace.jpg">
</p>

Specifically, the DXR execution pipeline mimics all the interactions depicted above. This diagram summarizes the DXR execution pipeline:

<p align="center">
  <img src="https://github.com/CIS565-Fall-2019/Project5-DirectX-Procedural-Raytracing/blob/master/images/pipeline.png">
</p>

This does not prevent us from calling `TraceRay()` multiple times. In fact, any self-respecting raytracing project will allow multiple (~3) `TraceRay()` calls. The common denominator between ray and path tracing is the depth of the ray. In this project, we use a *minimum depth of 3* to allow tracing the following:

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

The algorithm sounds simple enough when serialized, but the challenge is to make it performant and fast. Enter: **GPUs**. Deploying a raytracing program on the GPU is not trivial. The difficulty of raytracing lies in the fact that **ALL data must be available before any raytracing happens**: 

    * all geometries must be positioned within an acceleration structure (KD-Tree, Bounding Volume Hierarchy, or whatever your choice is..)
    * the camera must be set up
    * the light sources must be configured
    * the shading logic must be configured
    * and the output buffer must be ready
In essence, the entire *ray tracing pipeline* must be ready on the GPU. In rasterization, this does not need to hold: you can render shadows *after* you render diffuse colors for example. So a good chunk of DXR is spent setting that up from the CPU. Once the GPU knows about all the details on the pipeline, it can execute the ray tracing algorithm. The DXR API is made to facilitate setting up all of these things, so you will spend a good amount of time learning how to do that.

# Part 1: Understanding (some of) the codebase
NOTE: These are relatively unimportant files (well, for the sake of your time!):

      * DirectXRaytracingHelper.h
      * PerformanceTimers.h/.cpp
      * StepTimer.h
      * DXProjectHelper.h

We've heavily documented a lot of the code since setting up a DXR project can be a convoluted process. Specifically, we've thrown the keyword `LOOKAT` for you to look for interesting things. To begin, search the codebase using the following section numbers. For example, to read about the interesting parts of `Main.cpp`, go to the file `Main.cpp` then `CTRL-F` for `LOOKAT-1.0`. These `LOOKAT`s are positioned above "interesting" or helpful code.

These files will contain code you probably won't touch, but should look at once:

    * LOOKAT-1.0: Main.cpp
    * LOOKAT-1.1: Win32Application.h
        * LOOKAT-1.1.0: Win32Application.cpp
    * LOOKAT-1.2: DXProject.h
        * LOOKAT-1.2.0: DXProject.cpp
    * LOOKAT-1.3: DeviceResources.h
    * LOOKAT-1.4: stdafx.cpp

Now, we've structured the assignment into multiple parts, starting with a section that deals with CPU-side code (DirectX-Raytracing CPU API code), and finishing off with a section dealing with GPU-side code (DirectX-Raytracing GPU API code).
To make it easier on you, we divided wach section (CPU or GPU) into multiple part that chronologically makes sense (with respect to the pipeline). Each part will be more or less self contained in a single source file, with each source file containing the familiar `LOOKAT`s above, with additional `TODO`s that **you must code up**.

Hopefully the process will be as simple as (1) reading the comments contained within `LOOKAT`s, then (2) filling in the instructions within the `TODO`s. We also encourage you to have the [DirectX documentation](https://docs.microsoft.com/en-us/windows/win32/directx) handy just in case a concept/struct is unclear.

**Each section will have a number labeling it (e.g: `3.4.1 - AABB and Spheres`). Use that number to look for `TODO`s (e.g: `TODO-3.4.1`).**

# Part 2: CPU-side DXR Code
## Summary
To begin, we advise you to look into the following files for `LOOKAT`s. These are files you do not need to edit. However, they might give you a good sense of what to expect from the CPU-side DXR code.

    * LOOKAT-1.5: RaytracingSceneDefines.h
    * LOOKAT-1.6: DXR-Structs.h
    * LOOKAT-1.7: DXProceduralProject.h
    * DXR-* files:
        * LOOKAT-1.8.0: DXR-AppLifeCyclecpp.cpp
        * LOOKAT-1.8.1: DXR-Common.cpp
        * LOOKAT-1.8.2: DXR-DescriptorHeap.cpp
        * LOOKAT-1.8.3: DXR-Other.cpp

These files will contain important CPU-side code used to setup the DXR pipeline. You will write most of your CPU code somewhere here.

    * LOOKAT-2.1: DXR-DynamicBuffers.cpp
    * LOOKAT-2.2: DXR-RootSignature.cpp
    * LOOKAT-2.3: DXR-ShaderNames.cpp, DXR-HitGroup.cpp
    * LOOKAT-2.4: DXR-Pipeline.cpp
    * LOOKAT-2.5: DXR-Geometry.cpp
    * LOOKAT-2.6: DXR-AccelerationStructure.cpp
    * LOOKAT-2.7: DXR-ShaderNames.cpp, DXR-ShaderTable.cpp
    * LOOKAT-2.8: DXR-DoRaytracing.cpp

## 2.1 - Scene Data, Constant Buffers, Structured Buffers
We begin by concerning ourselves by how to send scene data to the GPU. The scene contains a camera, lights, colors for objects, as well as transforms for the objects to render. To send this data to the GPU, we need to (1) populate the structs with the values we need, then (2) somehow map this data onto the CPU in a region that the GPU can read from (this is called allocating and uploading to the GPU).

The allocation and uploading parts are done using the struct `GpuUploadBuffer` defined in `DXR-Structs.h`. Specifically, we use 2 types of `GpuUploadBuffer`s in this section:

  * `ConstantBuffer`: a resource that holds a single struct. Constant is a misnomer: this struct might change each frame.
  * `StructuredBuffer`: a resource that holds multiple structs. Kind of like an array of ConstantBuffers.

Each `GpuUploadBuffer` is allocated as a [CommittedResource](https://docs.microsoft.com/en-us/windows/win32/direct3d12/uploading-resources). See the section on `CommittedResource`s in the provided link for more info.

We implemented this for the scene `ConstantBuffer`. You will be doing a similar thing for the `StructuredBuffer`s in the scene.

Files to checkout:
* DXR-DynamicBuffers.cpp
    * 3 TODOs

## 2.2 - Root Signatures
We need some way to programmatically read/write to various data from the GPU. The GPU itself needs to know where to allow us programmers to programmatically access the data we send to it. This is why we have root signatures. Root signatures are like function parameters: there are global parameteres that can be accessed by any GPU function that we define, and local parameters that only some shaders have access to.

Note: a root parameter can be initialized as a descriptor of some type of resource (`ConstantBufferView`, `ShaderResourceView`, or `UniformAccessView`) or as constant values. Read about descriptors [here](https://docs.microsoft.com/en-us/windows/win32/direct3d12/descriptors-overview).

**Illustrative Example**: 

You can tell the GPU to put the render target on `register 0` of type `UniformAccessView`. This is done by accomplishing 2 things:
1. filling in a global root signature that allocates a **slot** for the `UniformAccessView` on `register 0`.  **This is the part you will be doing.**
2. making sure you respect this slot you defined by doing the following in the GPU code:
    ```RWTexture2D<float4> g_renderTarget : register(u0); // output texture```. **We do this for you**

Somewhere down the line, you will need to fill in a descriptor of type `UniformAccessView` that tells the GPU how the resource (the render target in our example) should be read/written to. We do this by doing the following:
1. define the width, height, size, of this resource
2. create a resource on the GPU that fits those parameters
3. allocate a descriptor on the CPU that will describe how to read/write this resource
4. tell the GPU that it should allocate a descriptor on its side that matches the one defined on the CPU. This will give us a **descriptor pointer** on the GPU.
If you're curious to see the process, check out `CreateRaytracingOutputResource()` in `DXR-Other.cpp` (we've implemented it for you).

You must be wondering: now that I have a slot for this resource, and a descriptor that tells the GPU how to read said resource, how do I link the slot with the descriptor? It's like you're baking a cake: you made the batter (resource/descriptor), you have a pan (descriptor slot), but you still need to put the batter in the pan to bake the cake! Well, when time comes to execute the rendering pipeline, we tell the command list associated with the application that it should link the **slot** previously defined with the **descriptor pointer** also previously defined. We will get to this in section 2.8.

Files to checkout:
* DXR-RootSignature.cpp
    * 4 TODOs

## 2.3 - Hit Groups
So far we've been doing standard DirectX stuff (no raytracing involved). Now we will turn to do some DXR related stuff. Specifically, we will be creating hitgroup subobjects that will be built into the pipeline. The definition of a hitgroup is a `Closest Hit Shader`, at least one `Intersection Shader` (if the primitive is a triangle, this is not needed: triangle intersection is built into DXR), and an optional `Any Hit Shader` for transparency testing (which we won't do). Visually, the hitgroup is this:

<p align="center">
  <img src="https://github.com/CIS565-Fall-2019/Project5-DirectX-Procedural-Raytracing/blob/master/images/hitgroup.png">
</p>

What's as important as creating these hitgroups is binding a local root signature to them, which you defined in the previous section (they live in `m_raytracingLocalRootSignature`).

Files to checkout:
* DXR-ShaderNames.cpp
    * No TODOs
* DXR-HitGroups.cpp
    * 4 TODOs

## 2.4 - Ray Tracing Pipeline State Object (RTPSO)
Surprisingly, what you've done so far (in addition to other things we've done for you such as comiling the shaders in the form of a DXIL library - a compiled "dll" of shaders), is enough to create the RTPSO. An RTPSO represents a full set of shaders reachable by a `DispatchRays()` call, with all configuration options resolved such as local signatures and other state. This section will show you how to finalize creating the RTPSO. The actual geometry data will be brought at a later stage.

Files to checkout:
* DXR-Pipeline.cpp
    * 3 TODOs

## 2.5 - Geometry Data
The next most important part of rendering is unsurprisingly the shapes/data to render! This section will show you how to allocate and upload data to the GPU in the form of (1) triangle data (vertices, indices) and (2) procedural geometry data (axis-aligned bounding boxes, or AABBs).

Files to checkout:
* DXR-Geometry.cpp
    * 4 TODOs

## 2.6 - Acceleration Structure
Raytracing is an expensive process. The most prevalent way to boost performance is to create an acceleration structure around the geometry to be rendered. This acceleration structure defines a set of rules to be followed by the ray, which allows the ray to skip unnecessary geometry that it will never intersect with. The cool thing about DXR is that it has a built in acceleration structure generation. You just need to pass in the geometry data in a specified way and it will built straight onto the GPU.

On a high level, the entire scene is divided into `Top Level Acceleration Structures` (TLAS), which themselves hold multiple **instances** of `Bottom Level Acceleration Structures` (BLAS). In turn, a BLAS holds geometry data (the data you defined and uploaded before). Here is a visualization that explains this:

<p align="center">
  <img src="https://github.com/CIS565-Fall-2019/Project5-DirectX-Procedural-Raytracing/blob/master/images/accel.png">
</p>

In our project, we only have 1 TLAS, which in turn has 1 **instance** of a Triangle BLAS, and 1 **instance** of an AABB BLAS. The Triangle BLAS holds triangle data that will be used to render a horizontal plane, on top of which our other geometries will be drawn. The AABB BLAS holds *multiple* AABBs, one for each procedural geometry to render. This graph summarizes the project's AS:

<p align="center">
  <img src="https://github.com/CIS565-Fall-2019/Project5-DirectX-Procedural-Raytracing/blob/master/images/accelexplained.png">
</p>

In DXR, building the Acceleration Structure requires multiple steps:
1. Build geometry descriptors that describe how the geometry data is laid out. We will have 1 geometry descriptor per unique primitive. Example: 1 plane data (triangles), 1 box data, 1 sphere data, 1 metaballs data.
2. Build the bottom-level acceleration strucutre for each **grouping** of geometries of your choice. We will have 1 BLAS holding the triangle data, and 1 other BLAS holding all other procedural data.
    * The geometry descriptors you built will be passed in as acceleration structure inputs
    * You then query the *pre-build* info for these bottom-level AS. This will output 2 things: scratch size, and result data max size. Scratch size is like extra memory the driver needs to build the AS, and result data max is an upper bound for the size of the AS. You need to allocate 2 buffers for both of these.
    * Finally, you tell the command list that you want to build the acceleration structure using the scratch and result data allocated.
3. Create a function that builds **instances** of your BLAS. An instance of a BLAS is basically a BLAS but with a specific world-space transform. If you were to spawn multiple boxes in your scene, you would not create multiple box BLAS - you would create only one, but 4. Build the top-level acceleration structure. This is very similar to step (2) except now your inputs to the AS is the bottom-level AS. You will need to additionally call the function you created in step (3) to describe the instances that will be held by your TLAS.

Files to checkout:
* DXR-AccelerationStructure.cpp
    * 22 TODOs (many of them are one liners)

## 2.7 - Shader Tables
To allow GPU code (shaders) to execute differently depending on the type of the ray, the type of geometry the ray intersects with, and other parameters, DXR offers an abstraction called `Shader Table`s. At its core, a `Shader Table` is a `GpuUploadBuffer`, just like a `ConstantBuffer` or a `StructuredBuffer`. The data it holds is in the form of `Shader Records`, with each `Shader Record = {ShaderID, LocalRootArguments}`. It is important to note that a shader table contains a record of shaders of the same type (raygen, or miss, or hitgroups).

We first need to get an ID for every single shader we plan to have. We do this by querying the RTPSO for "spots", which give us back shader IDs.

Finally, we create the `GpuUploadBuffer` for each shader table. We then insert shader records one by one, each pointing to optional local root arguments. We save these for later usage.

Files to checkout:
* DXR-ShaderNames.cpp
    * No TODOs
* DXR-ShaderTable.cpp
    * 6 TODOs

## 2.8 - Executing the Command List, a.k.a DispatchRays()
Everything you've been doing so far has been preparing all the resources the GPU needs to do the raytracing. Right now, they all live somewhere on the GPU, but nothing actually glues them together in a coherent way that allows the GPU to do correct raytracing. This part will glue everything together. The main mechanism that does this is functions of the type `Set...()` called on the command list. After everything is "set", we proceed to call `DispatchRays()` on the command list, which acts like a CUDA kernel: it will dispatch one thread per pixel, each thread taking on the role of a ray.

Files to checkout:
* DXR-DoRaytracing.cpp
    * 7 TODOs

# Part 3: GPU-side DXR Code
## Summary
These files will contain important GPU-side code used to do the actual raytracing. You will write most of your GPU/shader code somewhere here:

    * LOOKAT-1.9.0: RaytracingHlslCompat.h
    * LOOKAT-1.9.1: RaytracingShaderHelper.hlsli
    * LOOKAT-1.9.2: ProceduralPrimitivesLibrary.hlsli
    * LOOKAT-1.9.3: AnalyticPrimitives.hlsli
    * LOOKAT-1.9.4: VolumetricPrimitives.hlsli
    * LOOKAT-1.9.5: Raytracing.hlsl

## 3.1 - Ray Generation
Implement ray generation. This is very similar to path tracing ray generation. Rays are emitted from the camera, with each pixel having a corresponding ray. The rays should be output in world coordinates. When ray generation is complete, you should see a very dark image with slight ambient occlusion going on [adjust brightness settings / check pixel-by-pixel but this image is not actually 100% black though it may seem to be]:

<p align="center">
  <img src="https://github.com/CIS565-Fall-2019/Project5-DirectX-Procedural-Raytracing/blob/master/images/after-ray-gen.png">
</p>

Files to checkout:
* RaytracingShaderHelper.hlsli
    * 1 TODO
* Raytracing.hlsl
    * 1 TODO

## 3.2 - Trace Ray
Implement the functions that call the [TraceRay()](https://docs.microsoft.com/en-us/windows/win32/direct3d12/traceray-function) DXR function. We implemented the radiance ray one. You should implement one for the shadow ray.

Files to checkout:
* Raytracing.hlsl
    * 1 TODO

## 3.3 - Miss Shaders
Implement what happens when a ray hits absolutely nothing in the acceleration structure. You should implement one for the radiance ray, and another for the shadow ray.

Files to checkout:
* Raytracing.hlsl
    * 2 TODOs

## 3.4 - Intersection Shaders
Implement the intersection shaders that should (1) compute the hit point and (2) the normal at that hit point. Triangle intersections are automatically done by the DXR driver, but any other custom intersections (say, for spheres, boxes, metaballs) have to be manually coded.

### 3.4.1 - AABB and Spheres
We have implemented the AABB intersection in AnalyticPrimitives.hlsli, and implemented the intersection function for a single hollow sphere. We ask you to extend the sphere intersection to support intersection multiple spheres (see AnalyticPrimitives.hlsli).

Files to checkout:
* ProceduralPrimitivesLibrary.hlsli
    * LOOKAT-3.4.1
* Raytracing.hlsl
    * LOOKAT-3.4.1
* AnalyticPrimitives.hlsli
    * 1 TODO

### 3.4.2 - Metaballs
Section 3.4.1 should have given you a good understanding on how intersection shaders are configured and called and what they end up doing. We ask you to implement a similar process but for Metaballs. Metaballs (a.k.a blobs) are special spheres: they have this *potential* that causes adjacent spheres to merge together. This process can be rendered with a special technique using potential thresholding.

At a high level:
1. We begin by identifying the beginning intersection point around these spheres, and the end intersection point.
2. We raymarch between this beginning and end point with a fixed number of steps.
3. At each raymarched point, we compute the sum potential caused by the metaballs. If this potential exceeds some threshold, we may render the point (conditioned on it being not culled). Otherwsie, we keep raymarching.
  * If we choose to render the point, we compute a normal vector at that point using the change in potential (derivatives = change in the function).

Rendering static Metaballs is not so interesting. We want to see clear shape deformations. Therefore, you should implemented to animation interpolant in `RaytracingShaderHelper.hlsli`, which will be used in initializing the Metaball positions.

Files to checkout:
* ProceduralPrimitivesLibrary.hlsli
    * LOOKAT-3.4.2
* Raytracing.hlsl
    * 1 TODO
* VolumetricPrimitives.hlsli
    * 3 TODOs
* RaytracingShaderHelper.hlsli
    * 1 TODO

## 3.5 - Phong Lighting Model Shading, Schlick's Approximation
Implement the [Phong](https://en.wikipedia.org/wiki/Phong_reflection_model) lighting model (not Blinn-Phong, just Phong). See the `Description` section in the Wikipedia article for all the details you need.

Also implement [Schlick's Approximation](https://en.wikipedia.org/wiki/Schlick%27s_approximation) for Fresnel reflection effects. This should be done in the file `RaytracingShaderHelper.hlsli`.

Files to checkout:
* Raytracing.hlsl
    * 3 TODOs
* RaytracingShaderHelper.hlsli
    * 1 TODO

## 3.6 - Closest Hit Shaders
A closest hit shader is what gets called after the rays have finished intersecting with all possible geometries in the acceleration structure. At the end, only one geometry wins: the closest one.
We have implemented the closest hit shader for triangle intersections, which you may use a reference. We ask you to (1) implement a distance falloff function inside the triangle closest hit shader so that faraway triangles are slightly blurred and (2) implement the closest hit shader for a procedural geometry.

Files to checkout:
* Raytracing.hlsl
    * 2 TODOs

# Part 4 - Performance Analysis
Do a short performance analysis on the effects of ray depth on FPS. Try values between 3 and 10. We expect a nice labeled graph.

# Part 5 - Submission

## README
As usual, we expect a polished README highlighting what you've accomplished.
Consider that you can use the following keys to move the scene around, so use those to take shots of the renders you make.
* C - enable/disable camera animation.
* G - enable/disable geometry animation.
* L - enable/disable light animation.

## Submit
Open a GitHub pull request so that we can see that you have finished.
The title should be "Project 5: YOUR NAME".
The template of the comment section of your pull request is attached below, you can do some copy and paste:  

* [Repo Link](https://link-to-your-repo)
* (Briefly) Mentions features that you've completed. Especially those bells and whistles you want to highlight
    * Feature 0
    * Feature 1
    * ...
* **Feedback on the project itself. This is the first DXR project of CIS 565, so we do need you to post feedback so we can improve the project for future years.**


And you're done!


# Part 6 - Extra Credit Options
## Prettier Scene
**+5 points**

Add more spheres/metaballs/boxes to be rendered in the scene. We expect a performance analysis that showcases the FPS effects the more geometries you render.

## GLTF Mesh Loading
**+10 points, extra for loading textures**

You may use [tinyGLTF](https://github.com/syoyo/tinygltf) loader for this. This is **very** similar to loading triangles onto the GPU (just like we did to render the plane). Please credit tinyGLTF if you do use their loader.

## Signed Distance Fields
**+10 points, extra depending on rendered primitives**

[Signed distance fields](https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm) are another type of procedural geometries that are easy to put together. This will require you to add another type of AABBs (SDFs) in addition to the ones we have (Analytic, Volumetric).

## Optimizing Metaball Rendering
**+7 points**

Our metaball rendering algorithm is very inefficient even for a small number of metaballs. The key lies in the intersection: if a ray does not intersect a metaball, then its potential effect on the ray is guaranteed to be 0. Use this idea to only loop over metaballs that are *active*. Showcase performance improvement by rendering 5 metaballs with and without this optimization.

## Multiple Light Sources and Other Lens Effects
**+10 points, extra for other lens effects**

The scene we have only has 1 light source. Extend the raytracer to support at least 3 light sources. This will require editing the scene constant buffer to contain new light sources, and will require editing the lighting model to support adding multiple colors at once. See [this article](https://www.scratchapixel.com/lessons/3d-basic-rendering/introduction-to-shading/shading-multiple-lights) for an overview of how to handle multiple lights.
*Other Lens Effects*: Add Anti-Aliasing and Depth of Field for extra points.

# Credits
This project was heavily based on the fallback layer DXR sample made by Microsoft. We modified the project to be more assignment-friendly. Here are the license details:

```
//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************
```
