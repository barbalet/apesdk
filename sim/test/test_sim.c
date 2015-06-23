/****************************************************************

 test_sim.c

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

/* this doesn't currently work, it is included here for unit
    testing in the future */

#include <stdio.h>
#include <time.h>

#include "../noble/noble.h"
#include "../entity/entity.h"
#include "../universe/universe.h"

n_int draw_error(n_constant_string error_text, n_constant_string location, n_int line_number)
{
    printf("ERROR: %s @ %s %ld\n",(const n_string) error_text, location, line_number);
    return -1;
}

int main(int argc, const char * argv[])
{
    n_int   counter = 0;

    sim_init(2, 0x12738291, MAP_AREA, 0);
    
    {
        noble_simulation * local_sim = sim_sim();
        noble_being      * first_being = &(local_sim->beings[0]);
        noble_being      * second_being = &(local_sim->beings[1]);
        noble_being_constant * first_being_constant = &(first_being->constant);
        noble_being_delta * first_being_delta = &(first_being->delta);
    
        n_uint being_hash = math_hash((n_byte *)first_being, sizeof(noble_being));
        n_uint being_constant_hash = math_hash((n_byte *)first_being_constant, sizeof(noble_being_constant));
        n_uint being_delta_hash = math_hash((n_byte *)first_being_delta, sizeof(noble_being_delta));
        
        if (being_hash != 0x469b11cf427b0816)
        {
            printf("being hash %lx\n", being_hash);
        }
        
        if (being_constant_hash != 0xacbebd3b4ec84e3e)
        {
            printf("being constant %lx\n", being_constant_hash);
        }
        if (being_delta_hash != 0x7e4f9731742fa5d4)
        {
            printf("being delta %lx\n", being_delta_hash);
        }
        
        while (counter < 1000)
        {
            sim_cycle();
            counter ++;
        }
 
        being_hash = math_hash((n_byte *)first_being, sizeof(noble_being));
        being_constant_hash = math_hash((n_byte *)first_being_constant, sizeof(noble_being_constant));
        being_delta_hash = math_hash((n_byte *)first_being_delta, sizeof(noble_being_delta));
        
        if (being_hash != 0x91084c7f4d3fb1ad)
        {
            printf("being hash %lx\n", being_hash);
        }
        
        if (being_constant_hash != 0xacbebd3b4ec84e3e)
        {
            printf("being constant %lx\n", being_constant_hash);
        }
        if (being_delta_hash != 0x31916ae7c6bf83b7)
        {
            printf("being delta %lx\n", being_delta_hash);
        }
    }
    sim_close();
    
    return 1;
}

