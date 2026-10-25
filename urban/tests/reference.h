/****************************************************************

 reference.h

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

/* Fixed-width records keep SDK-private C types inside the reference translation units. */
#ifndef PRA_TEST_REFERENCE_H
#define PRA_TEST_REFERENCE_H
#include <stdint.h>
typedef struct
{
    int64_t rooms, genetics[64], points[16][32][2];
} reference_house;
typedef struct
{
    int64_t points[32], radius, center[2];
} reference_tree;
void reference_create_house(uint16_t seed[2], const int64_t center[2], reference_house *out);
void reference_create_tree(uint16_t seed[2], const int64_t center[2], reference_tree *out);
void reference_create_fence(uint16_t seed[2], uint8_t direction, const int64_t center[2],
                            int64_t out[8][2][2]);
uint16_t reference_random(uint16_t seed[2]);
#endif
