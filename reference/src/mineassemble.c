// putenv declaration
#define _XOPEN_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

// SDL library
#include <SDL/SDL.h>
#include <SDL/SDL_video.h>

// Types
typedef struct vec3 {
    float x, y, z;
} vec3;

// Functions
void initVideo();
void mainLoop();
void drawFrame(Uint32* pixels);

void setView(float yaw, float pitch);
void rayDir(int x, int y, vec3* n);
Uint32 rgb(Uint8 r, Uint8 g, Uint8 b);

// Globals
SDL_Surface* screen;

vec3 playerPos = {0, 0, 0};

// The sine and cosine are the same for all pixels
float pitch = 0.0f;
float pitchC = 1.0f;
float pitchS = 0.0f;

float yaw = 0.0f;
float yawC = 1.0f;
float yawS = 0.0f;

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
}

void mainLoop() {
    SDL_Event windowEvent;

    int frames = 0;
    int lsec = 0;
    char titleBuf[64];

    while (true) {
        if (SDL_PollEvent(&windowEvent)) {
            if (windowEvent.type == SDL_QUIT) return;
        }

        SDL_LockSurface(screen);

        drawFrame((Uint32*) screen->pixels);

        SDL_UnlockSurface(screen);

        SDL_UpdateRect(screen, 0, 0, 320, 200);

        frames++;
        if (lsec != time(0)) {
            sprintf(titleBuf, "MineAssemble (FPS: %d)", frames);
            SDL_WM_SetCaption(titleBuf, 0);

            frames = 0;
            lsec = time(0);
        }
    }
}

//
// Code below this line is restricted to 150 statements
//

void drawFrame(Uint32* pixels) {
    float x = 0;
    float y = 0;

    setView(0.0f, 0.0f);

    for (int i = 0; i < 320 * 200; i++) {
        vec3 dir;
        rayDir(x, y, &dir);

        *(pixels + i) =
            rgb(fabs(dir.x * 255.0f),
                fabs(dir.y * 255.0f),
                fabs(dir.z * 255.0f));

        x++;
        if (x == 320) {
            x = 0;
            y++;
        }
    }
}

void setView(float p, float y) {
    pitch = p;
    pitchS = sinf(pitch);
    pitchC = cosf(pitch);

    yaw = y;
    yawS = sinf(yaw);
    yawC = cosf(yaw);
}

void rayDir(int x, int y, vec3* n) {
    // This is simply a precomputed version of the actual linear
    // transformation, which is the inverse of the common view and
    // projection transformation used in rasterization.
    float clipX = x / 160.0f - 1.0f;
    float clipY = 1.0f - y / 100.0f;

    n->x = 1.6f * yawC * clipX + yawS * pitchS * clipY - pitchC * yawS;
    n->y = pitchC * clipY + pitchS;
    n->z = -1.6f * yawS * clipX + yawC * pitchS * clipY - pitchC * yawC;

    // Normalize
    float length = sqrtf(n->x * n->x + n->y * n->y + n->z * n->z);
    n->x /= length;
    n->y /= length;
    n->z /= length;
}

Uint32 rgb(Uint8 r, Uint8 g, Uint8 b) {
    return (0xFF << 24) | (r << 16) | (g << 8) | b;
}