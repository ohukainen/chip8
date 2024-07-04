#include "game.hpp"

#include <SDL_events.h>
#include <cstdint>
#include <iostream>
#include <array>

Game::Game(const std::string& title) :
mIsRunning(false), mWidth(640), mHeight(320) // TODO: make windowsize dynamic
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

void Game::handleEvents(std::array<bool, 16>& keyState) {
    SDL_Event event;
    SDL_PollEvent(&event);
    
    if (event.type == SDL_QUIT) {
        mIsRunning = false;
    }
    if (event.type == SDL_KEYDOWN) {
        if (SDLK_1 == event.key.keysym.sym) {
            keyState[1] = true;
        }
        if (SDLK_2 == event.key.keysym.sym) {
            keyState[2] = true;
        }
        if (SDLK_3 == event.key.keysym.sym) {
            keyState[3] = true;
        }
        if (SDLK_4 == event.key.keysym.sym) {
            keyState[12] = true;
        }
        if (SDLK_q == event.key.keysym.sym) {
            keyState[4] = true;
        }
        if (SDLK_w == event.key.keysym.sym) {
            keyState[5] = true;
        }
        if (SDLK_e == event.key.keysym.sym) {
            keyState[6] = true;
        }
        if (SDLK_r == event.key.keysym.sym) {
            keyState[13] = true;
        }
        if (SDLK_a == event.key.keysym.sym) {
            keyState[7] = true;
        }
        if (SDLK_s == event.key.keysym.sym) {
            keyState[8] = true;
        }
        if (SDLK_d == event.key.keysym.sym) {
            keyState[9] = true;
        }
        if (SDLK_f == event.key.keysym.sym) {
            keyState[14] = true;
        }
        if (SDLK_z == event.key.keysym.sym) {
            keyState[10] = true;
        }
        if (SDLK_x == event.key.keysym.sym) {
            keyState[0] = true;
        }
        if (SDLK_c == event.key.keysym.sym) {
            keyState[11] = true;
        }
        if (SDLK_v == event.key.keysym.sym) {
            keyState[15] = true;
        }
    }
    if (event.type == SDL_KEYUP) {
        if (SDLK_1 == event.key.keysym.sym) {
            keyState[1] = false;
        }
        if (SDLK_2 == event.key.keysym.sym) {
            keyState[2] = false;
        }
        if (SDLK_3 == event.key.keysym.sym) {
            keyState[3] = false;
        }
        if (SDLK_4 == event.key.keysym.sym) {
            keyState[12] = false;
        }
        if (SDLK_q == event.key.keysym.sym) {
            keyState[4] = false;
        }
        if (SDLK_w == event.key.keysym.sym) {
            keyState[5] = false;
        }
        if (SDLK_e == event.key.keysym.sym) {
            keyState[6] = false;
        }
        if (SDLK_r == event.key.keysym.sym) {
            keyState[13] = false;
        }
        if (SDLK_a == event.key.keysym.sym) {
            keyState[7] = false;
        }
        if (SDLK_s == event.key.keysym.sym) {
            keyState[8] = false;
        }
        if (SDLK_d == event.key.keysym.sym) {
            keyState[9] = false;
        }
        if (SDLK_f == event.key.keysym.sym) {
            keyState[14] = false;
        }
        if (SDLK_z == event.key.keysym.sym) {
            keyState[10] = false;
        }
        if (SDLK_x == event.key.keysym.sym) {
            keyState[0] = false;
        }
        if (SDLK_c == event.key.keysym.sym) {
            keyState[11] = false;
        }
        if (SDLK_v == event.key.keysym.sym) {
            keyState[15] = false;
        }
    }
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

    SDL_UpdateTexture(mTextureP, NULL, sdl2Pixels.data(), mWidth * sizeof(uint32_t));
    SDL_RenderClear(mRendererP);
    SDL_RenderCopy(mRendererP, mTextureP, NULL, NULL);
    SDL_RenderPresent(mRendererP);
}

bool Game::isRunning() {
    return mIsRunning;
}
