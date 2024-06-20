#include "game.hpp"

#include <iostream>
#include <array>

Game::Game(const std::string& title) :
mFPS(60), mWidth(640), mHeight(320)
{
    mIsRunning = false;

    mWindowP = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, mWidth, mHeight, SDL_WINDOW_SHOWN);
    if (mWindowP == NULL) {
        std::cout << "Window could not be created, SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return;
    } 

    mRendererP = SDL_CreateRenderer(mWindowP, -1, SDL_RENDERER_SOFTWARE);
    if (mRendererP == NULL) {
        std::cout << "Renderer could not be created, SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(mWindowP);
        SDL_Quit();
        return;
    }

    mTextureP = SDL_CreateTexture(mRendererP, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, mWidth, mHeight);
    if (mTextureP == NULL) {
        std::cout << "Texture could not be created, SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(mWindowP);
        SDL_Quit();
        return;
    }
    mIsRunning = true;
}

Game::~Game() {
    SDL_DestroyRenderer(mRendererP);
    SDL_DestroyWindow(mWindowP);

    SDL_Quit();
}

void Game::handleEvents() {
    SDL_Event event;
    SDL_PollEvent(&event);
    
    switch (event.type) {
        case SDL_QUIT: 
            mIsRunning = false;
            break;

        default: 
            break;
    }
}

void Game::drawScreen(std::array<uint8_t, 64 * 32> screenState) {
    SDL_RenderClear(mRendererP);

    int x;
    int y;
    int screenX;    
    int screenY; 
    std::array<uint32_t, 640 * 320> argbPixels;
    
    for (int pixel = 0; pixel < 640 * 320; pixel++) {
        x = pixel % mWidth;
        y = pixel / mWidth;
        screenX = x / 10;    
        screenY = y / 10;
        if (screenState[screenX + (screenY * 64)] != 0) {
            argbPixels[x + (y * mWidth)] = 0xFFFFFFFF; // White (ARGB: 255, 255, 255, 255)
        } 
        else {
            argbPixels[x + (y * mWidth)] = 0xFF000000; // Black (ARGB: 255, 0, 0, 0)
        }
    }

    SDL_UpdateTexture(mTextureP, NULL, argbPixels.data(), mWidth * sizeof(uint32_t));
    SDL_RenderCopy(mRendererP, mTextureP, NULL, NULL);
    SDL_RenderPresent(mRendererP);
    SDL_DestroyTexture(mTextureP);
}

bool Game::isRunning() {
    return mIsRunning;
}
