/****************************************************************

 test.c

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
//
//  MacSimulation_Tests.m
//  MacSimulation Tests
//
//  Created by Thomas Barbalet on 7/3/13.
//  Copyright (c) 2013 Thomas Barbalet. All rights reserved.
//

#include "noble/noble.h"
#include "entity/entity.h"
#include "universe/universe.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

n_land      * environment;
noble_being * test;

n_int draw_error(n_constant_string error_text)
{
    printf("ERROR: %s\n", error_text);
    return -1;
}

void setUp(void)
{    
    environment = (n_land *)io_new(sizeof(n_land));
    environment->map = io_new(512*512);
    n_byte * swap = io_new(512*512);

    environment->genetics[0] = 0x6273;
    environment->genetics[1] = 0x9283;

    land_clear(environment, KIND_START_UP, 0);
    land_init(environment , swap);

    io_free(swap);

    test = (noble_being *)io_new(sizeof(noble_being));

    test->brain = (n_byte *) io_new(DOUBLE_BRAIN);
    test->episodic = (episodic_memory *)io_new(EPISODIC_SIZE * sizeof(episodic_memory));
    test->social = (social_link *)io_new(SOCIAL_SIZE * sizeof(social_link));

    n_byte2  local_random[2]={0x2637, 0x8ad3};

    being_init(environment, 0L, 0, test, 0L, local_random);
}

void tearDown(void)
{
    io_free(test->social);
    io_free(test->episodic);
    io_free(test->brain);

    io_free(test);

    io_free(environment->map);

    io_free(environment);
}

void initialTest(void)
{

    NA_ASSERT(test->social, "social is not created");
    NA_ASSERT(test->episodic, "episodic is not created");
    NA_ASSERT(test->brain, "brain is not created");

    NA_ASSERT(test, "being is not created");

    NA_ASSERT(environment->map, "map is not created");
    
    NA_ASSERT(environment, "land is not created");
}

int main(void)
{
    setUp();
    
    initialTest();
    
    tearDown();
    
    return 1;
}