# GPU CPU Rasterizer

The Goal for this project was to be able to seemplessly Rendering the scene on the GPU and then switch to rendering on the CPU.
Toggeling between GPU and CPU by the click of a buttton. 
This project was done using the Effects shader system from DirectX.

# What the rasterizer can do:

- Triangle Mesh Rasterizing
- Camera, Dynamic movement with Keyboard & Mouse, adjustable FOV
- Runtime Rendermode change from CPU to GPU (switch from Right handedness to Left handedness)
- Transparancy(not entirely implemented on CPU-Rasterizer)
- Cycle SamplingState: Point, Linear, Anisotropic (only GPU)
- Cycle Cullmodes: None, Front-Face and Back-Face Culling
- Togeling the use of textures (CPU only)
- Cycling Scenes if multiple are added
- Turntable
- Frustrum culling

# Comparison
Hardware Rasterizing

![HRasterizer2](https://user-images.githubusercontent.com/28813555/136845176-25f0e536-64eb-4801-ab18-d46f891ed8d1.png)

Software Rasterixing

![SRasterizer2](https://user-images.githubusercontent.com/28813555/136844972-2ccd23a0-ed3d-412b-bd33-ad750f33eff5.png)

Overlayd

![Hardware Software Rasterizer](https://user-images.githubusercontent.com/28813555/136846630-3f87cd7c-80c7-42bb-90e8-0f9a3e28c37c.png)

# Remarks
As can be seen in the image above, I did not finallize my implementation of transparacy for the CPU Rasterizer, this is something I hope to come back to.
