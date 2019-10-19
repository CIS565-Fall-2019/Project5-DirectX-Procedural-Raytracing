**University of Pennsylvania, CIS 565: GPU Programming and Architecture,
Project 5 - DirectX Procedural Raytracing**

* Eric
  * 
* Tested on: gtx1660

### Conceptual Questions

# 1 Ray tracing begins by firing off rays from the camera's perspective, with 1 ray corresponding to 1 pixel. Say the viewport is (1280 by 720), how would you convert these pixel locations into rays, with each ray being defined by an Origin and a Direction, such that Ray = Origin + t * Direction? Consult this intro to camera transformations and this explanation of world-to-screen/screen-to-world space article to formulate an answer in your own words.
# 2 Each procedural geometry can be defined using 3 things: the Axis-Aligned Bounding Box (AABB) (e.g. bottom left corner at (-1,-1,-1) and top right corner at (1,1,1)) that surrounds it, the Type (e.g. Sphere) of the procedural geometry contained within the AABB, and an Equation describing the procedural geometry (e.g. Sphere: (x - center)^2 = r^2). Using these 3 constructs, conceptually explain how one could go about rendering the procedural geometry. To be specific, consider how to proceed when a ray enters the AABB of the procedural geometry.
# 3 Draw a diagram of the DXR Top-Level/Bottom-Level Acceleration Structures of the following scene. Refer to section 2.6 below for an explanation of DXR Acceleration Structures. We require that you limit your answer to 1 TLAS. You may use multiple BLASes, but you must define the Geometry contained within each BLAS.

![](images/TLAS_and_BLAS)
