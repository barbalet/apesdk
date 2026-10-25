/* Copyright 2026 Tom Barbalet. MIT; see ../object.c. */
#include "../object_json.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    size_t calls, fail, live;
} allocator_state;
static void *allocate(void *context, size_t n)
{
    allocator_state *s = context;
    if (s->calls++ == s->fail)
        return NULL;
    void *p = malloc(n);
    if (p)
        s->live++;
    return p;
}
static void release(void *context, void *p)
{
    allocator_state *s = context;
    if (p)
    {
        assert(s->live);
        s->live--;
        free(p);
    }
}
int main(void)
{
    const char *input = "{\"text\":\"a\\u0000b\\uD83C\\uDF33\",\"max\":18446744073709551615,"
                        "\"min\":-9223372036854775808,\"zero\":-0,\"array\":[null,true,2.5]}";
    object_json_doc *d = object_json_read(input, strlen(input), 0);
    assert(d);
    object_json_val *root = object_json_doc_get_root(d), *text = object_json_obj_get(root, "text");
    assert(object_json_get_len(text) == 7 &&
           !memcmp(object_json_get_str(text), "a\0b\xf0\x9f\x8c\xb3", 7));
    assert(object_json_get_uint(object_json_obj_get(root, "max")) == UINT64_MAX);
    assert(object_json_get_sint(object_json_obj_get(root, "min")) == INT64_MIN);
    assert(signbit(object_json_get_real(object_json_obj_get(root, "zero"))));
    assert(root->model.primitive.type == OBJECT_OBJECT && root->model.primitive.data);
    object_json_doc *copy = object_json_doc_mut_copy(d, NULL);
    assert(copy);
    object_json_doc_free(d);
    size_t n;
    char *encoded = object_json_mut_write(copy, OBJECT_JSON_WRITE_PRETTY, &n);
    assert(encoded && strchr(encoded, '\n'));
    d = object_json_read(encoded, n, 0);
    assert(d);
    free(encoded);
    object_json_val *a = object_json_obj_get(object_json_doc_get_root(d), "array");
    assert(object_json_is_null(object_json_arr_get(a, 0)) &&
           object_json_is_true(object_json_arr_get(a, 1)));
    object_json_doc_free(d);
    object_json_doc_free(copy);
    const char *bad[] = {"[1,]",
                         "{\"x\":}",
                         "01",
                         "NaN",
                         "1e400",
                         "{}{}",
                         "\"\\ud800\"",
                         "\"\\udfff\"",
                         "\"\xc0\x80\"",
                         "\"\xed\xa0\x80\"",
                         "\"\xf4\x90\x80\x80\""};
    for (size_t i = 0; i < sizeof bad / sizeof *bad; i++)
        assert(!object_json_read(bad[i], strlen(bad[i]), 0));
    const char nul[] = {'[', '0', ']', 0};
    assert(!object_json_read(nul, sizeof nul, 0));
    char deep[300];
    memset(deep, '[', 150);
    memset(deep + 150, ']', 150);
    assert(!object_json_read(deep, sizeof deep, 0));
    bool completed = false;
    for (size_t fail = 0; fail < 100; fail++)
    {
        allocator_state state = {0, fail, 0};
        object_json_alc allocator = {allocate, NULL, release, &state};
        d = object_json_read_opts((char *)input, strlen(input), 0, &allocator, NULL);
        if (d)
        {
            encoded = object_json_mut_write_opts(d, 0, &allocator, &n, NULL);
            if (encoded)
            {
                release(&state, encoded);
                completed = true;
            }
            object_json_doc_free(d);
        }
        assert(!state.live);
    }
    assert(completed);
    d = object_json_mut_doc_new(NULL);
    assert(d);
    a = object_json_mut_arr(d);
    object_json_mut_doc_set_root(d, a);
    uint64_t seed = 123;
    for (unsigned i = 0; i < 10000; i++)
    {
        seed = seed * UINT64_C(6364136223846793005) + 1;
        double value;
        memcpy(&value, &seed, sizeof value);
        if (!isfinite(value))
            continue;
        object_json_val number = {0};
        object_json_set_real(&number, value);
        char out[64];
        assert(object_json_write_number(&number, out));
        double back = strtod(out, NULL);
        assert(back == value && !!signbit(back) == !!signbit(value));
    }
    object_json_mut_arr_append(a, object_json_mut_uint(d, 7));
    object_json_val *o = object_json_mut_obj(d);
    object_json_mut_obj_add_val(d, o, "copied", object_json_mut_arr_get(a, 0));
    assert(object_json_get_uint(object_json_obj_get(o, "copied")) == 7);
    assert(object_json_get_uint(object_json_arr_get(a, 0)) == 7);
    const char key[] = {'x', 0, 'y'};
    object_json_mut_obj_put(o, object_json_mut_strncpy(d, key, sizeof key),
                            object_json_mut_uint(d, 3));
    object_json_mut_obj_put(o, object_json_mut_strncpy(d, key, sizeof key),
                            object_json_mut_uint(d, 9));
    assert(object_json_get_uint(object_json_obj_getn(o, key, sizeof key)) == 9);
    assert(object_json_mut_obj_remove_keyn(o, key, sizeof key) == 1);
    object_json_doc_free(d);
    puts("ApeSDK document JSON: model, UTF-8/NUL, uint64, depth, mutation, 10000 numbers and "
         "allocation cleanup passed");
    return 0;
}
