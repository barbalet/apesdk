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

#ifndef MONKEYMIND_EVENTS_H
#define MONKEYMIND_EVENTS_H

#include <stdio.h>
#include <string.h>
#include "noble.h"
#include "monkeymind_rand.h"
#include "monkeymind_time.h"
#include "monkeymind_object.h"

/* the maximum number of events in the sequence */
#define MM_EVENT_MEMORY_SIZE 16

typedef struct
{
    /* the sequence of observed events */
    mm_object sequence[MM_EVENT_MEMORY_SIZE];

    /* current index within the buffer */
    n_uint index;
} mm_episodic;

typedef struct {
    n_uint agent_id;
    n_uint hits;
    n_uint category[MM_CATEGORIES];
} mm_protagonist;

void mm_episodic_init(mm_episodic * events);
void mm_episodic_add(mm_episodic * events,
                     mm_object * observation);
n_uint mm_episodic_max(mm_episodic * events);
mm_object * mm_episodic_get(mm_episodic * events, n_uint timestep);
mm_object * mm_episodic_get_relative(mm_episodic * events,
                                     n_uint max_steps, n_uint timestep);

n_int mm_episodic_protagonists(mm_episodic * events,
                               n_uint timestep_start,
                               n_uint timestep_end,
                               mm_protagonist * protagonists,
                               n_uint max_protagonists);
n_byte mm_event_exists(mm_episodic * events_list, mm_object * event);

#endif
