#ifndef BRZ_VEC_H
#define BRZ_VEC_H

/*
 * brz_vec.h/.c - small drop-in dynamic vector (growable array)
 *
 * Design goals:
 *  - "Drop-in": tiny C99 implementation, no dependencies beyond stdlib/string
 *  - Generic: stores elements by value (copies bytes) in a contiguous buffer
 *  - Predictable: doubling growth, returns bool for OOM handling
 *
 * Usage:
 *   BrzVec v;
 *   brz_vec_init(&v, sizeof(MyType));
 *   MyType x = {...};
 *   brz_vec_push(&v, &x);
 *   MyType* p = (MyType*)brz_vec_at(&v, i);
 *   brz_vec_destroy(&v);
 *
 * Typed helper macro:
 *   BRZ_VEC_DECL(MyType, MyTypeVec)
 *   MyTypeVec mv; MyTypeVec_init(&mv); ...
 */

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BrzVec {
    void*  data;
    size_t len;
    size_t cap;
    size_t elem_size;
} BrzVec;

void  brz_vec_init(BrzVec* v, size_t elem_size);
void  brz_vec_destroy(BrzVec* v);
void  brz_vec_clear(BrzVec* v);

bool  brz_vec_reserve(BrzVec* v, size_t min_cap);
bool  brz_vec_push(BrzVec* v, const void* elem);
bool  brz_vec_pop(BrzVec* v, void* out_elem); /* optional out */
void* brz_vec_at(BrzVec* v, size_t idx);
const void* brz_vec_cat(const BrzVec* v, size_t idx);

static inline size_t brz_vec_len(const BrzVec* v) { return v ? v->len : 0; }

#ifdef __cplusplus
}
#endif

/* ---------- typed wrappers ---------- */

#define BRZ_VEC_DECL(T, Name) \
typedef struct Name { BrzVec v; } Name; \
static inline void Name##_init(Name* x) { brz_vec_init(&x->v, sizeof(T)); } \
static inline void Name##_destroy(Name* x) { brz_vec_destroy(&x->v); } \
static inline void Name##_clear(Name* x) { brz_vec_clear(&x->v); } \
static inline size_t Name##_len(const Name* x) { return brz_vec_len(&x->v); } \
static inline T* Name##_at(Name* x, size_t i) { return (T*)brz_vec_at(&x->v, i); } \
static inline const T* Name##_cat(const Name* x, size_t i) { return (const T*)brz_vec_cat(&x->v, i); } \
static inline bool Name##_push(Name* x, const T* e) { return brz_vec_push(&x->v, e); }

#endif /* BRZ_VEC_H */
