# PixelFox

A pixel-based survival game written entirely from scratch in C++20. Although a graphics API is used to present the image to the GPU, the image itself is the product of a CPU rasterizer - no graphics API is used for the actual rendering.

**No engines, no frameworks** - every system is handcrafted from the ground up.

works like:   
only render onto a texture on the CPU, then upload that texture to the GPU and display it on a fullscreen quad.

## Output
![Game Output](docs/final-output-main-menu.gif)

![Game Output](docs/final-gameplay.gif)

## Overview

**PixelFox** is a 2D pixel-based survival title built with a library-based engine, PixelFoxEngine, which is also coded from scratch. The project itself emphasizes low-level systems programming, algorithmic efficiency, and threaded CPU rendering - showcasing that complex gameplay and smooth performance can be achieved without traditional graphics APIs or engines.

The player must survive against endless waves of enemies under increasing difficulty. Movement, dashing, and combat mechanics are built around inertia, positioning, and reaction - all computed through a custom-built physics and rendering pipeline.

## Core Concepts

- **CPU-Based Renderer**  
  Multi-threaded 2D rasterizer that renders every pixel manually on the CPU.  
  Uses a **dual-thread model** (logic + render) with **8 worker threads** for parallel texture rasterization.

- **Physics System**  
  Lightweight AABB-based 2D physics and collision resolution.  
  Features penetration correction, rigidbody2D, and object tagging for selective collision handling.

- **Event System**  
  Custom **event queue** allowing objects to post and listen for gameplay events such as buffs, damage, and cooldown triggers.

- **AI & Enemies**  
  Reactive AI with melee and ranged enemy types that dynamically chase or attack the player.  
  Supports distance-based aggression scaling and line-of-sight targeting.

- **Tile-Based Map**  
  The world is structured on a 32*32 pixel tile grid. (means 1 Unit = 32px, or 1 scale of 1x1 means 32pxx32px) 
  Each tile defines terrain properties (e.g., static, dynamic, trigger or just pixels), simplifying spatial queries and collision lookups.

---

## Architecture

| Module | Description |
|:--|:--|
| **PixelFoxEngine** | Core engine handling rendering, physics, collision, input, and timing exposes RenderAPI for higher level (i.e for pixel-game) |
| **PixelFoxCore** | Custom data containers - a very light replica of std::vector, unordered_map and list |
| **PixelFoxMath** | Provides Vector2D, Matrix2D (Affine Matrix) with fixed type delc, and some mathematical utilities |
| **PixelFoxGame** | Main gameplay layer integrating all subsystems 

### Thread Model

- **Main Thread:** Handles logic, input, physics, and AI updates.  
- **Render Thread:** Dedicated to pixel rasterization and frame compositing.  
- **Worker Threads (up to 8):** Assist the renderer with large texture regions and parallel drawing tasks.


## Build Instructions

### Requirements
- Visual Studio 2022 (C++20)
- Windows 10 or 11
- DirectX 11 SDK

### Steps
1. Clone the repository.  
2. Build it with **Visual Studio** in **Release mode** (Debug will be very slow because of the terminal-based logger).

> "No frameworks. No shortcuts. Just pixels and pure C++."