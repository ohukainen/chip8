#include "game.hpp"

#include <SDL_render.h>
#include <SDL_video.h>
#include <iostream>
#include <array>

Game::Game(const std::string& title) :
mIsRunning(false), mFPS(60), mWidth(640), mHeight(320) // TODO: make windowsize dynamic
{
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
    SDL_DestroyTexture(mTextureP);

    SDL_Quit();
}

std::array<bool, 16> Game::handleEvents() {
    SDL_Event event;
    SDL_PollEvent(&event);
    
    switch (event.type) {
        case SDL_QUIT: 
            mIsRunning = false;
            break;

        default: 
            break;
    }
    
    return std::array<bool, 16>();
}

void Game::drawScreen(const std::array<uint8_t, 64 * 32>& screenState) {
    int sdl2X;
    int sdl2Y;
    int chip8Col;    
    int chip8Row; 
    std::array<uint32_t, 640 * 320> sdl2Pixels;
    
    for (int pixel = 0; pixel < 640 * 320; pixel++) {
        sdl2X = pixel % mWidth;
        sdl2Y = pixel / mWidth;
        chip8Col = sdl2X / 10;    
        chip8Row = sdl2Y / 10;
        if (screenState[chip8Col + (chip8Row * 64)] != 0) {
            sdl2Pixels[sdl2X + (sdl2Y * mWidth)] = 0xFFFFFFFF; // White (ARGB: 255, 255, 255, 255)
        } 
        else {
            sdl2Pixels[sdl2X + (sdl2Y * mWidth)] = 0xFF000000; // Black (ARGB: 255, 0, 0, 0)
        }
    }

    SDL_RenderClear(mRendererP);
    SDL_UpdateTexture(mTextureP, NULL, sdl2Pixels.data(), mWidth * sizeof(uint32_t));
    SDL_RenderCopy(mRendererP, mTextureP, NULL, NULL);
    SDL_RenderPresent(mRendererP);
    SDL_DestroyTexture(mTextureP);
}

bool Game::isRunning() {
    return mIsRunning;
}
