/****************************************************************

 object.c

 =============================================================

 Copyright 1996-2020 Tom Barbalet. All rights reserved.

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

 This software is a continuing work of Tom Barbalet, begun on
 13 June 1996. No apes or cats were harmed in the writing of
 this software.

 ****************************************************************/

/*! \file   object.c
 *  \brief  Provides a primitive object type to allow for dynamic
 *          object creation and also reading and writing in a 
 *          JSON format.
 */

#include "toolkit.h"
#include <stdio.h>

static void object_write_object(n_file * file, n_object *start);
static void object_write_array(n_file * file, n_array *start);

#undef OBJECT_DEBUG

#undef OBJECT_RETAIN1
#define OBJECT_RETAIN2 // diff name
#define OBJECT_RETAIN3 // warfare to run but with leak


#ifdef OBJECT_RETAIN1

#define STRING_COPY1(value) io_string_copy(value)

#else

#define STRING_COPY1(value) (value)

#endif

#ifdef OBJECT_RETAIN2

#define STRING_COPY2(value) io_string_copy(value)

#else

#define STRING_COPY2(value) (value)

#endif

#ifdef OBJECT_RETAIN3

#define STRING_COPY3(value) io_string_copy(value)

#else

#define STRING_COPY3(value) (value)

#endif

#ifdef OBJECT_DEBUG

#define OBJ_DBG( test, string ) if (test == 0L) printf("%s\n", string)

#else

#define OBJ_DBG( test, string ) /* test string */

#endif

static n_object * object_file_base(n_file * file);

static n_object_type object_type(n_array * array)
{
    return array->type;
}

static void object_erase(n_object * object)
{
    memory_erase((n_byte*)object, sizeof(n_object));
}

static n_object * object_new(void)
{
    n_object * return_object = (n_object *)memory_new(sizeof(n_object));
    if (return_object)
    {
        object_erase(return_object);
    }
    return return_object;
}

void obj_free(n_object ** object);

static void obj_free_array(n_int is_array, void ** payload, n_object_type type)
{
    n_array *array = 0L;
    
    if (is_array)
    {
        array = (n_array *)(*payload);
    }
    else
    {
        n_object * array_object =  (n_object *)(*payload);
        array = (n_array *)&(array_object->primitive);
    }
    
    
    if (type == OBJECT_NUMBER)
    {

    }
    if (type == OBJECT_STRING)
    {
        memory_free((void**)&array->data);
    }
    if (type == OBJECT_ARRAY)
    {
        n_array *local_array = obj_get_array(array->data);
        obj_free_array(1, (void**)&local_array, local_array->type);
    }
    if (type == OBJECT_OBJECT)
    {
        n_object *object = obj_get_object(array->data);
        obj_free(&object);
    }
    if (array->next)
    {
        if (is_array)
        {
            n_array * next_array = (n_array *)array->next;
            obj_free_array(1, (void **)&next_array, next_array->type);
        }
        else
        {
            obj_free((n_object **)&array->next);
        }
    }
    memory_free(payload);
}

void obj_free(n_object ** object)
{
    n_array *string_primitive = &((*object)->primitive);
    memory_free((void**)&(*object)->name);
    obj_free_array(0, (void **) object, string_primitive->type);
}

void object_top_object(n_file * file, n_object * top_level)
{
    io_write(file, "{", 0);
    object_write_object(file, top_level);
    io_write(file, "}", 0);
}

static void object_top_array(n_file * file, n_array * top_level)
{
    io_write(file, "[", 0);
    object_write_array(file, top_level);
    io_write(file, "]", 0);
}

static void * object_write_primitive(n_file * file, n_array * primitive)
{
    switch (object_type(primitive))
    {
        case OBJECT_NUMBER:
        {
            n_int * int_data = (n_int *)&primitive->data;
            io_writenumber(file, int_data[0],1,0);
        }
            break;
        case OBJECT_STRING:
            io_write(file, "\"", 0);
            io_write(file, primitive->data, 0);
            io_write(file, "\"", 0);
            break;
        case OBJECT_OBJECT:
            object_top_object(file, (n_object *)primitive->data);
            break;
        case OBJECT_ARRAY:
            object_top_array(file, (n_array *)primitive->data);
            break;
        default:
            (void)SHOW_ERROR("Object kind not found");
            return 0L;
    }
    if (primitive->next)
    {
        io_write(file, ",", 0);
    }
    return primitive->next;
}

