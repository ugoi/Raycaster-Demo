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
    
    // Run the main game loop
    int result = engine_run(&engine);
    
    // Cleanup and exit
    engine_cleanup(&engine);
    
    return result;
} 