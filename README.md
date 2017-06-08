# PathMarchCL #

A realtime raymarching renderer, written in C++ with SDL2, OpenGL and OpenCL.

![Kaleidoscopic IFS Fractal](https://raw.githubusercontent.com/Philipp-M/PathMarchCL/master/images/kaleidoscopic.jpg)
![Menger Sponge Fractal](https://raw.githubusercontent.com/Philipp-M/PathMarchCL/master/images/menger.jpg)

## Features ##

This is an example how raymarching can be done with OpenCL with OpenGL interoperability.
It features a camera, which can be moved and controlled regarding field of view or movement speed.

The renderer renders different scenes (currently a menger sponge and a Kaleidoscopic IFS fractal are available) with continuously new samples for nice Antialiasing.
A tent filter with a combined Tausworthe and Linear Congruential Generator random generator is used for achieving this.

The default scene is the kaleidoscopic IFS Fractal, which features smooth shadows and Ambient Occlusion. This obviously needs some performance (especially with the detail which goes down to floating point precision errors), for slower GPUs the Mengersponge Scene might be better.
The scene can be changed by changing the following line in the file `kernels/kernels.cl` from

```opencl
#include "raymarch_kaleido.cl"
```
to
```opencl
#include "raymarch_menger.cl"
```

## Controls ##
	* **Mouse Motion** rotate the camera
	* **w,a,s,d, arrow keys** move the camera
	* **Mouse Wheel, +, -** change the movement speed
	* **q,e** rotate the camera around the *"viewing at"* axis
	* **f** toggle FPS camera mode
  * **v** increase FOV
  * **b** decrease FOV
  * **i** save the current rendered screen in the format `render_{CURRENT_TIME}_{SAMPLE_COUNT_PER_PIXEL}_Spp.bmp`
  * **x** exit program
