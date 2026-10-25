/****************************************************************

 object.c

 =============================================================

 Copyright 1996-2026 Tom Barbalet. All rights reserved.

 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or
 sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following
 conditions:

 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.

 ****************************************************************/

/*! \file   object.c
 *  \brief  Provides a primitive object type to allow for dynamic
 *          object creation and also reading and writing in a
 *          JSON format.
 */

#include "toolkit.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>

static n_int object_write_object( n_file *file, n_object *start );
static n_int object_write_array( n_file *file, n_array *start );

#undef OBJECT_DEBUG

#ifdef OBJECT_DEBUG

#define OBJ_DBG( test, string ) if (test == 0L) printf("%s\n", string)

#else

#define OBJ_DBG( test, string ) /* test string */

#endif



static number_array_list    * object_number_array_list = 0L;

static n_uint * object_hashes = 0L;
static n_uint object_hash_count = 0;


n_uint object_get_hash_count(void)
{
    return object_hash_count;
}

void object_init(n_uint * hashes, n_uint hash_count)
{
    object_number_array_list = number_array_list_new();
    object_hashes = hashes;
    object_hash_count = hash_count;
}

void object_ptr_debug(void * ptr)
{
    n_uint      ptr_repr = (n_uint)ptr;

    printf("%ld\n", ptr_repr);
}

void object_close(void)
{
    if (!object_number_array_list) return;
    number_array * checkptr = (number_array *)object_number_array_list->data;
    n_int          loop = 0, array_count = object_number_array_list->count;
    printf("\n array_count %ld\n", array_count);
    while (loop < array_count)
    {
        number_array * location = &checkptr[loop];
        int_list     * il_repr = location->number;
        printf("\n");
        object_ptr_debug(location->array);
        printf("\n");
        int_list_debug(il_repr);

        loop ++;
    }


    number_array_list_free(&object_number_array_list);
}

void object_array_add_number(void * array, n_int number)
{
    if (object_number_array_list)
    {
        if (array)
        {
            number_array * num_array = number_array_list_find_add(object_number_array_list, array);
            if (num_array)
            {
                number_array_number(num_array, number);

                /*                printf("number_array_number\n");*/

            }
            else
            {
                printf("no num_array\n");
            }
        }
        else
        {
            /*            printf("no array\n");*/
        }
    }
    else
    {
        /*        printf("no object_number_array_list\n");*/
    }
}

void object_array_not_number(void * array)
{
    if (object_number_array_list)
    {
        if (array)
        {
            number_array * num_array = number_array_list_find_add(object_number_array_list, array);
            if (num_array)
            {
                number_array_not_number(num_array);
            }
        }
    }
}

static n_object_type object_type( n_array *array )
{
    return array->type;
}

static void object_erase( n_object *object )
{
    memory_erase( ( n_byte * )object, sizeof( n_object ) );
}

static n_object *object_new( void )
{
    n_object *return_object = ( n_object * )memory_new( sizeof( n_object ) );
    if ( return_object )
    {
        object_erase( return_object );
    }
    return return_object;
}

void obj_free( n_object **object );

static void obj_free_array(n_int is_array, void **payload);

/* Release only this value's owned payload, leaving the sibling link intact. */
static void object_release_data(n_array *value)
{
    switch (value->type)
    {
    case OBJECT_STRING:
    case OBJECT_DECIMAL:
        memory_free((void **)&value->data);
        break;
    case OBJECT_ARRAY:
        obj_free_array(1, (void **)&value->data);
        break;
    case OBJECT_OBJECT:
        obj_free((n_object **)&value->data);
        break;
    default:
        break;
    }
    value->data = 0L;
}

static void obj_free_array(n_int is_array, void **payload)
{
    void *current;
    if (!payload) return;
    current = *payload;
    while (current)
    {
        n_array *value = is_array ? (n_array *)current : &((n_object *)current)->primitive;
        void *next = value->next;
        object_release_data(value);
        if (!is_array) memory_free((void **)&((n_object *)current)->name);
        memory_free(&current);
        current = next;
    }
    *payload = 0L;
}

void obj_free(n_object **object)
{
    obj_free_array(0, (void **)object);
}

/* Propagate write failures so unknown_json never returns partial JSON. */
#define JSON_WRITE(text) do { if (io_write(file, (text), 0) == -1) return -1; } while (0)

static n_int object_write_top_object(n_file *file, n_object *value)
{
    JSON_WRITE("{");
    if (object_write_object(file, value) == -1) return -1;
    JSON_WRITE("}");
    return 0;
}

