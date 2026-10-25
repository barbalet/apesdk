/****************************************************************

 math.c

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

#include "pra_internal.h"
#include <limits.h>
static const n_int new_sd[256] = {
    0,      659,    1318,   1977,   2634,   3290,   3944,   4595,   5244,   5889,   6531,   7169,
    7802,   8431,   9055,   9673,   10286,  10892,  11492,  12085,  12671,  13249,  13819,  14380,
    14933,  15477,  16012,  16537,  17052,  17557,  18051,  18534,  19007,  19467,  19916,  20353,
    20778,  21190,  21590,  21976,  22349,  22709,  23055,  23387,  23706,  24009,  24299,  24573,
    24833,  25078,  25308,  25523,  25722,  25906,  26074,  26226,  26363,  26484,  26589,  26677,
    26750,  26807,  26847,  26871,  26880,  26871,  26847,  26807,  26750,  26677,  26589,  26484,
    26363,  26226,  26074,  25906,  25722,  25523,  25308,  25078,  24833,  24573,  24299,  24009,
    23706,  23387,  23055,  22709,  22349,  21976,  21590,  21190,  20778,  20353,  19916,  19467,
    19007,  18534,  18051,  17557,  17052,  16537,  16012,  15477,  14933,  14380,  13819,  13249,
    12671,  12085,  11492,  10892,  10286,  9673,   9055,   8431,   7802,   7169,   6531,   5889,
    5244,   4595,   3944,   3290,   2634,   1977,   1318,   659,    0,      -659,   -1318,  -1977,
    -2634,  -3290,  -3944,  -4595,  -5244,  -5889,  -6531,  -7169,  -7802,  -8431,  -9055,  -9673,
    -10286, -10892, -11492, -12085, -12671, -13249, -13819, -14380, -14933, -15477, -16012, -16537,
    -17052, -17557, -18051, -18534, -19007, -19467, -19916, -20353, -20778, -21190, -21590, -21976,
    -22349, -22709, -23055, -23387, -23706, -24009, -24299, -24573, -24833, -25078, -25308, -25523,
    -25722, -25906, -26074, -26226, -26363, -26484, -26589, -26677, -26750, -26807, -26847, -26871,
    -26880, -26871, -26847, -26807, -26750, -26677, -26589, -26484, -26363, -26226, -26074, -25906,
    -25722, -25523, -25308, -25078, -24833, -24573, -24299, -24009, -23706, -23387, -23055, -22709,
    -22349, -21976, -21590, -21190, -20778, -20353, -19916, -19467, -19007, -18534, -18051, -17557,
    -17052, -16537, -16012, -15477, -14933, -14380, -13819, -13249, -12671, -12085, -11492, -10892,
    -10286, -9673,  -9055,  -8431,  -7802,  -7169,  -6531,  -5889,  -5244,  -4595,  -3944,  -3290,
    -2634,  -1977,  -1318,  -659};

n_byte2 math_random(n_byte2 *seed)
{
    n_byte2 a = seed[0], b = seed[1];
    seed[0] = b;
    switch (a & 7)
    {
    case 0:
        seed[0] = (n_byte2)(b ^ (a >> 3) ^ 23141);
        break;
    case 3:
        seed[1] = (n_byte2)(a ^ (b >> 1) ^ 53289);
        break;
    case 5:
        seed[1] = (n_byte2)(b ^ (a >> 2) ^ 44550);
        break;
    default:
        seed[1] = (n_byte2)(a ^ (b >> 1));
        break;
    }
    return b;
}
void vect2_populate(n_vect2 *p, n_int x, n_int y)
{
    p->x = x;
    p->y = y;
}
void vect2_add(n_vect2 *p, n_vect2 *a, n_vect2 *b)
{
    p->x = a->x + b->x;
    p->y = a->y + b->y;
}
void vect2_subtract(n_vect2 *p, n_vect2 *a, n_vect2 *b)
{
    p->x = a->x - b->x;
    p->y = a->y - b->y;
}
void vect2_center(n_vect2 *p, n_vect2 *a, n_vect2 *b)
{
    vect2_add(p, a, b);
    p->x /= 2;
    p->y /= 2;
}
void vect2_delta(n_vect2 *p, n_vect2 *d)
{
    vect2_add(p, p, d);
}
n_int vect2_nonzero(n_vect2 *p)
{
    return p->x != 0 || p->y != 0;
}
void vect2_direction(n_vect2 *p, n_int direction, n_int divisor)
{
    p->x = new_sd[(direction + 64) & 255] / divisor;
    p->y = new_sd[direction & 255] / divisor;
}
void vect2_rotation(n_vect2 *p, n_vect2 *r)
{
    n_int x = (p->x * r->x + p->y * r->y) / SINE_MAXIMUM;
    n_int y = (p->x * r->y - p->y * r->x) / SINE_MAXIMUM;
    p->x = x;
    p->y = y;
}
n_vect2 *vect2_min_max_init(void)
{
    n_vect2 *p = malloc(2 * sizeof *p);
    if (p)
    {
        vect2_populate(p, INT64_MAX, INT64_MAX);
        vect2_populate(p + 1, INT64_MIN, INT64_MIN);
    }
    return p;
}
void vect2_min_max(n_vect2 *p, n_int count, n_vect2 *out)
{
    for (n_int i = 0; i < count; i++)
    {
        if (p[i].x < out[0].x)
            out[0].x = p[i].x;
        if (p[i].y < out[0].y)
            out[0].y = p[i].y;
        if (p[i].x > out[1].x)
            out[1].x = p[i].x;
        if (p[i].y > out[1].y)
            out[1].y = p[i].y;
    }
}
void memory_free(void **p)
{
    free(*p);
    *p = NULL;
}
