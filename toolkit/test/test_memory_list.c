/****************************************************************

 test_memory_list.c

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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../toolkit.h"

n_int draw_error( n_constant_string error_text, n_constant_string location, n_int line_number )
{
    printf( "ERROR: %s @%s, %ld\n", error_text, location, line_number );
    exit(EXIT_FAILURE);
    return -1;
}

void memory_list_debug(memory_list * value)
{
    printf("count %ld\n", value->count);
    printf("max %ld\n", value->max);
    printf("unit_size %ld\n", value->unit_size);
}

void fill_vector(n_vect2* vect, n_int num)
{
    vect->x = 6 - num;
    vect->y = 3 * num;
}

void sum_vector(memory_list * vector)
{
    n_vect2 * vect_data = (n_vect2 *) vector->data;
    n_int loop = 0, count = vector->count;
    n_vect2 sum = {0, 0};
    while (loop < count)
    {
        sum.x += vect_data[loop].x;
        sum.y += vect_data[loop].y;
        loop++;
    }
    printf("X %ld Y %ld\n\n", sum.x, sum.y);
}

void vect_roll(n_array * local_array, n_int *filling_value0, n_int *filling_value1)
{
    n_vect2 *data_down = (n_vect2*) local_array->data;
    *filling_value0 = data_down->data[0];
    local_array = (n_array *) local_array->data;
    data_down = (n_vect2*) local_array->next;
    *filling_value1 = data_down->data[0];
}

memory_list * quad_vector(n_uint number)
{
    n_vect2  v1, v2, v3, v4, v01, v02, v03, v04;
    memory_list * return_value = memory_list_new(sizeof(n_vect2), number);
    n_int number2 = 10;

    fill_vector(&v1, number2++);
    fill_vector(&v2, number2++);
    fill_vector(&v3, number2++);
    fill_vector(&v4, number2++);

    fill_vector(&v01, number2++);
    fill_vector(&v02, number2++);
    fill_vector(&v03, number2++);
    fill_vector(&v04, number2++);

    memory_list_copy(return_value, (n_byte*)&v1, sizeof (n_vect2));
    memory_list_copy(return_value, (n_byte*)&v2, sizeof (n_vect2));
    memory_list_copy(return_value, (n_byte*)&v3, sizeof (n_vect2));
    memory_list_copy(return_value, (n_byte*)&v4, sizeof (n_vect2));

    memory_list_copy(return_value, (n_byte*)&v01, sizeof (n_vect2));
    memory_list_copy(return_value, (n_byte*)&v02, sizeof (n_vect2));
    memory_list_copy(return_value, (n_byte*)&v03, sizeof (n_vect2));
    memory_list_copy(return_value, (n_byte*)&v04, sizeof (n_vect2));

    return return_value;
}

void memory_list_test(void)
{
    memory_list *value1 = {0};
    memory_list *value2 = {0};
    memory_list *value3 = {0};

    printf("----- memory_list start ----- ----- -----\n");


    value1 = quad_vector(1);
    value2 = quad_vector(7);
    value3 = quad_vector(10);

    memory_list_debug(value1);
    sum_vector(value1);
    memory_list_debug(value2);
    sum_vector(value2);
    memory_list_debug(value3);
    sum_vector(value3);
    printf("----- memory_list end   ----- ----- -----\n");

    memory_list_free(&value1);
    memory_list_free(&value2);
    memory_list_free(&value3);
}

int main( int argc, const char *argv[] )
{
    memory_list_test();
    exit (EXIT_SUCCESS);
}
