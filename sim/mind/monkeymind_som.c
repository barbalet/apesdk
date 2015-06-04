/****************************************************************

 Monkeymind - an experimental cogitive architecture

 =============================================================

 Copyright 2013-2014 Bob Mottram

 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or
 sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the followingp
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

#include "monkeymind_som.h"

void mm_som_init(mm_som * s,
                 n_uint dimension,
                 n_uint vector_length,
                 n_uint learning_radius,
                 mm_random_seed * seed)
{
    n_uint i;

    s->dimension = dimension;
    s->vector_length = vector_length;
    s->learning_radius = learning_radius;
    s->seed = seed;

    memset((void*)s->activation, '\0',
           MM_SOM_MAX_DIMENSION*MM_SOM_MAX_DIMENSION*
           sizeof(n_int));

    /* random weights */
    for (i = 0;
         i < s->dimension*s->dimension*s->vector_length;
         i++) {
        s->weight[i] = (n_byte)(mm_rand(s->seed) & 255);
    }
}

void mm_som_update(mm_som * s,
                   n_byte * vector,
                   n_uint * x, n_uint * y)
{
    n_uint xx, yy, i, ctr = 0, n = 0;
    n_int diff, min_diff = 0;

    *x = 0;
    *y = 0;

    for (yy = 0; yy < s->dimension; yy++) {
        for (xx = 0; xx < s->dimension; xx++, ctr++) {
            /* activation is the Euclidean distance between
               the weights and the input vector */
            s->activation[ctr] = 0;
            for (i = 0; i < s->vector_length; i++, n++) {
                diff = (n_int)vector[i] - (n_int)s->weight[n];
                if (diff < 0) diff = -diff;
                s->activation[ctr] += diff;
            }
            /* record the highest activation */
            if ((s->activation[ctr] < min_diff) || (ctr==0)) {
                *x = xx;
                *y = yy;
                min_diff = s->activation[ctr];
            }
        }
    }
}

/* adjust the weights depending upon the location of the peak response */
void mm_som_learn(mm_som * s,
                  n_byte * vector,
                  n_uint x, n_uint y)
{
    n_int xx, yy, dx, dy, max_radius, inner_radius, r, n, i;
    n_int increment, v;

    max_radius =
        s->learning_radius*s->learning_radius;
    inner_radius =
        s->learning_radius*s->learning_radius/4;

    for (xx = (n_int)x - s->learning_radius;
         xx <= (n_int)x + s->learning_radius;
         xx++) {
        if ((xx < 0) || (xx >= s->dimension)) {
            continue;
        }
        dx = xx - (n_int)x;
        for (yy = (n_int)y - s->learning_radius;
             yy <= (n_int)y + s->learning_radius;
             yy++) {
            if ((yy < 0) || (yy >= s->dimension)) {
                continue;
            }
            dy = yy - (n_int)y;
            r = dx*dx + dy*dy;
            if (r > max_radius) continue;

            /* location within the map */
            n = (yy*s->dimension + xx) * s->vector_length;

            if (r < inner_radius) {
                increment = 2;
            }
            else {
                increment = 1;
            }

            /* adjust the weights */
            for (i = 0; i < s->vector_length; i++,n++) {
                if (s->weight[n] > vector[i]) {
                    v = (n_int)s->weight[n] - increment;
                }
                else if (s->weight[n] < vector[i]) {
                    v = (n_int)s->weight[n] + increment;
                }
                else {
                    continue;
                }
                if (v < 0) v = 0;
                if (v > 255) v = 255;
                s->weight[n] = v;
            }
        }
    }
}
