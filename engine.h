#ifndef ENGINE_H
#define ENGINE_H

#include <SDL.h>
#include <SDL_image.h>

#ifdef __cplusplus
extern "C" {
#endif

// Screen dimensions
#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

// Texture dimensions
#define TEX_WIDTH 64
#define TEX_HEIGHT 64
#define NUM_TEXTURES 5

// Map dimensions
#define MAP_WIDTH 24
#define MAP_HEIGHT 24

// Map tile types
#define TILE_EMPTY 0
#define TILE_WALL 1
#define TILE_WALL2 2
#define TILE_WALL3 3
#define TILE_WALL4 4

// Structure representing the player.
typedef struct Player {
    double posX;    // Player X position
    double posY;    // Player Y position
    double dirX;    // Direction vector X component
    double dirY;    // Direction vector Y component
    double planeX;  // Camera plane X component
    double planeY;  // Camera plane Y component
    double moveSpeed; // Movement speed
    double rotSpeed;  // Rotation speed
} Player;

// Structure representing the map
typedef struct Map {
    int data[MAP_HEIGHT][MAP_WIDTH];
    int width;
    int height;
} Map;

// Structure for the textures
typedef struct Textures {
    SDL_Texture* textures[NUM_TEXTURES];
} Textures;

// Structure holding the engine state and configuration.
typedef struct Engine {
    SDL_Window *window;
    SDL_Renderer *renderer;
    Player player;
    Map map;
    Textures textures;
    Uint32 lastTime;  // For timing
    const Uint8 *keystate;  // For input
    int running;  // Game state
} Engine;

// PUBLIC API:

// Initialize the engine with default settings
int engine_init(Engine *engine);

// Clean up resources allocated by the engine
void engine_cleanup(Engine *engine);

// Initialize player with starting position
void engine_init_player(Engine *engine, double posX, double posY);

// Initialize the map
void engine_init_map(Engine *engine);

// Initialize textures system
int engine_init_textures(Engine *engine);

// Cleanup textures
void engine_cleanup_textures(Engine *engine);

// Update player position based on input with collision detection
void engine_move_player(Engine *engine, double deltaTime);

// Render the current scene using raycasting
void engine_render_scene(Engine *engine);

// Main game loop
int engine_run(Engine *engine);

// Note: Internal functions like engine_get_wall_color, engine_calculate_delta_time,
// engine_handle_events, etc. are not exposed in this public API.
// They are implemented as static functions in engine.c

#ifdef __cplusplus
}
#endif

#endif // ENGINE_H 