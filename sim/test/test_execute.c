/****************************************************************
 
 test_execute.c
 
 =============================================================
 
 Copyright 1996-2013 Tom Barbalet. All rights reserved.
 
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

/* this doesn't currently work, it is included here for unit 
    testing in the future */

#include <stdio.h>

#include "../noble/noble.h"

static n_int               conclude_count = 0;

n_int draw_error(n_constant_string error_text)
{
    if (error_text)
    {
        printf("ERROR: %s\n", error_text);
    }
    return -1;
}

n_int calculate_c(void * read_data, void * write_data)
{
    n_int * read_int = read_data;
    n_int * write_int = write_data;
    
    write_int[0] ^= read_int[0];
    return 0;
}


n_int calculate_b(void * read_data, void * write_data)
{
    n_int * read_int = read_data;
    n_int * write_int = write_data;
    n_int loop = 0;
    
    printf("Calculate B!\n");
    
    while (loop < 10500)
    {
        n_int location = loop % 5;
        write_int[location] += read_int[location];
        loop++;
    }
    
    execute_add(calculate_c, &read_int[1], &write_int[0]);
    execute_add(calculate_c, &read_int[2], &write_int[1]);
    execute_add(calculate_c, &read_int[3], &write_int[2]);
    execute_add(calculate_c, &read_int[4], &write_int[3]);
    return 0;
}


n_int calculate_a(void * read_data, void * write_data)
{
    n_int * read_int = read_data;
    n_int * write_int = write_data;
    n_int loop = 0;
    
    printf("Calculate A!\n");

    while (loop < 10500)
    {
        n_int location = loop % 20;
        write_int[location] += read_int[location];
        loop++;
    }
    
    execute_add(calculate_b, &read_int[5], &write_int[0]);
    execute_add(calculate_b, &read_int[10], &write_int[5]);
    execute_add(calculate_b, &read_int[15], &write_int[10]);
    execute_add(calculate_b, &read_int[0], &write_int[15]);

    
    conclude_count++;
    
    if (conclude_count == 10)
        return -1;
        
    return 0;
}

static n_int start_cycle_again(void)
{
    n_int     read_data[20] = {1,2,3,4,5,6,7,8,9,10,11,12}, write_data[20] = {0};
    
    printf("Cycle again!\n");
    
    execute_add(calculate_a, read_data, write_data);
    return 0;
}



int main(int argc, const char * argv[])
{
    execute_main_loop(start_cycle_again, start_cycle_again);
    return 0;
}

