/****************************************************************

 test_object.c

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

static void check_object(void)
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

    obj_object(new_object, "example", sub_object);

    io_file_debug(obj_json(new_object));

    obj_number(sub_object, "top", 4);

    io_file_debug(obj_json(new_object));
    io_file_debug(obj_json(sub_object));
    obj_free((n_array**)&new_object);
}

int main(int argc, const char * argv[])
{
    check_object();
    return 0;
}

