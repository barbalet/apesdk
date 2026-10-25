#ifndef WASM_Z_PRA_INTERNAL_H
#define WASM_Z_PRA_INTERNAL_H
#include "pra.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#ifdef PRA_REFERENCE_SDK
#include PRA_REFERENCE_SDK
#else
typedef int64_t n_int;
typedef uint8_t n_byte;
typedef uint16_t n_byte2;
typedef union
{
    struct
    {
        n_int x, y;
    };
    n_int data[2];
} n_vect2;
#define math_random pra_compat_random
#define vect2_add pra_compat_add
#define vect2_subtract pra_compat_subtract
#define vect2_center pra_compat_center
#define vect2_populate pra_compat_populate
#define vect2_direction pra_compat_direction
#define vect2_rotation pra_compat_rotation
#define vect2_delta pra_compat_delta
#define vect2_nonzero pra_compat_nonzero
#define vect2_min_max_init pra_compat_min_max_init
#define vect2_min_max pra_compat_min_max
#define memory_free pra_compat_free
#endif
#ifdef PRA_REFERENCE
#define house_create pra_original_house_create
#define house_init pra_original_house_init
#define house_window_present pra_original_window_present
#define house_door_present pra_original_door_present
#define tree_create pra_original_tree_create
#define tree_init pra_original_tree_init
#define tree_populated pra_original_tree_populated
#define fence_init pra_original_fence_init
#else
#define house_create pra_legacy_house_create
#define house_init pra_legacy_house_init
#define house_window_present pra_legacy_window_present
#define house_door_present pra_legacy_door_present
#define tree_create pra_legacy_tree_create
#define tree_init pra_legacy_tree_init
#define tree_populated pra_legacy_tree_populated
#define fence_init pra_legacy_fence_init
#endif
#ifndef SINE_MAXIMUM
#define SINE_MAXIMUM 26880
#endif
#define POINTS_PER_ROOM 32
#define POINTS_PER_ROOM_STRUCTURE 8
#define MAX_ROOMS 16
#define GENETICS_COUNT 64
#define POINTS_PER_TREE 32
#define TREE_SPACE 400
#define RESIDENCE_SPACE 800
#define FENCE_WABBLE_SPACE 100
#define FENCE_OFFSET 400
#define FENCE_START (-250)
#define FENCE_END_LONG ((RESIDENCE_SPACE * 3) + 250)
#define FENCE_END_SHORT (RESIDENCE_SPACE + 250)
enum
{
    DC_NONE = 0,
    DC_NORTH = 1,
    DC_SOUTH = 2,
    DC_EAST = 4,
    DC_WEST = 8
};
typedef struct
{
    n_vect2 points[2];
} simulated_fence;
typedef struct
{
    n_vect2 points[32];
} simulated_room;
typedef struct
{
    n_int points[32], radius;
    n_vect2 center;
} simulated_tree;
typedef struct
{
    simulated_room room[MAX_ROOMS];
    n_int house[64], roomcount;
    n_byte rotation;
    simulated_tree trees[4];
    /* Additional metadata does not modify original room/seed results. */
    simulated_room local_room[MAX_ROOMS];
    n_vect2 pivot, direction;
    int captured;
} simulated_building;
#ifndef PRA_REFERENCE_SDK
n_byte2 math_random(n_byte2 *seed);
void vect2_add(n_vect2 *, n_vect2 *, n_vect2 *);
void vect2_subtract(n_vect2 *, n_vect2 *, n_vect2 *);
void vect2_center(n_vect2 *, n_vect2 *, n_vect2 *);
void vect2_populate(n_vect2 *, n_int, n_int);
void vect2_direction(n_vect2 *, n_int, n_int);
void vect2_rotation(n_vect2 *, n_vect2 *);
void vect2_delta(n_vect2 *, n_vect2 *);
n_int vect2_nonzero(n_vect2 *);
n_vect2 *vect2_min_max_init(void);
void vect2_min_max(n_vect2 *, n_int, n_vect2 *);
void memory_free(void **);
#endif
void house_create(simulated_building *, n_byte2 *, n_vect2 *);
void house_init(n_byte2 *, n_vect2 *, simulated_building *);
n_int house_window_present(n_vect2 *);
n_int house_door_present(n_vect2 *);
void tree_create(simulated_tree *, n_byte2 *, n_vect2 *);
void tree_init(simulated_tree *, n_byte2 *, n_vect2 *);
n_int tree_populated(simulated_tree *);
void fence_init(n_byte2 *, n_byte, n_vect2 *, simulated_fence *);

#define U_WALLS 512
#define U_OPENINGS 128
#define U_HEIGHT 2.7
#define U_THICKNESS 0.12
#define U_DOOR_TOP 2.1
#define U_SILL 0.9
#define U_WINDOW_TOP 2.15
#define U_SCALE PRA_METRES_PER_UNIT
typedef struct
{
    double a[2], b[2];
    int owners;
} u_wall;
typedef struct
{
    double center[2], width, inward[2];
    uint32_t id;
    int axis, window, open, exterior, hinge;
} u_opening;
typedef struct
{
    double a[2], b[2];
} u_line;
typedef struct
{
    simulated_building raw;
    uint16_t seed[2];
    double center[2], m[2];
    u_wall walls[U_WALLS];
    size_t wall_count;
    u_opening openings[U_OPENINGS];
    size_t opening_count;
    simulated_tree trees[4];
    u_line fences[5];
    double entry[2];
} u_house;
struct pra_scene
{
    pra_config config;
    u_house *house;
};
#define u_world pra_internal_world
#define u_local pra_internal_local
#define u_distance pra_internal_distance
#define u_panel pra_internal_panel
#define u_plan pra_internal_plan
void u_world(const u_house *, const double local[2], double world[2]);
void u_local(const u_house *, const double world[2], double local[2]);
double u_distance(const double p[2], const double a[2], const double b[2]);
void u_panel(const u_opening *, double a[2], double b[2]);
int u_plan(u_house *, uint32_t);
#endif
