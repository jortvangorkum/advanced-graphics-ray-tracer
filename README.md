### Students:
Jort van Gorkum - 6142834
Vince van Noort - 6187021

## Part 2

### BVH building - Monster
created polygon soup for 123216 triangles in 0.005s
Building BVH Time: 107ms

Monster Kajiya
![image info](./renders/monster-kajiya.JPG)
Monster Whitted
![image info](./renders/monster-whitted.JPG)

## Part 1
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