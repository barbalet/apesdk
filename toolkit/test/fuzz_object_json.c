/* libFuzzer target: parser bounds, preprocessing parity, and stable round trips. */
#include "../toolkit.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#define FUZZ_CHECK(condition) do { if (!(condition)) { \
    const char *path = getenv("FUZZ_FAILURE_PATH"); \
    FILE *artifact = path ? fopen(path, "wb") : 0L; \
    if (artifact) { fwrite(data, 1, size, artifact); fclose(artifact); } \
    fprintf(stderr, "Fuzz invariant failed: %s at line %d\n", #condition, __LINE__); \
    abort(); \
} } while (0)

n_int draw_error(n_constant_string text, n_constant_string location, n_int line)
{
    (void)text; (void)location; (void)line;
    return -1;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    n_file input = {0}, compact = {0};
    n_object_type type, compact_type;
    void *tree, *compact_tree;
    if (!size) return 0;
    input.data = malloc(size);
    compact.data = malloc(size);
    if (!input.data || !compact.data) abort();
    memcpy(input.data, data, size);
    memcpy(compact.data, data, size);
    input.size = input.location = compact.size = compact.location = size;
    tree = unknown_file_to_tree(&input, &type);
    io_whitespace_json(&compact);
    compact_tree = unknown_file_to_tree(&compact, &compact_type);
    FUZZ_CHECK((tree != 0L) == (compact_tree != 0L));
    if (tree)
    {
        n_file *output = unknown_json(tree, type);
        n_file *again;
        n_object_type next_type;
        void *next_tree;
        FUZZ_CHECK(output);
        next_tree = unknown_file_to_tree(output, &next_type);
        FUZZ_CHECK(next_tree && next_type == type);
        again = unknown_json(next_tree, next_type);
        FUZZ_CHECK(again && again->location == output->size);
        FUZZ_CHECK(!memcmp(again->data, output->data, again->location));
        io_file_free(&again);
        unknown_free(&next_tree, next_type);
        io_file_free(&output);
    }
    unknown_free(&tree, type);
    unknown_free(&compact_tree, compact_type);
    free(input.data);
    free(compact.data);
    return 0;
}
