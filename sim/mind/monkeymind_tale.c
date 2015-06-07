/****************************************************************

 Monkeymind - an experimental cogitive architecture

 A tale is an individual narrative, consisting of a series
 of events or things. There is a separate narratives object
 for collections of tales.

 The reason why this is called "tale" and not "narrative"
 is just to avoid confusion with the narratives collection.

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

#include "monkeymind_tale.h"

/**
 * @brief Initialise a tale
 * @param tale Tale object
 * @param seed Random number generator seed
 */
void mm_tale_init(mm_tale * tale, mm_random_seed * seed)
{
    mm_id_create(seed, &tale->id);
    tale->length = 0;
    tale->times_told = 0;
    mm_obj_init(&tale->properties);

    // clear the tale
    memset((void*)tale->step, '\0',
           MM_MAX_TALE_SIZE*sizeof(mm_object));
}

/**
 * @brief Remove an event from the given tale
 * @param tale Tale object
 * @param index Array index of the event to be removed
 * @return zero on success
 */
n_int mm_tale_remove(mm_tale * tale,
                     n_uint index)
{
    n_uint i;

    if ((tale->length == 0) ||
        (index >= tale->length) ||
        (index >= MM_MAX_TALE_SIZE)) {
        return -1;
    }

    for (i = index+1; i < tale->length; i++) {
        mm_obj_copy(&tale->step[i],
                    &tale->step[i-1]);
    }
    tale->length--;

    return 0;
}

/**
 * @brief Insert an event at a given index within the tale
 * @param tale Tale object
 * @param obj Object containing the event details
 * @param index Array index to store the event at
 * @return zero on success
 */
n_int mm_tale_insert(mm_tale * tale,
                     mm_object * obj, n_uint index)
{
    if ((index > tale->length) ||
        (tale->length >= MM_MAX_TALE_SIZE)) {
        return -1;
    }

    /* default properties */
    mm_obj_prop_add(obj,
                    MM_PROPERTY_TALE_ACT, 0);
    mm_obj_prop_add(obj,
                    MM_PROPERTY_TALE_SCENE, 0);
    mm_obj_prop_add(obj,
                    MM_PROPERTY_TALE_VIEWPOINT, 0);

    mm_obj_copy(obj, &tale->step[index]);
    return 0;
}

/**
 * @brief Adds a tale step
 * @param tale Tale object
 * @param obj The event or thing to be added to the tale sequence
 * @return zero on success
 */
n_int mm_tale_add(mm_tale * tale, mm_object * obj)
{
    if (tale->length >= MM_MAX_TALE_SIZE) return -1;
    if (mm_tale_insert(tale, obj, tale->length) != 0) {
        return -1;
    }
    tale->length++;
    return 0;
}

/**
 * @brief Gets a tale event
 * @param tale Tale object
 * @param index Array index of the event
 * @return Object representing the event or thing
 */
mm_object * mm_tale_get(mm_tale * tale, n_uint index)
{
    if ((index > tale->length) ||
        (tale->length >= MM_MAX_TALE_SIZE)) {
        return 0;
    }
    return &tale->step[index];
}

/**
 * @brief Returns the similarity between two tales
 * @param tale1 The first tale
 * @param tale2 The second tale
 * @param offset Returned array index offset within the first tale
 *        where the best match occurs
 * @return Similarity score
 */
n_int mm_tale_match(mm_tale * tale1, mm_tale * tale2, n_int * offset)
{
    n_int similarity, max_similarity=0;
    n_uint i, off;
    mm_tale * t1, * t2;

    if (tale2->length > tale1->length) {
        t1 = tale2;
        t2 = tale1;
    }
    else {
        t1 = tale1;
        t2 = tale2;
    }

    *offset = -1;
    for (off = 0; off < t1->length - t2->length; off++) {
        similarity = mm_obj_match(&t1->properties, &t2->properties);
        for (i = 0; i < t2->length; i++) {
            similarity += mm_obj_match(&t1->step[off+i], &t2->step[i]);
        }
        if (similarity > max_similarity) {
            max_similarity = similarity;
            *offset = off;
        }
    }

    return max_similarity;
}

