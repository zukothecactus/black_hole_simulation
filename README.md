# Black Hole Simulator (originally Gravity Simulator)

![Initial view of the black hole](Initial%20position.png)

A simulator built in C++ and OpenGL that presents a real-time 3D model of a black hole.
This project uses compute shaders for the purpose of raytracing photon trajectories through curved spacetime, which then create a mathematically accurate visual representation of a spinning black hole and its accretion disk.

## Features

![Relativistic beaming and curvature](Curviture%20example%201.png)

* **Kerr-Schild Metric Simulation:**
    * Using the Kerr-Schild metric we can model the curved spacetime of the spinning black hole, using exact analytical geodesic equations generated using a Python script (SymPy)

* **Runge-Kutta (RK4) Raytracing:**
    * RK4 integration is used for tracing photon paths with dynamic step sizes, based on proximity to both the event horizon and the accretion disk.

* **Volumetric Accretion Disk:**
    * The glowing plasma disk around a black hole
    * The Gaussina vertical falloff and smooth radial bounds were use to simulate gas density

* **Relativistic effects:**
    * Accurate Doppler shifting (blueshift and redshift)
    * Asymmetric beaming (intensity scaling)
    * Both the Doppler shifting and asymmetric beaming are based on the plasma's 4-velocity relative to the observer

* **Compute Shader Pipeline:**
    * Heavy tensor math and integration loops are offloaded to the GPU (instead of the CPU)
    * The offloading is done via GLSL compute shaders for real-time performance

* **Automated Equation Generation:**
    * Includes a Python script that calculates the Christoffel symbols and outputs a highly optimized GLSL code using common subexpressive elimination


## Architecture and Pipeline

![Spacetime grid distortion](Curviture%20example%202.png)

The app uses a split rendering pipeline where the physics simulation is mathematically pre-computed, handled by the GPU, and then drawn to the screen:
1. **Geodesic generation:**
    * The complex Christoffel symbols for the Kerr-Schild metric are calculated by the [`kerr_generator.py`](kerr_generator.py) script (using SymPy)
    * The script optimizes the resulting acceleration equations and formats them directly int GLSL code for the shader
2. **Initialization:**
    * The GLFW window and GLAD OpenGL function pointers are initialized by the [`Engine`](Engine.cpp) class
    * The app than creates a `BlackHole` instance and configures a floating-point texture (`GL_RGBA32F`) designed for the compute shader output
3. **Compute dispatch:**
    * Every frame: [`Main.cpp`](Main.cpp) binds the camera parameters (position, front, up, right) and dispatches the [`raytrace.comp`](raytrace.comp) shader across a 2D grid matching the texture resolution.
4. **Ray integration:**
    * In [`raytrace.comp`](raytrace.comp) shader, each thread calculates a starting ray direction and uses RK4 to guide the ray, step-by-step, through the spacetime metric.
    * The shader then check for event horizon intersections, calculates volumetric emission when intersecting the plasma disk, and samples a background skybox grid for escaping rays
5. **Rasterization:**
    * When the compute shader finishes writing to the texture, a memory barrier is set in place to ensure all writes are complete
    * The texture is then mapped onto a full-screen quad using standard vertex ([`default.vert`](default.vert)) and fragment ([`default.frag`](default.frag)) shaders.

## Project Structure
* [`Main.cpp`](Main.cpp): The core application loop, handling setup, shader dispatch, and quad rendering.
* [`Engine.cpp`](Engine.cpp) & [`Engine.h`](Engine.h): Manages the GLFW window context and viewport.
* [`Camera.cpp`](Camera.cpp) & [`Camera.h`](Camera.h): Implements an orbital camera system focused on the black hole.
* [`BlackHole.cpp`](BlackHole.cpp) & [`BlackHole.h`](BlackHole.h): Data structure defining the singularity's mass and position.
* [`raytrace.comp`](raytrace.comp): The primary compute shader responsible for all relativistic math, ray marching, and coloring.
* [`kerr_generator.py`](kerr_generator.py): A standalone Python utility for deriving and optimizing the metric equations.
* [`shaderClass.cpp`](shaderClass.cpp), [`VAO.cpp`](VAO.cpp), [`VBO.cpp`](VBO.cpp): Object-oriented wrappers for managing OpenGL shader compilation, Vertex Array Objects, and Vertex Buffer Objects.

## Controls

![Alternative angle of the black hole](Curviture%20example%203.png)

Navigation is handled by and orbital camera that orbits around the black hole at the center of the view
* **Orbit camera:** Click and hold the left mouse button while dragging your mouse to adjust the pitch and yaw around the black hole
* **Smooth zoom:** Use the scroll wheel to smoothly zoom in and out