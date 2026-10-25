/* Copyright 2026 Tom Barbalet. MIT; see object.c.
 * Strict, allocator-aware extension of the linked n_object/n_array model.
 * No process-global parser or allocator state. Maximum nesting: 128.
 */
#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum
{
    OJ_OBJ = 1,
    OJ_ARR,
    OJ_STR,
    OJ_UINT,
    OJ_SINT,
    OJ_REAL,
    OJ_BOOL,
    OJ_NULL,
    OJ_RAW
};
typedef struct oj_block
{
    struct oj_block *next;
    union
    {
        long double real;
        void *pointer;
        uint64_t integer;
    } align;
    unsigned char bytes[];
} oj_block;
struct object_json_doc
{
    object_json_alc allocator;
    oj_block *blocks;
    object_json_val *root;
    size_t allocated;
    bool failed;
};
static void *oj_malloc(void *c, size_t n)
{
    (void)c;
    return malloc(n);
}
static void oj_free(void *c, void *p)
{
    (void)c;
    free(p);
}
static object_json_alc oj_allocator(const object_json_alc *a)
{
    return a ? *a : (object_json_alc){oj_malloc, NULL, oj_free, NULL};
}
static void *oj_alloc(object_json_doc *d, size_t n)
{
    if (!d || d->failed)
        return NULL;
    if (n > 512u * 1024u * 1024u || d->allocated > 512u * 1024u * 1024u - n ||
        n > SIZE_MAX - sizeof(oj_block))
    {
        d->failed = true;
        return NULL;
    }
    oj_block *b = d->allocator.malloc(d->allocator.ctx, sizeof(*b) + n);
    if (!b)
    {
        d->failed = true;
        return NULL;
    }
    b->next = d->blocks;
    d->blocks = b;
    d->allocated += n;
    memset(b->bytes, 0, n);
    return b->bytes;
}
object_json_doc *object_json_mut_doc_new(const object_json_alc *a)
{
    object_json_alc allocator = oj_allocator(a);
    if (!allocator.malloc || !allocator.free)
        return NULL;
    object_json_doc *d = allocator.malloc(allocator.ctx, sizeof(*d));
    if (d)
    {
        memset(d, 0, sizeof(*d));
        d->allocator = allocator;
    }
    return d;
}
void object_json_doc_free(object_json_doc *d)
{
    if (!d)
        return;
    oj_block *b = d->blocks;
    while (b)
    {
        oj_block *next = b->next;
        d->allocator.free(d->allocator.ctx, b);
        b = next;
    }
    object_json_alc a = d->allocator;
    a.free(a.ctx, d);
}
void object_json_mut_doc_free(object_json_doc *d)
{
    object_json_doc_free(d);
}
object_json_val *object_json_doc_get_root(object_json_doc *d)
{
    return d ? d->root : NULL;
}
object_json_val *object_json_mut_doc_get_root(object_json_doc *d)
{
    return object_json_doc_get_root(d);
}
void object_json_mut_doc_set_root(object_json_doc *d, object_json_val *v)
{
    if (d)
    {
        d->root = v;
        if (!v)
            d->failed = true;
    }
}
static object_json_val *oj_new(object_json_doc *d, unsigned tag)
{
    object_json_val *v = oj_alloc(d, sizeof(*v));
    if (!v)
        return NULL;
    v->owner = d;
    v->tag = tag;
    v->model.primitive.type = tag == OJ_OBJ                      ? OBJECT_OBJECT
                              : tag == OJ_ARR                    ? OBJECT_ARRAY
                              : tag == OJ_STR                    ? OBJECT_STRING
                              : tag == OJ_BOOL                   ? OBJECT_BOOLEAN
                              : tag == OJ_NULL                   ? OBJECT_NULL
                              : tag == OJ_UINT || tag == OJ_SINT ? OBJECT_NUMBER
                                                                 : OBJECT_DECIMAL;
    return v;
}
#define OJ_IS(name, tagvalue)                                                                      \
    bool object_json_is_##name(const object_json_val *v)                                           \
    {                                                                                              \
        return v && v->tag == tagvalue;                                                            \
    }
OJ_IS(obj, OJ_OBJ)
OJ_IS(arr, OJ_ARR)
OJ_IS(str, OJ_STR)
OJ_IS(raw, OJ_RAW) OJ_IS(uint, OJ_UINT) OJ_IS(sint, OJ_SINT) OJ_IS(real, OJ_REAL)
    OJ_IS(bool, OJ_BOOL) OJ_IS(null, OJ_NULL)
