Davis Polito                        
*  [https://github.com/davispolito/Project0-Getting-Started/blob/master]()                                             
* Tested on: 
Windows 10, i7-8750H @ 2.20GHz 16GB, GTX 1060

## Conceptual Questions 
	1. How would you convert pixels into rays?
	We need to first transfer from the 4x4 *view matrix* to the *image plane matrix* by multiplying by the *camera projection matrix* and normalized via persepctive divide. This takes us to screen space by discarding the z component. From here we normalize to our specific screen. We then floor the screen space to pixel space assuming we know pixel width. Assuming Z = x we can invert all these matrices including the perspective divide stage. This is how we get the final ray equation. 
	2. How would one render procedural geometry?
	We simply turn its geometric function into an intersection function such that when the ray enters the object the function returns in an explicitly defined way. If the *(x,y,z)* of the ray solves the equation the point is rendered and the ray casted against its normal. 
	
	3. Draw the Accelleration structures for this scene. 

	TOP LEVEL
	Instance     |		Instance  	|	Instanceo

GEOM		-	MODEL		- 		AABB
*BLAS*			*BLAS*				*BLAS*
Plane Var -> Plane 1    Person T - > model 1, t		Spheres -> [] instacnes
Plane var -> Plane 2    Person laying -> model 1	Boxes -> [] instances
Plane var -> Plane 3
