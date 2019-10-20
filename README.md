**University of Pennsylvania, CIS 565: GPU Programming and Architecture,
Project 5 - DirectX Procedural Raytracing**

* Somanshu Agarwal
  * [LinkedIn](https://www.linkedin.com/in/somanshu25/)


### Conceptual Questions

##### Q1. Ray tracing begins by firing off rays from the camera's perspective, with 1 ray corresponding to 1 pixel. Say the viewport is (1280 by 720), how would you convert these pixel locations into rays, with each ray being defined by an Origin and a Direction, such that Ray = Origin + t * Direction? Consult this intro to camera transformations and this explanation of world-to-screen/screen-to-world space article to formulate an answer in your own words.

Here, we can assume the ray will pass through the mid point of every pixel in the grid.

1. Hence, given the grid of 1280 by 720, we can find the normalized device coordinates (2D pint) with respect to the camera   space by using the below equations:

<p align="center"><img src="https://github.com/somanshu25/Project5-DirectX-Procedural-Raytracing/blob/master/images/NDC%20Coordinates.png" width="600"/></p>

Here, `P'x` and `P'y` represents the pixel coordinates of the bottom left of the grid of each pixel.

2 . After obtaining the mnormalized device coordinates, we find the world coordinates using the camera to world transformation matrix. 

P<sub>world</sub> = P'<sub>normalized</sub>M<sub>camera-to-world</sub>


We need to convert 3D world space point to 2D camera point, which is a projection on the point on the pixel.

So, for tranforming the 3D point first to the 3D camera space point, we use world to camera transformation matrix and multiply with the world coordinated to get the camera coordinates. 


##### Q2. Each procedural geometry can be defined using 3 things: the Axis-Aligned Bounding Box (AABB) (e.g. bottom left corner at (-1,-1,-1) and top right corner at (1,1,1)) that surrounds it, the Type (e.g. Sphere) of the procedural geometry contained within the AABB, and an Equation describing the procedural geometry (e.g. Sphere: (x - center)^2 = r^2). Using these 3 constructs, conceptually explain how one could go about rendering the procedural geometry. To be specific, consider how to proceed when a ray enters the AABB of the procedural geometry.

When a ray from the camera piercing through a unique pixel of the camera plane enters the Align Axis Bounding Box(AABB) of the procedural geometry, then the shader will check whether the ray is hitting the object or not. According the the AABB of the geometry type it entered, its corresponding intersection shader will perform intersection check with the help of the equation which is provided in the procedural geometry and the ray it intersected. Note that since, we know the pixel the ray came from, we will have the equation of ray known to us. The shader will determine the position and normal for the intersection and will generate a ray which would go the light source and other reflective ray if the object is reflective. Later on, we will check for the ray going to the light whether it is a shadow ray or not mand for reflective ray, we will have another similar recusrion with the same above procedure.

##### Q3. Draw a diagram of the DXR Top-Level/Bottom-Level Acceleration Structures of the following scene. Refer to section 2.6 below for an explanation of DXR Acceleration Structures. We require that you limit your answer to 1 TLAS. You may use multiple BLASes, but you must define the Geometry contained within each BLAS.

<p align="center"><img src="https://github.com/somanshu25/Project5-DirectX-Procedural-Raytracing/blob/master/images/scene.png" width="600"/></p>

##### Sol:

<p align="center"><img src="https://github.com/somanshu25/Project5-DirectX-Procedural-Raytracing/blob/master/images/accelerated_structure.png" width="600"/></p>
