#pragma once

#include <SDL.h>
#include <cstdint>
#include <string>
#include <array>

class Game {
public:
    Game(const std::string& title);
    ~Game();
    
    void handleEvents(std::array<bool, 16>& keyState); 

    void drawScreen(const std::array<uint8_t, 64 * 32>& screenState);
    
    bool isRunning();
private: 
    bool mIsRunning;
    const int mWidth;
    const int mHeight;

    SDL_Window* mWindowP;
    SDL_Renderer* mRendererP;
    SDL_Texture* mTextureP;
};
