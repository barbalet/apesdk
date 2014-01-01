/****************************************************************
 
 test_math.c
 
 =============================================================
 
 Copyright 1996-2014 Tom Barbalet. All rights reserved.
 
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


n_int draw_error(n_constant_string error_text)
{
    if (error_text)
    {
        printf("ERROR: %s\n", error_text);
    }
    return -1;
}

n_int check_root(n_uint value, n_uint squared)
{
    n_uint result = math_root(squared);
    if (result != value)
    {
        printf("squared %ld expects value %ld, instead %ld\n", squared, value, result);
        return -1;
    }
    return 0;
}

void check_math(void)
{
    n_int   loop = 0;
    n_int   sum = 0;
    n_vect2 sum_vect = {0};
    
    (void)check_root(1234, 1522756);
    (void)check_root(4, 17);
    (void)check_root(4, 16);
    (void)check_root(3, 15);
    (void)check_root(3, 14);
    
    while (loop < 256)
    {
        n_vect2 each_vect;
        sum += math_sine(loop, 1);
        vect2_direction(&each_vect, loop, 1);
        vect2_d(&sum_vect, &each_vect, 1, 1);
        loop++;
    }
    
    if (sum != 0)
    {
        printf("scalar sum expecting 0 instead %ld\n", sum);
    }
    
    if (sum_vect.x != 0 || sum_vect.y != 0)
    {
        printf("vect sum expecting 0, 0 instead %ld, %ld\n", sum_vect.x, sum_vect.y);
    }
}

int main(int argc, const char * argv[])
{
    check_math();
    return 0;
}

