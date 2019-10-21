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



### DXR Top-Level / Bottom-Level

# (TODO: Your README)

Include screenshots, analysis, etc. (Remember, this is public, so don't put
anything here that you don't want to share with the world.)