void object_top_object(n_file *file, n_object *value)
{
    (void)object_write_top_object(file, value);
}

static n_int object_top_array(n_file *file, n_array *value)
{
    JSON_WRITE("[");
    if (object_write_array(file, value) == -1) return -1;
    JSON_WRITE("]");
    return 0;
}

static n_int object_write_primitive(n_file *file, n_array *value)
{
    switch (value->type)
    {
    case OBJECT_NULL:
        JSON_WRITE("null");
        break;
    case OBJECT_DECIMAL:
        JSON_WRITE(value->data);
        break;
    case OBJECT_NUMBER:
    {
        char text[32];
        snprintf(text, sizeof(text), "%lld", (long long)obj_get_number(value->data));
        JSON_WRITE(text);
        break;
    }
    case OBJECT_BOOLEAN:
        JSON_WRITE(obj_get_boolean(value->data) ? "true" : "false");
        break;
    case OBJECT_STRING:
        JSON_WRITE("\"");
        JSON_WRITE(value->data);
        JSON_WRITE("\"");
        break;
    case OBJECT_OBJECT:
        if (object_write_top_object(file, (n_object *)value->data) == -1) return -1;
        break;
    case OBJECT_ARRAY:
        if (object_top_array(file, (n_array *)value->data) == -1) return -1;
        break;
    default:
        return SHOW_ERROR("Object kind not found");
    }
    if (value->next) JSON_WRITE(",");
    return 0;
}

n_array *obj_get_array( n_string array )
{
    return ( n_array * )array;
}

n_object *obj_get_object( n_string object )
{
    return ( n_object * )object;
}

n_int obj_get_number( n_string object )
{
    n_int *data = ( n_int * )&object;
    return data[0];
}

n_int obj_get_boolean( n_string object )
{
    return obj_get_number( object );
}

static n_int object_write_object(n_file *file, n_object *current)
{
    while (current && current->primitive.type != OBJECT_EMPTY)
    {
        JSON_WRITE("\"");
        JSON_WRITE(current->name);
        JSON_WRITE("\":");
        if (object_write_primitive(file, &current->primitive) == -1) return -1;
        current = current->primitive.next;
    }
    return 0;
}

static n_int object_write_array(n_file *file, n_array *current)
{
    while (current && current->type != OBJECT_EMPTY)
    {
        if (object_write_primitive(file, current) == -1) return -1;
        current = current->next;
    }
    return 0;
}

#undef JSON_WRITE

n_file *unknown_json(void *unknown, n_object_type type)
{
    n_file *output;
    n_int status;
    if (type < OBJECT_STRING || type > OBJECT_NULL) return 0L;
    if (!unknown && type != OBJECT_OBJECT && type != OBJECT_ARRAY) return 0L;
    output = io_file_new();
    if (!output) return 0L;
    if (type == OBJECT_OBJECT) status = object_write_top_object(output, (n_object *)unknown);
    else if (type == OBJECT_ARRAY) status = object_top_array(output, (n_array *)unknown);
    else status = object_write_primitive(output, (n_array *)unknown);
    if (status == -1) io_file_free(&output);
    return output;
}

static n_object *object_end_or_find( n_object *object, n_string name )
{
    n_object *previous_object = 0L;
    n_int      string_length = io_length( name, STRING_BLOCK_SIZE );
    if ( string_length >= 0 )
    {
        n_uint     hash = math_hash( ( n_byte * )name, ( n_uint ) string_length );
        if ( object == 0L )
        {
            return 0L;
        }
        do
        {
            if ( hash == object->name_hash && !strcmp(name, object->name) )
            {
                return previous_object;
            }
            previous_object = object;
            object = object->primitive.next;
        }
        while ( object );
    }
    return previous_object;
}

static n_object *obj_get( n_object *object, n_string name )
{
    n_object *set_object;
    if (!name) return 0L;
    n_int      string_length = io_length( name, STRING_BLOCK_SIZE );

    if ( string_length >= 0 )
    {
        n_uint     hash = math_hash( ( n_byte * )name, ( n_uint )string_length );
        if ( object == 0L )
        {
            object = object_new();
            if (!object) { memory_free((void **)&name); return 0L; }
        }
        if ( object_type( &object->primitive ) == OBJECT_EMPTY )
        {
            set_object = object;
        }
        else
        {
            n_object *previous_object = object_end_or_find( object, name );
            if ( previous_object == 0L )
            {
                set_object = object;
            }
            else
            {
                set_object = previous_object->primitive.next;
                if ( set_object == 0L )
                {
                    set_object = object_new();
                    if (!set_object) { memory_free((void **)&name); return 0L; }
                }
                previous_object->primitive.next = set_object;
            }
        }

        if (set_object->name)
        {
            object_release_data(&set_object->primitive);
            memory_free((void **)&set_object->name);
        }
        set_object->name = name;
        set_object->name_hash = hash;

        return set_object;
    }
    return 0L;
}

