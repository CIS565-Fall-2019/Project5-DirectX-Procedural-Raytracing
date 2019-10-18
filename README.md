**University of Pennsylvania, CIS 565: GPU Programming and Architecture,
Project 5 - DirectX Procedural Raytracing**

* Joshua Nadel
  * https://www.linkedin.com/in/joshua-nadel-379382136/, http://www.joshnadel.com/
* Tested on: Windows 10, i7-6700HQ @ 2.60GHz 16GB, GTX 970M (Personal laptop)

### Conceptual Questions
1. First, set the ray origin to the eye location of the camera. This is the same for every pixel. For a given pixel at screen coordinate (x, y), the direction of the ray travelling through that pixel can be computed as follows. First, we need to convert our pixel coordinate from 0 to resolution pixel space to -1 to 1 screen space. This can be done by dividing the coordinates by the screen dimensions, multiplying them by 2, then subtracting 1 from each coordinate. Now, tranform the coordinate by the inverse of the projection matrix, then the inverse of the view matrix. This gives us a 3D coordinate of the pixel in world space. Subtract the camera eye from this location to get the ray direction.

2. Rendering procedural geometry involves using the right equation to check intersections against each pixel's ray. But computing these intersection equations can become slow for complex procedural shapes and meshes. So, we check collisions against the object's axis-aligned-bounding-box first to see if our ray passes through the object at all. If the ray hits the bounding box, we can then check whether the system of equations describing the ray and the object's surface has a solution. Which equation we use for the object's surface depends on the object's type. If a solution exists, then the ray intersects with that object and we color that pixel in according to the object's shader.

3.