n_array * obj_get_array(n_string array)
{
    return (n_array *)array;
}

n_object * obj_get_object(n_string object)
{
    return (n_object *)object;
}

n_int obj_get_number(n_string object)
{
    n_int * data = (n_int *)&object;
    return data[0];
}

static void object_write_object(n_file * file, n_object *start)
{
    n_object * current = start;
    do{
        io_write(file, "\"", 0);
        io_write(file, current->name, 0);
        io_write(file, "\":", 0);
        current = (n_object *)object_write_primitive(file, &current->primitive);
    }while (current);
}

static void object_write_array(n_file * file, n_array *start)
{
    n_array * current = start;
    do{
        current = (n_array *)object_write_primitive(file, current);
    }while (current);
}

n_file * obj_json(n_object * object)
{
    n_file * output_file = io_file_new();
    object_top_object(output_file, object);
    return output_file;
}

static n_object * object_end_or_find(n_object * object, n_string name)
{
    n_object * previous_object = 0L;
    n_int      string_length = io_length(name, STRING_BLOCK_SIZE);
    if (string_length > 0)
    {
        n_uint     hash = math_hash((n_byte *)name, (n_uint) string_length);
        if (object == 0L)
        {
            return 0L;
        }
        do
        {
            if (hash == object->name_hash)
            {
                return previous_object;
            }
            previous_object = object;
            object = object->primitive.next;
        }while (object);
    }
    return previous_object;
}

static n_object * obj_get(n_object * object, n_string name)
{
    n_object * set_object;
    n_int      string_length = io_length(name, STRING_BLOCK_SIZE);
    
    if (string_length > 0)
    {
        n_uint     hash = math_hash((n_byte *)name, (n_uint)string_length);
        if (object == 0L)
        {
            object = object_new();
        }
        if (object_type(&object->primitive) == OBJECT_EMPTY)
        {
            set_object = object;
        }
        else
        {
            n_object * previous_object = object_end_or_find(object, name);
            if (previous_object == 0L)
            {
                set_object = object;
            }
            else
            {
                set_object = previous_object->primitive.next;
                if (set_object == 0L)
                {
                    set_object = object_new();
                }
                previous_object->primitive.next = set_object;
            }
        }
        
        set_object->name = STRING_COPY1(name);
        set_object->name_hash = hash;
        
        return set_object;
    }
    return 0L;
}

n_array * array_add(n_array * array, n_array * element)
{
    if (array)
    {
        n_array * next = array;
        do{
            if (next->next)
            {
                next = next->next;
            }
        }while (next->next);
        next->next = element;
    }

    return element;
}

static void * ar_pass_through(void * ptr)
{
    if (ptr == 0L)
    {
        ptr = memory_new(sizeof(n_array));
        if (ptr)
        {
            memory_erase((n_byte *)ptr, sizeof(n_array));
        }
    }
    return ptr;
}

static void * ar_number(void * ptr, n_int set_number)
{
    n_array * cleaned = (n_array *)ar_pass_through(ptr);
    if (cleaned)
    {
        n_int    * number;
        cleaned->type = OBJECT_NUMBER;
        number = (n_int *)&cleaned->data;
        number[0] = set_number;
    }
    return (void *)cleaned;
}

static void * ar_string(void * ptr, n_string set_string)
{
    n_array * cleaned = (n_array *)ar_pass_through(ptr);
    if (cleaned)
    {
        cleaned->type = OBJECT_STRING;
        cleaned->data = STRING_COPY2(set_string);
    }
    return (void *)cleaned;
}

static void * ar_object(void * ptr, n_object * set_object)
{
    n_array * cleaned = (n_array *)ar_pass_through(ptr);
    if (cleaned)
    {
        cleaned->type = OBJECT_OBJECT;
        cleaned->data = (n_string)set_object;
    }
    return (void *)cleaned;
}

static void * ar_array(void * ptr, n_array * set_array)
{
    n_array * cleaned = (n_array *)ar_pass_through(ptr);
    if (cleaned)
    {
        cleaned->type = OBJECT_ARRAY;
        cleaned->data = (n_string)set_array;
    }
    return (void *)cleaned;
}