#undef OJ_IS
        bool object_json_is_int(const object_json_val *v)
{
    return object_json_is_uint(v) || object_json_is_sint(v);
}
bool object_json_is_num(const object_json_val *v)
{
    return object_json_is_int(v) || object_json_is_real(v);
}
bool object_json_get_bool(const object_json_val *v)
{
    return object_json_is_bool(v) && v->number.u != 0;
}
bool object_json_is_true(const object_json_val *v)
{
    return object_json_is_bool(v) && object_json_get_bool(v);
}
bool object_json_is_false(const object_json_val *v)
{
    return object_json_is_bool(v) && !object_json_get_bool(v);
}
uint64_t object_json_get_uint(const object_json_val *v)
{
    return object_json_is_uint(v)   ? v->number.u
           : object_json_is_sint(v) ? (uint64_t)v->number.i
                                    : 0;
}
int64_t object_json_get_sint(const object_json_val *v)
{
    return object_json_is_sint(v) ? v->number.i : object_json_is_uint(v) ? (int64_t)v->number.u : 0;
}
double object_json_get_num(const object_json_val *v)
{
    return object_json_is_real(v)   ? v->number.f
           : object_json_is_sint(v) ? (double)v->number.i
           : object_json_is_uint(v) ? (double)v->number.u
                                    : 0;
}
double object_json_get_real(const object_json_val *v)
{
    return object_json_is_real(v) ? v->number.f : 0;
}
const char *object_json_get_str(const object_json_val *v)
{
    return object_json_is_str(v) ? v->model.primitive.data : NULL;
}
const char *object_json_mut_get_str(const object_json_val *v)
{
    return object_json_get_str(v);
}
const char *object_json_get_raw(const object_json_val *v)
{
    return object_json_is_raw(v) ? v->model.primitive.data : NULL;
}
size_t object_json_get_len(const object_json_val *v)
{
    return v ? v->length : 0;
}
bool object_json_equals_str(const object_json_val *v, const char *s)
{
    return s && object_json_is_str(v) && strlen(s) == v->length &&
           !memcmp(s, v->model.primitive.data, v->length);
}
size_t object_json_arr_size(const object_json_val *v)
{
    return object_json_is_arr(v) ? v->length : 0;
}
size_t object_json_obj_size(const object_json_val *v)
{
    return object_json_is_obj(v) ? v->length : 0;
}
object_json_val *object_json_first(object_json_val *v)
{
    return v && (v->tag == OJ_OBJ || v->tag == OJ_ARR) ? (object_json_val *)v->model.primitive.data
                                                       : NULL;
}
object_json_val *object_json_next(object_json_val *v)
{
    return v ? (object_json_val *)v->model.primitive.next : NULL;
}
object_json_val *object_json_key(object_json_val *v)
{
    return v ? v->key : NULL;
}
object_json_val *object_json_arr_get(object_json_val *v, size_t i)
{
    if (!object_json_is_arr(v) || i >= v->length)
        return NULL;
    size_t index = 0;
    object_json_val *p = object_json_first(v);
    if (v->cursor && i >= v->cursor_index)
    {
        index = v->cursor_index;
        p = v->cursor;
    }
    while (index++ < i)
        p = object_json_next(p);
    v->cursor = p;
    v->cursor_index = i;
    return p;
}
object_json_val *object_json_mut_arr_get(object_json_val *v, size_t i)
{
    return object_json_arr_get(v, i);
}
object_json_val *object_json_obj_getn(object_json_val *v, const char *s, size_t n)
{
    if (!s || !object_json_is_obj(v))
        return NULL;
    for (object_json_val *p = object_json_first(v); p; p = object_json_next(p))
        if (p->key && p->key->length == n && !memcmp(object_json_get_str(p->key), s, n))
            return p;
    return NULL;
}
object_json_val *object_json_obj_get(object_json_val *v, const char *s)
{
    return s ? object_json_obj_getn(v, s, strlen(s)) : NULL;
}
object_json_val *object_json_mut_obj_get(object_json_val *v, const char *s)
{
    return object_json_obj_get(v, s);
}
object_json_val *object_json_mut_obj(object_json_doc *d)
{
    return oj_new(d, OJ_OBJ);
}
object_json_val *object_json_mut_arr(object_json_doc *d)
{
    return oj_new(d, OJ_ARR);
}
object_json_val *object_json_mut_null(object_json_doc *d)
{
    return oj_new(d, OJ_NULL);
}
object_json_val *object_json_mut_bool(object_json_doc *d, bool b)
{
    object_json_val *v = oj_new(d, OJ_BOOL);
    if (v)
        v->number.u = b;
    return v;
}
object_json_val *object_json_mut_uint(object_json_doc *d, uint64_t n)
{
    object_json_val *v = oj_new(d, OJ_UINT);
    if (v)
        v->number.u = n;
    return v;
}
object_json_val *object_json_mut_sint(object_json_doc *d, int64_t n)
{
    object_json_val *v = oj_new(d, OJ_SINT);
    if (v)
        v->number.i = n;
    return v;
}
object_json_val *object_json_mut_real(object_json_doc *d, double n)
{
    object_json_val *v = oj_new(d, OJ_REAL);
    if (v)
        v->number.f = n;
    return v;
}
static object_json_val *oj_text(object_json_doc *d, const char *s, size_t n, unsigned tag)
{
    if (!s || n == SIZE_MAX)
    {
        if (d)
            d->failed = true;
        return NULL;
    }
    object_json_val *v = oj_new(d, tag);
    char *text = oj_alloc(d, n + 1);
    if (!v || !text)
        return NULL;
    memcpy(text, s, n);
    text[n] = 0;
    v->model.primitive.data = text;
    v->length = n;
    return v;
}
object_json_val *object_json_mut_strncpy(object_json_doc *d, const char *s, size_t n)
{
    return oj_text(d, s, n, OJ_STR);
}
object_json_val *object_json_mut_strcpy(object_json_doc *d, const char *s)
{
    return oj_text(d, s, s ? strlen(s) : 0, OJ_STR);
}
object_json_val *object_json_mut_str(object_json_doc *d, const char *s)
{
    return object_json_mut_strcpy(d, s);
}
object_json_val *object_json_mut_rawcpy(object_json_doc *d, const char *s)
{
    return oj_text(d, s, s ? strlen(s) : 0, OJ_RAW);
}
static bool oj_append(object_json_val *v, object_json_val *child)
{
    if (!v || !child || v == child || v->owner != child->owner || child->model.primitive.next)
    {
        if (v)
            v->owner->failed = true;
        return false;
    }
    if (v->tail)
        v->tail->model.primitive.next = child;
    else
        v->model.primitive.data = (char *)child;
    v->tail = child;
    v->length++;
    return true;
}
static object_json_val *oj_copy(object_json_doc *, object_json_val *, unsigned);
bool object_json_mut_obj_add(object_json_val *v, object_json_val *key, object_json_val *child)
{
    if (!object_json_is_obj(v) || !object_json_is_str(key) || !child)
        return false;
    if (child == v || child->key || child->model.primitive.next || child->owner != v->owner)
        child = oj_copy(v->owner, child, 0);
    if (key->cursor || key->owner != v->owner)
        key = oj_copy(v->owner, key, 0);
    if (!child || !key)
    {
        v->owner->failed = true;
        return false;
    }
    child->key = key;
    key->cursor = child;
    child->model.name = key->model.primitive.data;
    return oj_append(v, child);
}
bool object_json_mut_arr_append(object_json_val *v, object_json_val *child)
{
    if (!object_json_is_arr(v) || !child)
        return false;
    if (child == v || child->key || child->model.primitive.next || child->owner != v->owner)
        child = oj_copy(v->owner, child, 0);
    return oj_append(v, child);
}
bool object_json_mut_arr_add_val(object_json_val *v, object_json_val *child)
{
    return object_json_mut_arr_append(v, child);
}
bool object_json_mut_obj_add_val(object_json_doc *d, object_json_val *v, const char *key,
                                 object_json_val *child)
{
    return object_json_mut_obj_add(v, object_json_mut_strcpy(d, key), child);
}
#define OJ_ADD(name, type, constructor)                                                            \
    bool object_json_mut_obj_add_##name(object_json_doc *d, object_json_val *v, const char *k,     \
                                        type n)                                                    \
    {                                                                                              \
        return object_json_mut_obj_add_val(d, v, k, constructor(d, n));                            \
    }                                                                                              \
    bool object_json_mut_arr_add_##name(object_json_doc *d, object_json_val *v, type n)            \
    {                                                                                              \
        return object_json_mut_arr_append(v, constructor(d, n));                                   \
    }
