/****************************************************************
 
 Noble_ApeTests.m
 
 =============================================================
 
 Copyright 1996-2019 Tom Barbalet. All rights reserved.
 
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

#import <XCTest/XCTest.h>

#include "noble.h"
#include "entity.h"
#include "universe.h"
#include "shared.h"

@interface Noble_ApeTests : XCTestCase

@end

@implementation Noble_ApeTests

noble_simulation * value = 0L;
noble_being      * being = 0L;

- (void)setUp
{
    [super setUp];
    shared_new(12345);
}

- (void)tearDown
{
    /*shared_close();*/
    // Tear-down code here.
    [super tearDown];
}
/*
- (void) testTime
{
    n_byte4 time_value = land_time();
    XCTAssertTrue(land_time() == 2, @"Time is %u", time_value);
}
*/
- (void)testCheckInitialDate
{
    XCTAssertTrue(land_date() == AGE_OF_MATURITY, @"Days is not initially AGE_OF_MATURITY (%ld)", (n_int)AGE_OF_MATURITY);
}

- (void)testMapCheckSum
{
    n_uint hash = math_hash((n_byte *)land_topography(), MAP_AREA);
    XCTAssertTrue(hash == 0x4c2cc85376bf6e3e, @"Hash doesn't comply with prior map hashes (%lx)", hash);
}
/*
- (void)testBeingCheckSum
{
    
    n_int length = sizeof(noble_being) - sizeof(n_byte *) - sizeof(noble_social *) - sizeof(noble_episodic *);
    n_uint hash;
    
    value = sim_sim();
    
    being = &(value->beings[2]);
    
    hash = math_hash((n_byte *)being, length);
    XCTAssertTrue(length == 70248, @"Length doesn't comply with prior length (%ld)", length);
    
    XCTAssertTrue(hash == 0x25d693ac3c13b59d, @"Hash doesn't comply with prior being hashes (%lx)", hash);
}
*/

- (void)testBrainCheckSum
{
    n_uint hash;
    
    value = sim_sim();
    
    being = &(value->beings[2]);
    
    hash = math_hash(being->braindata.brain, DOUBLE_BRAIN);
    XCTAssertTrue(hash == 0xecbf7d37c888ada0, @"Hash doesn't comply with prior brain hashes (%lx)", hash);
}

- (void)testSocialCheckSum
{
    n_uint hash;
    value = sim_sim();
    
    being = &(value->beings[2]);
    hash = math_hash((n_byte *)being->events.social, SOCIAL_SIZE * sizeof(noble_social));
    XCTAssertTrue(hash == 0x7f0aee9728e122b6, @"Hash doesn't comply with prior social hashes (%lx)", hash);
}

- (void)testEpisodicCheckSum
{
    n_uint hash;
    
    value = sim_sim();
    
    being = &(value->beings[2]);
    
    hash = math_hash((n_byte *)being->events.episodic, EPISODIC_SIZE * sizeof(noble_episodic));
    XCTAssertTrue(hash == 0xe9e6c3c12d7a37ce, @"Hash doesn't comply with prior episodic hashes (%lx)", hash);
}


- (void)actionMouseClick
{
    n_int   map_x, map_y;
    
    value = sim_sim();
    
    being = &(value->beings[2]);
    
    map_x = APESPACE_TO_MAPSPACE(being->delta.location[0]);
    map_y = APESPACE_TO_MAPSPACE(being->delta.location[1]);
    
    shared_mouseReceived(map_x, map_y, NUM_VIEW);
}

- (void)testMouseClick
{
    [self actionMouseClick];
    shared_mouseUp();

    shared_cycle(1000, NUM_VIEW);
    shared_cycle(1001, NUM_TERRAIN);
    shared_cycle(1002, NUM_VIEW);
    shared_cycle(1003, NUM_TERRAIN);
    /*
    value = sim_sim();
    
    being = &(value->beings[2]);
    */
    XCTAssertTrue(value->select == being, @"Selected being is found");
}

- (void)testMouseDrag
{
    [self actionMouseClick];
    
    shared_mouseReceived(100, 100, NUM_VIEW);
    shared_mouseOption(1);

    shared_mouseOption(0);
    shared_mouseUp();

    value = sim_sim();
    
    being = &(value->beings[2]);
    
    NSLog(@"Location %d %d", being->delta.location[0], being->delta.location[1]);
    
    XCTAssertTrue(being->delta.location[0] == 7131, @"X value is wrong (%d)", being->delta.location[0]);
    XCTAssertTrue(being->delta.location[1] == 13627, @"Y value is wrong (%d)", being->delta.location[1]);
}


- (void)testShorttermSimulationCompare
{
    n_int  time = 3;
    n_uint  hash;
    
    value = sim_sim();
    
    being = &(value->beings[2]);
    
    hash = math_hash((n_byte *)&being->constant, 60);
    
    XCTAssertTrue(hash == 0xe55d6cfddcc974c8, @"Starting hash doesn't comply with prior being hashes (%lx)", hash);
    
    shared_cycle(2, NUM_VIEW);
    shared_cycle(2, NUM_TERRAIN);
    
    value = sim_sim();
    
    being = &(value->beings[2]);
    
    hash = math_hash((n_byte *)&being->constant, 60);
    XCTAssertTrue(hash == 0x4dab5dbc442674b4, @"Short-term hash doesn't comply with prior being hashes (%lx)", hash);
    
    while (time < 1023)
    {
        shared_cycle(2+time, NUM_VIEW);
        shared_cycle(2+time, NUM_TERRAIN);
        time++;
    }
    
    value = sim_sim();
    
    being = &(value->beings[2]);
    
    hash = math_hash((n_byte *)&being->constant, 60);
    XCTAssertTrue(hash == 0x4dab5dbc442674b4, @"Short-term hash doesn't comply with prior being hashes (%lx)", hash);
}


- (void)testLongtermSimulationCompare
{

    n_uint  hash;
    
    value = sim_sim();
    
    being = &(value->beings[2]);
    hash = math_hash((n_byte *)being, sizeof(noble_being));
    
    XCTAssertTrue(hash == 0x4339aa2ef072a1d5, @"Starting hash doesn't comply with prior being hashes (%lx)", hash);
    
    shared_cycle(2, NUM_VIEW);
    shared_cycle(2, NUM_TERRAIN);

    
    value = sim_sim();
    
    being = &(value->beings[2]);
    hash = math_hash((n_byte *)being, 60);
    XCTAssertTrue(hash == 0xfd9f28ee82fa76bc, @"Longterm hash doesn't comply with prior being hashes (%lx)", hash);

    shared_cycle(998, NUM_VIEW);
    shared_cycle(998, NUM_TERRAIN);

    value = sim_sim();
    
    being = &(value->beings[2]);
    hash = math_hash((n_byte *)being, 60);
    XCTAssertTrue(hash == 0xfd9f28ee82fa76bc, @"Longterm hash doesn't comply with prior being hashes (%lx)", hash);
    
    hash = math_hash((n_byte *)being, 30228);
    
    XCTAssertTrue(hash == 0xe0db608ca1e3f505, @"Longterm hash doesn't comply with prior being hashes (%lx)", hash);
}

@end
