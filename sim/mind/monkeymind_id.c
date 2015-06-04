#include "monkeymind_id.h"

/* create an id */
void mm_id_create(mm_random_seed * seed, mm_id * id)
{
    n_uint i;

    for (i = 0; i < MM_ID_LENGTH; i++) {
        id->value[i] = mm_rand(seed);
    }
}

/* are two ids equal ? */
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

/* copy an id */
void mm_id_copy(mm_id * source, mm_id * destination)
{
    n_uint i;

    for (i = 0; i < MM_ID_LENGTH; i++) {
        destination->value[i] = source->value[i];
    }
}

/* sets the id to a given value */
void mm_id_set(mm_id * id, n_uint value)
{
    memset((void*)id->value,'\0',MM_ID_LENGTH*sizeof(n_uint));
    id->value[0] = value;
}

/* returns a given number from the id */
n_uint mm_id_get(mm_id * id, n_uint index)
{
    return id->value[index];
}

/* returns true if the id is non-zero */
n_int mm_id_exists(mm_id * id)
{
    return (mm_id_get(id, 0) > 0);
}

void mm_id_clear(mm_id * id)
{
    n_uint i;

    for (i = 0; i < MM_ID_LENGTH; i++) {
        id->value[i] = 0;
    }
}
