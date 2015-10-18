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

static void object_top_level(n_file * file, n_object * top_level);
static void object_write_chain(n_file * file, n_object *start);


static n_object_type object_type(n_object * object)
{
    return object->type;
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
        io_write(file, current->name, 0);
        io_write(file, ":", 0);
        switch (object_type(current))
        {
            case OBJECT_NUMBER:
                {
                    n_int * int_data = (n_int *)current->data;
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
        if (current)
        {
            io_write(file, ",", 0);
        }
        current = (n_object *) current->next;
    }while (current);
}


n_file * object_json_out(n_object * object)
{
    n_file * output_file = io_file_new();
    
    object_top_level(output_file, object);
    
    return output_file;
}

n_object * object_json_in(n_file * file)
{
    return 0;
}

n_string object_as_string(n_object * object, n_string name)
{
    return 0;
}

n_int object_as_number(n_object * object, n_string name)
{
    return 0;
}

n_object * object_array_get_first(n_object * array)
{
    n_object * object = 0L;
    
    if (array)
    {
        object = (n_object *)array->data;
    }
    return object;
}

n_object * object_as_object(n_object * object, n_string name)
{
    n_uint hash = math_hash((n_byte *)name, io_length(name, STRING_BLOCK_SIZE));
    n_object *running_object = object;
    
    if (object == 0L)
    {
        (void)SHOW_ERROR("Initial object empty");
        return 0L;
    }
    if (object_type(object) == OBJECT_EMPTY)
    {
        (void)SHOW_ERROR("Initial object empty");
        return 0L;
    }
    
    do
    {
        if (running_object->name_hash == hash)
        {
            return running_object;
        }
        running_object = (n_object *)running_object->next;
    }while (running_object);
    
    (void)SHOW_ERROR("Object not found");
    return 0L;
}

n_object * object_get_array_element(n_object * object, n_uint element)
{
    return 0L;
}

n_object * object_get_next_array_element(n_object * object)
{
    return 0L;
}

void object_add_array_element(n_object * object, n_object * added_object)
{
    void * next = object->next;
    
    if (added_object)
    {
        object->next = (void *)added_object;
        added_object->next = next;
    }
}

void object_remove_array_element(n_object * object, n_object * remove_object)
{
/*    n_object * entry_object = object;
    n_object * previous_object = 0L;
    
    do
    {
        
    } while (entry_object);*/
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

void object_set_array(n_object * object, n_string name, n_object * set_array)
{
    object_erase(object);
}

void object_set_number(n_object * object, n_string name, n_int set_number)
{
    n_object * previous_object = object_end_or_find(object, name);
    n_uint     hash = math_hash((n_byte *)name, io_length(name, STRING_BLOCK_SIZE));
    n_object * set_object;
    n_int    * number;
    
    if (previous_object == 0L)
    {
        return;
    }
    
    set_object = previous_object->next;
    
    if (set_object)
    {
        object_erase(set_object);
    }
    else
    {
        set_object = object_new();
    }
    
    set_object->type = OBJECT_NUMBER;
    number = (n_int *)set_object->data;
    number[0] = set_number;
    set_object->name_hash = hash;
    previous_object->next = set_object;
}

void object_set_string(n_object * object, n_string name, n_string set_string)
{
    n_object * previous_object = object_end_or_find(object, name);
    n_uint     hash = math_hash((n_byte *)name, io_length(name, STRING_BLOCK_SIZE));
    n_object * set_object;
    n_uint     location = 0;
    
    if (previous_object == 0L)
    {
        return;
    }
    set_object = previous_object->next;
    if (set_object)
    {
        object_erase(set_object);
    }
    else
    {
        set_object = object_new();
    }
    set_object->type = OBJECT_STRING;
    if (name[location])
    {
        do
        {
            if (name[location])
            {
                set_object->data[location] = name[location];
            }
            location++;
        }while (name[location]);
    }
    set_object->name_hash = hash;
    previous_object->next = set_object;
}

void object_set_object(n_object * object, n_string name, n_object * set_object)
{
    n_object * previous_object = object_end_or_find(object, name);
    n_uint     hash = math_hash((n_byte *)name, io_length(name, STRING_BLOCK_SIZE));
    n_object * containing_object;
    if (previous_object == 0L)
    {
        return;
    }
    containing_object = previous_object->next;
    if (containing_object)
    {
        object_erase(containing_object);
    }
    else
    {
        containing_object = object_new();
    }
    containing_object->type = OBJECT_OBJECT;
    
    io_copy((n_byte *)set_object, (n_byte *)containing_object->data, sizeof(n_object));
    
    containing_object->name_hash = hash;
    previous_object->next = object;
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
        n_object * current = *object;
        n_object * next = (n_object *)((*object)->next);
        object_specific_free(object);
        if (next)
        {
            do{
                next = (n_object *)(current->next);
                object_specific_free(&current);
                current = next;
            }while (current);
        }
    }
}