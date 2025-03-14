#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <dirent.h>

#include "engine.h"

// A simple 24x24 default map
// 0 = empty space
// 1-4 = different wall types
static const int DEFAULT_MAP[MAP_HEIGHT][MAP_WIDTH] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,2,2,2,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
    {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,3,0,0,0,3,0,0,0,1},
    {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,2,2,0,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,4,0,0,0,0,5,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,4,0,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

// Example maps collection
#define MAX_MAPS 10

// Map format key codes for file parsing
#define MAP_MARKER "MAP:"
#define NAME_MARKER "NAME:"
#define START_MARKER "START:"
#define DATA_MARKER "DATA:"

// ****************************************************
// Private (static) function declarations
// ****************************************************

// Handle events (keyboard input, quit events)
static int engine_handle_events(Engine *engine);

// Calculate time delta for frame-rate independent movement
static double engine_calculate_delta_time(Engine *engine);

// Get wall color based on map value and side
static void engine_get_wall_color(Engine *engine, int mapValue, int side, SDL_Color *color);

// Get texture by index
static SDL_Texture* engine_get_texture(Engine *engine, int index);

// Draw a textured vertical line
static void engine_draw_textured_line(Engine *engine, int x, int drawStart, int drawEnd, 
                            double wallX, int texNum, double perpWallDist, int side);

// Load a map from a string buffer
static int engine_load_map_from_buffer(Engine *engine, const char *buffer, Map *map);

// ****************************************************
// Public API Implementation
// ****************************************************

// Initialize the engine with default settings
int engine_init(Engine *engine) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
        return 0;
    }
    
    // Create window
    engine->window = SDL_CreateWindow(
        "Raycaster Demo",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    
    if (engine->window == NULL) {
        fprintf(stderr, "Window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 0;
    }
    
    // Create renderer
    engine->renderer = SDL_CreateRenderer(
        engine->window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    
    if (engine->renderer == NULL) {
        fprintf(stderr, "Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(engine->window);
        SDL_Quit();
        return 0;
    }
    
    // Initialize map system
    engine->availableMaps = NULL;
    engine->mapCount = 0;
    engine->currentMapIndex = 0;
    
    // Initialize timing system
    engine->lastTime = SDL_GetTicks();
    
    // Initialize map
    engine_init_map(engine);
    
    // Initialize textures
    if (!engine_init_textures(engine)) {
        fprintf(stderr, "Failed to initialize textures!\n");
        engine_cleanup(engine);
        return 0;
    }
    
    // Initialize player at starting position
    engine_init_player(engine, engine->map.startX, engine->map.startY);
    
    // Set running flag
    engine->running = 1;
    
    return 1;
}

// Clean up resources allocated by the engine
void engine_cleanup(Engine *engine) {
    engine_cleanup_textures(engine);
    
    // Clean up maps
    if (engine->availableMaps) {
        free(engine->availableMaps);
        engine->availableMaps = NULL;
    }
    
    if (engine->renderer) {
        SDL_DestroyRenderer(engine->renderer);
        engine->renderer = NULL;
    }
    
    if (engine->window) {
        SDL_DestroyWindow(engine->window);
        engine->window = NULL;
    }
    
    SDL_Quit();
}

// Initialize player with starting position
void engine_init_player(Engine *engine, double posX, double posY) {
    engine->player.posX = posX;
    engine->player.posY = posY;
    engine->player.dirX = -1.0;  // Initial direction vector (looking west)
    engine->player.dirY = 0.0;
    engine->player.planeX = 0.0;  // Camera plane perpendicular to direction vector
    engine->player.planeY = 0.66; // Field of view is about 2 * atan(0.66/1.0) = 66 degrees
    engine->player.moveSpeed = 5.0; // Units per second
    engine->player.rotSpeed = 3.0;  // Radians per second
}

// Initialize the default map
void engine_init_map(Engine *engine) {
    engine->map.width = MAP_WIDTH;
    engine->map.height = MAP_HEIGHT;
    engine->map.startX = 22.0;
    engine->map.startY = 12.0;
    strcpy(engine->map.name, "Default Map");
    
    // Copy default map
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            engine->map.data[y][x] = DEFAULT_MAP[y][x];
        }
    }
}

// Create a map with the given data
int engine_create_map(Engine *engine, const int *mapData, int width, int height, 
                     double startX, double startY, const char *name) {
    // Validate input parameters
    if (!mapData || width <= 0 || height <= 0 || width > MAP_WIDTH || height > MAP_HEIGHT) {
        return 0;
    }
    
    // Expand map array if needed
    if (engine->mapCount == 0) {
        engine->availableMaps = (Map*)malloc(MAX_MAPS * sizeof(Map));
        if (!engine->availableMaps) {
            return 0;  // Memory allocation failed
        }
    } else if (engine->mapCount >= MAX_MAPS) {
        return 0;  // Reached maximum map count
    }
    
    // Fill in the new map
    Map *newMap = &engine->availableMaps[engine->mapCount];
    newMap->width = width;
    newMap->height = height;
    newMap->startX = startX;
    newMap->startY = startY;
    strncpy(newMap->name, name, sizeof(newMap->name) - 1);
    newMap->name[sizeof(newMap->name) - 1] = '\0';  // Ensure null termination
    
    // Copy the map data
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            newMap->data[y][x] = mapData[y * width + x];
        }
    }
    
    // Clear any remaining cells if the map is smaller than MAP_WIDTH/MAP_HEIGHT
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            if (y >= height || x >= width) {
                newMap->data[y][x] = 0;
            }
        }
    }
    
    engine->mapCount++;
    return 1;
}

