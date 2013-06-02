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
Uint32 raytrace(vec3 pos, vec3 dir);
vec3 rayDir(int x, int y);
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
    vec3 pos = {0.2f, 0.2f, 0.2f};
    vec3 dir = {0.2f, 0.4f, 0.0f};
    raytrace(pos, dir);

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
        //*(pixels + i) = raytrace(playerPos, rayDir(x, y));
        *pixels = rgb(255, 0, 0);

        pixels++;
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

Uint32 raytrace(vec3 pos, vec3 dir) {
    int x = (int) pos.x;
    int y = (int) pos.y;
    int z = (int) pos.z;

    int x_dir = dir.x >= 0.0f ? 1 : -1;
    int y_dir = dir.y >= 0.0f ? 1 : -1;
    int z_dir = dir.z >= 0.0f ? 1 : -1;

    float dx_off = dir.x >= 0.0f ? 1.0f : 0.0f;
    float dy_off = dir.y >= 0.0f ? 1.0f : 0.0f;
    float dz_off = dir.z >= 0.0f ? 1.0f : 0.0f;

    // TEMP
    int n = 10;
    
    for (int i = 0; i < n; i++) {
        float dx = x - pos.x + dx_off;
        float dy = y - pos.y + dy_off;
        float dz = z - pos.z + dz_off;
        
        float t1 = dx / dir.x;
        float t2 = dy / dir.y;
        float t3 = dz / dir.z;

        printf("%d, %d, %d\n", x, y, z);
        
        if (t1 <= t2 && t1 <= t3) {
            pos.x += dx;
            pos.y += t1 * dir.y;
            pos.z += t1 * dir.z;
            x += x_dir;
        }
        if (t2 <= t1 && t2 <= t3) {
            pos.x += t2 * dir.x;
            pos.y += dy;
            pos.z += t2 * dir.z;
            y += y_dir;
        }
        if (t3 <= t1 && t3 <= t2) {
            pos.x += t3 * dir.x;
            pos.y += t3 * dir.y;
            pos.z += dz;
            z += z_dir;
        }
    }

    printf("%d, %d, %d\n", x, y, z);

    return rgb(255, 0, 0);
}

vec3 rayDir(int x, int y) {
    vec3 d;

    // This is simply a precomputed version of the actual linear
    // transformation, which is the inverse of the common view and
    // projection transformation used in rasterization.
    float clipX = x / 160.0f - 1.0f;
    float clipY = 1.0f - y / 100.0f;

    d.x = 1.6f * yawC * clipX + yawS * pitchS * clipY - pitchC * yawS;
    d.y = pitchC * clipY + pitchS;
    d.z = -1.6f * yawS * clipX + yawC * pitchS * clipY - pitchC * yawC;

    // Normalize
    float length = sqrtf(d.x * d.x + d.y * d.y + d.z * d.z);
    d.x /= length;
    d.y /= length;
    d.z /= length;

    return d;
}

Uint32 rgb(Uint8 r, Uint8 g, Uint8 b) {
    return (0xFF << 24) | (r << 16) | (g << 8) | b;
}