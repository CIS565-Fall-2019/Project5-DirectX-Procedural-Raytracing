# DirectX Procedural Raytracing
**University of Pennsylvania, CIS 565: GPU Programming and Architecture,
Project 5**

Caroline Lachanski: [LinkedIn](https://www.linkedin.com/in/caroline-lachanski/), [personal website](http://carolinelachanski.com/)

Tested on: Windows 10, i5-6500 @ 3.20GHz 16GB, GTX 1660 (personal computer)

## Conceptual Questions

1. In ray tracing, we shoot a ray for each pixel and need to determine that ray's intersection with the scene, which is in world space coordinates. Rays are shot from the camera, so each ray's origin is set to the camera's position (which is in world space). However, every pixel's ray will have a different ray direction. To find this direction, we need to determine the pixel's location if it was projected into in world space coordinates. We currently have the coordinates in pixel space (some (x, y) between (0, 1280) and (0, 720), respectively). We first convert them to normalized device coordinates (NDC), which range from -1 to 1. We do this with simple formulas: `xNDC = (xPixelSpace / 1280) * 2 - 1` and `yNDC = 1 - (yPixelSpace / 720) * 2`. 

From NDC space, which is a 2D space, we want to convert the coordinates into a 3D space. To get from 3D to 2D we project the coordinates into NDC screen space using the projection matrix (this matrix is what mapped x and y to (-1, 1) and mapped z to (0, 1)). To reverse that operation, we multiply our coordinates with the inverse of the projection matrix. However, since the projection matrix is a 4x4 matrix, we need to turn our (x, y) coordinates into (x, y, z, w) coordinates. We set z to 0 (somewhat arbitrarily, the depth doesn't really matter) and w to 1. Multiplying these coordinates with the inverse of the projection matrix will bring us into camera space coordinates, in which everything is defined relative to the camera.

To go from camera space to world space, we need to multiply our coordinates with the inverse of the camera's view matrix, which is defined using the camera's position and axes. Now we have the pixel's coordinates in world space.

We then set the ray direction to be `direction = normalize(worldSpaceCoords - cameraPos)`.

2. First, we check if the ray intersects the AABB at all (this is as simple as a ray-box intersection, since an AABB is essentially a box). If the ray does not intersect the AABB, it does not intersect the procedural geometry inside. If it does intersect the AABB, we may need to perform another check. For example, if the Type is a Sphere, we will need to do another check to see if that ray intersects the Sphere held by the AABB. For the sphere, we would then do a ray-sphere intersection using the equation of the sphere. Since a position along a ray can be represented by an equation `Pos = Origin + t * Direction`, we can use this equation in conjunction with the Sphere's equation to see if there is a valid `t` for which our `pos` is on the surface of the Sphere (meaning we intersected with the Sphere). Depending on the procedural geometry's Type and Equation, we can do different ray-geometry intersection tests. Another example is the Type Box; if we intersect an AABB that contains a Box, we are already done, since intersection with the AABB and intersection with the Box it contains are the same thing. Another ty

3. insert image here
