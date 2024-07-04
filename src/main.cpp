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
    std::string gameFilepath = argv[1];

    Game game(gameFilepath);

    Chip8 chip8;
    chip8.initialize();

    if (!chip8.loadGame(gameFilepath)) {
        std::cout << "Game to large!" << std::endl;
        return 0;
    }
    
    while (game.isRunning()) {
        Word operationCode = chip8.fetchNextOperationCode();
        chip8.executeOperationCode(operationCode);
        
        if (chip8.getDrawFlag()) {
            game.drawScreen(chip8.getGraphix());
        }

        game.handleEvents();

        chip8.setKeys(game.handleEvents());
    }
    return 0;
}
