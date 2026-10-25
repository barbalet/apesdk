/* Copyright 2026 Tom Barbalet. MIT; see object.c.
 * Additive, document-owned object.c JSON model. Existing n_object/n_array APIs
 * retain their ownership and escaped-string behavior. This API decodes strings,
 * tracks byte lengths (including embedded NUL), and owns every value in a doc.
 * The n_object model is the first member of each value; model.primitive.data
 * points to string bytes or the first linked child, and next links siblings.
 * Free documents with object_json_doc_free, never obj_free/unknown_free.
 */
#ifndef APESDK_OBJECT_JSON_H
#define APESDK_OBJECT_JSON_H
#include "toolkit.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct
{
    void *(*malloc)(void *, size_t);
    void *(*realloc)(void *, void *, size_t, size_t);
    void (*free)(void *, void *);
    void *ctx;
} object_json_alc;
typedef struct object_json_doc object_json_doc;
typedef struct object_json_val
{
    n_object model;
    object_json_doc *owner;
    struct object_json_val *key, *tail, *cursor;
    size_t length, cursor_index;
    union
    {
        uint64_t u;
        int64_t i;
        double f;
    } number;
    unsigned tag;
    bool single;
} object_json_val;
typedef object_json_val object_json_mut_val;
typedef object_json_doc object_json_mut_doc;
#define OBJECT_JSON_READ_NUMBER_AS_RAW 0u
#define OBJECT_JSON_WRITE_LOWERCASE_HEX 0u
#define OBJECT_JSON_WRITE_ALLOW_INF_AND_NAN 1u
#define OBJECT_JSON_WRITE_PRETTY 2u
object_json_doc *object_json_read_opts(char *, size_t, unsigned, const object_json_alc *, void *);
object_json_doc *object_json_read(const char *, size_t, unsigned);
object_json_mut_doc *object_json_mut_doc_new(const object_json_alc *);
void object_json_doc_free(object_json_doc *);
void object_json_mut_doc_free(object_json_mut_doc *);
object_json_val *object_json_doc_get_root(object_json_doc *);
object_json_val *object_json_mut_doc_get_root(object_json_doc *);
void object_json_mut_doc_set_root(object_json_doc *, object_json_val *);
bool object_json_is_obj(const object_json_val *);
bool object_json_is_arr(const object_json_val *);
bool object_json_is_str(const object_json_val *);
bool object_json_is_raw(const object_json_val *);
bool object_json_is_uint(const object_json_val *);
bool object_json_is_sint(const object_json_val *);
bool object_json_is_int(const object_json_val *);
bool object_json_is_real(const object_json_val *);
bool object_json_is_num(const object_json_val *);
bool object_json_is_bool(const object_json_val *);
bool object_json_is_true(const object_json_val *);
bool object_json_is_false(const object_json_val *);
bool object_json_is_null(const object_json_val *);
uint64_t object_json_get_uint(const object_json_val *);
int64_t object_json_get_sint(const object_json_val *);
double object_json_get_num(const object_json_val *);
double object_json_get_real(const object_json_val *);
bool object_json_get_bool(const object_json_val *);
const char *object_json_get_str(const object_json_val *);
const char *object_json_mut_get_str(const object_json_val *);
const char *object_json_get_raw(const object_json_val *);
size_t object_json_get_len(const object_json_val *);
bool object_json_equals_str(const object_json_val *, const char *);
size_t object_json_arr_size(const object_json_val *);
size_t object_json_obj_size(const object_json_val *);
object_json_val *object_json_arr_get(object_json_val *, size_t);
object_json_val *object_json_mut_arr_get(object_json_val *, size_t);
object_json_val *object_json_obj_get(object_json_val *, const char *);
object_json_val *object_json_obj_getn(object_json_val *, const char *, size_t);
object_json_val *object_json_mut_obj_get(object_json_val *, const char *);
object_json_val *object_json_first(object_json_val *);
object_json_val *object_json_next(object_json_val *);
object_json_val *object_json_key(object_json_val *);
#define object_json_arr_foreach(a, i, n, v)                                                        \
    for ((i) = 0, (n) = object_json_arr_size(a), (v) = object_json_first(a); (i) < (n) && (v);     \
         ++(i), (v) = object_json_next(v))
