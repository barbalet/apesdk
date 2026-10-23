/* Regression coverage for JSON scalars, mixed containers, and owned values. */
#include "../toolkit.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

n_int draw_error(n_constant_string text, n_constant_string location, n_int line)
{
    fprintf(stderr, "%s: %s:%ld\n", text, location, line);
    return -1;
}

static void round_trip(const char *text)
{
    n_file input = {0};
    n_object_type type;
    void *tree;
    n_file *output;
    /* n_file's fields are initialized explicitly to avoid relying on order. */
    input.data = (n_byte *)io_string_copy((n_string)text);
    input.size = strlen(text);
    input.location = 0;
    tree = unknown_file_to_tree(&input, &type);
    assert(tree);
    output = unknown_json(tree, type);
    assert(output && output->location == strlen(text));
    assert(!memcmp(output->data, text, output->location));
    io_file_free(&output);
    unknown_free(&tree, type);
    assert(!tree);
    memory_free((void **)&input.data);
}

static void check_reassignment(void)
{
    n_int value;
    n_object *child = object_number(0L, "n", 1);
    n_object *root = object_object(0L, "child", child);
    n_array *numbers = array_number(-2);
    object_array(root, "numbers", numbers);
    assert(object_object(root, "child", child));
    assert(object_array(root, "numbers", numbers));
    assert(obj_contains_number(child, "n", &value) && value == 1);
    assert(obj_get_array(obj_contains(root, "numbers", OBJECT_ARRAY)) == numbers);
    assert(obj_get_number(numbers->data) == -2);
    /* Replacing a different owned child must still release the old value. */
    object_object(root, "child", object_number(0L, "n", 3));
    object_array(root, "numbers", array_decimal(-4.5));
    child = obj_get_object(obj_contains(root, "child", OBJECT_OBJECT));
    assert(obj_contains_number(child, "n", &value) && value == 3);
    obj_free(&root);
}

static void check_whitespace(void)
{
    const char *invalid[] = {
        "[1\v,2]", "[1\f,2]", "[1 2]", "[1 e2]", "[- 1]",
        "[1 .5]", "[1e +2]", "[n ull]", "[t rue]", "[fa lse]"
    };
    size_t i;
    int preprocess;
    for (preprocess = 0; preprocess <= 1; preprocess++)
    {
        for (i = 0; i < sizeof(invalid)/sizeof(*invalid); i++)
        {
            n_file input = {0};
            n_object_type type;
            void *tree;
            input.data = (n_byte *)io_string_copy((n_string)invalid[i]);
            input.size = strlen(invalid[i]);
            if (preprocess) io_whitespace_json(&input);
            tree = unknown_file_to_tree(&input, &type);
            assert(!tree);
            memory_free((void **)&input.data);
        }
        {
            n_file input = {0};
            n_object_type type;
            n_int number;
            void *tree;
            input.data = (n_byte *)io_string_copy(" \t\r\n{ \"n\": -42, \"text\": \"a b\" } ");
            input.size = strlen((char *)input.data);
            if (preprocess) io_whitespace_json(&input);
            tree = unknown_file_to_tree(&input, &type);
            assert(tree && obj_contains_number(tree, "n", &number) && number == -42);
            assert(!strcmp(obj_contains(tree, "text", OBJECT_STRING), "a b"));
            unknown_free(&tree, type);
            memory_free((void **)&input.data);
        }
    }
}

/* Use exact-length buffers without a trailing NUL to exercise parser bounds. */
static void check_number_tokens(void)
{
    const char *valid[] = {
        "0", "-0", "42", "-42", "0.0", "-0.0", "12.3400",
        "1e0", "-1E+2", "0e-2", "1e9999", "1e-9999", "null"
    };
    const char *invalid[] = {
        "+1", "00", "-01", "01.2", ".1", "-.1", "1.", "1.e2",
        "1e", "1E-", "1E+", "--1", "1e+-2", "1e2.3", "NaN",
        "Infinity", "NULL", "Null", "nul", "null0"
    };
    size_t i;
    int context;
    for (context = 0; context < 3; context++)
    {
        int accepted;
        for (accepted = 0; accepted <= 1; accepted++)
        {
            const char **tokens = accepted ? valid : invalid;
            size_t count = accepted ? sizeof(valid)/sizeof(*valid) : sizeof(invalid)/sizeof(*invalid);
            for (i = 0; i < count; i++)
            {
                char text[128];
                n_file input = {0};
                n_object_type type;
                void *tree;
                if (context == 0) snprintf(text, sizeof(text), "%s", tokens[i]);
                else if (context == 1) snprintf(text, sizeof(text), "[null,%s,null]", tokens[i]);
                else snprintf(text, sizeof(text), "{\"before\":null,\"value\":%s,\"after\":null}", tokens[i]);
                input.size = strlen(text);
                input.data = memory_new(input.size);
                memcpy(input.data, text, input.size);
                tree = unknown_file_to_tree(&input, &type);
                assert((tree != 0L) == accepted);
                if (tree)
                {
                    n_file *output = unknown_json(tree, type);
                    /* The integer API normalizes -0 to 0; fractional -0.0 is exact. */
                    if (strcmp(tokens[i], "-0"))
                    {
                        assert(output->location == strlen(text));
                        assert(!memcmp(output->data, text, output->location));
                    }
                    io_file_free(&output);
                    unknown_free(&tree, type);
                }
                memory_free((void **)&input.data);
            }
        }
    }
    /* A valid prefix must not cause an unterminated container to be accepted. */
    {
        const char *text = "{\"value\":[null,-12.345e-6,null]}";
        size_t length;
        for (length = 1; length < strlen(text); length++)
        {
            n_file input = {0};
            n_object_type type;
            void *tree;
            input.size = length;
            input.data = memory_new(length);
            memcpy(input.data, text, length);
            tree = unknown_file_to_tree(&input, &type);
            assert(!tree);
            memory_free((void **)&input.data);
        }
    }
}