// Load a specific map by index
int engine_set_map(Engine *engine, int mapIndex) {
    if (mapIndex < 0 || mapIndex >= engine->mapCount) {
        return 0;
    }
    
    // Copy the selected map to the active map
    Map *selectedMap = &engine->availableMaps[mapIndex];
    memcpy(&engine->map, selectedMap, sizeof(Map));
    
    // Reset player position to map's starting position
    engine_init_player(engine, engine->map.startX, engine->map.startY);
    
    engine->currentMapIndex = mapIndex;
    return 1;
}

// Get a list of available map names
const char** engine_get_map_names(Engine *engine) {
    if (engine->mapCount == 0) {
        return NULL;
    }
    
    const char **names = (const char**)malloc(engine->mapCount * sizeof(char*));
    if (!names) {
        return NULL;
    }
    
    for (int i = 0; i < engine->mapCount; i++) {
        names[i] = engine->availableMaps[i].name;
    }
    
    return names;
}

// Load a map from a file
int engine_load_map_from_file(Engine *engine, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Could not open map file: %s\n", filename);
        return 0;
    }
    
    // Read file into a buffer
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);
    
    char *buffer = (char*)malloc(fileSize + 1);
    if (!buffer) {
        fclose(file);
        return 0;
    }
    
    size_t bytesRead = fread(buffer, 1, fileSize, file);
    buffer[bytesRead] = '\0';  // Null terminate the string
    fclose(file);
    
    // Load the map from the buffer into a new map
    if (engine->mapCount >= MAX_MAPS) {
        free(buffer);
        return 0;  // Reached maximum map count
    }
    
    // If this is the first map, allocate the maps array
    if (engine->mapCount == 0) {
        engine->availableMaps = (Map*)malloc(MAX_MAPS * sizeof(Map));
        if (!engine->availableMaps) {
            free(buffer);
            return 0;  // Memory allocation failed
        }
    }
    
    // Parse the map from the buffer
    Map *newMap = &engine->availableMaps[engine->mapCount];
    int result = engine_load_map_from_buffer(engine, buffer, newMap);
    
    free(buffer);
    
    if (result) {
        engine->mapCount++;
        return 1;
    }
    
    return 0;
}

