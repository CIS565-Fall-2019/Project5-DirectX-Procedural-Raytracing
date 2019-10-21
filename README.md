# Conceptual Questions Answers

**Q1**:
This involves an inverse projective transformation from *screen* space (2D) to *world* space (3D). We will start by explaining the process required to transform a 3D point in *world* space into *screen* space (projective transformation). For the rest of this explanation, we assume that the 'canvas' (The plane upon which points in 3D are transformed into 2D) is at distance Z=1, where the Z-axis defines the 'look' vector of the camera.
1. We first require a view Matrix. This matrix transforms 3D points in *world* space into *view* space. This is a 4x4 homogenous matrix. 
2. We then require a camera projection matrix, that transforms points in the view space into Image plane space (taking care of near/far-clip planes distance and FOV of the camera we are using) -> Next, the **Z-coordinate is normalized via perspective divide**. 
3. We then have to transfer to *screen* space by simply getting rid of the Z-coordinate that exists in the image plane representation of the 3D point. This will leave us with an *x* and *y*, representing the point in screen space (where the origin is at the center of the screen. NOTE: We are still not representing the point in Pixels, but rather as floats in Screen space) 
4. We then normalize our *x*, *y* coordinates w.r.t to the Size of our screen. For some screen space point **p = [p.x, p.y]**, and a screen of resolution (Width:1080, Height:720), we do. **p_norm.x = (p.x + 1080/2)/1080** and **p_norm.y = (p.y + 720/2)/720**.
5. After normalizing our values, we assume to know the **pixel_width** and **pixel_height** to then discretize these normalized values into pixel space. We do so as follows: **p_pixel.x = floor(p_norm.x * pixel_width** and **p_pixel.y = floor(p_norm.y * pixel_height)** 

Now, we have converted from 3D space into Pixel Space. If we assume rays emanate from the center of each pixel, notice that all of the above transformations are invertible (projective transformations are invertible, multiplications/divisions are invertible when we know all of the variables neccessarry etc.) EXCEPT for when we lose information about the **Z** coordinate of the object in 3D space due to the perspective divide. In order to invert that step, we require the Z-coordinate, but do not have access to it. This is the need for ray-tracing. We invert all the transformations, and assume Z=1 for inverting the perspective divide step. This will define the origin of our ray. The ray itself is defined by setting **Z = t**, which results in the full ray equation, where the direction vector is given by the un-normalized point in *view* space. 

**Q2**:
Consider a sphere in the AABB space. We can manipulate the sphere equation into an 'intersection' equations by realizing that if any x, y, z solves the sphere equation, then it must intersect with the sphere (simply involves some algebraic manipulation and use of the quadratic equation). Thus, when a ray enters the AABB of the procedural geometry, we continue tracing its path until some x,y,z solves the sphere equation. If it exits the AABB without any solution, then we don't render anything because there were no intersections. If some x,y,z solves  the sphere equation, then there is an intersection and we shade that point. The normal to this point is simply the ray from the center to this intersection. 

This same methodology applies to any AABB, Type, and Equation. Once a ray enters an AABB, we continue tracing it until a point solves the intersection Equation defined by that an instance of that Type. If it exits the AABB without any solution, then we render nothing. If we have a solution, then we render that point and continue reflection/casting shadow rays as neccesarry. 

**Q3**
[img.JPG]