static void check_numeric_types(void)
{
    n_file input = {0};
    n_object_type type;
    void *tree;
    n_int integer = 0;
    double decimal = 0.0;
    const char *text = "{\"integer\":-42,\"decimal\":-0.25,\"empty\":null,\"values\":[1,null,2.5]}";
    input.size = strlen(text);
    input.data = memory_new(input.size);
    memcpy(input.data, text, input.size);
    tree = unknown_file_to_tree(&input, &type);
    assert(tree && type == OBJECT_OBJECT);
    assert(obj_contains_number(tree, "integer", &integer) && integer == -42);
    assert(obj_contains_decimal(tree, "decimal", &decimal) && decimal == -0.25);
    assert(!obj_contains_number(tree, "decimal", &integer));
    assert(!obj_contains_number(tree, "empty", &integer));
    assert(!obj_contains_decimal(tree, "integer", &decimal));
    assert(obj_contains_null(tree, "empty") && !obj_contains_null(tree, "missing"));
    {
        n_int numbers[3];
        assert(obj_contains_array_numbers(tree, "values", numbers, 3) == -1);
    }
    unknown_free(&tree, type);
    memory_free((void **)&input.data);
}

static void check_long_numbers(void)
{
    int fraction;
    for (fraction = 0; fraction <= 1; fraction++)
    {
        n_file input = {0};
        n_object_type type;
        n_file *output;
        void *tree;
        size_t length = STRING_BLOCK_SIZE + 256;
        input.size = length;
        input.data = memory_new(length);
        memset(input.data, '7', length);
        input.data[0] = '-';
        if (fraction) input.data[2] = '.';
        tree = unknown_file_to_tree(&input, &type);
        assert(tree && type == OBJECT_DECIMAL);
        output = unknown_json(tree, type);
        assert(output->location == length);
        assert(!memcmp(output->data, input.data, length));
        io_file_free(&output);
        unknown_free(&tree, type);
        memory_free((void **)&input.data);
    }
}

static n_object *set_cleanup_value(n_object *root, int kind)
{
    switch (kind)
    {
    case 0: return object_number(root, "value", -7);
    case 1: return object_boolean(root, "value", 0);
    case 2: return object_string(root, "value", "owned text");
    case 3: return object_decimal(root, "value", -0.125);
    case 4: return object_null(root, "value");
    case 5: return object_object(root, "value", object_string(0L, "child", "owned child"));
    case 6: return object_array(root, "value", array_decimal(2.5));
    case 7: return object_object(root, "value", 0L);
    default: return object_array(root, "value", 0L);
    }
}

