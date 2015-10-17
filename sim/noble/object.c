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

n_file * object_json_out(n_object * object)
{
    return 0;
}

n_object * object_json_in(n_file * file)
{
    return 0;
}

n_string object_as_string(n_object * object, n_string name, n_int * error)
{
    return 0;
}

n_int object_as_number(n_object * object, n_string name, n_int * error)
{
    return 0;
}

n_object * object_as_object(n_object * object, n_string name, n_int * error)
{
    return 0;
}

void object_set_number(n_object * object, n_string name, n_int set_number)
{
    
}

void object_set_string(n_object * object, n_string name, n_string set_string)
{
    
}

void object_set_object(n_object * object, n_string name, n_object * set_object)
{
    
}

n_object * object_new(void)
{
    return (n_object *)io_new(sizeof(n_object));
}

void object_free(n_object ** object)
{
    
}