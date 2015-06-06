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

#ifndef MONKEYMIND_TALE_H
#define MONKEYMIND_TALE_H

#include <stdio.h>
#include <string.h>
#include "monkeymind_id.h"
#include "monkeymind_object.h"
#include "monkeymind_episodic.h"

/* the maximum number of steps within a tale sequence */
#define MM_MAX_TALE_SIZE       (MM_EVENT_MEMORY_SIZE)

typedef struct
{
    /* a unique reference for the tale */
    mm_id id;

    /* properties of the tale */
    mm_object properties;

    /* the number of steps in the tale */
    n_uint length;

    /* array storing the steps in the tale */
    mm_object step[MM_MAX_TALE_SIZE];

    /* the number of times told or heard */
    n_uint times_told;
    n_uint times_heard;
} mm_tale;

void mm_tale_init(mm_tale * tale, mm_random_seed * seed);
n_int mm_tale_insert(mm_tale * tale,
                     mm_object * obj, n_uint index);
n_int mm_tale_remove(mm_tale * tale,
                     n_uint index);
n_int mm_tale_add(mm_tale * tale, mm_object * obj);
mm_object * mm_tale_get(mm_tale * tale, n_uint index);
n_int mm_tale_match(mm_tale * tale1, mm_tale * tale2, n_int * offset);
n_int mm_tale_match_events(mm_tale * tale, mm_episodic * events, n_int * offset);
void mm_tale_confabulate(mm_tale * source, mm_tale * destination,
                         n_uint percent, mm_random_seed * seed);
void mm_tale_change_perspective(mm_tale * tale,
                                mm_id * from_id, n_uint from_name,
                                mm_id * to_id, n_uint to_name);
void mm_episodic_confabulate(mm_episodic * events, mm_tale * tale,
                             n_uint percent, mm_random_seed * seed);

#endif
