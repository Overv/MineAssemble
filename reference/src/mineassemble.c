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

#define skyColor RGB(158, 207, 255)

#define hFov 90

// Macros
#define IN_WORLD(x, y, z) \
    (x >= 0 && y >= 0 && z >= 0 && x < worldSX && y < worldSY && z < worldSZ)

#define RGB(r, g, b) ((0xff << 24) | ((r) << 16) | ((g) << 8) | (b))

#define RED(col) (((col) & 0xff0000) >> 16)
#define GREEN(col) (((col) & 0x00ff00) >> 8)
#define BLUE(col) ((col) & 0x0000ff)

#define INV_COLOR(col) RGB(255 - RED(col), 255 - GREEN(col), 255 - BLUE(col))

#define UNLIT_COL(col) (UNLIT_RED(col) | UNLIT_GREEN(col) | UNLIT_BLUE(col))
#define UNLIT_RED(col) ((RED(col) * 2 / 3) << 16)
#define UNLIT_GREEN(col) ((GREEN(col) * 2 / 3) << 8)
#define UNLIT_BLUE(col) (BLUE(col) * 2 / 3)

#define CURTIME() (clock() / (float) CLOCKS_PER_SEC)

// Resources
extern unsigned int texGrass[];
extern unsigned int texDirt[];
extern unsigned int texGrassSide[];

// Types
typedef struct vec3_t {
    float x, y, z;
} vec3;

typedef struct hit_t {
    bool hit;
    int x, y, z;
    int nx, ny, nz;
} hit;

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

void handleInput(SDLKey key, bool down);
void update(float dt);
void drawFrame(Uint32* pixels);

void setPos(float x, float y, float z);
void setView(float yaw, float pitch);
Uint32 raytrace(vec3 pos, vec3 dir, hit* info);
Uint32 rayColor(int x, int y, int z, int tex, int face);
void faceNormal(int face, int* x, int* y, int* z);
int texIndex(vec3 pos, int face);
vec3 rayDir(int x, int y);

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

// Input
float lastUpdate = 0.0f;

float dPitch = 0.0f;
float dYaw = 0.0f;

bool keyA = false;
bool keyW = false;
bool keyS = false;
bool keyD = false;

vec3 velocity = {0, 0, 0};

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
    SDL_Event event;

    int frames = 0;
    int lsec = 0;
    char titleBuf[64];

    while (true) {
        // Handle input and other events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    return;

                case SDL_KEYDOWN:
                    handleInput(event.key.keysym.sym, true);
                    break;

                case SDL_KEYUP:
                    handleInput(event.key.keysym.sym, false);
                    break;
            }
        }

        // Update
        update(CURTIME() - lastUpdate);
        lastUpdate = CURTIME();

        // Draw frame
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
    setBlock(11, 8, 4, BLOCK_DIRT);
    setBlock(11, 9, 4, BLOCK_DIRT);
    setBlock(11, 10, 4, BLOCK_DIRT);
    setBlock(10, 10, 4, BLOCK_DIRT);
    setBlock(9, 10, 4, BLOCK_DIRT);
    setBlock(9, 9, 4, BLOCK_DIRT);
    setBlock(9, 8, 4, BLOCK_DIRT);
    setBlock(9, 12, 4, BLOCK_DIRT);

    // Initial player position
    setPos(8.0f, 9.8f, 8.0f);
    setView(0.0f, -0.35f);
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
    } else if (type == BLOCK_AIR && lighting[lightIdx] <= y) {
        y = worldSY - 1;

        while (y > 0 && getBlock(x, y, z) == BLOCK_AIR) {
            y--;
        }

        lighting[lightIdx] = y;
    }
}

void handleInput(SDLKey key, bool down) {
    hit info;

    switch (key) {
        // View
        case SDLK_UP: dPitch += down ? 1.0f : -1.0f; break;
        case SDLK_DOWN: dPitch += down ? -1.0f : 1.0f; break;

        case SDLK_LEFT: dYaw += down ? 1.0f : -1.0f; break;
        case SDLK_RIGHT: dYaw += down ? -1.0f : 1.0f; break;

        // Movement
        case SDLK_a: keyA = down; break;
        case SDLK_w: keyW = down; break;
        case SDLK_s: keyS = down; break;
        case SDLK_d: keyD = down; break;

        // Check if a block was hit and place a new block next to it
        case SDLK_q:
            if (!down) {
                raytrace(playerPos, rayDir(160, 100), &info);

                if (info.hit) {
                    int bx = info.x + info.nx;
                    int by = info.y + info.ny;
                    int bz = info.z + info.nz;

                    if (IN_WORLD(bx, by, bz)) {
                        setBlock(bx, by, bz, BLOCK_DIRT);
                    }
                }
            }
            break;

        // Check if a block was hit and remove it
        case SDLK_e:
            if (!down) {
                raytrace(playerPos, rayDir(160, 100), &info);

                if (info.hit) {
                    setBlock(info.x, info.y, info.z, BLOCK_AIR);
                }
            }
            break;

        default: break;
    }
}

