#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#define M_PI 3.14159265358979323846

// Externals
extern uint8_t palette[];
extern uint8_t keys[];

extern uint32_t worldSX, worldSY, worldSZ;

// Configuration
#define skyColor 0x02

// Key scancodes
#define KEY_ESC 0x01

#define KEY_Q 0x10
#define KEY_E 0x12

#define KEY_W 0x11
#define KEY_A 0x1E
#define KEY_S 0x1F
#define KEY_D 0x20

#define KEY_L 0x26

#define KEY_SPACE 0x39

#define KEY_UP 0x48
#define KEY_LEFT 0x4B
#define KEY_RIGHT 0x4D
#define KEY_DOWN 0x50

// Macros
#define IN_WORLD(x, y, z) \
    (x >= 0 && y >= 0 && z >= 0 && x < worldSX && y < worldSY && z < worldSZ)

// Resources
extern uint8_t tex_grass[];
extern uint8_t tex_dirt[];
extern uint8_t tex_grass_side[];

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
extern void init_world();
extern uint8_t get_block(int x, int y, int z);
extern void set_block(int x, int y, int z, uint8_t type);

extern void handle_collision(vec3 pos, vec3* velocity);

extern void set_pos(float x, float y, float z);
extern void set_view(float yaw, float pitch);

uint8_t raytrace(vec3 pos, vec3 dir, hit* info);
extern uint8_t ray_color(int x, int y, int z, vec3 pos, int tex, int face);
extern void face_normal(int face, int* x, int* y, int* z);
extern int tex_index(vec3 pos, int face);
extern vec3 ray_dir(int x, int y);

// Globals
extern uint8_t* vga;

extern uint8_t world[];
extern vec3 sunDir;

extern vec3 playerPos;
extern float pitch, pitchS, pitchC;
extern float yaw, yawS, yawC;

extern float lastUpdate, dPitch, dYaw;
extern vec3 velocity;

// IRQ1 interrupt handler sets keys buffer for this function to read
void handle_key(uint8_t key) {
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
                raytrace(playerPos, ray_dir(160, 100), &info);

                if (info.hit) {
                    int bx = info.x + info.nx;
                    int by = info.y + info.ny;
                    int bz = info.z + info.nz;

                    if (IN_WORLD(bx, by, bz)) {
                        set_block(bx, by, bz, BLOCK_DIRT);
                    }
                }
            }
            break;

        // Check if a block was hit and remove it
        case KEY_E:
            if (!down) {
                raytrace(playerPos, ray_dir(160, 100), &info);

                if (info.hit) {
                    set_block(info.x, info.y, info.z, BLOCK_AIR);
                }
            }
            break;

        case KEY_ESC:
            init_world();
            break;
    }
}

void update(float dt) {
    // Update view
    pitch += 1.2f * dPitch * dt;
    yaw += 1.2f * dYaw * dt;

    set_view(pitch, yaw);

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

    handle_collision(headPos, &velocity);
    handle_collision(lowerPos, &velocity);
    handle_collision(footPos, &velocity);

    // Apply motion
    playerPos.x += velocity.x * dt;
    playerPos.y += velocity.y * dt;
    playerPos.z += velocity.z * dt;
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
        if (get_block(x, y, z) != BLOCK_AIR) {
            float dx = start.x - pos.x;
            float dy = start.y - pos.y;
            float dz = start.z - pos.z;
            float dist = dx*dx + dy*dy + dz*dz;

            vec3 relPos = pos;
            relPos.x -= x;
            relPos.y -= y;
            relPos.z -= z;

            // If hit info is requested, no color computation is done
            if (info != NULL) {
                int nx, ny, nz;
                face_normal(face, &nx, &ny, &nz);

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

            int tex = tex_index(relPos, face);

            return ray_color(x, y, z, pos, tex, face);
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
    return skyColor;
}