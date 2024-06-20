#include "game.hpp"

#include <SDL.h>
#include <chip8.hpp>
#include <iostream>

int main(int argc, char** argv) {
    if (argc > 2) {
        std::cout << "To many arguments";
    }
    else if (argc < 2) {
        std::cout << "To few arguments";
    }

    if (argc != 2) {
        std::cout << ", usage: 'chip8 <filepath>" << std::endl;
        return 0;
    }
    std::cout << "Started emulator: " << argv[0] << std::endl;

    Game game(argv[1]);

    Chip8 chip8;
    chip8.initialize();

    std::array<uint8_t, 64 * 32> screenState;
    screenState.fill(0);
    
    int comp = 1;
    for (int i = 0; i < 64 * 32; i++) {
        if ( i % 64 == 0) {
            if (comp == 1) {
                comp = 0;
            }
            else {
                comp = 1;
            }
        }
        if (i % 2 == comp) {
            screenState[i] = 0;
        }
        else {
            screenState[i] = 1;
        }
    }

    // TODO: LoadGame with reference to input
    // chip8.loadGame("game.rom");
            game.drawScreen(screenState);
    
    while (game.isRunning()) {
        // chip8.emulateCycle();

        // if (chip8.getDrawFlag()) {
        // }
        
        game.handleEvents();
        // chip8.setKeys(keyState);
    }
    return 0;
}
