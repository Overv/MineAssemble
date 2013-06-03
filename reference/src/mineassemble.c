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

// Configuration
#define worldSX 16
#define worldSY 16
#define worldSZ 16

// Macros
#define IN_WORLD(x, y, z) \
    (x >= 0 && y >= 0 && z >= 0 && x < worldSX && y < worldSY && worldSZ)

// Types
typedef struct vec3 {
    float x, y, z;
} vec3;

enum block_t {
    BLOCK_AIR,
    BLOCK_DIRT
};

enum face_t {
    FACE_LEFT,
    FACE_RIGHT,
    FACE_BOTTOM,
    FACE_TOP,
    FACE_BACK,
    FACE_FRONT
};

// Functions
void initVideo();
void mainLoop();

void initWorld();
int getLight(int x, int z);
Uint8 getBlock(int x, int y, int z);
void setBlock(int x, int y, int z, Uint8 type);

void drawFrame(Uint32* pixels);

void setView(float yaw, float pitch);
Uint32 raytrace(vec3 pos, vec3 dir);
Uint32 rayColor(int x, int y, int z, int face);
void faceNormal(int face, int* x, int* y, int* z);
vec3 rayDir(int x, int y);
Uint32 rgb(Uint8 r, Uint8 g, Uint8 b);

// Globals
SDL_Surface* screen;

Uint8 world[worldSX * worldSY * worldSZ] = {0};
Uint8 lighting[worldSX * worldSY] = {0};

vec3 playerPos = {8, 10, 8};

// The sine and cosine are the same for all pixels
float pitch = 0.0f;
float pitchC = 1.0f;
float pitchS = 0.0f;

float yaw = 0.0f;
float yawC = 1.0f;
float yawS = 0.0f;

int main() {
    initVideo();

    initWorld();

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
        while (SDL_PollEvent(&windowEvent)) {
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
// Code below this line is not part of boilerplate
//

void initWorld() {
    // Make flat grass landscape
    for (int x = 0; x < worldSX; x++) {
        for (int y = 0; y < worldSY; y++) {
            for (int z = 0; z < worldSZ; z++) {
                setBlock(x, y, z, y >= worldSY / 2 ? BLOCK_AIR : BLOCK_DIRT);
            }
        }
    }

    // Add arch
    //setBlock(11, 8, 4, BLOCK_DIRT);
    //setBlock(11, 9, 4, BLOCK_DIRT);
    setBlock(11, 10, 4, BLOCK_DIRT);
    setBlock(10, 10, 4, BLOCK_DIRT);
    setBlock(9, 10, 4, BLOCK_DIRT);
    setBlock(9, 9, 4, BLOCK_DIRT);
    setBlock(9, 8, 4, BLOCK_DIRT);
}

int getLight(int x, int z) {
    return lighting[x * worldSZ + z];
}

Uint8 getBlock(int x, int y, int z) {
    return world[x * worldSY * worldSZ + y * worldSZ + z];
}

void setBlock(int x, int y, int z, Uint8 type) {
    world[x * worldSY * worldSZ + y * worldSZ + z] = type;

    // Update lightmap
    int lightIdx = x * worldSZ + z;

    if (type != BLOCK_AIR && lighting[lightIdx] < y) {
        lighting[lightIdx] = y;
    } else if (type == BLOCK_AIR && lighting[lightIdx] < y) {
        y = worldSY - 1;

        while (y > 0 && getBlock(x, y, z) == BLOCK_AIR) {
            y--;
        }

        lighting[lightIdx] = y;
    }
}

void drawFrame(Uint32* pixels) {
    float x = 0;
    float y = 0;

    setView(0.0f, 0.0f);

    for (int i = 0; i < 320 * 200; i++) {
        *pixels = raytrace(playerPos, rayDir(x, y));

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

// Returns final color
Uint32 raytrace(vec3 pos, vec3 dir) {
    int x = (int) pos.x;
    int y = (int) pos.y;
    int z = (int) pos.z;

    int x_dir = dir.x >= 0.0f ? 1 : -1;
    int y_dir = dir.y >= 0.0f ? 1 : -1;
    int z_dir = dir.z >= 0.0f ? 1 : -1;

    float dx_off = x_dir > 0 ? 1.0f : 0.0f;
    float dy_off = y_dir > 0 ? 1.0f : 0.0f;
    float dz_off = z_dir > 0 ? 1.0f : 0.0f;

    int x_face = x_dir > 0 ? FACE_LEFT : FACE_RIGHT;
    int y_face = y_dir > 0 ? FACE_BOTTOM : FACE_TOP;
    int z_face = z_dir > 0 ? FACE_BACK : FACE_FRONT;

    int face = FACE_TOP;
    
    // Assumption is made that the camera is never outside the world
    while (IN_WORLD(x, y, z)) {
        // Determine if block is solid
        if (getBlock(x, y, z) != BLOCK_AIR) {
            return rayColor(x, y, z, face);
        }

        // Remaining distance inside this block given ray direction
        float dx = x - pos.x + dx_off;
        float dy = y - pos.y + dy_off;
        float dz = z - pos.z + dz_off;
        
        // Calculate distance for each dimension
        float t1 = dx / dir.x;
        float t2 = dy / dir.y;
        float t3 = dz / dir.z;
        
        // Find closest hit
        if (t1 <= t2 && t1 <= t3) {
            pos.x += dx;
            pos.y += t1 * dir.y;
            pos.z += t1 * dir.z;
            x += x_dir;
            face = x_face;
        }
        if (t2 <= t1 && t2 <= t3) {
            pos.x += t2 * dir.x;
            pos.y += dy;
            pos.z += t2 * dir.z;
            y += y_dir;
            face = y_face;
        }
        if (t3 <= t1 && t3 <= t2) {
            pos.x += t3 * dir.x;
            pos.y += t3 * dir.y;
            pos.z += dz;
            z += z_dir;
            face = z_face;
        }
    }

    return rgb(0, 0, 0);
}

Uint32 rayColor(int x, int y, int z, int face) {
    int nx, ny, nz;
    faceNormal(face, &nx, &ny, &nz);

    // Side is dark if there are higher blocks in the column faced by it
    // Left and back sides are always dark to simulate a sun angle
    if (IN_WORLD(x + nx, y, z + nz) && getLight(x + nx, z + nz) > y) {
        return rgb(abs(nx) * 200, abs(ny) * 200, abs(nz) * 200);
    } else if (face == FACE_LEFT || face == FACE_BACK) {
        return rgb(abs(nx) * 200, abs(ny) * 200, abs(nz) * 200);
    } else {
        return rgb(abs(nx) * 255, abs(ny) * 255, abs(nz) * 255);
    }
}

void faceNormal(int face, int* x, int* y, int* z) {
    *x = 0;
    *y = 0;
    *z = 0;

    switch (face) {
        case FACE_LEFT: *x = -1; break;
        case FACE_RIGHT: *x = 1; break;
        case FACE_BOTTOM: *y = -1; break;
        case FACE_TOP: *y = 1; break;
        case FACE_BACK: *z = -1; break;
        case FACE_FRONT: *z = 1; break;
    }
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