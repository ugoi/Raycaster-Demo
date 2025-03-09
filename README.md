# Raycaster Demo

A simple raycaster implementation in C using SDL2.

## Description

This project is a basic raycaster engine that simulates a 3D environment from a 2D map, similar to early first-person games like Wolfenstein 3D. It demonstrates fundamental raycasting principles, including:

- Raycasting from a 2D map to create a 3D-like view
- Player movement and rotation
- Texture mapping on walls
- Simple collision detection

## Requirements

- C compiler (GCC or Clang recommended)
- SDL2 library
- SDL2_image library (for textures)

## Installation

### macOS

```bash
brew install sdl2 sdl2_image
```

### Ubuntu/Debian

```bash
sudo apt-get install libsdl2-dev libsdl2-image-dev
```

### Windows

Download the SDL2 and SDL2_image development libraries for MinGW or Visual Studio from [SDL's website](https://www.libsdl.org/download-2.0.php) and [SDL_image's website](https://www.libsdl.org/projects/SDL_image/).

## Building and Running

```bash
make
./raycaster
```

## Controls

- W: Move forward
- S: Move backward
- A: Rotate left
- D: Rotate right
- ESC: Exit the game

## Project Structure

- `main.c`: Entry point and game loop
- `raycaster.c/h`: Raycasting implementation
- `player.c/h`: Player state and movement
- `map.c/h`: Map definition and functions
- `textures.c/h`: Texture loading and handling
- `Makefile`: Build configuration
