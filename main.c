#include <stdio.h>
#include <stdlib.h>
#include "engine.h"

int main(int argc, char *argv[]) {
    (void)argc; // Unused parameter
    (void)argv; // Unused parameter

    // Create and initialize the engine
    Engine engine;
    
    // Initialize the raycasting engine
    if (!engine_init(&engine)) {
        fprintf(stderr, "Failed to initialize engine!\n");
        return 1;
    }
    
    // Load maps from the maps directory
    int mapsLoaded = engine_load_maps(&engine, "maps");
    if (mapsLoaded > 0) {
        printf("Loaded %d maps\n", mapsLoaded);
        printf("Press 1-%d keys to switch between maps\n", mapsLoaded);
        
        // Set the first map as active
        engine_set_map(&engine, 1);
    } else {
        printf("No maps loaded, using default map\n");
    }
    
    // Run the main game loop
    int result = engine_run(&engine);
    
    // Cleanup and exit
    engine_cleanup(&engine);
    
    return result;
} 