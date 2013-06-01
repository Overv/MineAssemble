// putenv declaration
#define _XOPEN_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// SDL library
#include <SDL/SDL.h>
#include <SDL/SDL_video.h>

// Functions
void initVideo();
void mainLoop();
void drawFrame(Uint32* pixels);

// Globals
SDL_Surface* screen;

int main() {
    initVideo();

    mainLoop();

    return EXIT_SUCCESS;
}

void initVideo() {
    putenv("SDL_VIDEO_CENTERED=1");

    SDL_Init(SDL_INIT_VIDEO);
    atexit(SDL_Quit);

    screen = SDL_SetVideoMode(320, 200, 32, SDL_SWSURFACE);
    SDL_WM_SetCaption("MineAssemble", 0);
}

void mainLoop() {
    SDL_Event windowEvent;

    while (true) {
        if (SDL_PollEvent(&windowEvent)) {
            if (windowEvent.type == SDL_QUIT) return;
        }

        SDL_LockSurface(screen);

        drawFrame((Uint32*) screen->pixels);

        SDL_UnlockSurface(screen);

        SDL_UpdateRect(screen, 0, 0, 320, 200);
    }
}

void drawFrame(Uint32* pixels) {
    for (int i = 0; i < 320 * 200; i++)
        *(pixels + i) = 0xFFFF0000;
}