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
        noble_being_events * first_being_events = &(first_being->events);
        noble_being_brain * first_being_brain = &(first_being->braindata);
        noble_immune_system * first_being_immune = &(first_being->immune_system);
        noble_being_volatile * first_being_volatile = &(first_being->wrong);
        
        n_uint being_hash = math_hash((n_byte *)first_being, sizeof(noble_being));
        n_uint being_constant_hash = math_hash((n_byte *)first_being_constant, sizeof(noble_being_constant));
        n_uint being_delta_hash = math_hash((n_byte *)first_being_delta, sizeof(noble_being_delta));
        n_uint being_events_hash = math_hash((n_byte *)first_being_events, sizeof(noble_being_events));
        n_uint being_brain_hash = math_hash((n_byte *)first_being_brain, sizeof(noble_being_brain));
        n_uint being_immune_hash = math_hash((n_byte*)first_being_immune, sizeof(noble_immune_system));
        n_uint being_volatile_hash = math_hash((n_byte*)first_being_volatile, sizeof(noble_being_volatile));

        printf("(1)being hash %lx\n", being_hash);
        
        printf("(1)being constant %lx\n", being_constant_hash);

        printf("(1)being delta %lx\n", being_delta_hash);

        printf("(1)being events %lx\n", being_events_hash);

        printf("(1)being brain %lx\n", being_brain_hash);
        
        printf("(1)being immune %lx\n", being_immune_hash);
        printf("(1)being volatile %lx\n", being_volatile_hash);

        while (counter < 1000)
        {
            sim_cycle();
            counter ++;
        }
 
        being_hash = math_hash((n_byte *)first_being, sizeof(noble_being));
        being_constant_hash = math_hash((n_byte *)first_being_constant, sizeof(noble_being_constant));
        being_delta_hash = math_hash((n_byte *)first_being_delta, sizeof(noble_being_delta));
        
        being_events_hash = math_hash((n_byte *)first_being_events, sizeof(noble_being_events));
        being_brain_hash = math_hash((n_byte *)first_being_brain, sizeof(noble_being_brain));
        being_immune_hash = math_hash((n_byte*)first_being_immune, sizeof(noble_immune_system));
        being_volatile_hash = math_hash((n_byte*)first_being_volatile, sizeof(noble_being_volatile));

        printf("(2)being hash %lx\n", being_hash);
        
        printf("(2)being constant %lx\n", being_constant_hash);
        
        printf("(2)being delta %lx\n", being_delta_hash);
        
        printf("(2)being events %lx\n", being_events_hash);
        
        printf("(2)being brain %lx\n", being_brain_hash);
        printf("(2)being immune %lx\n", being_immune_hash);
        printf("(2)being volatile %lx\n", being_volatile_hash);

    }
    sim_close();
    
    return 1;
}

