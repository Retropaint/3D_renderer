# 3D_renderer
A basic 3D renderer made in SFML. It started as a curiosity for how 3D graphics worked and has stayed as a long, on-going challenge to see how far I can go.
The main branch is software-rendered, while the hwa-rendered branch is for NVIDIA GPUs (due to using CUDA, and also because I'm on a GTX 860M).

# Build Instructions
1. Clone project
1. Build with CMake

## For HWA-rendering branch
This has to be done before building with CMake
1. compile hwaCrossProducts (don't link it) using `nvcc` (sfml libs have to be stated).
2. Create a lib out of the object file with `ar rcs`