n_array * array_number(n_int set_number)
{
    return ar_number(0L, set_number);
}

n_array * array_string(n_string set_string)
{
    return ar_string(0L, set_string);
}

n_array * array_object(n_object * set_object)
{
    return ar_object(0L, set_object);
}

n_array * array_array(n_array * set_array)
{
    return ar_array(0L, set_array);
}

static n_object * obj_number(n_object * obj, n_string name, n_int number)
{
    return ar_number(obj_get(obj, name), number);
}

static n_object * obj_string(n_object * obj, n_string name, n_string string)
{
    return ar_string(obj_get(obj, name), string);
}

static n_object * obj_object(n_object * obj, n_string name, n_object * object)
{
    return ar_object(obj_get(obj, name), object);
}

static n_object * obj_array(n_object * obj, n_string name, n_array * array)
{
    return ar_array(obj_get(obj, name), array);
}

n_object * object_number(n_object * obj, n_string name, n_int number)
{
    return obj_number(obj, io_string_copy(name), number);
}

n_object * object_string(n_object * obj, n_string name, n_string string)
{
    return obj_string(obj, io_string_copy(name), string);
}

n_object * object_object(n_object * obj, n_string name, n_object * object)
{
    return obj_object(obj, io_string_copy(name), object);
}

n_object * object_array(n_object * obj, n_string name, n_array * array)
{
    return obj_array(obj, io_string_copy(name), array);
}

static n_int tracking_array_open;
static n_int tracking_object_open;
static n_int tracking_string_quote;

#define CHECK_FILE_SIZE(error_string)   if (file->location >= file->size) \
                                        { \
                                            (void)SHOW_ERROR(error_string); \
                                            return 0L; \
                                        }

static n_string object_file_read_string(n_file * file)
{
    n_string return_string = 0L;
    n_string_block block_string = {0};
    n_int          location = 0;
    if (file->data[file->location] != '"')  // TODO: Replace with smart char handling
    {
        (void)SHOW_ERROR("json not string as expected");
        return return_string;
    }
    
    tracking_string_quote = 1;
    
    file->location ++;
    do{
        CHECK_FILE_SIZE("end of json file reach unexpectedly");
        if(file->data[file->location] != '"')
        {
            block_string[location] = (n_char)file->data[file->location];
            location++;
            file->location++;
        }
    }while (file->data[file->location] != '"');
    if (location == 0)
    {
        (void)SHOW_ERROR("blank string in json file");
        return 0L;
    }
    tracking_string_quote = 0;
    file->location++;
    CHECK_FILE_SIZE("end of json file reach unexpectedly");
    return_string = STRING_COPY3(block_string);
    return return_string;
}


static n_int object_file_read_number(n_file * file, n_int * with_error)
{
    n_int return_number = 0;
    n_string_block block_string = {0};
    n_int          location = 0;
    n_byte         read_char = file->data[file->location];
    n_int          char_okay = (ASCII_NUMBER(read_char) || (read_char == '-'));
    *with_error = 1;

    if (!char_okay)
    {
        (void)SHOW_ERROR("first character not number or minus");
        return 0;
    }
    
    CHECK_FILE_SIZE("end of json file reach unexpectedly for number");
    
    block_string[location] = (n_char)read_char;
    file->location++;
    location++;
    
    
    do{
        read_char = file->data[file->location];
        char_okay = ASCII_NUMBER(read_char);
        
        CHECK_FILE_SIZE("end of json file reach unexpectedly for number");

        if(char_okay)
        {
            block_string[location] = (n_char)read_char;
            location++;
            file->location++;
        }

    }while (char_okay);
    
    {
        n_int actual_value = 1;
        n_int decimal_divisor = 1;
        n_int error_number = io_number((n_string)block_string, &actual_value, &decimal_divisor);

        
        if (error_number == -1)
        {
            return 0;
        }
        
        if (decimal_divisor != 0)
        {
            (void)SHOW_ERROR("decimal number in json file");
            return 0;
        }
        return_number = actual_value;
    }
    *with_error = 0;
    return return_number;
}

