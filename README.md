**University of Pennsylvania, CIS 565: GPU Programming and Architecture,
Project 5 - DirectX Procedural Raytracing**

* Taylor Nelms
  * [LinkedIn](https://www.linkedin.com/in/taylor-k-7b2110191/), [twitter](https://twitter.com/nelms_taylor)
* Tested on: Windows 10, Intel i3 Coffee Lake 4-core 3.6GHz processor, 16GB RAM, NVidia GeForce GTX1650 4GB

### Conceptual Questions

#### 1 - Camera Rays

We can begin calculating the rays by creating a virtual canvas one unit away from a camera oriented at the origin, along the `Z` axis. The angle between the top-center pixel of our canvas and the bottom-center pixel of our canvas will be our **field of view** (`FoV`). We will consider this top point to reside at (0, `sin(FoV/2)`, 1), and the bottom point to reside at (0, `-sin(FoV/2)`, 1). The width of our viewport will be equal to `resolutionX * (2sin(FoV/2) / resolutionY`, to keep our pixels square. In that way, we can construct the corners of our canvas.

Once we have the extents of our canvas, we can easily distribute endpoints for our view-rays along the canvas. Then, for each one, we consider the position to be our direction (since our camera origin is at (0, 0, 0)), and we can normalize each. We now have a camera-space with origin at (0, 0, 0), view-direction of (0, 0, 1), an up-direction of (0, 1, 0), and a right-direction of (1, 0, 0), along with all of the rays we need to render our scene onto the screen.

We then want to transform each of these rays into our world space. The origin transformation is easy; we make the world-space camera origin to be the origin for each of our view-rays.

To transform the directions for each of our view-rays, we can construct a camera-to-world transformation matrix. This can be done by turning the right, up, and forward vectors of our camera's world-space directions into the rows of a rotation matrix. With some care regarding homogenous matrices, we can easily transform all of our rays into world space (and, similarly, create the transformation matrix for the reverse direction).

#### 2 - Procedural Geometry

When a ray enters the `AABB` for the procedural geometry, we can use the scale/translation of the `AABB` to transform the ray to a position aligned with an "untransformed" bounding box; this allows for us to, say, always be intersecting with a unit sphere, rather than a transformed ellipsoid.

We can then choose which kind of shader handles it based on the `Type` of the geometry. These shaders can encode the hit-or-miss capabilities for each geometric equation. To calculate whether a ray hits or misses a given geometry within an AABB, we do some math based on the transformed ray and the unit geometry, and use the results of that to determine whether our ray hit the geometry or not.

#### 3 - Acceleration Structures
