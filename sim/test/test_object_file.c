/****************************************************************

 test_object_file.c

 =============================================================

 Copyright 1996-2019 Tom Barbalet. All rights reserved.

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

#include <stdio.h>

#include "../noble/noble.h"

n_int draw_error(n_constant_string error_text, n_constant_string location, n_int line_number)
{
    if (error_text)
    {
        printf("ERROR: %s @ %s %ld\n", (n_constant_string)error_text, location, line_number);
    }
    return -1;
}

void tof_gather(n_string file_in)
{
    n_file   *in_file = io_file_new();
    n_file   *output_file;
    n_int    file_error = io_disk_read(in_file, file_in);
    n_string file_out = io_string_copy(file_in);
    
    io_whitespace_json(in_file);

    printf("%s --- \n", file_in);
    file_out[0]='2';
    if (file_error != -1)
    {
        n_object * returned_object = object_file_to_tree(in_file);
        io_file_free(&in_file);
        if (returned_object)
        {
            output_file = obj_json(returned_object);
            if (output_file)
            {
                file_error = io_disk_write(output_file, file_out);
                io_file_free(&output_file);
            }
            obj_free(&returned_object);
        }
    }
}

void tof_gather_string(n_string file_in)
{
    n_int    length = io_length(file_in, STRING_BLOCK_SIZE);
    n_file   local_file;

    local_file.data = (n_byte *)io_string_copy(file_in);
    local_file.location = 0;
    local_file.size = length;

    io_whitespace_json(&local_file);
    {
        n_object * returned_object = object_file_to_tree(&local_file);
        if (returned_object)
        {
            obj_free(&returned_object);
        }
    }
    memory_free((void**)&local_file.data);
}

static n_object * check_element(n_int value)
{
    n_string_block string_value = "0 The good ship";
    
    string_value[0] += value;
    
    {
        n_object * element_obj = obj_string(0L, "name", string_value);
        obj_number(element_obj, "number", value);
        obj_number(element_obj, "constant", 4321);
        
        return element_obj;
    }
}

void check_object(void)
{
    n_object * new_object = obj_number(0L, "index", 1);
    
    n_object * sub_object = obj_number(0L, "index", 2);
    n_array  * new_array = array_number(-1);
    
    array_add(new_array, array_number(-1));
    array_add(new_array, array_number(-1));
    array_add(new_array, array_string("hello"));
    array_add(new_array, array_number(2));
    array_add(new_array, array_string("is"));
    array_add(new_array, array_number(4));
    array_add(new_array, array_string("it me"));
    array_add(new_array, array_number(50));
    
    obj_number(sub_object, "top", 3);
    
    obj_array(sub_object, "array", new_array);
    
    array_add(new_array, array_number(10));
    array_add(new_array, array_number(20));
    array_add(new_array, array_number(30));
    
    io_file_debug(obj_json(sub_object));
    
    obj_number(new_object, "top", 2);
    obj_string(new_object, "name", "Benson");
    
    obj_string(new_object, "another", "corner");
    
    io_file_debug(obj_json(new_object));
    
    obj_string(new_object, "name", "Kevin");
    
    io_file_debug(obj_json(sub_object));
    io_file_debug(obj_json(new_object));
    
    {
        n_uint        count = 1;
        n_object    *being_object = check_element(0);
        n_array     *beings = array_object(being_object);
        while (count < 9)
        {
            being_object = check_element(count++);
            array_add(beings, array_object(being_object));
        }
        obj_array(new_object, "beings", beings);
    }
    
    io_file_debug(obj_json(new_object));
    
    obj_object(new_object, "example", sub_object);
    
    io_file_debug(obj_json(new_object));
    
    obj_number(sub_object, "top", 4);
    
//    io_file_debug(obj_json(new_object));
//    io_file_debug(obj_json(sub_object));
    
    obj_free(&new_object);
}


#ifndef OBJECT_TEST

int main(int argc, const char * argv[])
{
    if (argc == 2)
    {
        tof_gather((n_string)argv[1]);
    }
    return 0;
}

#endif
