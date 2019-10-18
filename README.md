# DirectX Procedural Raytracing
**University of Pennsylvania, CIS 565: GPU Programming and Architecture, Project 5**

* Jiangping Xu
  * [LinkedIn](https://www.linkedin.com/in/jiangping-xu-365b19134/)

## Conceptual Questions
1. Ray tracing begins by firing off rays from the camera's perspective, with 1 ray corresponding to 1 pixel. Say the viewport is (1280 by 720), **how would you convert these pixel locations into rays**, with each ray being defined by an `Origin` and a `Direction`, such that `Ray = Origin + t * Direction`? Consult this [intro](https://www.scratchapixel.com/lessons/3d-basic-rendering/computing-pixel-coordinates-of-3d-point/mathematics-computing-2d-coordinates-of-3d-points) to camera transformations and this [explanation](http://webglfactory.blogspot.com/2011/05/how-to-convert-world-to-screen.html) of world-to-screen/screen-to-world space article to formulate an answer in your own words.

__Answer:__ For each pixel `p_screen(i, j)` in screen space, we first transform it to NDC.
`p_ndc(sx, sy) = (2 * i / screen_width - 1, 1 - 2 * j / screen_height)`
where screen_width = 1280, screen_height = 720 in the question. Then we can calculate the direction of the ray by

<p align="center">
    V = U * len * tan(alpha)
    <br>
    H = R * len * tan(alpha) * aspect_ratio
    <br>
    direction = dis * F + sx * H + sy * V
</p>

where F, U, R is the forward, up and right direction of camera space respectively, and alpha is half of the fov. The origin of the ray is the camera position in world space.


<p align="center">
    <img src = images/illustration1.png>
</p>

2. Each procedural geometry can be defined using 3 things: the `Axis-Aligned Bounding Box` (AABB) (e.g. bottom left corner at (-1,-1,-1) and top right corner at (1,1,1)) that surrounds it, the `Type` (e.g. Sphere) of the procedural geometry contained within the AABB, and an `Equation` describing the procedural geometry (e.g. Sphere: `(x - center)^2 = r^2`). **Using these 3 constructs, conceptually explain how one could go about rendering the procedural geometry**. To be specific, consider how to proceed when a ray enters the AABB of the procedural geometry.

We need the closet intersection point and the corresponding normal at that point to render the procedural geometry.

Intersection Point: there are basically two ways to get the intersection. The first is to solve the equations of the ray and the geometry. This method is more accurate but only applies to some types of geometry that has an analytic expression of the answer to the equations. The other method is raymarching, i.e. marching along the ray from the entry point of the bounding box to the exit point. For each step we evaluate the equation of the geometry. If it's smaller than a threshold, we approximate the current position as the intersection point. If not, we step forward along the ray for a small distance and repeat the evaluation.

Surface Normal: there are also two ways to calculate the normal: analytic way and numerical way. For simple geometry like sphere, we can calculate the normal by calculate the direction from sphere center to the intersection point. For complex procedural geometries, we can calculate the normal at a point P of a geometry defined by f(p) by the following expression

<p align="center">
    normal = (f(P + dx) - f(P), f(P + dy) - f(P), f(P + dz) - f(P))
</p>

3. **Draw a diagram of the DXR Top-Level/Bottom-Level Acceleration Structures** of the following scene. Refer to section 2.6 below for an explanation of DXR Acceleration Structures. We require that you limit your answer to 1 TLAS. You may use multiple BLASes, but you must define the Geometry contained within each BLAS.
<p align="center">
    <img src = images/illustration2.png>
</p>

