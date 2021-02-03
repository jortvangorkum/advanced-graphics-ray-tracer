### Students:
Jort van Gorkum - 6142834
Vince van Noort - 6187021


# Part 3
Implemented:
- Adaptive Sampling 
- Next Event Estimation
- Multiple Importance Sampling

References:
- https://www.mff.cuni.cz/veda/konference/wds/proc/pdf13/WDS13_110_i5-Sik.pdf

Inspired by:
- http://www.realtimerendering.com/raytracinggems/unofficial_RayTracingGems_v1.7.pdf
  - page: 213

Below we show some example renders to demonstrate our implementations of adaptive sampling and next event estimation.

### 4k Version
![image info](./renders/atst-4k-11-frame.png)

### With Adaptive Sampling, Next Event Estimation & Multiple Importance Sampling
32th frame (~180s)
![image info](./renders/atst-with-adpt-nee-mis-thirtied-frame.png)

1th frame (~6s)
![image info](./renders/atst-with-adpt-nee-mis-first-frame.png)

### With Adaptive Sampling & Next Event Estimation
32th frame (~180s)
![image info](./renders/atst-with-nee-thirtied-frame.JPG)

1th frame (~6s)
![image info](./renders/atst-with-nee-first-frame.JPG)

### With Adaptive Sampling
32th frame (~180s)
![image info](./renders/atst-with-adapsampl-sixtiest-frame.JPG)

1th frame (~6s)
![image info](./renders/atst-with-adapsampl-first-frame.JPG)

### Without both
54th frame (~180s)
![image info](./renders/atst-without-both-sixtiest-frame.JPG)

1th frame (~4s)
![image info](./renders/atst-without-both-first-frame.JPG)

# Part 2
All basics done.

### Additional points implemented:
- Construct the BVH for a 2M triangle scene in less than 1 second (still using SAH; 1 pt).

Nefertiti
![image info](./renders/build-bvh-under-1-sec.JPG)

Configuration:
- bins = 4
- split on z-axis

Computer Specs:
- CPU: Intel i7 8750H
- Memory: 16GB

### BVH building - Monster
created polygon soup for 123216 triangles in 0.005s
Building BVH Time: 75ms

Monster Kajiya - Render time per frame ~4500ms
![image info](./renders/monster-kajiya.JPG)
Monster Whitted - Render time per frame ~9600ms (with 4x anti-aliasing)
![image info](./renders/monster-whitted.JPG)

# Part 1
All basics done.

### Additional points implemented:
- Anti-aliasing (In Whitted)
- Image postprocessing: gamma correction, vignetting and chromatic aberration (in Whitted)

### Whitted Renders:
No post processing
![image info](./renders/whitted-no-post-processing.JPG)
Anti aliasing (4x)
![image info](./renders/whitted-no-post-processing-anti4x.JPG)
With post processing
![image info](./renders/whitted-with-post-processing.JPG)


### Kajiya Renders:
5 iterations
![image info](./renders/kajiya-5-iterations.JPG)
25 iterations
![image info](./renders/kajiya-25-iterations.JPG)
40 iterations
![image info](./renders/kajiya-40-iterations.JPG)

Sources: 
- https://www.scratchapixel.com/lessons/3d-basic-rendering/introduction-to-shading/reflection-refraction-fresnel
- https://www.scratchapixel.com/lessons/3d-basic-rendering/global-illumination-path-tracing/global-illumination-path-tracing-practical-implementation
- https://www.shadertoy.com/view/ltcyRf
