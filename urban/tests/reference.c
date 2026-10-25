/****************************************************************

 reference.c

 =============================================================

 Copyright 1996-2026 Tom Barbalet. All rights reserved.

 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or
 sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following
 conditions:

 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.

 ****************************************************************/

/* Compiled against the full original SDK, never against the extracted helpers. */
#include "reference.h"
#include "pra_internal.h"
#include <assert.h>
#include <stdio.h>
n_int draw_error(n_constant_string message, n_constant_string location, n_int line)
{
    fprintf(stderr, "SDK error %s:%lld: %s\n", location, (long long)line, message);
    abort();
}
static void copy_seed(n_byte2 target[2], const uint16_t source[2])
{
    target[0] = source[0];
    target[1] = source[1];
}
static void save_seed(uint16_t target[2], const n_byte2 source[2])
{
    target[0] = source[0];
    target[1] = source[1];
}
void reference_create_house(uint16_t seed[2], const int64_t center[2], reference_house *out)
{
    simulated_building house = {0};
    n_byte2 local[2];
    copy_seed(local, seed);
    n_vect2 p;
    vect2_populate(&p, (n_int)center[0], (n_int)center[1]);
    house_create(&house, local, &p);
    assert(house.roomcount >= 1 && house.roomcount <= 16);
    out->rooms = house.roomcount;
    for (int i = 0; i < 64; i++)
        out->genetics[i] = house.house[i];
    for (n_int r = 0; r < house.roomcount; r++)
        for (int i = 0; i < 32; i++)
            for (int k = 0; k < 2; k++)
                out->points[r][i][k] = house.room[r].points[i].data[k];
    save_seed(seed, local);
}
void reference_create_tree(uint16_t seed[2], const int64_t center[2], reference_tree *out)
{
    simulated_tree tree = {0};
    n_byte2 local[2];
    copy_seed(local, seed);
    n_vect2 p;
    vect2_populate(&p, (n_int)center[0], (n_int)center[1]);
    tree_create(&tree, local, &p);
    for (int i = 0; i < 32; i++)
        out->points[i] = tree.points[i];
    out->radius = tree.radius;
    out->center[0] = tree.center.x;
    out->center[1] = tree.center.y;
    save_seed(seed, local);
}
void reference_create_fence(uint16_t seed[2], uint8_t direction, const int64_t center[2],
                            int64_t out[8][2][2])
{
    simulated_fence fence[8] = {0};
    n_byte2 local[2];
    copy_seed(local, seed);
    n_vect2 p;
    vect2_populate(&p, (n_int)center[0], (n_int)center[1]);
    fence_init(local, direction, &p, fence);
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 2; j++)
            for (int k = 0; k < 2; k++)
                out[i][j][k] = fence[i].points[j].data[k];
    save_seed(seed, local);
}
uint16_t reference_random(uint16_t seed[2])
{
    n_byte2 local[2];
    copy_seed(local, seed);
    n_byte2 result = math_random(local);
    save_seed(seed, local);
    return result;
}
