**University of Pennsylvania, CIS 565: GPU Programming and Architecture,
Project 5 - DirectX Procedural Raytracing**

* (TODO) YOUR NAME HERE
  * (TODO) [LinkedIn](), [personal website](), [twitter](), etc.
* Tested on: (TODO) Windows 22, i7-2222 @ 2.22GHz 22GB, GTX 222 222MB (Moore 2222 Lab)

### Conceptual Questions

# Q2. Each procedural geometry can be defined using 3 things: the Axis-Aligned Bounding Box (AABB) (e.g. bottom left corner at (-1,-1,-1) and top right corner at (1,1,1)) that surrounds it, the Type (e.g. Sphere) of the procedural geometry contained within the AABB, and an Equation describing the procedural geometry (e.g. Sphere: (x - center)^2 = r^2). Using these 3 constructs, conceptually explain how one could go about rendering the procedural geometry. To be specific, consider how to proceed when a ray enters the AABB of the procedural geometry.

When a ray from the camera piercing through a unique pixel of the camera plane enters the Align Axis Bounding Box(AABB) of the procedural geometry, then the shader will check whether the ray is hitting the object or not. According the the AABB of the geometry type it entered, its corresponding intersection shader will perform intersection check with the help of the equation which is provided in the procedural geometry and the ray it intersected. Note that since, we know the pixel the ray came from, we will have the equation of ray known to us. The shader will determine the position and normal for the intersection and will generate a ray which would go the light source and other reflective ray if the object is reflective. Later on, we will check for the ray going to the light whether it is a shadow ray or not mand for reflective ray, we will have another similar recusrion with the same above procedure.


