/****************************************************************

 Monkeymind - an experimental cogitive architecture

 =============================================================

 Copyright 2013-2015 Bob Mottram

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

#ifndef MONKEYMIND_SOM_H
#define MONKEYMIND_SOM_H

#include <stdio.h>
#include <string.h>
#include "noble.h"
#include "monkeymind_rand.h"

#define MM_SOM_MAX_DIMENSION     64
#define MM_SOM_MAX_VECTOR_LENGTH 128

typedef struct
{
    n_uint dimension;
    n_uint vector_length;
    n_uint learning_radius;
    mm_random_seed * seed;
    n_byte weight[MM_SOM_MAX_DIMENSION*
                  MM_SOM_MAX_DIMENSION*
                  MM_SOM_MAX_VECTOR_LENGTH];
    n_int activation[MM_SOM_MAX_DIMENSION*
                     MM_SOM_MAX_DIMENSION];
} mm_som;

void mm_som_init(mm_som * s,
                 n_uint dimension,
                 n_uint vector_length,
                 n_uint learning_radius,
                 mm_random_seed * seed);

void mm_som_update(mm_som * s,
                   n_byte vector[],
                   n_uint * x, n_uint * y);

void mm_som_learn(mm_som * s,
                  n_byte vector[],
                  n_uint x, n_uint y);

#endif
