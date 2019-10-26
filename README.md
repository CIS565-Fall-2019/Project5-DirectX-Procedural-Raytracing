<p align="center">
    <img src = images/demo.gif>
</p>

**University of Pennsylvania, CIS 565: GPU Programming and Architecture,
Project 5 - DirectX Procedural Raytracing**

* Jiangping Xu
  * [LinkedIn](https://www.linkedin.com/in/jiangping-xu-365b19134/)
* Tested on: Windows 10, i9-8950HK @ 2.90GHz 32GB, GTX 1070 8G
__________
[Introduction](#Stream-Compaction) - [Performance Analysis](#performance-analysis)
__________
## Introduction

This is a directX raytracing project using the recently released DirectX Raytracing API. There is one `Top Level Acceleration Structures` (TLAS), which has one instance of a Triangle `Bottom Level Acceleration Structures` (BLAS), and one instance of an `Axis-Aligned Bounding Box` (AABB) BLAS. The Triangle BLAS holds triangle data that will be used to render a horizontal plane. The AABB BLAS holds procedural geometry data, including a box, three balls, and a meta ball. 

<p align="center">
  <img src="images/accelexplained.png">
</p>

## Performance Analysis
<p align="center">
    <img src = images/FPS.png>
</p>

The figure above shows the frame rate changes as increasing maximum raytracing depth. FPS goes doen as expected when tracing depth increases.