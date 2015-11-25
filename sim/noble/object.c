/****************************************************************

 object.c

 =============================================================

 Copyright 1996-2015 Tom Barbalet. All rights reserved.

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

 This software and Noble Ape are a continuing work of Tom Barbalet,
 begun on 13 June 1996. No apes or cats were harmed in the writing
 of this software.

 ****************************************************************/

/*! \file   object.c
 *  \brief  Provides a primitive object type to allow for dynamic
 *          object creation and also reading and writing in a 
 *          JSON format.
 */

#include "noble.h"
#include <stdio.h>

static void object_write_object(n_file * file, n_object *start);
static void object_write_array(n_file * file, n_array *start);

static n_object_type object_type(n_array * array)
{
    return array->type;
}

static void object_erase(n_object * object)
{
    io_erase((n_byte*)object, sizeof(n_object));
}

static n_object * object_new(void)
{
    n_object * return_object = (n_object *)io_new(sizeof(n_object));
    if (return_object)
    {
        object_erase(return_object);
    }
    return return_object;
}

static void object_primitive_free(n_array ** array)
{
    n_array * referenced_array = * array;
    switch(object_type(referenced_array))
    {
        case OBJECT_ARRAY:
        case OBJECT_OBJECT:
        {
            n_array * child = (n_array *)referenced_array->data;
            obj_free(&child);
        }
        default:
            io_free((void **)array);
            break;
    }
}

void obj_free(n_array ** array)
{
    if (*array)
    {
        n_array * next = (n_array *)((*array)->next);
        if (next)
        {
            obj_free(&next);
        }
        object_primitive_free(array);
    }
}

static void object_top_object(n_file * file, n_object * top_level)
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
    n_uint     hash = math_hash((n_byte *)name, io_length(name, STRING_BLOCK_SIZE));
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
    return previous_object;
}

n_object * obj_get(n_object * object, n_string name)
{
    n_object * set_object;
    n_int      string_length = io_length(name, STRING_BLOCK_SIZE);
    n_uint     hash = math_hash((n_byte *)name, string_length);
    
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
    
    set_object->name = name;
    set_object->name_hash = hash;
    
    return set_object;
}


n_array * pr_add_element(n_array * array, n_array * element)
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

void * pr_pass_through(void * ptr)
{
    if (ptr == 0L)
    {
        ptr = io_new(sizeof(n_array));
        if (ptr)
        {
            io_erase((n_byte *)ptr, sizeof(n_array));
        }
    }
    return ptr;
}

void * pr_number(void * ptr, n_int set_number)
{
    n_array * cleaned = (n_array *)pr_pass_through(ptr);
    if (cleaned)
    {
        n_int    * number;
        cleaned->type = OBJECT_NUMBER;
        number = (n_int *)&cleaned->data;
        number[0] = set_number;
    }
    return (void *)cleaned;
}

void * pr_string(void * ptr, n_string set_string)
{
    n_array * cleaned = (n_array *)pr_pass_through(ptr);
    if (cleaned)
    {
        cleaned->type = OBJECT_STRING;
        cleaned->data = set_string;
    }
    return (void *)cleaned;
}

void * pr_object(void * ptr, n_object * set_object)
{
    n_array * cleaned = (n_array *)pr_pass_through(ptr);
    if (cleaned)
    {
        cleaned->type = OBJECT_OBJECT;
        cleaned->data = (n_string)set_object;
    }
    return (void *)cleaned;
}

void * pr_array(void * ptr, n_array * set_array)
{
    n_array * cleaned = (n_array *)pr_pass_through(ptr);
    if (cleaned)
    {
        cleaned->type = OBJECT_ARRAY;
        cleaned->data = (n_string)set_array;
    }
    return (void *)cleaned;
}

n_object * obj_number(n_object * obj, n_string name, n_int number)
{
    return pr_number(obj_get(obj, name), number);
}

n_object * obj_string(n_object * obj, n_string name, n_string string)
{
    return pr_string(obj_get(obj, name), string);
}

n_object * obj_object(n_object * obj, n_string name, n_object * object)
{
    return pr_object(obj_get(obj, name), object);
}

n_object * obj_array(n_object * obj, n_string name, n_array * array)
{
    return pr_array(obj_get(obj, name), array);
}