#define object_json_obj_foreach(a, i, n, k, v)                                                     \
    for ((i) = 0, (n) = object_json_obj_size(a), (v) = object_json_first(a),                       \
        (k) = object_json_key(v);                                                                  \
         (i) < (n) && (v); ++(i), (v) = object_json_next(v), (k) = object_json_key(v))
object_json_val *object_json_mut_obj(object_json_doc *);
object_json_val *object_json_mut_arr(object_json_doc *);
object_json_val *object_json_mut_null(object_json_doc *);
object_json_val *object_json_mut_bool(object_json_doc *, bool);
object_json_val *object_json_mut_uint(object_json_doc *, uint64_t);
object_json_val *object_json_mut_sint(object_json_doc *, int64_t);
object_json_val *object_json_mut_real(object_json_doc *, double);
object_json_val *object_json_mut_strncpy(object_json_doc *, const char *, size_t);
object_json_val *object_json_mut_strcpy(object_json_doc *, const char *);
object_json_val *object_json_mut_str(object_json_doc *, const char *);
object_json_val *object_json_mut_rawcpy(object_json_doc *, const char *);
bool object_json_mut_obj_add(object_json_val *, object_json_val *, object_json_val *);
bool object_json_mut_arr_append(object_json_val *, object_json_val *);
bool object_json_mut_arr_add_val(object_json_val *, object_json_val *);
bool object_json_mut_obj_add_val(object_json_doc *, object_json_val *, const char *,
                                 object_json_val *);
size_t object_json_mut_obj_remove_key(object_json_val *, const char *);
size_t object_json_mut_obj_remove_keyn(object_json_val *, const char *, size_t);
object_json_val *object_json_val_mut_copy(object_json_doc *, object_json_val *);
object_json_val *object_json_mut_val_mut_copy(object_json_doc *, object_json_val *);
#define APESDK_JSON_ADD_DECL(name, type)                                                           \
    bool object_json_mut_obj_add_##name(object_json_doc *, object_json_val *, const char *, type); \
    bool object_json_mut_arr_add_##name(object_json_doc *, object_json_val *, type)
APESDK_JSON_ADD_DECL(uint, uint64_t);
APESDK_JSON_ADD_DECL(sint, int64_t);
APESDK_JSON_ADD_DECL(int, int64_t);
APESDK_JSON_ADD_DECL(real, double);
APESDK_JSON_ADD_DECL(bool, bool);
APESDK_JSON_ADD_DECL(str, const char *);
APESDK_JSON_ADD_DECL(strcpy, const char *);
#undef APESDK_JSON_ADD_DECL
bool object_json_mut_obj_add_null(object_json_doc *, object_json_val *, const char *);
void object_json_set_real(object_json_val *, double);
void object_json_set_fp_to_float(object_json_val *, bool);
char *object_json_write_number(const object_json_val *, char *);
char *object_json_val_write_opts(const object_json_val *, unsigned, const object_json_alc *,
                                 size_t *, void *);
char *object_json_mut_val_write_opts(const object_json_val *, unsigned, const object_json_alc *,
                                     size_t *, void *);
char *object_json_mut_val_write(const object_json_val *, unsigned, size_t *);
char *object_json_mut_write_opts(object_json_doc *, unsigned, const object_json_alc *, size_t *,
                                 void *);
typedef struct
{
    object_json_val *next;
} object_json_mut_obj_iter;
object_json_doc *object_json_doc_mut_copy(object_json_doc *, const object_json_alc *);
char *object_json_mut_write(object_json_doc *, unsigned, size_t *);
char *object_json_val_write(const object_json_val *, unsigned, size_t *);
bool object_json_mut_obj_put(object_json_val *, object_json_val *, object_json_val *);
object_json_val *object_json_mut_arr_replace(object_json_val *, size_t, object_json_val *);
object_json_val *object_json_mut_arr_with_real(object_json_doc *, const double *, size_t);
uint64_t object_json_mut_get_uint(const object_json_val *);
bool object_json_mut_set_uint(object_json_val *, uint64_t);
object_json_mut_obj_iter object_json_mut_obj_iter_with(object_json_val *);
object_json_val *object_json_mut_obj_iter_next(object_json_mut_obj_iter *);
object_json_val *object_json_mut_obj_iter_get_val(object_json_val *);
#endif
