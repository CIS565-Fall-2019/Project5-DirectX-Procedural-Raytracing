# Project 5 - DirectX Procedural Raytracing
================

**University of Pennsylvania, CIS 565: GPU Programming and Architecture, Project 5**

* Weiqi Chen
  * [LinkedIn](https://www.linkedin.com/in/weiqi-ricky-chen-2b04b2ab/)
* Tested on: Windows 10, i7-8750H @ 2.20GHz 2.21GHz, 16GB, GTX 1050 2GB

## Conceptual Questions

1. In order to get the ray origin and direction for a pixel, we need to do the reverse of rasterization and transform a pixel's coordinate into the world space. Given a viewport of 1280 x 720 and a pixel coordinate (x, y) in the range of [0, 1279/719], We need to do the following steps:

* Convert the point to Normalized Device Coordinate space. x = x / Pixel Width and y = -y / Pixel Height + 1
* Convert the point in NDC space to screen space. x = x * width - width / 2 and y = y * height - height / 2.
* Set the z coordinate to be -1 for each pixel and convert the pixel coordinate and camera position with the inverse of the view projection matrix to get world space coordinates.
* The origin of each pixel will be the world space camera position and its direction `dir` will be (world space pixel coordinate - world space camera position)


2. When a ray enters the AABB of the procedural geometry, we need to find out if the ray intersects with the geometry or not based on the Type of the geometry. This can be done using analytical solutions to for example ray-cube intersection or raymarching. If the ray hits the geometry, the intersection point and the surface normal will be used for rendering and the ray will be colored. The ray will then try to go to the light source directly. If it fails, then the ray will be shadowed since it is blocked by some other geometry.


3. ![](images/1.jpg)


## Summary

## Raytracing