n_array *array_add( n_array *array, n_array * element )
{
    if (array && array->type == OBJECT_EMPTY && element)
    {
        *array = *element;
        memory_free((void **)&element);
        return array;
    }
    if ( array )
    {
        n_array *next = array;
        do
        {
            if ( next->next )
            {
                next = next->next;
            }
        }
        while ( next->next );
        next->next = element;
    }

    return element;
}

void array_add_empty(n_array ** array, n_array * element)
{
    if (*array)
    {
        array_add(*array, element);
    }
    else
    {
        *array = array_add(0L, element);
    }
}

static void *ar_pass_through( void *ptr )
{
    if ( ptr == 0L )
    {
        ptr = memory_new( sizeof( n_array ) );
        if ( ptr )
        {
            memory_erase( ( n_byte * )ptr, sizeof( n_array ) );
        }
    }
    return ptr;
}

static void *ar_number( void *ptr, n_int set_number )
{
    n_array *cleaned = ( n_array * )ar_pass_through( ptr );
    if ( cleaned )
    {
        n_int     *number;
        cleaned->type = OBJECT_NUMBER;
        number = ( n_int * )&cleaned->data;
        number[0] = set_number;
    }
    return ( void * )cleaned;
}

static void *ar_boolean( void *ptr, n_int set_boolean )
{
    n_array *cleaned = ( n_array * )ar_pass_through( ptr );
    if ( cleaned )
    {
        n_int     *number;
        cleaned->type = OBJECT_BOOLEAN;
        number = ( n_int * )&cleaned->data;
        number[0] = set_boolean;
    }
    return ( void * )cleaned;
}

static void *ar_string(void *ptr, n_string text)
{
    n_string copy = io_string_copy(text);
    n_array *value;
    if (!copy) return 0L;
    value = ar_pass_through(ptr);
    if (!value) { memory_free((void **)&copy); return 0L; }
    value->type = OBJECT_STRING;
    value->data = copy;
    return value;
}

static void *ar_object( void *ptr, n_object *set_object )
{
    n_array *cleaned = ( n_array * )ar_pass_through( ptr );
    if ( cleaned )
    {
        cleaned->type = OBJECT_OBJECT;
        cleaned->data = ( n_string )set_object;
    }
    return ( void * )cleaned;
}

static void *ar_array( void *ptr, n_array *set_array )
{
    n_array *cleaned = ( n_array * )ar_pass_through( ptr );
    if ( cleaned )
    {
        cleaned->type = OBJECT_ARRAY;
        cleaned->data = ( n_string )set_array;
    }
    return ( void * )cleaned;
}

n_array *array_boolean( n_int set_boolean )
{
    return ar_boolean( 0L, set_boolean );
}

n_array *array_number( n_int set_number )
{
    return ar_number( 0L, set_number );
}

n_array *array_string( n_string set_string )
{
    return ar_string( 0L, set_string );
}

n_array *array_object( n_object *set_object )
{
    return ar_object( 0L, set_object );
}

n_array *array_array( n_array *set_array )
{
    return ar_array( 0L, set_array );
}

static n_object *obj_boolean( n_object *obj, n_string name, n_int boolean )
{
    n_object *entry = obj_get(obj, name);
    return entry ? ar_boolean(entry, boolean) : 0L;
}

static n_object *obj_number( n_object *obj, n_string name, n_int number )
{
    n_object *entry = obj_get(obj, name);
    return entry ? ar_number(entry, number) : 0L;
}

static n_object *obj_string( n_object *obj, n_string name, n_string string )
{
    n_string copy;
    n_object *entry;
    if (!name) return 0L;
    copy = io_string_copy(string);
    if (!copy) { memory_free((void **)&name); return 0L; }
    entry = obj_get(obj, name);
    if (!entry) { memory_free((void **)&copy); return 0L; }
    entry->primitive.type = OBJECT_STRING;
    entry->primitive.data = copy;
    return entry;
}

/* Reattaching the same owned child is an idempotent operation. The incoming
 * name is owned by the setter even when the existing entry is retained. */