// Load maps from files in a directory
int engine_load_maps(Engine *engine, const char *directory) {
    DIR *dir;
    struct dirent *ent;
    
    if ((dir = opendir(directory)) == NULL) {
        fprintf(stderr, "Could not open directory: %s\n", directory);
        return 0;
    }
    
    int mapsLoaded = 0;
    
    while ((ent = readdir(dir)) != NULL) {
        // Skip . and ..
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
            continue;
        }
        
        // Check if the file has a .map extension
        char *ext = strrchr(ent->d_name, '.');
        if (!ext || strcmp(ext, ".map") != 0) {
            continue;
        }
        
        // Build the full path
        char fullPath[512];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", directory, ent->d_name);
        
        // Load the map
        if (engine_load_map_from_file(engine, fullPath)) {
            mapsLoaded++;
        }
    }
    
    closedir(dir);
    return mapsLoaded;
}

// ****************************************************
// Private functions implementation
// ****************************************************

// Load a map from a string buffer
static int engine_load_map_from_buffer(Engine *engine, const char *buffer, Map *map) {
    // Default values
    map->width = MAP_WIDTH;
    map->height = MAP_HEIGHT;
    map->startX = 22.0;
    map->startY = 12.0;
    strcpy(map->name, "Unnamed Map");
    
    // Clear the map data
    memset(map->data, 0, sizeof(map->data));
    
    // Parse the buffer line by line
    char line[512];
    int lineNum = 0;
    int dataLine = 0;
    int parsingData = 0;
    
    const char *bufferPtr = buffer;
    
    while (*bufferPtr) {
        // Read a line
        int i = 0;
        while (*bufferPtr && *bufferPtr != '\n' && i < sizeof(line) - 1) {
            line[i++] = *bufferPtr++;
        }
        line[i] = '\0';
        
        // Skip the newline character
        if (*bufferPtr == '\n') {
            bufferPtr++;
        }
        
        // Skip empty lines
        if (strlen(line) == 0) {
            continue;
        }
        
        // Check for section markers
        if (strncmp(line, NAME_MARKER, strlen(NAME_MARKER)) == 0) {
            // Parse map name
            strncpy(map->name, line + strlen(NAME_MARKER), sizeof(map->name) - 1);
            map->name[sizeof(map->name) - 1] = '\0';  // Ensure null termination
            continue;
        } else if (strncmp(line, START_MARKER, strlen(START_MARKER)) == 0) {
            // Parse starting position
            sscanf(line + strlen(START_MARKER), "%lf,%lf", &map->startX, &map->startY);
            continue;
        } else if (strncmp(line, DATA_MARKER, strlen(DATA_MARKER)) == 0) {
            // Start parsing map data
            parsingData = 1;
            dataLine = 0;
            continue;
        }
        
        if (parsingData && dataLine < MAP_HEIGHT) {
            // Parse a row of map data
            int col = 0;
            char *token = strtok(line, " ,\t");
            
            while (token && col < MAP_WIDTH) {
                map->data[dataLine][col++] = atoi(token);
                token = strtok(NULL, " ,\t");
            }
            
            dataLine++;
        }
        
        lineNum++;
    }
    
    // Ensure the map has at least some data
    if (dataLine == 0) {
        return 0;
    }
    
    map->height = dataLine;
    return 1;
}

// Get wall color based on map value and side
static void engine_get_wall_color(Engine *engine, int mapValue, int side, SDL_Color *color) {
    (void)engine; // Suppress unused parameter warning
    
    // Base colors for different wall types
    switch (mapValue) {
        case 1: // Red wall
            color->r = 255;
            color->g = 0;
            color->b = 0;
            break;
        case 2: // Green wall
            color->r = 0;
            color->g = 255;
            color->b = 0;
            break;
        case 3: // Blue wall
            color->r = 0;
            color->g = 0;
            color->b = 255;
            break;
        case 4: // Yellow wall
            color->r = 255;
            color->g = 255;
            color->b = 0;
            break;
        default: // Default gray
            color->r = 128;
            color->g = 128;
            color->b = 128;
            break;
    }
    
    // Make sides darker to create 3D effect
    if (side == 1) {
        color->r = color->r / 2;
        color->g = color->g / 2;
        color->b = color->b / 2;
    }
    
    color->a = 255; // Fully opaque
}