OJ_ADD(uint, uint64_t, object_json_mut_uint)
OJ_ADD(sint, int64_t, object_json_mut_sint)
OJ_ADD(int, int64_t, object_json_mut_sint)
OJ_ADD(real, double, object_json_mut_real) OJ_ADD(bool, bool, object_json_mut_bool)
    OJ_ADD(str, const char *, object_json_mut_str)
        OJ_ADD(strcpy, const char *, object_json_mut_strcpy)
#undef OJ_ADD
            bool object_json_mut_obj_add_null(object_json_doc *d, object_json_val *v, const char *k)
{
    return object_json_mut_obj_add_val(d, v, k, object_json_mut_null(d));
}
size_t object_json_mut_obj_remove_keyn(object_json_val *v, const char *key, size_t length)
{
    if (!object_json_is_obj(v))
        return 0;
    size_t removed = 0;
    object_json_val *previous = NULL, *p = object_json_first(v);
    while (p)
    {
        object_json_val *next = object_json_next(p);
        if (key && p->key && p->key->length == length &&
            !memcmp(object_json_get_str(p->key), key, length))
        {
            if (previous)
                previous->model.primitive.next = next;
            else
                v->model.primitive.data = (char *)next;
            if (v->tail == p)
                v->tail = previous;
            p->model.primitive.next = NULL;
            v->length--;
            removed++;
        }
        else
            previous = p;
        p = next;
    }
    return removed;
}
size_t object_json_mut_obj_remove_key(object_json_val *v, const char *key)
{
    return key ? object_json_mut_obj_remove_keyn(v, key, strlen(key)) : 0;
}
static object_json_val *oj_copy(object_json_doc *d, object_json_val *v, unsigned depth)
{
    if (!v || depth > 128 || (depth == 128 && (v->tag == OJ_OBJ || v->tag == OJ_ARR)))
        return NULL;
    object_json_val *copy = oj_new(d, v->tag);
    if (!copy)
        return NULL;
    copy->number = v->number;
    copy->single = v->single;
    if (v->tag == OJ_STR || v->tag == OJ_RAW)
    {
        char *p = oj_alloc(d, v->length + 1);
        if (!p)
            return NULL;
        memcpy(p, v->model.primitive.data, v->length);
        copy->model.primitive.data = p;
        copy->length = v->length;
    }
    else if (v->tag == OJ_OBJ || v->tag == OJ_ARR)
        for (object_json_val *p = object_json_first(v); p; p = object_json_next(p))
        {
            object_json_val *child = oj_copy(d, p, depth + 1);
            if (v->tag == OJ_OBJ)
            {
                if (!object_json_mut_obj_add(copy, oj_copy(d, p->key, depth + 1), child))
                    return NULL;
            }
            else if (!object_json_mut_arr_append(copy, child))
                return NULL;
        }
    return copy;
}
object_json_val *object_json_val_mut_copy(object_json_doc *d, object_json_val *v)
{
    return oj_copy(d, v, 0);
}
object_json_val *object_json_mut_val_mut_copy(object_json_doc *d, object_json_val *v)
{
    return oj_copy(d, v, 0);
}
void object_json_set_real(object_json_val *v, double n)
{
    if (v)
    {
        v->tag = OJ_REAL;
        v->model.primitive.type = OBJECT_DECIMAL;
        v->number.f = n;
    }
}
void object_json_set_fp_to_float(object_json_val *v, bool b)
{
    if (v)
        v->single = b;
}
char *object_json_write_number(const object_json_val *v, char *out)
{
    if (!v || !out)
        return NULL;
    if (v->tag == OJ_UINT)
    {
        int n = snprintf(out, 64, "%" PRIu64, v->number.u);
        return n > 0 ? out + n : NULL;
    }
    if (v->tag == OJ_SINT)
    {
        int n = snprintf(out, 64, "%" PRId64, v->number.i);
        return n > 0 ? out + n : NULL;
    }
    if (v->tag != OJ_REAL)
        return NULL;
    double n = v->single ? (double)(float)v->number.f : v->number.f;
    if (!isfinite(n))
        return NULL;
    for (int precision = 1; precision <= (v->single ? 9 : 17); precision++)
    {
        snprintf(out, 64, "%.*g", precision, n);
        double back = strtod(out, NULL);
        if (v->single ? (float)back == (float)n : back == n)
            return out + strlen(out);
    }
    return NULL;
}
/* UTF-8 decoder rejects overlong sequences, surrogate codepoints and > U+10FFFF. */
static int oj_utf8(const unsigned char *p, size_t length, size_t *used)
{
    if (!length)
        return -1;
    unsigned c = p[0], n = c < 128                  ? 1
                           : c >= 0xc2 && c <= 0xdf ? 2
                           : c >= 0xe0 && c <= 0xef ? 3
                           : c >= 0xf0 && c <= 0xf4 ? 4
                                                    : 0;
    if (!n || length < n)
        return -1;
    unsigned cp = c & (n == 1 ? 127 : n == 2 ? 31 : n == 3 ? 15 : 7);
    for (unsigned i = 1; i < n; i++)
    {
        if ((p[i] & 0xc0) != 0x80)
            return -1;
        cp = (cp << 6) | (p[i] & 63);
    }
    if ((n == 2 && cp < 128) || (n == 3 && cp < 2048) || (n == 4 && cp < 65536) ||
        (cp >= 0xd800 && cp <= 0xdfff) || cp > 0x10ffff)
        return -1;
    *used = n;
    return (int)cp;
}
static size_t oj_encode(unsigned cp, char *p)
{
    if (cp < 128)
    {
        p[0] = (char)cp;
        return 1;
    }
    if (cp < 2048)
    {
        p[0] = (char)(0xc0 | (cp >> 6));
        p[1] = (char)(0x80 | (cp & 63));
        return 2;
    }
    if (cp < 65536)
    {
        p[0] = (char)(0xe0 | (cp >> 12));
        p[1] = (char)(0x80 | ((cp >> 6) & 63));
        p[2] = (char)(0x80 | (cp & 63));
        return 3;
    }
    p[0] = (char)(0xf0 | (cp >> 18));
    p[1] = (char)(0x80 | ((cp >> 12) & 63));
    p[2] = (char)(0x80 | ((cp >> 6) & 63));
    p[3] = (char)(0x80 | (cp & 63));
    return 4;
}
typedef struct
{
    const unsigned char *source;
    size_t length, at;
    object_json_doc *doc;
} oj_parser;
static void oj_space(oj_parser *p)
{
    while (p->at < p->length && (p->source[p->at] == ' ' || p->source[p->at] == '\t' ||
                                 p->source[p->at] == '\r' || p->source[p->at] == '\n'))
        p->at++;
}
static int oj_hex(unsigned c)
{
    return c >= '0' && c <= '9'   ? (int)c - '0'
           : c >= 'a' && c <= 'f' ? (int)c - 'a' + 10
           : c >= 'A' && c <= 'F' ? (int)c - 'A' + 10
                                  : -1;
}
static int oj_hex4(oj_parser *p)
{
    if (p->length - p->at < 4)
        return -1;
    int n = 0;
    for (unsigned i = 0; i < 4; i++)
    {
        int c = oj_hex(p->source[p->at++]);
        if (c < 0)
            return -1;
        n = n * 16 + c;
    }
    return n;
}
static object_json_val *oj_string(oj_parser *p)
{
    if (p->at >= p->length || p->source[p->at++] != '"')
        return NULL;
    size_t start = p->at, end = start;
    while (end < p->length)
    {
        if (p->source[end] == '"')
            break;
        if (p->source[end] == '\\')
        {
            if (++end == p->length)
                return NULL;
        }
        end++;
    }
    if (end == p->length)
        return NULL;
    char *text = oj_alloc(p->doc, end - start + 1);
    if (!text)
        return NULL;
    size_t n = 0;
    while (p->at < end)
    {
        unsigned c = p->source[p->at++];
        if (c < 32)
            return NULL;
        if (c == '\\')
        {
            if (p->at >= end)
                return NULL;
            c = p->source[p->at++];
            switch (c)
            {
            case '"':
            case '\\':
            case '/':
                text[n++] = (char)c;
                break;
            case 'b':
                text[n++] = '\b';
                break;
            case 'f':
                text[n++] = '\f';
                break;
            case 'n':
                text[n++] = '\n';
                break;
            case 'r':
                text[n++] = '\r';
                break;
            case 't':
                text[n++] = '\t';
                break;
            case 'u':
            {
                int cp = oj_hex4(p);
                if (cp < 0)
                    return NULL;
                if (cp >= 0xd800 && cp <= 0xdbff)
                {
                    if (p->length - p->at < 2 || p->source[p->at++] != '\\' ||
                        p->source[p->at++] != 'u')
                        return NULL;
                    int low = oj_hex4(p);
                    if (low < 0xdc00 || low > 0xdfff)
                        return NULL;
                    cp = 0x10000 + ((cp - 0xd800) << 10) + (low - 0xdc00);
                }
                else if (cp >= 0xdc00 && cp <= 0xdfff)
                    return NULL;
                n += oj_encode((unsigned)cp, text + n);
                break;
            }
            default:
                return NULL;
            }
        }
        else if (c < 128)
            text[n++] = (char)c;
        else
        {
            size_t used = 0;
            --p->at;
            if (oj_utf8(p->source + p->at, end - p->at, &used) < 0)
                return NULL;
            memcpy(text + n, p->source + p->at, used);
            n += used;
            p->at += used;
        }
    }
    if (p->at != end)
        return NULL;
    p->at++;
    object_json_val *v = oj_new(p->doc, OJ_STR);
    if (v)
    {
        v->model.primitive.data = text;
        v->length = n;
    }
    return v;
}
static bool oj_digit(unsigned c)
{
    return c >= '0' && c <= '9';
}
static object_json_val *oj_parse(oj_parser *p, unsigned depth)
{
    if (depth > 128)
        return NULL;
    oj_space(p);
    if (p->at >= p->length)
        return NULL;
    unsigned c = p->source[p->at];
    if (c == '"')
        return oj_string(p);
    if (c == '{' || c == '[')
    {
        if (depth >= 128)
            return NULL;
        bool object = c == '{';
        unsigned close = object ? '}' : ']';
        p->at++;
        object_json_val *v = object ? object_json_mut_obj(p->doc) : object_json_mut_arr(p->doc);
        if (!v)
            return NULL;
        oj_space(p);
        if (p->at < p->length && p->source[p->at] == close)
        {
            p->at++;
            return v;
        }
        for (;;)
        {
            object_json_val *key = NULL, *child;
            if (object)
            {
                oj_space(p);
                key = oj_string(p);
                oj_space(p);
                if (!key || p->at >= p->length || p->source[p->at++] != ':')
                    return NULL;
            }
            child = oj_parse(p, depth + 1);
            if (!child)
                return NULL;
            if (object)
            {
                if (!object_json_mut_obj_add(v, key, child))
                    return NULL;
            }
            else if (!object_json_mut_arr_append(v, child))
                return NULL;
            oj_space(p);
            if (p->at >= p->length)
                return NULL;
            c = p->source[p->at++];
            if (c == close)
                return v;
            if (c != ',')
                return NULL;
        }
    }
    const char *literal = c == 'n' ? "null" : c == 't' ? "true" : c == 'f' ? "false" : NULL;
    if (literal)
    {
        size_t n = strlen(literal);
        if (p->length - p->at < n || memcmp(p->source + p->at, literal, n))
            return NULL;
        p->at += n;
        return c == 'n' ? object_json_mut_null(p->doc) : object_json_mut_bool(p->doc, c == 't');
    }
    if (c == '-' || oj_digit(c))
    {
        size_t start = p->at;
        bool negative = c == '-', real = false;
        if (negative && ++p->at == p->length)
            return NULL;
        c = p->source[p->at];
        if (!oj_digit(c))
            return NULL;
        p->at++;
        if (c != '0')
            while (p->at < p->length && oj_digit(p->source[p->at]))
                p->at++;
        if (p->at < p->length && p->source[p->at] == '.')
        {
            real = true;
            p->at++;
            size_t at = p->at;
            while (p->at < p->length && oj_digit(p->source[p->at]))
                p->at++;
            if (at == p->at)
                return NULL;
        }
        if (p->at < p->length && (p->source[p->at] == 'e' || p->source[p->at] == 'E'))
        {
            real = true;
            p->at++;
            if (p->at < p->length && (p->source[p->at] == '+' || p->source[p->at] == '-'))
                p->at++;
            size_t at = p->at;
            while (p->at < p->length && oj_digit(p->source[p->at]))
                p->at++;
            if (at == p->at)
                return NULL;
        }
        size_t n = p->at - start;
        uint64_t integer = 0;
        bool overflow = false;
        if (!real)
            for (size_t i = start + (negative ? 1 : 0); i < p->at; i++)
            {
                unsigned digit = p->source[i] - '0';
                if (integer > (UINT64_MAX - digit) / 10)
                {
                    overflow = true;
                    break;
                }
                integer = integer * 10 + digit;
            }
        if (!real && !overflow && (!negative || integer <= UINT64_C(9223372036854775808)))
        {
            if (!negative)
                return object_json_mut_uint(p->doc, integer);
            if (integer)
                return object_json_mut_sint(p->doc, integer == UINT64_C(9223372036854775808)
                                                        ? INT64_MIN
                                                        : -(int64_t)integer);
        }
        char *text = oj_alloc(p->doc, n + 1);
        if (!text)
            return NULL;
        memcpy(text, p->source + start, n);
        text[n] = 0;
        double number = strtod(text, NULL);
        if (!isfinite(number))
            return NULL;
        return object_json_mut_real(p->doc, number);
    }
    return NULL;
}
object_json_doc *object_json_read_opts(char *s, size_t n, unsigned flags, const object_json_alc *a,
                                       void *error)
{
    (void)flags;
    (void)error;
    if (!s || n > 32u * 1024u * 1024u)
        return NULL;
    object_json_doc *d = object_json_mut_doc_new(a);
    if (!d)
        return NULL;
    oj_parser p = {(const unsigned char *)s, n, 0, d};
    d->root = oj_parse(&p, 0);
    oj_space(&p);
    if (!d->root || d->failed || p.at != n)
    {
        object_json_doc_free(d);
        return NULL;
    }
    return d;
}
object_json_doc *object_json_read(const char *s, size_t n, unsigned flags)
{
    return object_json_read_opts((char *)s, n, flags, NULL, NULL);
}
typedef struct
{
    object_json_alc allocator;
    char *data;
    size_t length, capacity;
    bool failed;
} oj_output;
static void oj_put(oj_output *o, const char *s, size_t n)
{
    if (o->failed)
        return;
    if (n > 32u * 1024u * 1024u - o->length)
    {
        o->failed = true;
        return;
    }
    size_t need = o->length + n + 1;
    if (need > o->capacity)
    {
        size_t capacity = o->capacity ? o->capacity : 256;
        while (capacity < need)
            capacity *= 2;
        char *next = o->allocator.malloc(o->allocator.ctx, capacity);
        if (!next)
        {
            o->failed = true;
            return;
        }
        if (o->length)
            memcpy(next, o->data, o->length);
        if (o->data)
            o->allocator.free(o->allocator.ctx, o->data);
        o->data = next;
        o->capacity = capacity;
    }
    memcpy(o->data + o->length, s, n);
    o->length += n;
    o->data[o->length] = 0;
}
static void oj_quote(oj_output *o, const char *s, size_t n)
{
    oj_put(o, "\"", 1);
    for (size_t i = 0; i < n;)
    {
        unsigned c = (unsigned char)s[i];
        if (c == '"' || c == '\\')
        {
            char pair[2] = {'\\', (char)c};
            oj_put(o, pair, 2);
            i++;
        }
        else if (c < 32)
        {
            const char *escape = c == '\b'   ? "\\b"
                                 : c == '\f' ? "\\f"
                                 : c == '\n' ? "\\n"
                                 : c == '\r' ? "\\r"
                                 : c == '\t' ? "\\t"
                                             : NULL;
            if (escape)
                oj_put(o, escape, 2);
            else
            {
                char hex[7];
                snprintf(hex, sizeof hex, "\\u%04x", c);
                oj_put(o, hex, 6);
            }
            i++;
        }
        else if (c < 128)
        {
            oj_put(o, s + i, 1);
            i++;
        }
        else
        {
            size_t used = 0;
            if (oj_utf8((const unsigned char *)s + i, n - i, &used) < 0)
            {
                o->failed = true;
                return;
            }
            oj_put(o, s + i, used);
            i += used;
        }
    }
    oj_put(o, "\"", 1);
}
static void oj_write(oj_output *o, const object_json_val *v, unsigned depth, unsigned flags)
{
    if (!v || depth > 128 || (depth == 128 && (v->tag == OJ_OBJ || v->tag == OJ_ARR)))
    {
        o->failed = true;
        return;
    }
    char number[64];
    if (v->tag == OJ_OBJ || v->tag == OJ_ARR)
    {
        bool object = v->tag == OJ_OBJ;
        oj_put(o, object ? "{" : "[", 1);
        size_t count = 0;
        bool pretty = (flags & OBJECT_JSON_WRITE_PRETTY) != 0;
        for (object_json_val *p = object_json_first((object_json_val *)v); p;
             p = object_json_next(p))
        {
            if (count++)
                oj_put(o, ",", 1);
            if (pretty)
            {
                oj_put(o, "\n", 1);
                for (unsigned i = 0; i <= depth; i++)
                    oj_put(o, "  ", 2);
            }
            if (count > v->length)
            {
                o->failed = true;
                return;
            }
            if (object)
            {
                if (!p->key)
                {
                    o->failed = true;
                    return;
                }
                oj_quote(o, object_json_get_str(p->key), p->key->length);
                oj_put(o, pretty ? ": " : ":", pretty ? 2 : 1);
            }
            oj_write(o, p, depth + 1, flags);
            if (o->failed)
                return;
        }
        if (pretty && count)
        {
            oj_put(o, "\n", 1);
            for (unsigned i = 0; i < depth; i++)
                oj_put(o, "  ", 2);
        }
        oj_put(o, object ? "}" : "]", 1);
    }
    else if (v->tag == OJ_STR)
        oj_quote(o, v->model.primitive.data, v->length);
    else if (v->tag == OJ_RAW)
        oj_put(o, v->model.primitive.data, v->length);
    else if (v->tag == OJ_NULL)
        oj_put(o, "null", 4);
    else if (v->tag == OJ_BOOL)
        oj_put(o, v->number.u ? "true" : "false", v->number.u ? 4 : 5);
    else if (object_json_write_number(v, number))
    {
        size_t n = strlen(number);
        oj_put(o, number, n);
        if (v->tag == OJ_REAL && !strchr(number, '.') && !strchr(number, 'e') &&
            !strchr(number, 'E'))
            oj_put(o, ".0", 2);
    }
    else if ((flags & OBJECT_JSON_WRITE_ALLOW_INF_AND_NAN) && v->tag == OJ_REAL)
    {
        const char *s = isnan(v->number.f) ? "NaN" : v->number.f < 0 ? "-Infinity" : "Infinity";
        oj_put(o, s, strlen(s));
    }
    else
        o->failed = true;
}
char *object_json_val_write_opts(const object_json_val *v, unsigned flags, const object_json_alc *a,
                                 size_t *length, void *error)
{
    (void)error;
    if (length)
        *length = 0;
    if (!v || (v->owner && v->owner->failed))
        return NULL;
    oj_output o = {0};
    o.allocator = oj_allocator(a);
    if (!o.allocator.malloc || !o.allocator.free)
        return NULL;
    oj_write(&o, v, 0, flags);
    if (o.failed)
    {
        if (o.data)
            o.allocator.free(o.allocator.ctx, o.data);
        return NULL;
    }
    if (length)
        *length = o.length;
    return o.data;
}
char *object_json_mut_val_write_opts(const object_json_val *v, unsigned f, const object_json_alc *a,
                                     size_t *n, void *e)
{
    return object_json_val_write_opts(v, f, a, n, e);
}
char *object_json_mut_val_write(const object_json_val *v, unsigned f, size_t *n)
{
    return object_json_val_write_opts(v, f, NULL, n, NULL);
}
char *object_json_mut_write_opts(object_json_doc *d, unsigned f, const object_json_alc *a,
                                 size_t *n, void *e)
{
    return object_json_val_write_opts(d ? d->root : NULL, f, a, n, e);
}

