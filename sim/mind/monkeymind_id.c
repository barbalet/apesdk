/****************************************************************

 Monkeymind - an experimental cogitive architecture

 IDs within monkeymind are strings of random numbers -
 just like a hash value. This enables the system to scale up to
 many minds while avoiding collisions

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

#include "monkeymind_id.h"

/**
 * @brief Creates an id
 * @param seed Random number generator seed
 * @param id The returned ID
 */
void mm_id_create(mm_random_seed * seed, mm_id * id)
{
    n_uint i;

    for (i = 0; i < MM_ID_LENGTH; i++) {
        id->value[i] = mm_rand(seed);
    }
}

/**
 * @brief Are two ids equal ?
 * @param id1 The first ID
 * @param id2 The second ID
 * @return non-zero value if the two IDs are the same
 */
n_int mm_id_equals(mm_id * id1, mm_id * id2)
{
    n_uint i;

    for (i = 0; i < MM_ID_LENGTH; i++) {
        if (id1->value[i] != id2->value[i]) {
            return 0;
        }
    }
    return 1;
}

/**
 * @brief Copy an ID
 * @param source Source ID
 * @param destination Destination ID
 */
void mm_id_copy(mm_id * source, mm_id * destination)
{
    n_uint i;

    for (i = 0; i < MM_ID_LENGTH; i++) {
        destination->value[i] = source->value[i];
    }
}

/**
 * @brief Sets the ID to a given value
 *        Note that this is rather crude and only sets the first value
 * @param id The ID to be set
 * @param value The value to set the ID to
 */
void mm_id_set(mm_id * id, n_uint value)
{
    memset((void*)id->value,'\0',MM_ID_LENGTH*sizeof(n_uint));
    id->value[0] = value;
}

/**
 * @brief Returns a given number from the ID
 * @param id The ID
 * @param index Array index within the ID
 * @return Value from the ID
 */
n_uint mm_id_get(mm_id * id, n_uint index)
{
    return id->value[index];
}

/**
 * @brief Returns true if the id is non-zero
 * @param id ID to be checked
 * @return True if the ID is non-zero
 */
n_int mm_id_exists(mm_id * id)
{
    return (mm_id_get(id, 0) > 0);
}

/**
 * @brief Sets an ID to zero
 * @param id The ID to be cleared
 */
void mm_id_clear(mm_id * id)
{
    n_uint i;

    for (i = 0; i < MM_ID_LENGTH; i++) {
        id->value[i] = 0;
    }
}