static void check_cleanup(void)
{
    int from, to;
    for (from = 0; from < 9; from++)
    {
        for (to = 0; to < 9; to++)
        {
            n_int marker;
            n_object *root = set_cleanup_value(0L, from);
            object_number(root, "marker", 777);
            assert(set_cleanup_value(root, to) == root);
            assert(obj_contains_number(root, "marker", &marker) && marker == 777);
            {
                n_file *output = unknown_json(root, OBJECT_OBJECT);
                assert(output);
                io_file_free(&output);
            }
            obj_free(&root);
            assert(!root);
            obj_free(&root);
        }
    }
    /* Failure after replacement must release both discarded and retained data. */
    {
        const char *invalid[] = {
            "{\"v\":\"owned\",\"v\":[null,1.5,{}],\"bad\":",
            "[{},[],null,\"owned\",-0.5,{\"nested\":[null,] } ]",
            "{\"v\":{\"child\":\"old\"},\"v\":null,\"bad\":["
        };
        size_t i;
        for (i = 0; i < sizeof(invalid)/sizeof(*invalid); i++)
        {
            n_file input = {0};
            n_object_type type;
            void *tree;
            input.data = (n_byte *)io_string_copy((n_string)invalid[i]);
            input.size = strlen(invalid[i]);
            tree = unknown_file_to_tree(&input, &type);
            assert(!tree && type == OBJECT_EMPTY);
            unknown_free(&tree, type);
            memory_free((void **)&input.data);
        }
    }
    obj_free(0L);
    unknown_free(0L, OBJECT_NULL);
    {
        void *nothing = 0L;
        unknown_free(&nothing, OBJECT_ARRAY);
        unknown_free(&nothing, OBJECT_OBJECT);
    }
    /* Construct flat lists in linear time; cleanup must not recurse per sibling. */
    {
        n_array *head = array_null(), *tail = head;
        n_object *objects = object_null(0L, "value"), *last = objects;
        int i;
        for (i = 0; i < 100000; i++)
        {
            n_array *next;
            n_object *entry = object_number(0L, "n", i);
            switch (i % 5)
            {
            case 0: next = array_null(); break;
            case 1: next = array_decimal(-0.5); break;
            case 2: next = array_string("owned"); break;
            case 3: next = array_array(0L); break;
            default: next = array_object(0L); break;
            }
            tail->next = next;
            tail = next;
            last->primitive.next = entry;
            last = entry;
        }
        unknown_free((void **)&head, OBJECT_ARRAY);
        obj_free(&objects);
        assert(!head && !objects);
    }
}

static void check_public_interfaces(void)
{
    n_object *root = object_null(0L, "optional");
    n_array *values = array_null();
    n_file *serialized;
    n_file input = {0};
    n_object_type type;
    void *parsed;
    n_int integer;
    double number = 99.0;
    array_add(values, array_decimal(-2.5));
    array_add(values, array_null());
    object_decimal(root, "negative", -0.125);
    object_decimal(root, "negativeZero", -0.0);
    object_number(root, "zero", 0);
    object_boolean(root, "false", 0);
    object_array(root, "values", values);
    assert(!obj_contains_decimal(root, "absent", &number) && number == 99.0);
    assert(!obj_contains_decimal(root, "optional", &number) && number == 99.0);
    assert(!obj_contains_decimal(root, "negative", 0L));
    assert(!object_decimal(root, "negative", NAN));
    assert(!object_decimal(root, "negative", -INFINITY));
    assert(obj_contains_decimal(root, "negative", &number) && number == -0.125);
    assert(obj_contains_decimal(root, "negativeZero", &number) && signbit(number));
    serialized = unknown_json(root, OBJECT_OBJECT);
    assert(serialized);
    {
        const char *expected = "{\"optional\":null,\"negative\":-0.125,\"negativeZero\":-0.0,\"zero\":0,\"false\":false,\"values\":[null,-2.5,null]}";
        assert(serialized->location == strlen(expected));
        assert(!memcmp(serialized->data, expected, serialized->location));
    }
    input.size = serialized->location;
    input.data = memory_new(input.size);
    memcpy(input.data, serialized->data, input.size);
    parsed = unknown_file_to_tree(&input, &type);
    assert(parsed && type == OBJECT_OBJECT);
    assert(obj_contains_null(parsed, "optional"));
    assert(!obj_contains_null(parsed, "missing"));
    assert(!obj_contains_null(parsed, "false") && !obj_contains_null(parsed, "zero"));
    assert(obj_contains_number(parsed, "zero", &integer) && integer == 0);
    assert(obj_contains_decimal(parsed, "negative", &number) && number == -0.125);
    assert(obj_contains_decimal(parsed, "negativeZero", &number) && signbit(number));
    {
        n_array *list = obj_get_array(obj_contains(parsed, "values", OBJECT_ARRAY));
        assert(obj_array_count(list) == 3 && list->type == OBJECT_NULL);
        list = obj_array_next(list, list);
        assert(list && list->type == OBJECT_DECIMAL && obj_get_decimal(list->data) == -2.5);
        list = obj_array_next(list, list);
        assert(list && list->type == OBJECT_NULL && !list->next);
    }
    unknown_free(&parsed, type);
    io_file_free(&serialized);
    memory_free((void **)&input.data);
    obj_free(&root);
    /* A null array entry also represents a scalar null root. */
    values = array_null();
    serialized = unknown_json(values, OBJECT_NULL);
    assert(serialized && serialized->location == 4 && !memcmp(serialized->data, "null", 4));
    io_file_free(&serialized);
    unknown_free((void **)&values, OBJECT_NULL);
}

