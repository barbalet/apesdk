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

#include "monkeymind_rand.h"

/* Xorshift */
n_uint mm_rand(mm_random_seed * seed)
{
    n_uint t;

    /* avoid singularity */
    if (seed->value[0]+seed->value[1]+
        seed->value[2]+seed->value[3] == 0) {
        seed->value[0] = 123456789;
        seed->value[1] = 362436069;
        seed->value[2] = 521288629;
        seed->value[3] = 88675123;
    }

    t = seed->value[0] ^ (seed->value[0] << 11);
    seed->value[0] = seed->value[1];
    seed->value[1] = seed->value[2];
    seed->value[2] = seed->value[3];
    seed->value[3] =
        seed->value[3] ^
        (seed->value[3] >> 19) ^
        (t ^ (t >> 8));
    return seed->value[3];
}

void mm_rand_copy(mm_random_seed * src, mm_random_seed * dest)
{
    memcpy((void*)dest->value,
           (void*)src->value,
           4*sizeof(n_uint));
}

/* This is a special version of rand used to generate ID
   numbers for objects or agents.
   Potentially it could differ from the usual
   random number generator */
n_uint mm_rand_id(mm_random_seed * seed)
{
    return mm_rand(seed);
}

/* seeds the randim number generator with four values */
void mm_rand_init(mm_random_seed * seed,
                  n_uint a, n_uint b, n_uint c, n_uint d)
{
    seed->value[0] = a;
    seed->value[1] = b;
    seed->value[2] = c;
    seed->value[3] = d;
}
