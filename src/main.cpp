#include "game.hpp"

#include <SDL.h>
#include <chip8.hpp>
#include <iostream>
#include <stdexcept>
#include <string>
#include <filesystem>

static void usage() {
    std::cout << "\nUsage: 'chip8 <filepath> [fps=60]'" << std::endl;
    std::cout << "  filepath    <required>      path to the game binary (absolute or relative)" << std::endl;
    std::cout << "  fps:        [optonal]       fps of the game (30 <= fps <= 1000)" << std::endl;
    std::cout << "\nmArguments in <> are required and arguments in [] are optional" << std::endl;
}

int main(int argc, char** argv) {
    if (argc > 3) {
        std::cout << "ERROR: To many arguments" << std::endl;
        usage();
        return 1;
    }
    else if (argc < 2) {
        std::cout << "ERROR: To few arguments" << std::endl;
        usage();
        return 1;
    }

    std::string gameFilepath = argv[1];
    if (!std::filesystem::exists(gameFilepath)) {
        std::cout << "ERROR: No file with path: '" << gameFilepath << "' found" << std::endl;
        usage();
        return 1;
    }

    uint64_t ticksPerSecond = 60;
    if (argc == 3) {
        try {
            int64_t fps = std::stoll(argv[2]); 
            if (fps < 30) {
                std::cout << "ERROR: To low fps" <<  std::endl;
                usage();
                return 1;
            }
            else if (fps > 1000) {
                std::cout << "ERROR: To high fps" <<  std::endl;
                usage();
                return 1;
            }
            ticksPerSecond = fps;
        }
        catch (const std::invalid_argument& ex) {
            std::cout << "ERROR: Second argument invalid, error message: '" << ex.what() << "'" << std::endl; 
            usage();
            return 1;
        }
        catch (const std::out_of_range& ex) {
            std::cout << "ERROR: Second argument out of range, error message: '" << ex.what() << "'" << std::endl; 
            usage();
            return 1;
        }
        catch (const std::exception& ex) {
            std::cout << "ERROR: Unknown error when parsing fps, error message: '" << ex.what() << "'" << std::endl; 
            usage();
            return 1;
        }
    }

    Game game(gameFilepath);

    Chip8 chip8;
    chip8.initialize(ticksPerSecond);

    if (!chip8.loadGame(gameFilepath)) {
        std::cout << "ERROR: game file to large" << std::endl;
        usage();
        return 0;
    }

        std::array<bool, 16> keyState = chip8.getKeys();
    
    while (game.isRunning()) {
        chip8.emulateCycle();
        
        if (chip8.getDrawFlag()) {
            game.drawScreen(chip8.getGraphix());
        }
        
        game.handleEvents(keyState);

        chip8.setKeys(keyState);
    }
    return 0;
}
