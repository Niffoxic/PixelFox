# PixelFox

A pixel-based game written from scratch in C++20 and DirectX 11  
(but CPU-side renderer: meaning I can't really use DirectX 11 to render, 

works like:   
only render onto a texture on the CPU, then upload that texture to the GPU and display it on a fullscreen quad).  
No engines, no frameworks: every system is built manually from scratch.

## Output
![Game Output](docs/output_7.gif)

## Structure
- **PixelFoxEngine**: handles rendering, input, and timing  
- **PixelFoxPhysics**: provides simple 2D physics and collision handling  
- **PixelFoxCore**: provides custom container utilities (restricted to containers only)  
- **PixelFoxGame**: main gameplay layer using all modules
- **PixelFoxMath**: provides Vector2D, affine matrix 2D and some helpers math functionality

## Build
Requires Visual Studio 2022 (C++20) its a must
