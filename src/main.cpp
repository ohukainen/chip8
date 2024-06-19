#include <SDL.h>
#include <chip8.hpp>
#include <iostream>

int main(int argc, char** argv) {
    Chip8 chip8;
    bool running = true;

    // TODO: set up graphics system with SDL 
    // TODO: setup keyboard input handling 

    if (argc != 1) {
        std::cout << "The program don't support input right now, please don't provide any" << std::endl;
        return 0;
    }
    std::cout << "Started emulator: " << argv[0] << std::endl;

    chip8.initialize();
    
    // TODO: LoadGame with reference to input
    // chip8.loadGame("game.rom");
    
    while (running) {
        chip8.emulateCycle();

        if (chip8.getDrawFlag()) {
            // TODO: Update the screen
        }
       
        //TODO: get keyState from input handling and notify chip8
        // chip8.setKeys(keyState);
        running = false;
    }
    std::cin.get();
    return 0;
}
