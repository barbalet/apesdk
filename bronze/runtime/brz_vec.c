#include "brz_vec.h"
#include <stdlib.h>
#include <string.h>

static bool brz_vec_grow_to(BrzVec* v, size_t new_cap)
{
    if(new_cap <= v->cap) return true;
    if(v->elem_size == 0) return false;

    /* clamp: avoid overflow */
    size_t bytes = new_cap * v->elem_size;
    if(new_cap != 0 && bytes / new_cap != v->elem_size) return false;

    void* p = realloc(v->data, bytes);
    if(!p) return false;

    v->data = p;
    v->cap = new_cap;
    return true;
}

void brz_vec_init(BrzVec* v, size_t elem_size)
{
    if(!v) return;
    v->data = NULL;
    v->len = 0;
    v->cap = 0;
    v->elem_size = elem_size;
}

void brz_vec_destroy(BrzVec* v)
{
    if(!v) return;
    free(v->data);
    v->data = NULL;
    v->len = 0;
    v->cap = 0;
    v->elem_size = 0;
}

void brz_vec_clear(BrzVec* v)
{
    if(!v) return;
    v->len = 0;
}

bool brz_vec_reserve(BrzVec* v, size_t min_cap)
{
    if(!v) return false;
    if(min_cap <= v->cap) return true;

    size_t new_cap = v->cap ? v->cap : 8;
    while(new_cap < min_cap)
    {
        size_t next = new_cap * 2;
        if(next <= new_cap) { new_cap = min_cap; break; } /* overflow fallback */
        new_cap = next;
    }
    return brz_vec_grow_to(v, new_cap);
}

bool brz_vec_push(BrzVec* v, const void* elem)
{
    if(!v || !elem) return false;
    if(!brz_vec_reserve(v, v->len + 1)) return false;

    unsigned char* dst = (unsigned char*)v->data + (v->len * v->elem_size);
    memcpy(dst, elem, v->elem_size);
    v->len++;
    return true;
}

bool brz_vec_pop(BrzVec* v, void* out_elem)
{
    if(!v || v->len == 0) return false;
    v->len--;
    if(out_elem)
    {
        unsigned char* src = (unsigned char*)v->data + (v->len * v->elem_size);
        memcpy(out_elem, src, v->elem_size);
    }
    return true;
}

void* brz_vec_at(BrzVec* v, size_t idx)
{
    if(!v || idx >= v->len) return NULL;
    return (unsigned char*)v->data + (idx * v->elem_size);
}

const void* brz_vec_cat(const BrzVec* v, size_t idx)
{
    if(!v || idx >= v->len) return NULL;
    return (const unsigned char*)v->data + (idx * v->elem_size);
}