// Initialize textures system
int engine_init_textures(Engine *engine) {
    // Initialize SDL_image
    int flags = IMG_INIT_PNG;
    if (!(IMG_Init(flags) & flags)) {
        fprintf(stderr, "SDL_image initialization failed: %s\n", IMG_GetError());
        return 0;
    }
    
    // For this template, we're just creating colorful textures programmatically
    // In a real game, you would load image files instead
    
    // Create surface for each texture
    SDL_Surface *surfaces[NUM_TEXTURES];
    
    for (int i = 0; i < NUM_TEXTURES; i++) {
        surfaces[i] = SDL_CreateRGBSurface(0, TEX_WIDTH, TEX_HEIGHT, 32, 
                                          0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
        
        if (!surfaces[i]) {
            fprintf(stderr, "Failed to create texture surface: %s\n", SDL_GetError());
            
            // Clean up already created surfaces
            for (int j = 0; j < i; j++) {
                SDL_FreeSurface(surfaces[j]);
            }
            
            IMG_Quit();
            return 0;
        }
        
        // Fill with a pattern based on texture number
        Uint32 *pixels = (Uint32*)surfaces[i]->pixels;
        
        for (int y = 0; y < TEX_HEIGHT; y++) {
            for (int x = 0; x < TEX_WIDTH; x++) {
                Uint8 r, g, b;
                
                // Create different patterns for each texture
                switch (i) {
                    case 0: // Red brick pattern
                        r = ((x % 8 == 0) || (y % 8 == 0)) ? 120 : 210;
                        g = ((x % 8 == 0) || (y % 8 == 0)) ? 60 : 30;
                        b = ((x % 8 == 0) || (y % 8 == 0)) ? 60 : 30;
                        break;
                    case 1: // Green pattern
                        r = (x * y) % 50;
                        g = 150 + ((x * y) % 100);
                        b = (x * y) % 50;
                        break;
                    case 2: // Blue pattern
                        r = 30;
                        g = 30 + ((x + y) % 50);
                        b = 150 + ((x ^ y) % 100);
                        break;
                    case 3: // Yellow/brown stone
                        r = 180 + ((x + y) % 50);
                        g = 120 + ((x ^ y) % 70);
                        b = 40 + ((x * y) % 30);
                        break;
                    case 4: // Gray metal
                    default:
                        {
                            int val = ((x / 4) ^ (y / 4)) % 2 ? 200 : 100;
                            r = g = b = val;
                        }
                        break;
                }
                
                pixels[y * TEX_WIDTH + x] = SDL_MapRGBA(surfaces[i]->format, r, g, b, 255);
            }
        }
        
        // Create texture from surface
        engine->textures.textures[i] = SDL_CreateTextureFromSurface(engine->renderer, surfaces[i]);
        
        // Free the surface as it's no longer needed
        SDL_FreeSurface(surfaces[i]);
        
        if (!engine->textures.textures[i]) {
            fprintf(stderr, "Failed to create texture: %s\n", SDL_GetError());
            
            // Clean up already created textures
            for (int j = 0; j < i; j++) {
                SDL_DestroyTexture(engine->textures.textures[j]);
                engine->textures.textures[j] = NULL;
            }
            
            IMG_Quit();
            return 0;
        }
    }
    
    return 1;
}

// Cleanup textures
void engine_cleanup_textures(Engine *engine) {
    for (int i = 0; i < NUM_TEXTURES; i++) {
        if (engine->textures.textures[i]) {
            SDL_DestroyTexture(engine->textures.textures[i]);
            engine->textures.textures[i] = NULL;
        }
    }
    
    IMG_Quit();
}

// Get texture by index
static SDL_Texture* engine_get_texture(Engine *engine, int index) {
    if (index >= 0 && index < NUM_TEXTURES) {
        return engine->textures.textures[index];
    }
    return NULL;
}

// Handle events (keyboard input, quit events)
static int engine_handle_events(Engine *engine) {
    SDL_Event event;
    
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            engine->running = 0;
            return 1;  // Signal to quit
        }
        
        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                engine->running = 0;
                return 1;  // ESC key pressed, signal to quit
            }
            
            // Map switching with number keys
            if (event.key.keysym.sym >= SDLK_1 && 
                event.key.keysym.sym <= SDLK_9 && 
                (event.key.keysym.sym - SDLK_1) < engine->mapCount) {
                
                int mapIndex = event.key.keysym.sym - SDLK_1;
                engine_set_map(engine, mapIndex);
            }
        }
    }
    
    // Get current keyboard state
    engine->keystate = SDL_GetKeyboardState(NULL);
    
    return 0;  // Continue execution
}

