**University of Pennsylvania, CIS 565: GPU Programming and Architecture,
Project 5 - DirectX Procedural Raytracing**

* Disha Jindal: [Linkedin](https://www.linkedin.com/in/disha-jindal/)
* Tested on: Windows 10 Education, Intel(R) Core(TM) i7-6700 CPU @ 3.40GHz 16GB, NVIDIA Quadro P1000 @ 4GB (Moore 100B Lab)

## Conceptual Questions

1. Ray tracing begins by firing off rays from the camera's perspective, with 1 ray corresponding to 1 pixel. Say the viewport is (1280 by 720), how would you convert these pixel locations into rays, with each ray being defined by an Origin and a Direction, such that Ray = Origin + t * Direction? Consult this intro to camera transformations and this explanation of world-to-screen/screen-to-world space article to formulate an answer in your own words.

**Problem Statement** Given a canvas of size 1280 by 720, the task is to convert each pixel on this canvas into a ray with respect to the world coordinates.

Point in various coordinate systems:
 - World Coordinate System: Pw
 - Screen Coordinate System: Ps
 - Normalized Screen Coordinate System: Pn
 
**Steps to convert the pixel from Screen Coordinate System to a ray in World Coordinate System:**
-  **Screen Coordinate System to Normalized Screen Coordinate System**: Pixels are in the range (0 to 1280, 0 to 720) and we need to bring them into (-1 to 1, -1 to 1). So, W = 1280, H = 720 
<p align="center"><img src="https://github.com/DishaJindal/Project5-DirectX-Procedural-Raytracing/blob/conceptual-questions/images/s_ndc.png"></p>

- **Camera Coordinate System to World Coordinate System**: Next, we need to convert these points to the world co-ordinate system. Given the camera coordinates, we have world projection matrix (M) from which we can calculate the screen to world projection matrix by taking the inverse. Multiplying this matrix with the camera coordinates give the world coordinates.
<p align="center"><img src="https://github.com/DishaJindal/Project5-DirectX-Procedural-Raytracing/blob/conceptual-questions/images/nw.png"></p>

- **Ray Generation**: Now, we have the origin of the ray (Camera coordinates) as well the direction it is going in, both of them in the world coordinates. So, we can use the following equation to calculate the ray for each pixel:
<p align="center"><img src="https://github.com/DishaJindal/Project5-DirectX-Procedural-Raytracing/blob/conceptual-questions/images/ray.png"></p>

2. Each procedural geometry can be defined using 3 things: the Axis-Aligned Bounding Box (AABB) (e.g. bottom left corner at (-1,-1,-1) and top right corner at (1,1,1)) that surrounds it, the Type (e.g. Sphere) of the procedural geometry contained within the AABB, and an Equation describing the procedural geometry (e.g. Sphere: (x - center)^2 = r^2). Using these 3 constructs, conceptually explain how one could go about rendering the procedural geometry. To be specific, consider how to proceed when a ray enters the AABB of the procedural geometry.

To renter the procedural geometry, we'll shoot rays at each pixel through the camera center. To render each pixel, we need the intersection point of the ray with the object and the surface normal. Using these we can decide the direction of the subsequent ray as well as the color at that pixel. Now, procedural geometry is defined using AABB, the type of the shape and the equation describing the procedural geometry. This equation is defined w.r.t. AABB coordinate system but the ray is defined w.r.t the world coordinates. So, given the bottom left and top right corners of AABB, we can calculate the center which is the origin of AABB coordinate system. 

Using this point, we have the world projection matrix from which we can calculate AABB to world projection matrix by taking the inverse. Multiply this inverse with the ray coordinates would give ray coordinates in the AABB coordinate system. Now both equations are in the same coordinate system, we can calculate t and surface normal using the appropriate intersection shader. Finally, we can shade the point using these and can also decide the direction of the outgoing ray. In case of reflective surfaces, we'll generate a reflective ray and a shadow ray. If the shadow ray hits an object on the way to light, that pixel is shadowed.

3. Draw a diagram of the DXR Top-Level/Bottom-Level Acceleration Structures of the following scene. Refer to section 2.6 below for an explanation of DXR Acceleration Structures. We require that you limit your answer to 1 TLAS. You may use multiple BLASes, but you must define the Geometry contained within each BLAS.

<p align="center"><img src="https://github.com/DishaJindal/Project5-DirectX-Procedural-Raytracing/blob/conceptual-questions/images/scene.png" width="500"></p>

**DXR Top-Level/Bottom-Level Acceleration Structures**

<p align="center"><img src="https://github.com/DishaJindal/Project5-DirectX-Procedural-Raytracing/blob/conceptual-questions/images/acc_str.png" width="700"></p>
