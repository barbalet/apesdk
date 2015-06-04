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

#ifndef MONKEYMIND_NARRATIVES_H
#define MONKEYMIND_NARRATIVES_H

#include <stdio.h>
#include <string.h>
#include "monkeymind_id.h"
#include "monkeymind_object.h"
#include "monkeymind_episodic.h"
#include "monkeymind_tale.h"

/* size of narrative memory */
#define MM_SIZE_NARRATIVES     32

typedef struct
{
    n_uint length;
    mm_tale tale[MM_SIZE_NARRATIVES];
} mm_narratives;

void mm_narratives_init(mm_narratives * narratives);
void mm_narratives_copy(mm_narratives * narratives,
                        n_uint index,
                        mm_tale * tale);
n_int mm_narratives_insert(mm_narratives * narratives,
                           n_uint index,
                           mm_tale * tale);
n_int mm_narratives_remove(mm_narratives * narratives,
                           n_uint index);
n_int mm_narratives_add(mm_narratives * narratives,
                        mm_tale * tale);
n_int mm_narratives_get(mm_narratives * narratives, mm_id * id);
n_int mm_narratives_least_heard(mm_narratives * narratives);
n_int mm_narratives_match_tale(mm_narratives * narratives,
                               mm_tale * tale,
                               n_int min_similarity,
                               n_int * offset);
n_int mm_narratives_match_episodic(mm_narratives * narratives,
                                   mm_episodic * events,
                                   n_int min_similarity,
                                   n_int * offset);
n_int mm_narratives_predict_events(mm_narratives * narratives,
                                   mm_episodic * events,
                                   n_uint no_of_past_events,
                                   n_uint max_results,
                                   n_uint * results);
#endif