static n_object_stream_type object_stream_char(n_byte value)
{
    if (value == '{')
    {
        return OBJ_TYPE_OBJECT_OPEN;
    }
    if (value == '}')
    {
        return OBJ_TYPE_OBJECT_CLOSE;
    }
    if (value == '[')
    {
        return OBJ_TYPE_ARRAY_OPEN;
    }
    if (value == ']')
    {
        return OBJ_TYPE_ARRAY_CLOSE;
    }
    if (ASCII_NUMBER(value) || (value == '-'))
    {
        return OBJ_TYPE_NUMBER;
    }
    if (ASCII_COMMA(value))
    {
        return OBJ_TYPE_COMMA;
    }
    if (ASCII_COLON(value))
    {
        return OBJ_TYPE_COLON;
    }
    if (ASCII_QUOTE(value))
    {
        return OBJ_TYPE_STRING_NOTATION;
    }
    return OBJ_TYPE_EMPTY;
}

static n_array * object_file_array(n_file * file)
{
    n_array * base_array = 0L;
    
    n_object_stream_type stream_type;
    n_object_stream_type stream_type_in_this_array = OBJ_TYPE_EMPTY;
    if (file->data[file->location] != '[') // TODO: Replace with smart char handling
    {
        (void)SHOW_ERROR("json not array as expected");
        return base_array;
    }
    
    tracking_array_open ++;
    
    file->location ++;
    do{
        CHECK_FILE_SIZE("end of json file reach unexpectedly");
        stream_type = object_stream_char(file->data[file->location]);
        
        if (stream_type_in_this_array == OBJ_TYPE_EMPTY)
        {
            stream_type_in_this_array = stream_type;
        }
        else if (stream_type_in_this_array != stream_type)
        {
            (void)SHOW_ERROR("array contains mutliple types");
            return 0L;
        }
        
        if (stream_type == OBJ_TYPE_ARRAY_OPEN)
        {
            n_array * array_value = object_file_array(file);
            if (array_value)
            {
                stream_type = object_stream_char(file->data[file->location]);
                
                if ((stream_type == OBJ_TYPE_ARRAY_CLOSE) || (stream_type == OBJ_TYPE_COMMA))
                {
                    if (base_array == 0L)
                    {
                        base_array = array_array(array_value);
                    }
                    else
                    {
                        array_add(base_array, array_array(array_value));
                    }
                }
            }
            stream_type = object_stream_char(file->data[file->location]);
        }
        if (stream_type == OBJ_TYPE_OBJECT_OPEN)
        {
            n_object * object_value = object_file_base(file);
            OBJ_DBG(object_value, "object value is nil?");
            if (object_value)
            {
                file->location++;
                CHECK_FILE_SIZE("end of json file reach unexpectedly");
                stream_type = object_stream_char(file->data[file->location]);
                
                if ((stream_type == OBJ_TYPE_ARRAY_CLOSE) || (stream_type == OBJ_TYPE_COMMA))
                {
                    if (base_array == 0L)
                    {
                        base_array = array_object(object_value);
                    }
                    else
                    {
                        array_add(base_array, array_object(object_value));
                    }
                }
            }
            
            OBJ_DBG(base_array, "base array still nil?");

            CHECK_FILE_SIZE("end of json file reach unexpectedly");
            stream_type = object_stream_char(file->data[file->location]);
        }
        if (stream_type == OBJ_TYPE_STRING_NOTATION)
        {
            n_string string_value = object_file_read_string(file);
            if (string_value)
            {
                stream_type = object_stream_char(file->data[file->location]);
                
                if ((stream_type == OBJ_TYPE_ARRAY_CLOSE) || (stream_type == OBJ_TYPE_COMMA))
                {
                    if (base_array == 0L)
                    {
                        base_array = array_string(string_value);
                    }
                    else
                    {
                        array_add(base_array, array_string(string_value));
                    }
                }
            }
            stream_type = object_stream_char(file->data[file->location]);
        }
        if (stream_type == OBJ_TYPE_NUMBER)
        {
            n_int with_error;
            n_int number_value = object_file_read_number(file, &with_error);
            
            if (with_error == 0)
            {
                stream_type = object_stream_char(file->data[file->location]);
                
                if ((stream_type == OBJ_TYPE_ARRAY_CLOSE) || (stream_type == OBJ_TYPE_COMMA))
                {
                    if (base_array == 0L)
                    {
                        base_array = array_number(number_value);
                    }
                    else
                    {
                        array_add(base_array, array_number(number_value));
                    }
                }
            }
            stream_type = object_stream_char(file->data[file->location]);
        }
        if (stream_type == OBJ_TYPE_ARRAY_CLOSE)
        {
            tracking_array_open --;
        }
        file->location ++;
        
        OBJ_DBG(base_array, "base array nil check?");
    }while (stream_type == OBJ_TYPE_COMMA);
    
    OBJ_DBG(base_array, "base array really nil?");
    
    return base_array;
}