// Calculate time delta for frame-rate independent movement
static double engine_calculate_delta_time(Engine *engine) {
    Uint32 currentTime = SDL_GetTicks();
    double deltaTime = (currentTime - engine->lastTime) / 1000.0;  // Convert to seconds
    
    // Cap delta time to avoid large jumps
    if (deltaTime > 0.05) {
        deltaTime = 0.05;
    }
    
    engine->lastTime = currentTime;
    return deltaTime;
}

// Update player position based on input with collision detection
void engine_move_player(Engine *engine, double deltaTime) {
    const Uint8 *keystate = engine->keystate;
    Player *player = &engine->player;
    
    if (!keystate) {
        return;  // No keyboard state available
    }
    
    // Move forward if W key is pressed
    if (keystate[SDL_SCANCODE_W]) {
        // Calculate new position
        double newX = player->posX + player->dirX * player->moveSpeed * deltaTime;
        double newY = player->posY + player->dirY * player->moveSpeed * deltaTime;
        
        // Check for collision with map boundaries
        if (newX < 0 || newX >= engine->map.width || 
            newY < 0 || newY >= engine->map.height) {
            return; // Don't move, we'd go out of bounds
        }
        
        // Check for collision with walls
        int mapX = (int)newX;
        int mapY = (int)newY;
        
        // Only move if new position is not inside a wall
        if (mapX < engine->map.width && mapY < engine->map.height && 
            engine->map.data[mapY][mapX] == 0) {
            player->posX = newX;
            player->posY = newY;
        }
    }
    
    // Move backward if S key is pressed
    if (keystate[SDL_SCANCODE_S]) {
        double newX = player->posX - player->dirX * player->moveSpeed * deltaTime;
        double newY = player->posY - player->dirY * player->moveSpeed * deltaTime;
        
        // Check for collision with map boundaries
        if (newX < 0 || newX >= engine->map.width || 
            newY < 0 || newY >= engine->map.height) {
            return; // Don't move, we'd go out of bounds
        }
        
        // Check for collision with walls
        int mapX = (int)newX;
        int mapY = (int)newY;
        
        // Only move if new position is not inside a wall
        if (mapX < engine->map.width && mapY < engine->map.height && 
            engine->map.data[mapY][mapX] == 0) {
            player->posX = newX;
            player->posY = newY;
        }
    }
    
    // Rotate right if D key is pressed (clockwise)
    if (keystate[SDL_SCANCODE_D]) {
        double oldDirX = player->dirX;
        double rotSpeed = player->rotSpeed * deltaTime;
        
        // Rotate direction vector
        player->dirX = player->dirX * cos(-rotSpeed) - player->dirY * sin(-rotSpeed);
        player->dirY = oldDirX * sin(-rotSpeed) + player->dirY * cos(-rotSpeed);
        
        // Rotate camera plane
        double oldPlaneX = player->planeX;
        player->planeX = player->planeX * cos(-rotSpeed) - player->planeY * sin(-rotSpeed);
        player->planeY = oldPlaneX * sin(-rotSpeed) + player->planeY * cos(-rotSpeed);
    }
    
    // Rotate left if A key is pressed (counter-clockwise)
    if (keystate[SDL_SCANCODE_A]) {
        double oldDirX = player->dirX;
        double rotSpeed = player->rotSpeed * deltaTime;
        
        // Rotate direction vector
        player->dirX = player->dirX * cos(rotSpeed) - player->dirY * sin(rotSpeed);
        player->dirY = oldDirX * sin(rotSpeed) + player->dirY * cos(rotSpeed);
        
        // Rotate camera plane
        double oldPlaneX = player->planeX;
        player->planeX = player->planeX * cos(rotSpeed) - player->planeY * sin(rotSpeed);
        player->planeY = oldPlaneX * sin(rotSpeed) + player->planeY * cos(rotSpeed);
    }
}