static n_object *obj_same_child(n_object *obj, n_string name,
                                n_object_type type, void *child)
{
    if (!name) return 0L;
    while (obj)
    {
        if (obj->name && !strcmp(obj->name, name))
        {
            if (obj->primitive.type == type && obj->primitive.data == child)
            {
                memory_free((void **)&name);
                return obj;
            }
            return 0L;
        }
        obj = obj->primitive.next;
    }
    return 0L;
}

static n_object *obj_object( n_object *obj, n_string name, n_object *object )
{
    n_object *same = obj_same_child(obj, name, OBJECT_OBJECT, object);
    n_object *entry = same ? same : obj_get(obj, name);
    return entry ? ar_object(entry, object) : 0L;
}

static n_object *obj_array( n_object *obj, n_string name, n_array *array )
{
    n_object *same = obj_same_child(obj, name, OBJECT_ARRAY, array);
    n_object *entry = same ? same : obj_get(obj, name);
    return entry ? ar_array(entry, array) : 0L;
}

n_object *object_number( n_object *obj, n_string name, n_int number )
{
    return obj_number( obj, io_string_copy( name ), number );
}

n_object *object_boolean( n_object *obj, n_string name, n_int boolean )
{
    return obj_boolean( obj, io_string_copy( name ), boolean );
}

n_object *object_string( n_object *obj, n_string name, n_string string )
{
    return obj_string( obj, io_string_copy( name ), string );
}
n_object *object_object( n_object *obj, n_string name, n_object *object )
{
    return obj_object( obj, io_string_copy( name ), object );
}

n_object *object_array( n_object *obj, n_string name, n_array *array )
{
    return obj_array( obj, io_string_copy( name ), array );
}

/* JSON decimal text is owned, just like a string payload. */
static n_array *array_decimal_text(n_string text)
{
    n_array *value = ar_pass_through(0L);
    size_t length = strlen(text);
    if (!value) return 0L;
    /* JSON numbers have no fixed token length. io_string_copy is capped at
     * STRING_BLOCK_SIZE, so it would silently change longer numeric values. */
    value->data = memory_new(length + 1);
    if (!value->data)
    {
        memory_free((void **)&value);
        return 0L;
    }
    memcpy(value->data, text, length + 1);
    value->type = OBJECT_DECIMAL;
    return value;
}

n_array *array_decimal(double value)
{
    char text[64];
    if (!isfinite(value)) return 0L;
    snprintf(text, sizeof(text), "%.17g", value);
    if (!strchr(text, '.') && !strchr(text, 'e') && !strchr(text, 'E'))
        strcat(text, ".0");
    return array_decimal_text(text);
}

n_array *array_null(void)
{
    n_array *value = ar_pass_through(0L);
    if (value) value->type = OBJECT_NULL;
    return value;
}

static n_object *object_value(n_object *obj, n_string name, n_array *value)
{
    n_object *entry;
    if (!value) { memory_free((void **)&name); return 0L; }
    entry = obj_get(obj, name);
    if (!entry) { obj_free_array(1, (void **)&value); return 0L; }
    entry->primitive.type = value->type;
    entry->primitive.data = value->data;
    memory_free((void **)&value);
    return entry;
}

n_object *object_decimal(n_object *obj, n_string name, double value)
{
    n_array *decimal = array_decimal(value);
    if (!decimal) return 0L;
    return object_value(obj, io_string_copy(name), decimal);
}

n_object *object_null(n_object *obj, n_string name)
{
    return object_value(obj, io_string_copy(name), array_null());
}

double obj_get_decimal(n_string data) { return data ? strtod(data, 0L) : 0.0; }

n_int obj_contains_decimal(n_object *obj, n_string name, double *value)
{
    n_string data = obj_contains(obj, name, OBJECT_DECIMAL);
    if (!data || !value) return 0;
    *value = obj_get_decimal(data);
    return 1;
}

n_int obj_contains_null(n_object *obj, n_string name)
{
    if (!name) return 0L;
    while (obj)
    {
        if (obj->name && !strcmp(obj->name, name)) return obj->primitive.type == OBJECT_NULL;
        obj = obj->primitive.next;
    }
    return 0;
}

void unknown_free(void **unknown, n_object_type type)
{
    if (!unknown || !*unknown) return;
    if (type == OBJECT_OBJECT) obj_free((n_object **)unknown);
    else obj_free_array(1, unknown);
}

#define JSON_WHITESPACE(c) ((c) == 9 || (c) == 10 || (c) == 13 || (c) == 32)

