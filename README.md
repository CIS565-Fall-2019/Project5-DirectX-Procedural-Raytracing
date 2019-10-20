**University of Pennsylvania, CIS 565: GPU Programming and Architecture,
Project 5 - DirectX Procedural Raytracing**

* Disha Jindal: [Linkedin](https://www.linkedin.com/in/disha-jindal/)
* Tested on: Windows 10 Education, Intel(R) Core(TM) i7-6700 CPU @ 3.40GHz 16GB, NVIDIA Quadro P1000 @ 4GB (Moore 100B Lab)

## Conceptual Questions

1. Ray tracing begins by firing off rays from the camera's perspective, with 1 ray corresponding to 1 pixel. Say the viewport is (1280 by 720), how would you convert these pixel locations into rays, with each ray being defined by an Origin and a Direction, such that Ray = Origin + t * Direction? Consult this intro to camera transformations and this explanation of world-to-screen/screen-to-world space article to formulate an answer in your own words.

**Problem Statement** We know the location of each pixel in the pixel cordinates and the task is to convery them into rays which is with respect to the world coordinates.

Four main coordinate systems in this are:
 - World Coordinate System
 - Camera Coordinate System
 - Screen Coordinate System
 - Normalized Screen Coordinate System
 
**Steps to convert the pixel from Screen Coordinate System to a ray in World Coordinate System:**
-  **Screen Coordinate System to Normalized Screen Coordinate System**: Pixels are in the range (0 to 1280, 0 to 720) and we need to bring them into (-1 to 1, -1 to 1). So, shift the center from (0,0) to (640,360) and divide by (640,360). This is the perspective projection of the points w.r.t the camera coordinate system on to the canvas.

- **Camera Coordinate System to World Coordinate System**: Next we need to convert these points to the world co-ordinate system. Given, the camera coordinates, we have world projection matrix from which we can calculate the screen to world projection matrix by taking the inverse. Multiplying this matrix with the camera coordinates give the world coordinates.

- **Ray Generation**: Now, we the origin of the ray as well the direction it is going in, both of them in the world coordinates. So, we can use the following equation to calculate the ray for each pixel:
Ray = Origin + t * Direction.

2. Each procedural geometry can be defined using 3 things: the Axis-Aligned Bounding Box (AABB) (e.g. bottom left corner at (-1,-1,-1) and top right corner at (1,1,1)) that surrounds it, the Type (e.g. Sphere) of the procedural geometry contained within the AABB, and an Equation describing the procedural geometry (e.g. Sphere: (x - center)^2 = r^2). Using these 3 constructs, conceptually explain how one could go about rendering the procedural geometry. To be specific, consider how to proceed when a ray enters the AABB of the procedural geometry.

To render a pixel, we need the intersection point of the ray with the object and the surface normal. Using these we can decide the direction of the subsequent ray as well as the color at that pixel. Now, procedural geometry can be defined using AABB, the type of the shape and the equation describing the procedural geometry. This equation is defined w.r.t. AABB coordinate system but the ray is defined w.r.t the world coordinates. So, given the bottom left and top right corners of AABB, we can calculate the center which is the origin of AABB coordinate system. Using this point, we have the world projection matrix from which we can calculate AABB to world projection matrix by taking the inverse. Multiply this inverse with the ray coordinates would give ray coordinates in the AABB coordinate system. Now both equations are in the same coordinate system, we can calculate t and surface normal using intersection shaders. Finally, we can shade the point using these and can also decide the direction of the outgoing ray.

3. Draw a diagram of the DXR Top-Level/Bottom-Level Acceleration Structures of the following scene. Refer to section 2.6 below for an explanation of DXR Acceleration Structures. We require that you limit your answer to 1 TLAS. You may use multiple BLASes, but you must define the Geometry contained within each BLAS.

