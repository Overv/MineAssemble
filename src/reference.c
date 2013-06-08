#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#define M_PI 3.14159265358979323846

// Externals
extern uint8_t palette[];
extern uint8_t keys[];
extern uint32_t time;

// Configuration
#define worldSX 16
#define worldSY 16
#define worldSZ 16

#define hFov 90

// Key scancodes
#define KEY_ESC 0x01

#define KEY_Q 0x10
#define KEY_E 0x12

#define KEY_W 0x11
#define KEY_A 0x1E
#define KEY_S 0x1F
#define KEY_D 0x20

#define KEY_SPACE 0x39

#define KEY_UP 0x48
#define KEY_LEFT 0x4B
#define KEY_RIGHT 0x4D
#define KEY_DOWN 0x50

// Macros
#define IN_WORLD(x, y, z) \
    (x >= 0 && y >= 0 && z >= 0 && x < worldSX && y < worldSY && z < worldSZ)

#define CURTIME() (time / 1000.0f)

// Resources
extern uint8_t texGrass[];
extern uint8_t texDirt[];
extern uint8_t texGrassSide[];

// Types
typedef struct vec3_t {
    float x, y, z;
} vec3;

typedef struct hit_t {
    bool hit;
    int x, y, z;
    int nx, ny, nz;
    float dist;
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
void mainLoop();

void initWorld();
int getLight(int x, int z);
uint8_t getBlock(int x, int y, int z);
void setBlock(int x, int y, int z, uint8_t type);

void handleInput();
void handleKey(uint8_t key);
void update(float dt);
void handleCollision(vec3 pos, vec3* velocity);
void drawFrame(uint8_t* pixels);
uint8_t reticleColor(uint8_t col);

void setPos(float x, float y, float z);
void setView(float yaw, float pitch);
uint8_t raytrace(vec3 pos, vec3 dir, hit* info);
uint8_t rayColor(int x, int y, int z, int tex, int face);
void faceNormal(int face, int* x, int* y, int* z);
int texIndex(vec3 pos, int face);
vec3 rayDir(int x, int y);

// Globals
uint8_t* vga = (uint8_t*) 0xa0000;

uint8_t world[worldSX * worldSY * worldSZ] = {0};
uint8_t lighting[worldSX * worldSY] = {0};

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

vec3 velocity = {0, 0, 0};

void main() {
    initWorld();

    mainLoop();
}

void mainLoop() {
    while (true) {
        // Handle any input
        handleInput();

        // Update world
        update(CURTIME() - lastUpdate);
        lastUpdate = CURTIME();

        // Draw frame
        drawFrame(vga);
    }
}

void initWorld() {
    // Make flat grass landscape
    for (int x = 0; x < worldSX; x++) {
        for (int y = 0; y < worldSY; y++) {
            for (int z = 0; z < worldSZ; z++) {
                setBlock(x, y, z, y >= worldSY / 2 ? BLOCK_AIR : BLOCK_DIRT);
            }
        }
    }

    // Initial player position
    setPos(8.0f, worldSY / 2 + 1.8f, 8.0f);
    setView(0.0f, 0.0f);
}

int getLight(int x, int z) {
    return lighting[x * worldSZ + z];
}

uint8_t getBlock(int x, int y, int z) {
    return world[x * worldSY * worldSZ + y * worldSZ + z];
}

void setBlock(int x, int y, int z, uint8_t type) {
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

void handleInput() {
    handleKey(KEY_UP);
    handleKey(KEY_DOWN);
    handleKey(KEY_LEFT);
    handleKey(KEY_RIGHT);
    handleKey(KEY_SPACE);
    handleKey(KEY_Q);
    handleKey(KEY_E);
    handleKey(KEY_ESC);
}

// IRQ1 interrupt handler sets keys buffer for this function to read
void handleKey(uint8_t key) {
    hit info;

    // If the highest bit is not set, this key has not changed
    if (!(keys[key] & 0x80)) {
        return;
    }

    bool down = keys[key] & 1;

    // Mark key state as read
    keys[key] &= 1;

    switch (key) {
        // View
        case KEY_UP: dPitch += down ? 1.0f : -1.0f; break;
        case KEY_DOWN: dPitch += down ? -1.0f : 1.0f; break;

        case KEY_LEFT: dYaw += down ? 1.0f : -1.0f; break;
        case KEY_RIGHT: dYaw += down ? -1.0f : 1.0f; break;

        case KEY_SPACE:
            if (down) {
                playerPos.y += 0.1f;
                velocity.y += 8.0f;
            }
            break;

        // Check if a block was hit and place a new block next to it
        case KEY_Q:
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
        case KEY_E:
            if (!down) {
                raytrace(playerPos, rayDir(160, 100), &info);

                if (info.hit) {
                    setBlock(info.x, info.y, info.z, BLOCK_AIR);
                }
            }
            break;

        case KEY_ESC:
            initWorld();
            break;
    }
}

void update(float dt) {
    // Update view
    pitch += 1.2f * dPitch * dt;
    yaw += 1.2f * dYaw * dt;

    setView(pitch, yaw);

    // Set X/Z velocity depending on input
    velocity.x = velocity.z = 0.0f;

    if (keys[KEY_A] & 1) {
        velocity.x += 3.0f * cosf(M_PI - yaw);
        velocity.z += 3.0f * sinf(M_PI - yaw);
    }
    if (keys[KEY_W] & 1) {
        velocity.x += 3.0f * cosf(-M_PI / 2 - yaw);
        velocity.z += 3.0f * sinf(-M_PI / 2 - yaw);
    }
    if (keys[KEY_S] & 1) {
        velocity.x += 3.0f * cosf(M_PI / 2 - yaw);
        velocity.z += 3.0f * sinf(M_PI / 2 - yaw);
    }
    if (keys[KEY_D] & 1) {
        velocity.x += 3.0f * cosf(-yaw);
        velocity.z += 3.0f * sinf(-yaw);
    }

    // Simulate gravity
    velocity.y -= 20.0f * dt;

    // Handle block collision (head, lower body and feet)
    vec3 headPos = playerPos;
    vec3 lowerPos = playerPos; lowerPos.y -= 1.0f;
    vec3 footPos = playerPos; footPos.y -= 1.8f;

    handleCollision(headPos, &velocity);
    handleCollision(lowerPos, &velocity);
    handleCollision(footPos, &velocity);

    // Apply motion
    playerPos.x += velocity.x * dt;
    playerPos.y += velocity.y * dt;
    playerPos.z += velocity.z * dt;
}

void handleCollision(vec3 pos, vec3* velocity) {
    // Check if new position is not inside block
    hit info;
    raytrace(pos, *velocity, &info);
    
    // If it is, create sliding motion by negating velocity based on hit normal
    if (info.hit && info.dist < 0.1f) {
        if (info.nx != 0) velocity->x = 0.0f;
        if (info.ny != 0) velocity->y = 0.0f;
        if (info.nz != 0) velocity->z = 0.0f;
    }
}

void drawFrame(uint8_t* pixels) {
    int x = 0;
    int y = 0;

    uint8_t* pixel = pixels;
    for (int i = 0; i < 320 * 200; i++) {
        // Draw world
        *pixel = raytrace(playerPos, rayDir(x, y), NULL);

        // Draw aim reticle
        if (x > 157 && x < 163 && y == 100) {
            *pixel = reticleColor(*pixel);
        } else if (y > 97 && y < 103 && x == 160) {
            *pixel = reticleColor(*pixel);
        }

        pixel++;
        x++;
        if (x == 320) {
            x = 0;
            y++;
        }
    }
}

uint8_t reticleColor(uint8_t col) {
    // Look up color in palette
    uint8_t r = palette[((int) col) * 3 + 0];
    uint8_t g = palette[((int) col) * 3 + 1];
    uint8_t b = palette[((int) col) * 3 + 2];

    // Determine if color is mostly green/brown/blue (grass/dirt/sky)
    if (g > r && g > b) {
        return 253;
    } else if (r >= b && r >= g) {
        return 254;
    } else {
        return 255;
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
uint8_t raytrace(vec3 pos, vec3 dir, hit* info) {
    // Finish early if there's no direction
    if (dir.x == 0.0f && dir.y == 0.0f && dir.z == 0.0f) {
        goto nohit;
    }

    vec3 start = pos;

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
            float dx = start.x - pos.x;
            float dy = start.y - pos.y;
            float dz = start.z - pos.z;
            float dist = sqrtf(dx*dx + dy*dy + dz*dz);

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
                info->dist = dist;

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

nohit:
    if (info != NULL) {
        info->hit = false;
    }

    // Sky color
    return 0;
}

uint8_t rayColor(int x, int y, int z, int tex, int face) {
    // Get normal
    int nx, ny, nz;
    faceNormal(face, &nx, &ny, &nz);

    // Block is dirt if there's another block directly on top of it
    bool isDirt = y < worldSY - 1 && getBlock(x, y + 1, z) != BLOCK_AIR;

    // Side is dark if there are higher blocks in the column faced by it
    // Left and back sides are always dark to simulate a sun angle
    if (IN_WORLD(x + nx, y, z + nz) && getLight(x + nx, z + nz) > y) {
        tex += 256;
    } else if (face == FACE_BOTTOM || face == FACE_LEFT || face == FACE_BACK) {
        tex += 256;
    }

    // Texture lookup
    if (face == FACE_BOTTOM || isDirt) {
        return texDirt[tex];
    } else if (face == FACE_TOP) {
        return texGrass[tex];
    } else {
        return texGrassSide[tex];
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
        vFov = 2.0f * atanf(tanf(hFov / 720.0f * M_PI) * 320.0f / 200.0f);
        fov = tanf(0.5f * vFov);
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