void update(float dt) {
    // Update view
    pitch += 1.2f * dPitch * dt;
    yaw += 1.2f * dYaw * dt;

    setView(pitch, yaw);

    // Set X/Z velocity depending on input
    velocity.x = velocity.z = 0.0f;

    if (keyA) {
        velocity.x += 2.0f * cos(M_PI - yaw);
        velocity.z += 2.0f * sin(M_PI - yaw);
    }
    if (keyW) {
        velocity.x += 2.0f * cos(M_PI * 3 / 2 - yaw);
        velocity.z += 2.0f * sin(M_PI * 3 / 2 - yaw);
    }
    if (keyS) {
        velocity.x += 2.0f * cos(M_PI / 2 - yaw);
        velocity.z += 2.0f * sin(M_PI / 2 - yaw);
    }
    if (keyD) {
        velocity.x += 2.0f * cos(-yaw);
        velocity.z += 2.0f * sin(-yaw);
    }

    // Apply motion
    playerPos.x += velocity.x * dt;
    playerPos.y += velocity.y * dt;
    playerPos.z += velocity.z * dt;
}

void drawFrame(Uint32* pixels) {
    int x = 0;
    int y = 0;

    // Draw world
    Uint32* pixel = pixels;
    for (int i = 0; i < 320 * 200; i++) {
        *pixel = raytrace(playerPos, rayDir(x, y), NULL);

        pixel++;
        x++;
        if (x == 320) {
            x = 0;
            y++;
        }
    }

    // Inverse colors in the center of screen to form an aim reticle
    for (x = 155; x < 165; x++) {
        pixels[99 * 320 + x] = INV_COLOR(pixels[99 * 320 + x]);
        pixels[100 * 320 + x] = INV_COLOR(pixels[100 * 320 + x]);
    }
    for (y = 95; y < 105; y++) {
        if (y == 99 || y == 100) continue;

        pixels[y * 320 + 159] = INV_COLOR(pixels[y * 320 + 159]);
        pixels[y * 320 + 160] = INV_COLOR(pixels[y * 320 + 160]);
    }
}

void setPos(float x, float y, float z) {
    playerPos.x = x;
    playerPos.y = y;
    playerPos.z = z;
}

void setView(float p, float y) {
    pitch = p;

    if (pitch > 1.57f) pitch = 1.57f;
    else if (pitch < -1.57f) pitch = -1.57f;

    pitchS = sinf(pitch);
    pitchC = cosf(pitch);

    yaw = y;
    yawS = sinf(yaw);
    yawC = cosf(yaw);
}

// Returns final color
Uint32 raytrace(vec3 pos, vec3 dir, hit* info) {
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
            pos.x -= x;
            pos.y -= y;
            pos.z -= z;

            // If hit info is requested, no color computation is done
            if (info != NULL) {
                int nx, ny, nz;
                faceNormal(face, &nx, &ny, &nz);

                info->hit = true;
                info->x = x;
                info->y = y;
                info->z = z;
                info->nx = nx;
                info->ny = ny;
                info->nz = nz;

                return 0;
            }

            int tex = texIndex(pos, face);

            return rayColor(x, y, z, tex, face);
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

    if (info != NULL) {
        info->hit = false;
    }

    return skyColor;
}

Uint32 rayColor(int x, int y, int z, int tex, int face) {
    // Get normal
    int nx, ny, nz;
    faceNormal(face, &nx, &ny, &nz);

    // Block is dirt if there's another block directly on top of it
    bool isDirt = y < worldSY - 1 && getBlock(x, y + 1, z) != BLOCK_AIR;

    // Texture lookup
    Uint32 texColor;

    if (face == FACE_BOTTOM || isDirt) {
        texColor = texDirt[tex];
    } else if (face == FACE_TOP) {
        texColor = texGrass[tex];
    } else {
        texColor = texGrassSide[tex];
    }

    // Side is dark if there are higher blocks in the column faced by it
    // Left and back sides are always dark to simulate a sun angle
    if (IN_WORLD(x + nx, y, z + nz) && getLight(x + nx, z + nz) > y) {
        return UNLIT_COL(texColor);
    } else if (face == FACE_LEFT || face == FACE_BACK) {
        return UNLIT_COL(texColor);
    } else {
        return texColor;
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

int texIndex(vec3 pos, int face) {
    float u = 0, v = 0;

    switch (face) {
        case FACE_LEFT: u = pos.z; v = pos.y; break;
        case FACE_RIGHT: u = pos.z; v = pos.y; break;
        case FACE_BOTTOM: u = pos.x; v = pos.z; break;
        case FACE_TOP: u = pos.x; v = pos.z; break;
        case FACE_BACK: u = pos.x; v = pos.y; break;
        case FACE_FRONT: u = pos.x; v = pos.y; break;
    }

    v = 1.0f - v;

    return ((int) (u * 15.0f)) * 16 + (int) (v * 15.0f);
}

vec3 rayDir(int x, int y) {
    static float vFov = -1, fov;

    // Calculate vertical fov and fov constant from specified horizontal fov
    if (vFov == -1) {
        vFov = 2.0f * atan(tan(hFov / 720.0f * M_PI) * 320.0f / 200.0f);
        fov = tan(0.5f * vFov);
    }

    // This is simply a precomputed version of the actual linear
    // transformation, which is the inverse of the common view and
    // projection transformation used in rasterization.
    float clipX = x / 160.0f - 1.0f;
    float clipY = 1.0f - y / 100.0f;

    vec3 d = {
        1.6f * fov * yawC * clipX + fov * yawS * pitchS * clipY - pitchC * yawS,
        fov * pitchC * clipY + pitchS,
        -1.6f * fov * yawS * clipX + fov * yawC * pitchS * clipY - pitchC * yawC
    };

    // Normalize
    float length = sqrtf(d.x * d.x + d.y * d.y + d.z * d.z);
    d.x /= length;
    d.y /= length;
    d.z /= length;

    return d;
}