object_json_doc *object_json_doc_mut_copy(object_json_doc *source, const object_json_alc *a)
{
    object_json_doc *d = object_json_mut_doc_new(a);
    if (!d)
        return NULL;
    d->root = oj_copy(d, object_json_doc_get_root(source), 0);
    if (!d->root)
    {
        object_json_doc_free(d);
        return NULL;
    }
    return d;
}
char *object_json_mut_write(object_json_doc *d, unsigned f, size_t *n)
{
    return object_json_mut_write_opts(d, f, NULL, n, NULL);
}
bool object_json_mut_obj_put(object_json_val *v, object_json_val *key, object_json_val *child)
{
    if (!object_json_is_str(key))
        return false;
    object_json_mut_obj_remove_keyn(v, object_json_get_str(key), object_json_get_len(key));
    return object_json_mut_obj_add(v, key, child);
}
object_json_val *object_json_mut_arr_replace(object_json_val *v, size_t index,
                                             object_json_val *child)
{
    if (!object_json_is_arr(v) || !child || index >= v->length || child->owner != v->owner)
        return NULL;
    object_json_val *previous = index ? object_json_arr_get(v, index - 1) : NULL,
                    *old = previous ? object_json_next(previous) : object_json_first(v);
    child->model.primitive.next = old->model.primitive.next;
    if (previous)
        previous->model.primitive.next = child;
    else
        v->model.primitive.data = (char *)child;
    if (v->tail == old)
        v->tail = child;
    old->model.primitive.next = NULL;
    v->cursor = NULL;
    return old;
}
object_json_val *object_json_mut_arr_with_real(object_json_doc *d, const double *numbers,
                                               size_t count)
{
    object_json_val *a = object_json_mut_arr(d);
    for (size_t i = 0; i < count; i++)
        if (!object_json_mut_arr_append(a, object_json_mut_real(d, numbers[i])))
            return NULL;
    return a;
}
uint64_t object_json_mut_get_uint(const object_json_val *v)
{
    return object_json_get_uint(v);
}
bool object_json_mut_set_uint(object_json_val *v, uint64_t n)
{
    if (!v)
        return false;
    v->tag = OJ_UINT;
    v->number.u = n;
    v->model.primitive.type = OBJECT_NUMBER;
    return true;
}
object_json_mut_obj_iter object_json_mut_obj_iter_with(object_json_val *v)
{
    return (object_json_mut_obj_iter){object_json_first(v)};
}
object_json_val *object_json_mut_obj_iter_next(object_json_mut_obj_iter *i)
{
    object_json_val *v = i->next;
    if (v)
        i->next = object_json_next(v);
    return object_json_key(v);
}
object_json_val *object_json_mut_obj_iter_get_val(object_json_val *key)
{
    return key ? key->cursor : NULL;
}

char *object_json_val_write(const object_json_val *v, unsigned f, size_t *n)
{
    return object_json_val_write_opts(v, f, NULL, n, NULL);
}