/**
 * @brief Returns the similarity between a tale and the given
 *        episodic sequence of events.
 *        Episodic memory is the same as a tale, except that it
 *        could be a different length and it's also a ring buffer
 * @param tale Tale object
 * @param events An episodic memory
 * @param offset Returned array index offset within the tale
 *        where the best match occurs
 * @return Similarity score
 */
n_int mm_tale_match_events(mm_tale * tale,
                           mm_episodic * events, n_int * offset)
{
    n_int similarity, max_similarity=0;
    n_uint i, off, episodic_length = mm_episodic_max(events);
    mm_object * ev;

    *offset = -1;
    if (episodic_length >= tale->length) {
        for (off = 0; off < episodic_length - tale->length; off++) {
            similarity = 0;
            for (i = 0; i < tale->length; i++) {
                ev = mm_episodic_get(events, off+i);
                if (ev != 0) {
                    similarity += mm_obj_match(ev, &tale->step[i]);
                }
                else {
                    printf("mm_tale_match_events: object %d not found\n",
                           (int)(off+i));
                }
            }
            if (similarity > max_similarity) {
                max_similarity = similarity;
                *offset = off;
            }
        }
    }
    else {
        for (off = 0; off < tale->length - episodic_length; off++) {
            similarity = 0;
            for (i = 0; i < episodic_length; i++) {
                ev = mm_episodic_get(events, i);
                if (ev != 0) {
                    similarity +=
                        mm_obj_match(ev, &tale->step[off+i]);
                }
                else {
                    printf("mm_tale_match_events: object %d not found\n",
                           (int)i);
                }
            }
            if (similarity > max_similarity) {
                max_similarity = similarity;
                *offset = off;
            }
        }
    }

    return max_similarity;
}

/**
 * @brief Change the destination tale so that it may include some
 *        percentage of elements from the source tale
 * @param source The source tale to be read from
 * @param destination The destination tale to be modified
 * @param percent The percentage of events to copy from source to destination
 * @param seed Random number generator seed
 */
void mm_tale_confabulate(mm_tale * source, mm_tale * destination,
                         n_uint percent, mm_random_seed * seed)
{
    n_int offset = 0;
    n_uint i;

    /* get the offset for the closest match between the tales */
    mm_tale_match(source, destination, &offset);

    if (source->length < destination->length) {
        for (i = 0; i < source->length; i++) {
            if (mm_rand(seed)%100 > percent) {
                continue;
            }
            mm_obj_copy(&source->step[i],
                        &destination->step[(n_uint)offset+i]);
        }
    }
    else {
        for (i = 0; i < destination->length; i++) {
            if (mm_rand(seed)%100 > percent) {
                continue;
            }
            mm_obj_copy(&source->step[(n_uint)offset+i],
                        &destination->step[i]);
        }
    }
}

/**
 * @brief Alter a percentage of current episodic memories based upon
 *        a given tale from narrative memory
 * @param events The destination episodic memory
 * @param tale The source tale to copy events from
 * @param percent The percentage of events to copy from source to
 *        episodic memory
 * @param seed Random number generator seed
 */
void mm_episodic_confabulate(mm_episodic * events, mm_tale * tale,
                             n_uint percent, mm_random_seed * seed)
{
    n_int offset = 0;
    n_uint i, episodic_length = mm_episodic_max(events);

    if (tale->length <= episodic_length) {
        /* get the offset for the closest match between the tales */
        mm_tale_match_events(tale, events, &offset);

        for (i = 0; i < tale->length; i++) {
            if (mm_rand(seed)%100 > percent) {
                continue;
            }
            mm_obj_copy(&tale->step[i], &events->sequence[(n_uint)offset+i]);
        }
    }
}

/**
 * @brief Change perspective of a tale
 * @param tale Tale object
 * @param from_id The ID to be changed
 * @param from_name The name to be changed
 * @param to_id The ID to change to
 * @param to_name The name to change to
 * @return The number of changes made
 */
n_uint mm_tale_change_perspective(mm_tale * tale,
                                  mm_id * from_id, n_uint from_name,
                                  mm_id * to_id, n_uint to_name)
{
    n_uint i, changes = 0;

    for (i = 0; i < tale->length; i++) {
        changes += mm_obj_change_perspective(&tale->step[i],
                                             from_id, from_name,
                                             to_id, to_name);
    }
}
