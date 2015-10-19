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

static void object_top_level(n_file * file, n_object * top_level);
static void object_write_chain(n_file * file, n_object *start);


static n_object_type object_type(n_object * object)
{
    return object->type;
}

void object_debug(n_object * object)
{
    printf("Name %s ",object->name);
    printf("Name Hash %lu ", object->name_hash);
    printf("Object ");
    switch (object_type(object))
    {
        case OBJECT_EMPTY:
            printf("OBJECT_EMPTY\n");
            break;
        case OBJECT_STRING:
            printf("OBJECT_STRING\n");
            break;
        case OBJECT_NUMBER:
            printf("OBJECT_NUMBER\n");
            break;
        case OBJECT_ARRAY:
            printf("OBJECT_ARRAY\n");
            break;
        case OBJECT_OBJECT:
            printf("OBJECT_OBJECT\n");
            break;
        default:
            printf("Unknown OBJECT\n");
            break;
    }
}

static void object_top_level(n_file * file, n_object * top_level)
{
    io_write(file, "{", 0);
    object_write_chain(file, top_level);
    io_write(file, "}", 0);
}

static void object_write_chain(n_file * file, n_object *start)
{
    n_object * current = start;
    do{
        io_write(file, "\"", 0);
        io_write(file, current->name, 0);
        io_write(file, "\":", 0);
        switch (object_type(current))
        {
            case OBJECT_NUMBER:
                {
                    n_int * int_data = (n_int *)&current->data;
                    io_writenumber(file, int_data[0],1,0);
                }
                break;
            case OBJECT_STRING:
                io_write(file, "\"", 0);
                io_write(file, current->data, 0);
                io_write(file, "\"", 0);
                break;
            case OBJECT_OBJECT:
                object_top_level(file, (n_object *)current->data);
                break;
            default:
                (void)SHOW_ERROR("Object kind not found");
                return;
        }
        current = (n_object *) current->next;
        if (current)
        {
            io_write(file, ",", 0);
        }
    }while (current);
}


n_file * object_json_out(n_object * object)
{
    n_file * output_file = io_file_new();
    object_top_level(output_file, object);
    return output_file;
}

static void object_erase(n_object * object)
{
    io_erase((n_byte*)object, sizeof(n_object));
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
        object = object->next;
    }while (object);
    return previous_object;
}

static n_object * object_get(n_object * object, n_string name)
{
    if (object_type(object) == OBJECT_EMPTY)
    {
        return object;
    }
    {
        n_object * previous_object = object_end_or_find(object, name);
        n_object * set_object;
        if (previous_object == 0L)
        {
            set_object = object;
        }
        else
        {
            set_object = previous_object->next;
            if (set_object)
            {
                object_erase(set_object);
            }
            else
            {
                set_object = object_new();
            }
            previous_object->next = set_object;
        }
        return set_object;
    }
}


void object_set_object(n_object * object, n_string name, n_object * active_object)
{
    n_int      string_length = io_length(name, STRING_BLOCK_SIZE);
    n_uint     hash = math_hash((n_byte *)name, string_length);
    n_object * set_object = object_get(object, name);
    
    set_object->name = name;
    set_object->name_hash = hash;
    set_object->type = OBJECT_OBJECT;
    set_object->data = (n_string)active_object;
}

void object_set_number(n_object * object, n_string name, n_int set_number)
{
    n_int      string_length = io_length(name, STRING_BLOCK_SIZE);
    n_uint     hash = math_hash((n_byte *)name, string_length);
    n_object * set_object = object_get(object, name);
    n_int    * number;
    
    set_object->name = name;
    set_object->name_hash = hash;
    set_object->type = OBJECT_NUMBER;
    number = (n_int *)&set_object->data;
    number[0] = set_number;
    
}

void object_set_string(n_object * object, n_string name, n_string set_string)
{
    n_int      string_length = io_length(name, STRING_BLOCK_SIZE);
    n_uint     hash = math_hash((n_byte *)name, string_length);
    n_object * set_object = object_get(object, name);
    
    set_object->name = name;
    set_object->name_hash = hash;
    set_object->type = OBJECT_STRING;
    set_object->data = set_string;
    set_object->name_hash = hash;
}

n_object * object_new(void)
{
    n_object * return_object = (n_object *)io_new(sizeof(n_object));
    if (return_object)
    {
        object_erase(return_object);
    }
    return return_object;
}

static void object_specific_free(n_object ** object)
{
    n_object * referenced_object = * object;
    switch(object_type(referenced_object))
    {
        case OBJECT_ARRAY:
            (void)SHOW_ERROR("No Object Implementation Yet");
            break;
        case OBJECT_OBJECT:
            {
                n_object * child = (n_object *)referenced_object->data;
                object_free(&child);
            }
        default:
            io_free((void **)object);
            break;
    }
}

void object_free(n_object ** object)
{
    if (*object)
    {
        n_object * next = (n_object *)((*object)->next);
        if (next)
        {
            object_free(&next);
        }
        object_specific_free(object);
    }
}
