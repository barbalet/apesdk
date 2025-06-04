/****************************************************************

 test_memory.c

 =============================================================

 Copyright 1996-2025 Tom Barbalet. All rights reserved.

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

#include "../toolkit.h"

#include <stdio.h>

n_int draw_error( n_constant_string error_text, n_constant_string location, n_int line_number )
{
    printf( "ERROR: %s @ %s %ld\n", ( const n_string ) error_text, location, line_number );
    return -1;
}


static void * example_a = (void *)0x678e4564;
static void * example_b = (void *)0x8e456467;
static void * example_c = (void *)0x4564678e;

int main( int argc, const char *argv[] )
{
    printf( " --- test memory --- start ----------------------------------------------\n" );

    number_array_list * nal = number_array_list_new();


    number_array * found_na = number_array_list_find_add(nal, example_a);

    number_array_number(found_na, 2000);
    number_array_number(found_na, 100);
    number_array_number(found_na, 300);
    number_array_number(found_na, 20);

    int_list_debug(found_na->number);

    number_array * found_nb = number_array_list_find_add(nal, example_b);

    number_array_number(found_nb, -230);
    number_array_number(found_nb, 20);
    number_array_number(found_nb, 1000);
    number_array_number(found_nb, 6000);
    number_array_number(found_nb, -240);

    int_list_debug(found_nb->number);

    number_array * found_nc = number_array_list_find_add(nal, example_c);

    number_array_number(found_nc, 2000);
    number_array_number(found_nc, 100);
    number_array_number(found_nc, 300);
    number_array_number(found_nc, 20);
    number_array_number(found_nc, -230);
    number_array_number(found_nc, 20);
    number_array_number(found_nc, 1000);
    number_array_number(found_nc, 6000);
    number_array_number(found_nc, -240);

    int_list_debug(found_nc->number);

    number_array * found_nd = number_array_list_find_add(nal, example_a);

    int_list_debug(found_nd->number);


    number_array_list_free(&nal);

    printf( " --- test memory ---  end  ----------------------------------------------\n" );

    return 0;
}

