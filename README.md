**University of Pennsylvania, CIS 565: GPU Programming and Architecture,
Project 5 - DirectX Procedural Raytracing**

Peyman Norouzi
* [LinkedIn](https://www.linkedin.com/in/peymannorouzi)
* Tested on: Windows 10, i7-6700 @ 3.40GHz 16GB, Quadro P1000 4096MB (Moore 100B Lab)

## Conceptual Questions:

1. Ray tracing begins by firing off rays from the camera's perspective, with 1 ray corresponding to 1 pixel. Say the viewport is (1280 by 720), **how would you convert these pixel locations into rays**, with each ray being defined by an `Origin` and a `Direction`, such that `Ray = Origin + t * Direction`? Consult this [intro](https://www.scratchapixel.com/lessons/3d-basic-rendering/computing-pixel-coordinates-of-3d-point/mathematics-computing-2d-coordinates-of-3d-points) to camera transformations and this [explanation](http://webglfactory.blogspot.com/2011/05/how-to-convert-world-to-screen.html) of world-to-screen/screen-to-world space article to formulate an answer in your own words.

I assume that we start pixel coordinates from top left corner of the screen. That makes the bottom right corner of the screen (1280,720). We need to take these pixel values and normilized them so we can then use them as local 2d coordinates and take them to global coordinates. The equations bellow will take the values and normilizes it to between -1 and 1 meaning that top left corner is (-1,-1) and the bottom right corner is (1,1): (Width= 1280 and Height= 720)


<p align="center">
<a href="https://www.codecogs.com/eqnedit.php?latex=\text{\hspace{0.2in}Normilized&space;Versions}\\&space;\\&space;P_{norm}.x&space;=&space;\frac{P.x&space;-&space;(width/2)}{width}&space;\times&space;2&space;\\\displaystyle&space;\displaystyle&space;P_{norm}.y&space;=&space;\frac{P.y&space;-&space;(height/2)}{height}&space;\times&space;2&space;\\&space;P_{norm}.z&space;=&space;1&space;\\" target="_blank"><img src="https://latex.codecogs.com/png.latex?\text{\hspace{0.2in}Normilized&space;Versions}\\&space;\\&space;P_{norm}.x&space;=&space;\frac{P.x&space;-&space;(width/2)}{width}&space;\times&space;2&space;\\\displaystyle&space;\displaystyle&space;P_{norm}.y&space;=&space;\frac{P.y&space;-&space;(height/2)}{height}&space;\times&space;2&space;\\&space;P_{norm}.z&space;=&space;1&space;\\" title="\text{\hspace{0.2in}Normilized Versions}\\ \\ P_{norm}.x = \frac{P.x - (width/2)}{width} \times 2 \\\displaystyle \displaystyle P_{norm}.y = \frac{P.y - (height/2)}{height} \times 2 \\ P_{norm}.z = 1 \\" /></a>
  </p>


After that we have to deal with the following (Camera is the starting point and the projection plane is the monitor screen). We have the following to go from the device coordinates to the 3D world Coordinates:

<p align="center">
<a href="https://www.codecogs.com/eqnedit.php?latex=P_{norm}&space;=&space;T_{projection}P_{world}" target="_blank"><img src="https://latex.codecogs.com/png.latex?P_{norm}&space;=&space;T_{projection}P_{world}" title="P_{norm} = T_{projection}P_{world}" /></a></p>

Where T is given and is the projection matrix that projectes world coordinates to camera. To be able to find to World Coordinates then we have to do the following:

<p align="center">
<a href="https://www.codecogs.com/eqnedit.php?latex=P_{world}&space;=&space;P_{norm}T_{projection}^{-1}" target="_blank"><img src="https://latex.codecogs.com/png.latex?P_{world}&space;=&space;P_{norm}T_{projection}^{-1}" title="P_{world} = P_{norm}T_{projection}^{-1}" /></a></p>

Then after that we have can the ray as following:

<p align="center">
<a href="https://www.codecogs.com/eqnedit.php?latex=Ray&space;=&space;P_{camera}&space;&plus;&space;t&space;\cdot&space;P_{world}" target="_blank"><img src="https://latex.codecogs.com/png.latex?Ray&space;=&space;P_{camera}&space;&plus;&space;t&space;\cdot&space;P_{world}" title="Ray = P_{camera} + t \cdot P_{world}" /></a></p>

where P_camera is just the camera coordinates in the world coordinate when we use [0,0,1] x inv(T_projection) just as we did for the P_world coordinate.

2. Each procedural geometry can be defined using 3 things: the `Axis-Aligned Bounding Box` (AABB) (e.g. bottom left corner at (-1,-1,-1) and top right corner at (1,1,1)) that surrounds it, the `Type` (e.g. Sphere) of the procedural geometry contained within the AABB, and an `Equation` describing the procedural geometry (e.g. Sphere: `(x - center)^2 = r^2`). **Using these 3 constructs, conceptually explain how one could go about rendering the procedural geometry**. To be specific, consider how to proceed when a ray enters the AABB of the procedural geometry.


3. **Draw a diagram of the DXR Top-Level/Bottom-Level Acceleration Structures** of the following scene. Refer to section 2.6 below for an explanation of DXR Acceleration Structures. We require that you limit your answer to 1 TLAS. You may use multiple BLASes, but you must define the Geometry contained within each BLAS.


<p align="center">
  <img src="https://github.com/CIS565-Fall-2019/Project5-DirectX-Procedural-Raytracing/blob/master/images/scene.png">
</p>


