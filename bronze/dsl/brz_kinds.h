#ifndef BRZ_KINDS_H
#define BRZ_KINDS_H

#include <stdbool.h>
#include <stddef.h>

#include "brz_vec.h"

/*
 * Dynamic kinds
 *
 * Resources and items are defined by the DSL file, not hard-coded enums.
 * A KindTable is an ordered list of unique names. The numeric id is the
 * index in that list.
 */

typedef struct {
    BrzVec names; /* elem = char* (owned) */
} KindTable;

void kind_table_init(KindTable* kt);
void kind_table_destroy(KindTable* kt);

/* Add (or find existing) kind name; returns id (>=0) or -1 on OOM */
int  kind_table_add(KindTable* kt, const char* name);

/* Find kind id by name; returns id (>=0) or -1 if missing */
int  kind_table_find(const KindTable* kt, const char* name);

/* Convenience: safe name lookup; returns "" if out of range */
const char* kind_table_name(const KindTable* kt, int id);

static inline size_t kind_table_count(const KindTable* kt)
{
    return kt ? brz_vec_len(&kt->names) : 0;
}

#endif /* BRZ_KINDS_H */