// Draw a textured vertical line
static void engine_draw_textured_line(Engine *engine, int x, int drawStart, int drawEnd, 
                             double wallX, int texNum, double perpWallDist, int side) {
    // Ensure valid texture number
    if (texNum < 0 || texNum >= NUM_TEXTURES || !engine->textures.textures[texNum]) {
        return;
    }
    
    // Texture X coordinate
    int texX = (int)(wallX * (double)TEX_WIDTH);
    
    // Flip texture coordinate for correct direction
    if ((side == 0 && perpWallDist > 0) || (side == 1 && perpWallDist < 0)) {
        texX = TEX_WIDTH - texX - 1;
    }
    
    // Height of the wall slice to draw
    int lineHeight = drawEnd - drawStart;
    
    // Draw the vertical textured line
    SDL_Rect srcRect = { texX, 0, 1, TEX_HEIGHT };
    SDL_Rect dstRect = { x, drawStart, 1, lineHeight };
    
    // Adjust brightness based on side and distance
    int alpha = 255;
    if (side == 1) {
        alpha = 192; // Make y-sides slightly darker
    }
    
    // Apply distance fog effect
    if (perpWallDist > 5.0) {
        alpha = (int)(alpha * (10.0 - perpWallDist) / 5.0);
        if (alpha < 0) alpha = 0;
    }
    
    // Set the blending mode and alpha
    SDL_SetTextureAlphaMod(engine->textures.textures[texNum], alpha);
    
    // Render the texture line
    SDL_RenderCopy(engine->renderer, engine->textures.textures[texNum], &srcRect, &dstRect);
}