void io_whitespace_json(n_file *file)
{
    n_uint i, out = 0;
    int quoted = 0, escaped = 0;
    n_uint length = file->location && file->location <= file->size ? file->location : file->size;
    for (i = 0; i < length; i++)
    {
        n_byte c = file->data[i];
        if (!quoted && JSON_WHITESPACE(c))
        {
            /* Keep a separator inside a malformed token: removing it would
             * turn inputs such as [1 e2] into valid JSON with a different value. */
            n_uint next = i + 1;
            while (next < length && JSON_WHITESPACE(file->data[next])) next++;
            if (out && next < length && file->data[next] &&
                !strchr("{}[],:", file->data[out - 1]) &&
                !strchr("{}[],:", file->data[next]))
                file->data[out++] = ' ';
            i = next - 1;
            continue;
        }
        file->data[out++] = c;
        if (c == '"' && !escaped) quoted = !quoted;
        escaped = quoted && c == '\\' && !escaped;
    }
    file->size = out;
    file->location = 0;
}

static int json_peek(n_file *f)
{
    return f->location < f->size ? f->data[f->location] : -1;
}

static void json_space(n_file *f)
{
    while (json_peek(f) != -1 && JSON_WHITESPACE(json_peek(f))) f->location++;
}

static int json_digit(int c) { return c >= '0' && c <= '9'; }

/* Keep escape sequences verbatim, matching the serializer's string interface. */
static n_string json_string(n_file *f)
{
    n_uint start, length;
    n_string text;
    if (json_peek(f) != '"') return 0L;
    start = ++f->location;
    while (json_peek(f) != -1 && json_peek(f) != '"')
    {
        int c = json_peek(f);
        if (c < 32) return 0L;
        f->location++;
        if (c == '\\')
        {
            c = json_peek(f);
            if (c == -1 || !strchr("\"\\/bfnrtu", c)) return 0L;
            f->location++;
            if (c == 'u')
            {
                int i;
                for (i = 0; i < 4; i++)
                {
                    c = json_peek(f);
                    if (!json_digit(c) && !(c >= 'a' && c <= 'f') && !(c >= 'A' && c <= 'F')) return 0L;
                    f->location++;
                }
            }
        }
    }
    if (json_peek(f) != '"') return 0L;
    length = f->location - start;
    text = memory_new(length + 1);
    if (!text) return 0L;
    memcpy(text, f->data + start, length);
    text[length] = 0;
    f->location++;
    return text;
}

static n_array *number_base_array;
static unsigned json_array_depth;

static void json_array_key(n_string key)
{
    n_uint i, hash = math_hash((n_byte *)key, strlen(key));
    for (i = 0; object_hashes && i < object_hash_count; i++)
        if (object_hashes[i] == hash)
        {
            object_array_add_number(number_base_array, BIG_NEGATIVE_INTEGER + i);
            break;
        }
}

