**University of Pennsylvania, CIS 565: GPU Programming and Architecture,
Project 5 - DirectX Procedural Raytracing**

* Srinath Rajagopalan
* [LinkedIn](https://www.linkedin.com/in/srinath-rajagopalan-07a43155/)

### DirectX Raytracing

1) _Ray tracing begins by firing off rays from the camera's perspective, with 1 ray corresponding to 1 pixel. Say the viewport is (1280 by 720), how would you convert these pixel locations into rays, with each ray being defined by an Origin and a Direction, such that Ray = Origin + t * Direction? Consult this intro to camera transformations and this explanation of world-to-screen/screen-to-world space article to formulate an answer in your own words_

We have to to generate rays for each pixel. Given a `WxH` image, for each pixel coordinates `(i,j)`, we shoot a ray through the center of the pixel. So it boils to finding the direction between `(c_x, c_y, c_z)` (coordinates of the camera) and `(x, y, z)`, coordinates of our pixel center on the image corresponding to pixel center `(i,j)` in the world frame. First we get the absolute coordinates of the pixel center of `(i,j)`. If top left of the screen is `(0,0)`, coordinates of `(i,j)` is `u_img = w*i + w/2` and `v_img = h*j + h/2` where `(w,h)` are the width and height of each pixel. From this we can use the camera calibration matrix to transform `(u_img, v_img)` to `(x, y, z)`. This is done as follows (using slides from [CIS: 581](https://alliance.seas.upenn.edu/~cis581/Lectures/Fall2019/cis581-02-2019-projection-simple.pdf)).
