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

#include "monkeymind_narratives.h"

void mm_narratives_init(mm_narratives * narratives)
{
    narratives->length = 0;
}

void mm_narratives_copy(mm_narratives * narratives,
                        n_uint index,
                        mm_tale * tale)
{
    memcpy((void*)&narratives->tale[index],
           (void*)tale,
           sizeof(mm_tale));
}

/* inserts a narrative into the array of narratives at the given
   array index */
n_int mm_narratives_insert(mm_narratives * narratives,
                           n_uint index,
                           mm_tale * tale)
{
    if (index >= MM_SIZE_NARRATIVES) return -1;
    if (index > narratives->length) return -2;

    mm_narratives_copy(narratives, index, tale);

    if (index == narratives->length) {
        narratives->length++;
    }

    return 0;
}

/* remove a tale at the given array index */
n_int mm_narratives_remove(mm_narratives * narratives,
                           n_uint index)
{
    n_uint i;

    if (index >= narratives->length) return -1;

    for (i = index+1; i < narratives->length; i++) {
        mm_narratives_copy(narratives, i-1,
                           &narratives->tale[i]);
    }
    narratives->length--;
    return 0;
}

/* adds a tale to the array */
n_int mm_narratives_add(mm_narratives * narratives,
                        mm_tale * tale)
{
    return mm_narratives_insert(narratives, narratives->length, tale);
}

/* returns the array index of the narrative with the given id */
n_int mm_narratives_get(mm_narratives * narratives, mm_id * id)
{
    n_uint i;

    for (i = 0; i < narratives->length; i++) {
        if (mm_id_equals(&narratives->tale[i].id, id)) return i;
    }
    return -1;
}

/* returns the array index of the least heard tale */
n_int mm_narratives_least_heard(mm_narratives * narratives)
{
    n_int i, index = 0;
    n_uint min_heard = 0;

    for (i = 0; i <narratives->length; i++) {
        if ((i == 0) ||
            (narratives->tale[i].times_heard < min_heard)) {
            min_heard = narratives->tale[i].times_heard;
            index = i;
        }
    }
    return index;
}

/* Returns the array index of the closest matching tale.
   This acts as a sort of frame matching */
n_int mm_narratives_match_tale(mm_narratives * narratives,
                               mm_tale * tale,
                               n_int min_similarity,
                               n_int * offset)
{
    n_int i, similarity, winner = -1;
    n_int max_similarity = min_similarity;
    n_int off = -1;

    for (i = 0; i <narratives->length; i++) {
        similarity = mm_tale_match(&narratives->tale[i], tale, &off);
        if (similarity > max_similarity) {
            max_similarity = similarity;
            winner = i;
            *offset = off;
        }
    }
    return winner;
}

/* Returns the narratives array index of the closest matching tale
   to the given episodic memory.
   This acts as a sort of frame matching */
n_int mm_narratives_match_episodic(mm_narratives * narratives,
                                   mm_episodic * events,
                                   n_int min_similarity,
                                   n_int * offset)
{
    n_int i, similarity, winner = -1;
    n_int max_similarity = min_similarity;
    n_int off = -1;

    for (i = 0; i <narratives->length; i++) {
        similarity = mm_tale_match_events(&narratives->tale[i], events, &off);
        if (similarity > max_similarity) {
            max_similarity = similarity;
            winner = i;
            *offset = off;
        }
    }
    return winner;
}

n_int mm_narratives_predict_events(mm_narratives * narratives,
                                   mm_episodic * events,
                                   n_uint no_of_past_events,
                                   n_uint max_results,
                                   n_uint * results)
{
    n_uint i, j, k, off;
    mm_tale * tale;
    n_uint no_of_results = 0;
    n_uint index, similarity;
    const n_uint results_fields = 3;

    for (i = 0; i < narratives->length; i++) {
        tale = &narratives->tale[i];
        if (tale->length <= no_of_past_events) {
            continue;
        }
        for (off = 0; off < tale->length - no_of_past_events; off++) {
            /* calculate the similarity of this series of events
               within the tale at the given offset */
            similarity = 0;
            for (j = 0; j < no_of_past_events; j++) {
                similarity +=
                    mm_obj_match(mm_episodic_get_relative(events,
                                                          no_of_past_events,
                                                          j),
                                 &tale->step[off+j]);
            }
            if (similarity > 0) {
                /* update the results table
                   fields are similarity, tale index and offset
                 */
                index = 0;
                for (j = 0; j < no_of_results; j++) {
                    if (similarity > results[j*results_fields]) {
                        index = j;
                        break;
                    }
                }
                if (index < max_results) {
                    /* move results with lower similarity down the list */
                    for (j = max_results-1; j > index; j--) {
                        for (k = 0; k < results_fields; k++) {
                            results[j*results_fields + k] =
                                results[(j-1)*results_fields + k];
                        }
                    }
                    /* insert the new result */
                    results[index*results_fields] = similarity;
                    results[index*results_fields + 1] = i;
                    results[index*results_fields + 2] = off+no_of_past_events;
                }
            }
        }
    }
    return no_of_results;
}