static n_array *json_value(n_file *f, unsigned depth)
{
    int c;
    json_space(f);
    c = json_peek(f);
    n_array *result = 0L;
    if (depth > 256) return 0L;
    if (c == '{' || c == '[')
    {
        int object = c == '{', close = object ? '}' : ']';
        n_object *head = 0L;
        n_array *array = 0L;
        f->location++;
        if (!object) json_array_depth++;
        json_space(f);
        if (json_peek(f) != close)
        {
            for (;;)
            {
                n_string key = 0L;
                n_array *child;
                if (object)
                {
                    json_space(f);
                    key = json_string(f);
                    json_space(f);
                    if (!key || json_peek(f) != ':') { memory_free((void **)&key); goto failed; }
                    f->location++;
                }
                child = json_value(f, depth + 1);
                if (!child) { memory_free((void **)&key); goto failed; }
                if (object)
                {
                    if (child->type == OBJECT_ARRAY) json_array_key(key);
                    n_object *entry = object_value(head, key, child);
                    if (!entry) goto failed;
                    if (!head) head = entry;
                }
                else
                {
                    array_add_empty(&array, child);
                    if (child->type == OBJECT_NUMBER)
                    {
                        if (!number_base_array) number_base_array = array;
                        object_array_add_number(number_base_array, obj_get_number(child->data));
                    }
                    else if (child->type != OBJECT_ARRAY && child->type != OBJECT_OBJECT)
                        object_array_not_number(number_base_array);
                }
                json_space(f);
                if (json_peek(f) != ',') break;
                f->location++;
            }
        }
        if (json_peek(f) != close) goto failed;
        f->location++;
        if (!object && !array)
        {
            array = ar_pass_through(0L);
            if (!array) goto failed;
        }
        if (!object)
        {
            json_array_depth--;
            if (json_array_depth == 2) object_array_add_number(number_base_array, BIG_INTEGER);
            if (json_array_depth == 1) number_base_array = 0L;
        }
        result = object ? array_object(head) : array_array(array);
        if (!result) goto failed;
        return result;
failed:
        obj_free(&head);
        if (array) obj_free_array(1, (void **)&array);
        return 0L;
    }
    if (c == '"')
    {
        n_string text = json_string(f);
        if (!text) return 0L;
        result = array_string(text);
        memory_free((void **)&text);
        return result;
    }
    if (c == 'n' || c == 't' || c == 'f')
    {
        const char *word = c == 'n' ? "null" : c == 't' ? "true" : "false";
        size_t length = strlen(word);
        if (f->size - f->location < length || memcmp(f->data + f->location, word, length)) return 0L;
        f->location += length;
        return c == 'n' ? array_null() : array_boolean(c == 't');
    }
    if (c == '-' || json_digit(c))
    {
        n_uint start = f->location, length;
        int decimal = 0;
        char *text, *end;
        long long integer;
        if (c == '-') f->location++;
        c = json_peek(f);
        if (!json_digit(c)) return 0L;
        f->location++;
        if (c != '0') while (json_digit(json_peek(f))) f->location++;
        if (json_peek(f) == '.')
        {
            decimal = 1; f->location++;
            if (!json_digit(json_peek(f))) return 0L;
            while (json_digit(json_peek(f))) f->location++;
        }
        if (json_peek(f) == 'e' || json_peek(f) == 'E')
        {
            decimal = 1; f->location++;
            if (json_peek(f) == '+' || json_peek(f) == '-') f->location++;
            if (!json_digit(json_peek(f))) return 0L;
            while (json_digit(json_peek(f))) f->location++;
        }
        length = f->location - start;
        text = memory_new(length + 1);
        if (!text) return 0L;
        memcpy(text, f->data + start, length); text[length] = 0;
        errno = 0;
        integer = strtoll(text, &end, 10);
        /* Large integers retain their exact text through the decimal interface. */
        {
            n_int maximum = (n_int)(~((n_uint)0) >> 1);
            int outside_integer = errno == ERANGE || *end ||
                                  integer < -maximum - 1 || integer > maximum;
            result = decimal || outside_integer ? array_decimal_text(text) : array_number((n_int)integer);
        }
        memory_free((void **)&text);
        return result;
    }
    return 0L;
}

void *unknown_file_to_tree(n_file *file, n_object_type *type)
{
    n_array *value;
    void *result;
    if (!file || !type || !file->data) return 0L;
    *type = OBJECT_EMPTY;
    number_base_array = 0L;
    json_array_depth = 0;
    /* n_file size is capacity; disk reads and writers expose length in location. */
    if (file->location > 0 && file->location <= file->size) file->size = file->location;
    file->location = 0;
    value = json_value(file, 0);
    json_space(file);
    if (!value || file->location != file->size)
    {
        if (value) obj_free_array(1, (void **)&value);
        return 0L;
    }
    *type = value->type;
    if (*type == OBJECT_OBJECT || *type == OBJECT_ARRAY)
    {
        result = value->data;
        if (!result && *type == OBJECT_OBJECT) result = object_new();
        memory_free((void **)&value);
        return result;
    }
    return value;
}

n_string obj_contains( n_object *base, n_string name, n_object_type type )
{
    n_object *return_object = base;
    n_int      string_length = io_length( name, STRING_BLOCK_SIZE );
    if ( string_length >= 0 )
    {
        n_uint     hash = math_hash( ( n_byte * )name, ( n_uint )string_length );
        if ( return_object == 0L )
        {
            return 0L;
        }
        do
        {
            if ( return_object->name && ( hash == return_object->name_hash ) && !strcmp(name, return_object->name) && ( type == object_type( &return_object->primitive ) ) )
            {
                return return_object->primitive.data;
            }
            return_object = return_object->primitive.next;
        }
        while ( return_object );
    }
    return 0L;
}

n_int obj_contains_number( n_object *base, n_string name, n_int *number )
{
    n_object *return_object = base;
    n_int      string_length = io_length( name, STRING_BLOCK_SIZE );
    if ( string_length >= 0 )
    {
        n_uint     hash = math_hash( ( n_byte * )name, ( n_uint )string_length );
        if ( return_object == 0L )
        {
            return 0;
        }
        do
        {
            if ( return_object->name && ( hash == return_object->name_hash ) && !strcmp(name, return_object->name) && ( OBJECT_NUMBER == object_type( &return_object->primitive ) ) )
            {
                n_int *data = ( n_int * )&return_object->primitive.data;
                number[0] = data[0];
                return 1;
            }
            return_object = return_object->primitive.next;
        }
        while ( return_object );
    }
    return 0;
}

