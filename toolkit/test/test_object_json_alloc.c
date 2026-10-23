/* Fail each allocation in turn; count live toolkit allocations on every path. */
#include "../toolkit.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void *live[10000];
static size_t live_count, attempts;
static long fail_at = -1;
static int injected;

void *memory_new(n_uint bytes)
{
    void *ptr;
    if (fail_at >= 0 && attempts++ == (size_t)fail_at) { injected = 1; return 0L; }
    ptr = malloc(bytes);
    if (ptr) { assert(live_count < 10000); live[live_count++] = ptr; }
    return ptr;
}
void memory_free(void **ptr)
{
    size_t i;
    if (!ptr || !*ptr) return;
    for (i = 0; i < live_count && live[i] != *ptr; i++) { }
    assert(i < live_count);
    live[i] = live[--live_count];
    free(*ptr); *ptr = 0L;
}
n_int draw_error(n_constant_string text, n_constant_string location, n_int line)
{ (void)text; (void)location; (void)line; return -1; }

static size_t parse_case(const char *text, long failure)
{
    n_file input = {0};
    n_object_type type;
    void *tree;
    input.size = input.location = strlen(text);
    input.data = (n_byte *)malloc(input.size);
    memcpy(input.data, text, input.size);
    fail_at = failure; attempts = 0; injected = 0;
    tree = unknown_file_to_tree(&input, &type);
    fail_at = -1;
    assert(injected ? tree == 0L : tree != 0L);
    unknown_free(&tree, type);
    free(input.data);
    assert(!live_count);
    return attempts;
}

static size_t setter_case(int kind, long failure)
{
    n_object *root = object_string(0L, "value", "original"), *result;
    n_object *child = object_null(0L, "child");
    n_array *array = array_decimal(-0.5);
    size_t count;
    fail_at = failure; attempts = 0; injected = 0;
    switch (kind)
    {
    case 0: result = object_string(root, "value", "replacement"); break;
    case 1: result = object_decimal(root, "value", -1.25); break;
    case 2: result = object_null(root, "value"); break;
    case 3: result = object_number(root, "new", -7); break;
    case 4: result = object_boolean(root, "new", 0); break;
    case 5: result = object_object(root, "value", child); if (result) child = 0L; break;
    default: result = object_array(root, "value", array); if (result) array = 0L; break;
    }
    count = attempts; fail_at = -1;
    assert(injected ? result == 0L : result != 0L);
    if (injected) assert(!strcmp(obj_contains(root, "value", OBJECT_STRING), "original"));
    obj_free(&root); obj_free(&child); unknown_free((void **)&array, OBJECT_ARRAY);
    assert(!live_count);
    return count;
}

static size_t serialize_case(int context, long failure)
{
    n_file input = {0};
    n_object_type type;
    void *tree;
    n_file *output;
    size_t count;
    const char *prefix = context == 0 ? "" : context == 1 ? "[" : "{\"v\":";
    const char *suffix = context == 0 ? "" : context == 1 ? "]" : "}";
    size_t prefix_size = strlen(prefix), suffix_size = strlen(suffix);
    input.size = input.location = 5000 + prefix_size + suffix_size;
    input.data = (n_byte *)malloc(input.size);
    memcpy(input.data, prefix, prefix_size);
    memset(input.data + prefix_size, '7', 5000);
    memcpy(input.data + prefix_size + 5000, suffix, suffix_size);
    tree = unknown_file_to_tree(&input, &type);
    assert(tree);
    fail_at = failure; attempts = 0; injected = 0;
    output = unknown_json(tree, type);
    count = attempts; fail_at = -1;
    assert(injected ? output == 0L : output != 0L);
    if (output) assert(output->location == input.size);
    io_file_free(&output);
    unknown_free(&tree, type);
    free(input.data);
    assert(!live_count);
    return count;
}

int main(void)
{
    const char *inputs[] = { "{}", "[]", "null", "-0.125e-2",
        "{\"n\":-42,\"s\":\"text\",\"list\":[null,{},[],1.5,true],\"child\":{\"v\":null}}",
        "{\"v\":\"old\",\"v\":[null,1.5],\"next\":null}" };
    size_t i, failure, count, total = 0;
    int kind;
    for (i = 0; i < sizeof(inputs)/sizeof(*inputs); i++)
    {
        count = parse_case(inputs[i], 100000);
        for (failure = 0; failure < count; failure++) { parse_case(inputs[i], failure); total++; }
    }
    for (kind = 0; kind < 7; kind++)
    {
        count = setter_case(kind, 100000);
        for (failure = 0; failure < count; failure++) { setter_case(kind, failure); total++; }
    }
    for (kind = 0; kind < 3; kind++)
    {
        count = serialize_case(kind, 100000);
        for (failure = 0; failure < count; failure++) { serialize_case(kind, failure); total++; }
    }
    printf("Allocation-failure checks passed: %zu injected failures, no tracked leaks\n", total);
    return 0;
}
