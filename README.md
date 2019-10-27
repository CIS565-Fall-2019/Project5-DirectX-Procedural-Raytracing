DXR Raytracing
================

**University of Pennsylvania, CIS 565: GPU Programming and Architecture, Project 3**

* John Marcao
  * [LinkedIn](https://www.linkedin.com/in/jmarcao/)
  * [Personal Website](https://jmarcao.github.io)
* Tested on: Windows 10, i5-4690K @ 3.50GHz, 8GB DDR3, RTX 2080 TI 3071MB (Personal)

<b>(Artifacts in the reflection are due to GIF encoding, will try to fix that)</b>

![](images/scene.gif)

# Project
This project went through the steps of setting up DXR Raytracing pipeline with triangle support, as well as simple geometries (spheres), Axis-Aligned Bounding Box (AABB), as well as volumetric Metaballs that become distorted with time.

The project was focused on simply understanding the API and applying the raytracing knowledge from Project 3 as well as additional geometries. I also perform some performance analysis on several parameters, including ray depth, metaball raymarching granularity, and number of elements in a metaball group.

# DXR API

The DXR API provides a lot of power, but also a lot of confusion. The API can handle triangles easily, but adding support for non-triangle geometries turned out to be complicated in my opinion. Below are some of the highlights from learning the API that can hopefully help somebody else out. DXR is all about structure. To automate a lot of steps, DXR just needs to be told about the layout of the data. From there, it can figure things out (such as triangle/ray intersections).

### Global and Local Root Signatures
In CUDA, arguments are defined through normal C-style function arguments. With the DXR API, arguments can be passed in through root signatures. Each GPU thread/shader will receive the same copy of the GlobalRootSignature. This makes it great for globally true values, such as lighting locations, acceleration structure data, etc. LocalRootSignatures, on the other hand, are unique for each shader. Each is structured the same, but the contents of the data can change. This allows shaders to get data on the specific geometry they are intersecting with. 

# Performance

I collected performance metrics on the DXR project by varying ray depth and metaball raymarching and complexity.

First, I increased the maximum ray depth parameter. The collected data is below.

![](images/raydepth_fps.png)

| Ray Depth | FPS    |
|-----------|--------|
| 3         | 550.92 |
| 4         | 510.75 |
| 5         | 482.51 |
| 6         | 471.74 |
| 7         | 486.36 |
| 8         | 467.83 |
| 9         | 477.81 |
| 10        | 471.27 |

As ray depth increases, the FPS goes down. This is expected, since a deeper depth adds more iterations of each ray. However, it also becomes clear that after a ray depth of 5, the difference is minimal. This is because most of the rays will, by that point, be executing on no-hit shaders. Those shaders perform minimal work and set the depth of ray to the maximum value, essentially removing them from the equation. 

Additionally, I analyzed the performance of the Metaball Raymarching algorithm. The algorithm looks at each point from the entry of the ray in the bounding box and the exit of the ray. It then calculates a "potential" based on the distance between the point and each other sphere. By modifying the steps taken, we can see how drastic the performance hit is of rendering these metaballs. I also include what each step variant looks like. As can be seen, The performance loss is pretty huge, which is expected since each ray needs to test against each sphere in the bounding box. However, the metaballs do not render "correctly" if not enough steps are taken. 

![](images/metablls.png)

| MB Steps | FPS    | GIF |
|----------|--------| ----|
| 2        | 1726   | ![](images/mb_2.gif) |
| 8        | 1660   | ![](images/mb_8.gif) |
| 32       | 1267   | ![](images/mb_32.gif) |
| 128      | 550.92 | ![](images/mb_128.gif) |
| 512      | 135    | ![](images/mb_512.gif) |

# Conceptual Questions

## Question 1
To get each pixel mapped to a ray, we need to make a couple transformations and mappings between the camera and the world. First of all, every object is mapped to the world space at first. Even the camera has a position and a normal in world space. So the first step is to take the local coordinates of the camera and transform it to world space. This is done by multiplying the camera's position by the transformation vector M<sub>camera-to-world</sub>.
This will move our coordinate system so that the camera is defined with respect to the world origin. We then have to map each (relevant) point in world space to camera space. This is done by mapping each point to a place one unit away from the camera point that is, in this case, 1280 pixels wide and 720 pixels tall. We start this by converting each point to be mapped with respect to the camera's position using the equation P<sub>Camera</sub> = P<sub>World</sub> * M<sub>World-to-Camera</sub>. The 3D points can then be mapped to 2D points on the Camera's plane by the following.

P'.x = P<sub>camera</sub>.x / -P<sub>camera</sub>.z

P'.y = P<sub>camera</sub>.y / -P<sub>camera</sub>.z

Note the negative sign, this is done because the camera is defined as looking down the negative Z-axis.
With the point P', we have define the position of a 3d object in the 2d plane seen by the camera. This space then is trimmed to match the height and width mentioned. If P'.x is greater than the width/2, or P'/y is greater than height/2, then the object will not be rendered in the screen.
The last step is to transform the point P' to raster space, that is such that its coordinate will align exactly with its pixel position on the screen. This is done by taking the origin to be the top left corner of the screen. Each point is normalized by the following.

P'<sub>normalized</sub>.x = (P'.x + width/2) / width

P'<sub>normalized</sub>.y = (P'.y + height/2) / height

And then finally, it is multiplied by the width and height provided.

P'<sub>raster</sub>.x = floor(P'<sub>normalized</sub>.x * Pixel Width)

P'<sub>raster</sub>.y = floor((1 - P'<sub>normalized</sub>.y) * Pixel Height)

Again, noting that y is inverted because the origin is the top left of the screen.

## Question 2
Each procedural geometry is defined by its Axis-Aligned Bounding Box (AABB), its Type, and its Type's associated Equation. When detecting collisions for each ray, the collision is first checked against the AABB. If the AABB is hit, then a more complicated check follows. Otherwise, the ray ignores the geometry. If a collision is detected with the AABB, then the shader calls a intersection test function depending on the Shape of the object. For example, a Sphere is defined by the function (x-center)<sup>2</sup> = r<sup>2</sup>. The ray is checked against this internal geometry to see if the collision happens or not. The origin and direction of the ray, with respect to the AABB, is tested against the geometry inside. If a collision is detected, then the rasterizer will render the ray at the point of intersection. If no intersection occurs inside the AABB, then no point is rendered.

## Question 3
Acceleration on the GPU is done by using Top Level Acceleration Structures (TLAS) and Bottom Level Acceleration Structures (BLAS). Each TLAS holds possibly many instances of BLAS. A BLAS may be referenced by multiple TLAS. For the example shown below...

![](images/scene.png)

... the acceleration structures can be defined like one of the two examples below. The top example assigns each instance in the TLAS to a different geometry type, while the second example joins together simple geometries together.

![](images/accel_example.png)