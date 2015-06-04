/****************************************************************

 Monkeymind - an experimental cogitive architecture

 The episodic buffer records the stream of perceived
 circumstances, then filtering out (or compressing) the
 relevant from the forgettable

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

#ifndef MONKEYMIND_ID_H
#define MONKEYMIND_ID_H

#include <stdio.h>
#include <string.h>
#include "monkeymind_datatypes.h"
#include "monkeymind_rand.h"

#define MM_ID_LENGTH 3
typedef struct {
    n_uint value[MM_ID_LENGTH];
} mm_id;

void mm_id_create(mm_random_seed * seed, mm_id * id);
n_int mm_id_equals(mm_id * id1, mm_id * id2);
void mm_id_copy(mm_id * source, mm_id * destination);
void mm_id_set(mm_id * id, n_uint value);
n_uint mm_id_get(mm_id * id, n_uint index);
n_int mm_id_exists(mm_id * id);
void mm_id_clear(mm_id * id);

#endif
