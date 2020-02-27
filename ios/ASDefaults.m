/****************************************************************

 ASDefaults.m

 =============================================================

 Copyright 1996-2020 Tom Barbalet. All rights reserved.

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

#include "../toolkit/toolkit.h"
#include "../script/script.h"
#include "../sim/sim.h"
#include "../universe/universe.h"
#include "../toolkit/shared.h"

#import "ASDefaults.h"

// ===================================

@implementation ASDefaults

- (NSDictionary*) simulatedBeingDictionary:(simulated_being*)being
{
    NSMutableDictionary * mutableBeing = [NSMutableDictionary dictionary];
    NSData * being_delta = [NSData dataWithBytesNoCopy:(void *)&(being->delta) length:sizeof(simulated_being_delta) freeWhenDone:NO];
    NSData * being_contant = [NSData dataWithBytesNoCopy:(void *)&(being->constant) length:sizeof(simulated_being_constant) freeWhenDone:NO];
    NSData * being_events = [NSData dataWithBytesNoCopy:(void *)&(being->events) length:sizeof(simulated_being_events) freeWhenDone:NO];
    NSData * being_braindata = [NSData dataWithBytesNoCopy:(void *)&(being->braindata) length:sizeof(simulated_being_brain) freeWhenDone:NO];
    NSData * being_changes = [NSData dataWithBytesNoCopy:(void *)&(being->changes) length:sizeof(simulated_being_volatile) freeWhenDone:NO];
#ifdef IMMUNE_ON
    NSData * being_immunesystem = [NSData dataWithBytesNoCopy:(void *)&(being->immune_system) length:sizeof(simulated_immune_system) freeWhenDone:NO];
#endif
    [mutableBeing setObject:being_delta forKey:@"delta"];
    [mutableBeing setObject:being_contant forKey:@"constant"];
    [mutableBeing setObject:being_events forKey:@"events"];
    [mutableBeing setObject:being_braindata forKey:@"braindata"];
    [mutableBeing setObject:being_changes forKey:@"changes"];
#ifdef IMMUNE_ON
    [mutableBeing setObject:being_immunesystem forKey:@"immune"];
#endif
    return mutableBeing;
}

- (NSDictionary*) simulatedFileDefinition
{
    NSMutableDictionary * mutableDefinition = [NSMutableDictionary dictionary];
    [mutableDefinition setObject:@(sizeof(simulated_being_delta)) forKey:@"delta_size"];
    [mutableDefinition setObject:@(sizeof(simulated_being_constant)) forKey:@"constant_size"];
    [mutableDefinition setObject:@(sizeof(simulated_being_events)) forKey:@"events_size"];
    [mutableDefinition setObject:@(sizeof(simulated_being_brain)) forKey:@"braindata_size"];
    [mutableDefinition setObject:@(sizeof(simulated_being_volatile)) forKey:@"changes_size"];
#ifdef IMMUNE_ON
    [mutableDefinition setObject:@(sizeof(simulated_immune_system)) forKey:@"immune_size"];
#endif
    [mutableDefinition setObject:@(sizeof(n_land)) forKey:@"land_size"];
    [mutableDefinition setObject:@(sizeof(simulated_remains)) forKey:@"remains_size"];
    
    [mutableDefinition setObject:@(VERSION_NUMBER) forKey:@"version"];
    [mutableDefinition setObject:@(SIMULATED_APE_SIGNATURE) forKey:@"signature"];
    return mutableDefinition;
}

- (NSDictionary*) apeSimulationDictionary
{
    simulated_group * group = sim_group();
    simulated_timing * timing = sim_timing();

    NSMutableDictionary * mutableSimulation = [NSMutableDictionary dictionary];
    NSMutableArray * mutableBeings = [NSMutableArray array];
    n_uint loop = 0;
    NSData * simulation_land = [NSData dataWithBytesNoCopy:(void *)land_ptr() length:sizeof(n_land) freeWhenDone:NO];
    NSData * simulation_remains = [NSData dataWithBytesNoCopy:(void *)group->remains length:sizeof(simulated_remains) freeWhenDone:NO];
    
    [mutableSimulation setObject:simulation_land forKey:@"land"];
    [mutableSimulation setObject:simulation_remains forKey:@"remains"];
    [mutableSimulation setObject:@(-1) forKey:@"selected"];

    while(loop < group->num)
    {
        simulated_being * current_being = &(group->beings[loop]);
        [mutableBeings addObject:[self simulatedBeingDictionary:current_being]];
        if (current_being == group->select)
        {
            [mutableSimulation setObject:@(loop) forKey:@"selected"];
        }
        loop++;
    }
    [mutableSimulation setObject:mutableBeings forKey:@"beings"];

    [mutableSimulation setObject:@(group->num) forKey:@"num"];
    [mutableSimulation setObject:@(group->max) forKey:@"max"];
    
    [mutableSimulation setObject:@(timing->real_time) forKey:@"real_time"];
    [mutableSimulation setObject:@(timing->last_time) forKey:@"last_time"];

    [mutableSimulation setObject:@(timing->delta_cycles) forKey:@"delta_cycles"];
    [mutableSimulation setObject:@(timing->count_cycles) forKey:@"count_cycles"];
    
    [mutableSimulation setObject:@(timing->delta_frames) forKey:@"delta_frames"];
    [mutableSimulation setObject:@(timing->count_frames) forKey:@"count_frames"];

    [mutableSimulation setObject:[self simulatedFileDefinition] forKey:@"definition"];
    
    return mutableSimulation;
}


- (void) saveSimulation;
{
    NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
    if (standardUserDefaults) {
        [standardUserDefaults setValuesForKeysWithDictionary:[self apeSimulationDictionary]];
        [standardUserDefaults synchronize];
    }
}

- (void) loadSimulation;
{
    
}

- (void) clearDefaults
{
    
}

@end
