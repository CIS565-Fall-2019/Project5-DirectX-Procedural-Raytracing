**University of Pennsylvania, CIS 565: GPU Programming and Architecture,
Project 5 - DirectX Procedural Raytracing**

* Taylor Nelms
  * [LinkedIn](https://www.linkedin.com/in/taylor-k-7b2110191/), [twitter](https://twitter.com/nelms_taylor)
* Tested on: Windows 10, Intel i3 Coffee Lake 4-core 3.6GHz processor, 16GB RAM, NVidia GeForce GTX1650 4GB

### Conceptual Questions

#### 1 - Camera Rays

#### 2 - Procedural Geometry

When a ray enters the `AABB` for the procedural geometry, we can use the scale/translation of the `AABB` to transform the ray to a position aligned with an "untransformed" bounding box; this allows for us to, say, always be intersecting with a unit sphere, rather than a transformed ellipsoid.

We can then choose which kind of shader handles it based on the `Type` of the geometry. These shaders can encode the hit-or-miss capabilities for each geometric equation. To calculate whether a ray hits or misses a given geometry within an AABB, we do some math based on the transformed ray and the unit geometry, and use the results of that to determine whether our ray hit the geometry or not.

#### 3 - Acceleration Structures