// Render the current scene using raycasting
void engine_render_scene(Engine *engine) {
    // Draw ceiling (top half of screen)
    SDL_SetRenderDrawColor(engine->renderer, 100, 100, 170, 255);  // Sky blue color
    SDL_Rect ceilingRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT / 2};
    SDL_RenderFillRect(engine->renderer, &ceilingRect);
    
    // Draw floor (bottom half of screen)
    SDL_SetRenderDrawColor(engine->renderer, 80, 80, 80, 255);  // Floor gray color
    SDL_Rect floorRect = {0, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT / 2};
    SDL_RenderFillRect(engine->renderer, &floorRect);
    
    // Get player pointer for convenience
    Player *player = &engine->player;
    
    // For each vertical column of the screen
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        // Calculate ray position and direction
        double cameraX = 2.0 * x / (double)SCREEN_WIDTH - 1.0; // x-coordinate in camera space
        double rayDirX = player->dirX + player->planeX * cameraX;
        double rayDirY = player->dirY + player->planeY * cameraX;
        
        // Which box of the map we're in
        int mapX = (int)player->posX;
        int mapY = (int)player->posY;
        
        // Length of ray from current position to next x or y-side
        double sideDistX;
        double sideDistY;
        
        // Length of ray from one x or y-side to next x or y-side
        double deltaDistX = fabs(1.0 / rayDirX);
        double deltaDistY = fabs(1.0 / rayDirY);
        double perpWallDist;
        
        // Direction to step in x or y direction (either +1 or -1)
        int stepX;
        int stepY;
        
        // Was a wall hit?
        int hit = 0;
        // Was it a NS or EW wall?
        int side;
        
        // Calculate step and initial sideDist
        if (rayDirX < 0) {
            stepX = -1;
            sideDistX = (player->posX - mapX) * deltaDistX;
        } else {
            stepX = 1;
            sideDistX = (mapX + 1.0 - player->posX) * deltaDistX;
        }
        
        if (rayDirY < 0) {
            stepY = -1;
            sideDistY = (player->posY - mapY) * deltaDistY;
        } else {
            stepY = 1;
            sideDistY = (mapY + 1.0 - player->posY) * deltaDistY;
        }
        
        // Perform DDA (Digital Differential Analysis)
        while (hit == 0) {
            // Jump to next map square
            if (sideDistX < sideDistY) {
                sideDistX += deltaDistX;
                mapX += stepX;
                side = 0;
            } else {
                sideDistY += deltaDistY;
                mapY += stepY;
                side = 1;
            }
            
            // Check if ray has hit a wall
            if (mapX < 0 || mapX >= engine->map.width || mapY < 0 || mapY >= engine->map.height) {
                // Out of bounds, break loop
                break;
            }
            
            if (engine->map.data[mapY][mapX] > 0) {
                hit = 1;
            }
        }
        
        // Calculate distance projected on camera direction
        if (!hit) {
            continue;  // Skip this ray if we didn't hit anything
        }
        
        if (side == 0) {
            perpWallDist = (mapX - player->posX + (1 - stepX) / 2) / rayDirX;
        } else {
            perpWallDist = (mapY - player->posY + (1 - stepY) / 2) / rayDirY;
        }
        
        // Calculate height of line to draw on screen
        int lineHeight = (int)(SCREEN_HEIGHT / perpWallDist);
        
        // Calculate lowest and highest pixel to fill in current stripe
        int drawStart = -lineHeight / 2 + SCREEN_HEIGHT / 2;
        if (drawStart < 0) drawStart = 0;
        
        int drawEnd = lineHeight / 2 + SCREEN_HEIGHT / 2;
        if (drawEnd >= SCREEN_HEIGHT) drawEnd = SCREEN_HEIGHT - 1;
        
        // Choose wall color
        SDL_Color wallColor;
        engine_get_wall_color(engine, engine->map.data[mapY][mapX], side, &wallColor);
        
        // Draw the vertical line
        SDL_SetRenderDrawColor(engine->renderer, wallColor.r, wallColor.g, wallColor.b, wallColor.a);
        SDL_RenderDrawLine(engine->renderer, x, drawStart, x, drawEnd);
        
        // For texture mapping, calculate where the wall was hit
        double wallX;
        if (side == 0) {
            wallX = player->posY + perpWallDist * rayDirY;
        } else {
            wallX = player->posX + perpWallDist * rayDirX;
        }
        wallX -= floor(wallX);  // Only fractional part
        
        // For textured version (uncomment if you want to use textures)
        // int texNum = engine->map.data[mapY][mapX] - 1;  // 1-indexed to 0-indexed for texture
        // engine_draw_textured_line(engine, x, drawStart, drawEnd, wallX, texNum, perpWallDist, side);
    }
}

// Main game loop
int engine_run(Engine *engine) {
    while (engine->running) {
        // Handle events (keyboard, mouse, quit)
        engine_handle_events(engine);
        
        // Calculate time delta for frame-rate independent movement
        double deltaTime = engine_calculate_delta_time(engine);
        
        // Update player position based on input
        engine_move_player(engine, deltaTime);
        
        // Clear screen
        SDL_SetRenderDrawColor(engine->renderer, 0, 0, 0, 255);
        SDL_RenderClear(engine->renderer);
        
        // Perform raycasting and render the scene
        engine_render_scene(engine);
        
        // Present the rendered scene
        SDL_RenderPresent(engine->renderer);
    }
    
    return 0;
} 