n_array *obj_array_next( n_array *array, n_array *element )
{
    if ( element == 0L )
    {
        return array && array->type != OBJECT_EMPTY ? array : 0L;
    }
    return ( n_array * )element->next;
}

n_int obj_array_count( n_array *array_obj )
{
    n_array *arr_second_follow = 0L;
    n_int     count = 0;
    if ( array_obj )
    {
        while ( ( arr_second_follow = obj_array_next( array_obj, arr_second_follow ) ) )
        {
            count++;
        }
    }
    return count;
}


n_int obj_contains_array_nbyte2( n_object *base, n_string name, n_byte2 *array_numbers, n_int size )
{
    n_string array_string;
    if ( ( array_string = obj_contains( base, name, OBJECT_ARRAY ) ) )
    {
        n_array *array_obj = obj_get_array( array_string );
        n_array *arr_follow = 0L;
        n_int     count = 0;
        n_int     estimated_size = obj_array_count( array_obj );
        if ( estimated_size != size )
        {
            return -1;
        }
        while ( ( arr_follow = obj_array_next( array_obj, arr_follow ) ) )
        {
            n_int follow_number = obj_get_number( arr_follow->data );
            array_numbers[count++] = follow_number;
            if ( count == size )
            {
                return 1;
            }
        }
    }
    return 0;
}

n_int obj_contains_array_numbers( n_object *base, n_string name, n_int *array_numbers, n_int size )
{
    n_string array_string;
    if ( ( array_string = obj_contains( base, name, OBJECT_ARRAY ) ) )
    {
        n_array *array_obj = obj_get_array( array_string );
        n_array *arr_follow = 0L;
        n_int     count = 0;
        n_int     estimated_size = obj_array_count( array_obj );
        if ( estimated_size != size )
        {
            return 0;
        }
        while ( ( arr_follow = obj_array_next( array_obj, arr_follow ) ) )
        {
            if (arr_follow->type == OBJECT_NUMBER)
            {
                n_int follow_number = obj_get_number( arr_follow->data );
                array_numbers[count++] = follow_number;
                if ( count == size )
                {
                    return 1;
                }
            }
            else
            {
                return -1;
            }
        }
    }
    return 0;
}

n_array * object_vect2_array(n_vect2 * value)
{
    n_array  * point = array_number(value->x);
    array_add( point, array_number(value->y) );
    return point;
}

n_array * object_vect2_pointer(n_vect2 * vect_array, n_uint count)
{
    n_array * json_array = 0L;
    n_int loop = 0;
    while (loop < count)
    {
        array_add_empty( &json_array, array_array(object_vect2_array(&vect_array[loop])));
        loop++;
    }
    return json_array;
}

n_int object_unwrap_four_vect2( n_string pass_through, n_byte * buffer)
{
    return vect2_unwrap_number_entry( pass_through, buffer, 4);
}

n_int object_unwrap_two_vect2( n_string pass_through, n_byte * buffer)
{
    return vect2_unwrap_number_entry( pass_through, buffer, 2);
}

n_int object_unwrap_vect2( n_string pass_through, n_byte * buffer)
{
    return vect2_unwrap_number_entry( pass_through, buffer, 1);
}

n_int object_count_name_vect2(n_vect2 * vect_array, n_uint count, object_unwrap * wrap_func, n_string name, n_object * object)
{
    n_string array_string;
    n_int out_value = 0;
    if ( ( array_string = obj_contains( object, name, OBJECT_ARRAY ) ) )
    {
        n_array * array_obj = obj_get_array( array_string );
        memory_list * mem_list = object_unwrap_array(array_obj, sizeof(n_vect2), wrap_func, OBJECT_ARRAY);

        if (mem_list->count == count)
        {
            n_int index = 0;
            n_vect2 * value = (n_vect2 *)mem_list->data;
            while (index < (count * 2))
            {
                vect_array[index].data[0] = value[index].data[0];
                vect_array[index].data[1] = value[index].data[1];
                index ++;

                vect_array[index].data[0] = value[index].data[0];
                vect_array[index].data[1] = value[index].data[1];
                index ++;
            }
            out_value = 1;
        }
        memory_list_free(&mem_list);
    }
    return out_value;
}

