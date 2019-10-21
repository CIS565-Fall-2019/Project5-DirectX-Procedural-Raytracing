**University of Pennsylvania, CIS 565: GPU Programming and Architecture,
Project 5 - DirectX Procedural Raytracing**

* Alexis Ward
  * [LinkedIn](https://www.linkedin.com/in/alexis-ward47/), [personal website](https://www.alexis-ward.tech/)
* Tested on: Windows 10, i7-8750H CPU @ 2.20GHz 16GB, GTX 1050 Ti 

# Ray Tracing Background (Conceptual Questions)

### How To Ray Trace

Ray tracing begins by firing off "rays" from the camera's perspective, with 1 ray corresponding to 1 pixel. Each ray is defined by an `Origin` and a `Direction`, such that `Ray = Origin + t * Direction`, with `t` being a distance along the ray. You can convert each pixel in the display to a ray by:

1. Setting `Origin` equal to the camera's "eye," or position.
2. Given pixel coordinates `p`, we get it's Screen Space counterpart by:
  * `screen.x = (p.x / ScreenXDimension) * 2.f - 1.f` and
  * `screen.y = 1.f - (p.y / ScreenYDimension) * 2.f`
3. Since we don't have a `z` or `w` value, we estimate the furthest possible point by using the camera's far-clip distance: `farPoint = vec4(x * -farClip, y * -farClip, farClip, farClip)`. This puts us into Unhomogenized Screen Space.
4. We now multiply this vector by the inverse of the camera's view and projection matrices: `inverse(u_View) * inverse(u_Project) * farPoint`
5. Set the `Direction` to the normalized difference between the above result and the `Origin`.

This is essentially converting a pervieved point in Pixel Space back to World Space, and then finding the vector from the camera to said point. We can now test for the ray's intersections with geometry.

### Locating Procedural Geometry

Every procedural geometry in this project is defined using 3 things: its `Axis-Aligned Bounding Box` (shortened to AABB, it is a tight bounds that encases the whole shape), its `Type` or shape, and the `Equation` determined by its `Type` and transformation. This `Equation` will most likely be a signed distance function, which tells us how far we are from the closest point on the geometry (but not necessarily the closest point along a ray).

To render procedural geometry, we check each pixel's corresponding ray for intersections with the scene. To simplify this, we can first see if the current ray passes through an `AABB` stored within our Bottom Level Acceleration Structure (BLAS). This tells us that the geometry encased within can potentially influence this pixel. We then use a Closest Hit Shader determined by the object's `Type` to trace our ray with the influence of this geometry's `Equation`. This equation should give us a `t` value, as shown in the aforementioned ray representation: `Ray = Origin + t * Direction`. We displace our ray by this value and test again. This part happens recursively until the `Equation` returns a zero (or a value within some epsilon factor), or if the ray leaves the `AABB`.

This method greatly decreases runtime because, by working with only the intersected AABBs, we avoid having to recursively compare the ray to every geometry equation equation in the scene: we only perform this for the relevant equations. The AABB intersection check is a simple one because of how they are defined by their min and max [x, y, z] positions. 

### DXR Top-Level / Bottom-Level Example

Say our scene looks like this:

<p align="center">
  <img src="https://github.com/CIS565-Fall-2019/Project5-DirectX-Procedural-Raytracing/blob/master/images/scene.png">
</p>

This is a diagram of the resulting acceleration structure:




# (TODO: My README)

Include screenshots, analysis, etc. (Remember, this is public, so don't put
anything here that you don't want to share with the world.)
