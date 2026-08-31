#include "./SDL2/include/SDL.h"
#include <cstdio>
#include <stdexcept>
#include <algorithm>
#include <iostream>

#undef main
const int SCREEN_WIDTH = 512, SCREEN_HEIGHT = 480, LOGICAL_WIDTH = 256, LOGICAL_HEIGHT = 240;

int main() {
    // SDL setup
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {

        printf("Error initializing SDL: %s\n", SDL_GetError());
        throw std::runtime_error("");

    }
    int y = 1;
    {
        int x = 0;
        printf("This is a test");
        y = x;
    }
    std::cout << y;

    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

    window = SDL_CreateWindow("ShayNES", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);

    if (window == NULL) {

        printf("Error creating SDL window: %s\n", SDL_GetError());
        throw std::runtime_error("");

    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (renderer == NULL) {

        printf("Error creating SDL renderer: %s\n", SDL_GetError());
        throw std::runtime_error("");

    }

    SDL_RenderSetLogicalSize(renderer, LOGICAL_WIDTH, LOGICAL_HEIGHT);

    // Create Texture
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, LOGICAL_WIDTH, LOGICAL_HEIGHT);

    // Display a blank screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
    bool running = true;
    uint8_t* locked_pixels = nullptr;
    int pitch = 0;
    uint8_t pixel_buff[LOGICAL_HEIGHT * LOGICAL_WIDTH * 4];
    for (int i = 0; i < LOGICAL_HEIGHT * LOGICAL_WIDTH * 4; i++) {
        if (i % 4 == 3) {
            pixel_buff[i] = 255;
        }
        else if (i % 4 == 0) {
            pixel_buff[i] = 255;
        }
        else {
            pixel_buff[i] = 0;
        }
    }
    SDL_LockTexture(texture, NULL, reinterpret_cast<void **>(&locked_pixels), &pitch);
    std::copy_n(pixel_buff, LOGICAL_WIDTH * LOGICAL_HEIGHT * 4, locked_pixels);
    SDL_UnlockTexture(texture);

    // Render the new pixel data
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {

            switch(event.type) {

                case SDL_QUIT:
                    running = false;
                    break;
    
            }

        }
    }
    SDL_Quit();
    return 0;
}