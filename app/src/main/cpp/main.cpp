#include <SDL3/SDL.h>
#include <stdio.h>

int main(void) 
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == false) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Pixel Pocket Core", 160 * 4, 144 * 4, 0);
    if (!window) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    printf("SDL3 Initialized successfully on %s!\n", SDL_GetPlatform());

    // Your emulation loop would go here

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