static n_array * object_vect2_list(memory_list * vect_list)
{
    n_array * json_array = 0L;
    if (vect_list->unit_size == sizeof(n_vect2))
    {
        return object_vect2_pointer((n_vect2 *)vect_list->data, vect_list->count);
    }
    return json_array;
}


n_string object_type_string(n_object_type objtype)
{
    return n_object_type_string[objtype];
}

n_int object_vect2_from_array(n_array * vect_element, n_vect2 * vect_list)
{
    n_int   loop = 0;
    n_array * arr_follow = 0L;
    while ( ( arr_follow = obj_array_next( vect_element, arr_follow ) ) )
    {
        if (arr_follow->type == OBJECT_NUMBER)
        {
            n_int number = obj_get_number( arr_follow->data );
            vect_list->data[loop++] = number;
        }
        else
        {
            printf("Expected OBJECT_NUMBER not %s\n\n", object_type_string(arr_follow->type));
            return -1;
        }

        if (loop == 2)
        {
            return 1;
        }
    }
    return 0;
}


memory_list * object_list_vect2(n_array * vect_array)
{
    memory_list * vect_list = 0L;
    if (vect_array)
    {
        n_int count_in_array = obj_array_count(vect_array);
        n_array * arr_follow = 0L;
        vect_list = memory_list_new(sizeof(n_vect2), count_in_array);
        while ( ( arr_follow = obj_array_next( vect_array, arr_follow ) ) )
        {
            if (arr_follow->type == OBJECT_ARRAY)
            {
                n_vect2 temp_vect;
                n_array * vect_element = obj_get_array( arr_follow->data );
                n_int error_response = object_vect2_from_array(vect_element, &temp_vect);
                if (error_response == 1)
                {
                    memory_list_copy(vect_list, (n_byte*)&temp_vect, sizeof(temp_vect));
                }

                if (error_response == -1)
                {
                    memory_list_free(&vect_list);
                    return 0L;
                }
            }
        }
        vect_list->count = count_in_array;
    }
    return vect_list;
}

n_object * object_vect2_names(n_string names, memory_list * vect_array)
{
    return object_array(0L, names, object_vect2_list(vect_array));
}

n_object * object_vect2_name( n_object *obj, n_string name, n_vect2 * value)
{
    return object_array( obj, name, object_vect2_array(value));
}

n_int   object_name_vect2(n_string name, n_vect2 * value, n_object * input_json)
{
    n_string str_point = obj_contains(input_json, name, OBJECT_ARRAY);
    if (str_point)
    {
        n_array * point_array = obj_get_array(str_point);
        n_int number_elements = obj_array_count(point_array);
        if (number_elements == 2)
        {
            n_int return_value = obj_contains_array_numbers(input_json, name, (n_int *)value, 2);
            if (return_value == 0)
            {
                return SHOW_ERROR("expecting two integers for point");
            }
            if (return_value == -1)
            {
                return SHOW_ERROR("expecting integers for point");
            }
            return 1;
        }
        return SHOW_ERROR("expecting two integers for point");
    }
    return SHOW_ERROR("expecting point element");
}

memory_list * object_unwrap_array(n_array * general_array, n_uint size, object_unwrap wrap_func, n_object_type type)
{
    memory_list * return_list = 0L;
    if (general_array)
    {
        n_int     general_array_count = obj_array_count(general_array);
        n_array * arr_follow = 0L;
        return_list = memory_list_new(size, general_array_count);
        while ( ( arr_follow = obj_array_next( general_array, arr_follow ) ) )
        {
            if (arr_follow->type == type)
            {
                n_byte * temp_buffer = memory_new(size);
                if (1 == wrap_func(arr_follow->data, temp_buffer))
                {
                    memory_list_copy(return_list, temp_buffer, size);
                }
                memory_free((void**)&temp_buffer);
            }
        }
    }
    return return_list;
}

void object_output_object(n_object * value)
{
    n_file *output = unknown_json( value, OBJECT_OBJECT );
    io_file_debug(output);
}

void object_output_array(n_array * value)
{
    n_file *output = unknown_json( value, OBJECT_ARRAY );
    io_file_debug(output);
}

n_array * object_onionskin(n_object * top, n_string value)
{
    n_string next_level = obj_contains(top, value, OBJECT_ARRAY);
    if (next_level)
    {
        n_array *next_level_array = obj_get_array(next_level);
        if (next_level_array)
        {
            return next_level_array;
        }
        return 0L;
    }
    return 0L;
}