static n_object * object_file_base(n_file * file)
{
    n_object * base_object = 0L;
    n_object_stream_type stream_type;
    CHECK_FILE_SIZE("file read outside end of file");
    
    stream_type = object_stream_char(file->data[file->location]);
    
    if (stream_type == OBJ_TYPE_OBJECT_OPEN)
    {
        tracking_object_open++;
        do{
            file->location++;
            CHECK_FILE_SIZE("file read outside end of file");
            stream_type = object_stream_char(file->data[file->location]);
            if (stream_type == OBJ_TYPE_STRING_NOTATION)
            {
                n_string string_key = object_file_read_string(file);
                if (string_key)
                {
                    stream_type = object_stream_char(file->data[file->location]);
                    if (stream_type == OBJ_TYPE_COLON)
                    {
                        file->location++;
                        CHECK_FILE_SIZE("file read outside end of file");
                        stream_type = object_stream_char(file->data[file->location]);
                        
                        if (stream_type == OBJ_TYPE_OBJECT_OPEN)
                        {
                            n_object * insert_object = object_file_base(file);
                            if (insert_object)
                            {
                                if (base_object)
                                {
                                    obj_object(base_object, string_key, insert_object);
                                }
                                else
                                {
                                    base_object = obj_object(base_object, string_key, insert_object);
                                }
                                file->location++;
                                CHECK_FILE_SIZE("file read outside end of file");
                                stream_type = object_stream_char(file->data[file->location]);
                            }
                        }
                        if (stream_type == OBJ_TYPE_NUMBER)
                        {
                            n_int number_error;
                            n_int number_value = object_file_read_number(file, &number_error);
                            if (number_error == 0)
                            {
                                stream_type = object_stream_char(file->data[file->location]);
                                if ((stream_type == OBJ_TYPE_OBJECT_CLOSE) || (stream_type == OBJ_TYPE_COMMA))
                                {
                                    if (base_object)
                                    {
                                        obj_number(base_object, string_key, number_value);
                                    }
                                    else
                                    {
                                        base_object = obj_number(base_object, string_key, number_value);
                                    }
                                }
                            }
                        }
                        if (stream_type == OBJ_TYPE_STRING_NOTATION)
                        {
                            n_string string_value = object_file_read_string(file);
                            if (string_value)
                            {
                                stream_type = object_stream_char(file->data[file->location]);
                                if ((stream_type == OBJ_TYPE_OBJECT_CLOSE) || (stream_type == OBJ_TYPE_COMMA))
                                {
                                    if (base_object)
                                    {
                                        obj_string(base_object, string_key, string_value);
                                    }
                                    else
                                    {
                                        base_object = obj_string(base_object, string_key, string_value);
                                    }
                                }
                            }
                        }
                        if (stream_type == OBJ_TYPE_ARRAY_OPEN)
                        {
                            n_array * array_value = object_file_array(file); // TODO: rename object_file_read_array
                            if (array_value)
                            {
                                stream_type = object_stream_char(file->data[file->location]);
                                if ((stream_type == OBJ_TYPE_OBJECT_CLOSE) || (stream_type == OBJ_TYPE_COMMA))
                                {
                                    if (base_object)
                                    {
                                        obj_array(base_object, string_key, array_value);
                                    }
                                    else
                                    {
                                        base_object = obj_array(base_object, string_key, array_value);
                                    }
                                }
                                CHECK_FILE_SIZE("file read outside end of file");
                                stream_type = object_stream_char(file->data[file->location]);
                            }
                            
                            OBJ_DBG(array_value, "array value nil?");
                        }
                    }
                }
            }
            if (stream_type == OBJ_TYPE_OBJECT_CLOSE)
            {
                tracking_object_open--;
            }
        } while (stream_type == OBJ_TYPE_COMMA);
    }
    return base_object;
}

