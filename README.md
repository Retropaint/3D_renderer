# 3D_renderer
A basic 3D renderer made in SFML. It started as a curiosity for how 3D graphics worked and has stayed as a long, on-going challenge to see how far I can go.
The main branch is software-rendered, while the hwa-rendered branch is for NVIDIA GPUs (due to using CUDA, and also because I'm on a GTX 860M).

# Build Instructions
1. Clone project
2. Build with CMake
2.1 Install CMake. Verify by running `cmake` in terminal. If you get instructions on how to use CMake, it's installed successfully.
2.2 create a new folder (preferrably beside the clone)
2.3 `cd` to the new folder in terminal and type `cmake [RELATIVE_PATH_TO_CLONE]`.
2.4 type `cmake build .`.