static void check_input_lengths(void)
{
    int preprocess;
    for (preprocess = 0; preprocess <= 1; preprocess++)
    {
        n_byte bytes[] = {'n','u','l','l',0,'x'};
        n_file input = {0};
        n_object_type type;
        void *tree;
        input.size = input.location = sizeof(bytes);
        input.data = bytes;
        if (preprocess) io_whitespace_json(&input);
        tree = unknown_file_to_tree(&input, &type);
        assert(!tree);
    }
    {
        n_file *input = io_file_new_from_string("[null,-0.5]", 11);
        n_object_type type;
        void *tree;
        assert(input);
        tree = unknown_file_to_tree(input, &type);
        assert(tree && type == OBJECT_ARRAY && obj_array_count(tree) == 2);
        unknown_free(&tree, type);
        io_file_free(&input);
    }
}

int main(void)
{
    const char *valid[] = {
        "{\"negative\":-42,\"fraction\":-0.125,\"exp\":1.25e-12,\"precise\":0.123456789012345678901}",
        "[null,1,-2.5,true,false,\"value\",null,{},[],[null]]",
        "{\"first\":null,\"next\":{\"value\":null},\"last\":null}",
        "{}", "[]", "null", "-42", "-0.0", "1E+3", "true", "false", "\"\"",
        "{\"\":null,\"empty\":\"\",\"escaped\":\"a\\\" b\\\\ c\\u0041\"}",
        "[9223372036854775808,-9223372036854775809]",
        "[-9223372036854775808,9223372036854775807]"
    };
    const char *invalid[] = {
        "", "-", "[01]", "[1.]", "[.5]", "[1e]", "[1e+]", "[--1]",
        "[nul]", "[nullx]", "{\"a\":null", "[null,]", "{\"a\":1,}",
        "[", "{", "\"unterminated", "[truefalse]", "{}[]", "[1 2]"
    };
    size_t i;
    n_object *obj;
    n_array *array;
    double value;
    check_input_lengths();
    check_public_interfaces();
    check_cleanup();
    check_number_tokens();
    check_long_numbers();
    check_numeric_types();
    check_reassignment();
    check_whitespace();
    for (i = 0; i < sizeof(valid)/sizeof(*valid); i++) round_trip(valid[i]);
    for (i = 0; i < sizeof(invalid)/sizeof(*invalid); i++)
    {
        n_file input = {0};
        n_object_type type;
        void *tree;
        input.data = (n_byte *)io_string_copy((n_string)invalid[i]);
        input.size = strlen(invalid[i]);
        tree = unknown_file_to_tree(&input, &type);
        if (tree) fprintf(stderr, "accepted invalid JSON: %s\n", invalid[i]);
        assert(!tree);
        memory_free((void **)&input.data);
    }
    {
        n_file input = {0};
        n_object_type type;
        void *tree;
        n_int number;
        input.data = (n_byte *)io_string_copy(" { \"a\": -42, \"b\": [ ] } ");
        input.size = strlen((char *)input.data);
        tree = unknown_file_to_tree(&input, &type);
        assert(tree && obj_contains_number(tree, "a", &number) && number == -42);
        {
            n_array *empty = obj_get_array(obj_contains(tree, "b", OBJECT_ARRAY));
            assert(obj_array_count(empty) == 0);
            array_add(empty, array_number(-3));
            assert(obj_array_count(empty) == 1 && obj_get_number(empty->data) == -3);
        }
        unknown_free(&tree, type);
        memory_free((void **)&input.data);
    }
    {
        n_file input = {0};
        n_object_type type;
        void *tree;
        input.data = (n_byte *)io_string_copy("{}");
        input.size = 2;
        tree = unknown_file_to_tree(&input, &type);
        assert(tree && !obj_contains_null(tree, "missing"));
        assert(!obj_contains(tree, "", OBJECT_STRING));
        unknown_free(&tree, type);
        memory_free((void **)&input.data);
    }
    obj = object_decimal(0L, "value", -2.5);
    assert(obj_contains_decimal(obj, "value", &value) && value == -2.5);
    object_null(obj, "value");
    assert(obj_contains_null(obj, "value"));
    assert(!obj_contains_null(obj, "missing"));
    assert(!obj_contains_decimal(obj, "value", &value));
    object_decimal(obj, "value", 0.0);
    assert(obj_contains_decimal(obj, "value", &value) && value == 0.0);
    object_array(obj, "value", array_decimal(1.25));
    object_null(obj, "value");
    object_string(obj, "value", "retained");
    object_string(obj, "value", obj_contains(obj, "value", OBJECT_STRING));
    assert(!strcmp(obj_contains(obj, "value", OBJECT_STRING), "retained"));
    obj_free(&obj);
    obj_free(&obj);
    array = array_decimal(-0.25);
    assert(array->type == OBJECT_DECIMAL && obj_get_decimal(array->data) == -0.25);
    unknown_free((void **)&array, OBJECT_ARRAY);
    assert(!array_decimal(INFINITY));
    assert(!array_decimal(NAN));
    puts("JSON regression tests passed");
    return 0;
}