n_object * object_file_to_tree(n_file * file)
{
    n_object * base_object = 0L;
    n_int      something_wrong = 0;
    
    tracking_array_open = 0;
    tracking_object_open = 0;
    tracking_string_quote = 0;
    io_whitespace_json(file);
    file->location = 0;

    base_object = object_file_base(file);
    
    if (tracking_array_open != 0)
    {
        (void)SHOW_ERROR("Array json does not match up");
        something_wrong = 1;
    }
    if (tracking_object_open != 0)
    {
        (void)SHOW_ERROR("Object json does not match up");
        something_wrong = 1;
    }
    if (tracking_string_quote != 0)
    {
        (void)SHOW_ERROR("String quote json does not match up");
        something_wrong = 1;
    }
    if (something_wrong)
    {
        obj_free(&base_object);
        return 0L;
    }
    
    return base_object;
}

n_string obj_contains(n_object* base, n_string name, n_object_type type)
{
    n_object * return_object = base;
    n_int      string_length = io_length(name, STRING_BLOCK_SIZE);
    if (string_length > 0)
    {
        n_uint     hash = math_hash((n_byte *)name, (n_uint)string_length);
        if (return_object == 0L)
        {
            return 0L;
        }
        do
        {
            if ((hash == return_object->name_hash) && (type == object_type(&return_object->primitive)))
            {
                return return_object->primitive.data;
            }
            return_object = return_object->primitive.next;
        }while (return_object);
    }
    return 0L;
}

n_int obj_contains_number(n_object* base, n_string name, n_int *number)
{
    n_object * return_object = base;
    n_int      string_length = io_length(name, STRING_BLOCK_SIZE);
    if (string_length > 0)
    {
        n_uint     hash = math_hash((n_byte *)name, (n_uint)string_length);
        if (return_object == 0L)
        {
            return 0;
        }
        do
        {
            if ((hash == return_object->name_hash) && (OBJECT_NUMBER == object_type(&return_object->primitive)))
            {
                n_int * data = (n_int *)&return_object->primitive.data;
                number[0] = data[0];
                return 1;
            }
            return_object = return_object->primitive.next;
        }while (return_object);
    }
    return 0;
}

n_array * obj_array_next(n_array * array, n_array * element)
{
    if (element == 0L)
    {
        return array;
    }
    return (n_array *)element->next;
}

n_int obj_array_count(n_array * array_obj)
{
    n_array * arr_second_follow = 0L;
    n_int     count = 0;
    if (array_obj)
    {
        while ((arr_second_follow = obj_array_next(array_obj, arr_second_follow)))
        {
            count++;
        }
    }
    return count;
}


n_int obj_contains_array_nbyte2(n_object* base, n_string name, n_byte2 * array_numbers, n_int size)
{
    n_string array_string;
    if ((array_string = obj_contains(base, name, OBJECT_ARRAY)))
    {
        n_array * array_obj = obj_get_array(array_string);
        n_array * arr_follow = 0L;
        n_int     count = 0;
        n_int     estimated_size = obj_array_count(array_obj);
        if (estimated_size != size)
        {
            return -1;
        }
        while ((arr_follow = obj_array_next(array_obj, arr_follow)))
        {
            n_int follow_number = obj_get_number(arr_follow->data);
            array_numbers[count++] = follow_number;
            if (count == size)
            {
                return 1;
            }
        }
    }
    return 0;
}

n_int obj_contains_array_numbers(n_object* base, n_string name, n_int * array_numbers, n_int size)
{
    n_string array_string;
    if ((array_string = obj_contains(base, name, OBJECT_ARRAY)))
    {
        n_array * array_obj = obj_get_array(array_string);
        n_array * arr_follow = 0L;
        n_int     count = 0;
        n_int     estimated_size = obj_array_count(array_obj);
        if (estimated_size != size)
        {
            return -1;
        }
        while ((arr_follow = obj_array_next(array_obj, arr_follow)))
        {
            n_int follow_number = obj_get_number(arr_follow->data);
            array_numbers[count++] = follow_number;
            if (count == size)
            {
                return 1;
            }
        }
    }
    return 0;
}
