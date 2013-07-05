//
//  MacSimulation_Tests.m
//  MacSimulation Tests
//
//  Created by Thomas Barbalet on 7/4/13.
//  Copyright (c) 2013 Thomas Barbalet. All rights reserved.
//

#import <XCTest/XCTest.h>

#include "../sim/noble/noble.h"
#include "../sim/entity/entity.h"
#include "../sim/universe/universe.h"
#include "../sim/gui/shared.h"


@interface MacSimulation_Tests : XCTestCase

@end

@implementation MacSimulation_Tests

noble_simulation * value = 0L;
noble_being      * being = 0L;

- (void)setUp
{
    [super setUp];
    
    shared_new(0x1382);

    value = sim_sim();
    
    being = &(value->beings[2]);
}

- (void)tearDown
{
    // Tear-down code here.
    
    [super tearDown];
}

- (void)testCheckInitialTime
{
    XCTAssertTrue(value->land->time == 0, @"Time is not initially zero");
}

- (void)testCheckInitialDate
{
    XCTAssertTrue(value->land->date[0] == AGE_OF_MATURITY, @"Days is not initially AGE_OF_MATURITY (%ld)", AGE_OF_MATURITY);
    XCTAssertTrue(value->land->date[1] == 0, @"Centuries is not initially 0");
}

- (void)testMapCheckSum
{
    n_uint hash = math_hash((n_byte *)value->land->map, MAP_AREA);    
    XCTAssertTrue(hash == 0xdd4de0fa74ffc961, @"Hash doesn't comply with prior map hashes (%lx)", hash);
}

- (void)testWeatherCheckSum
{
    n_uint hash = math_hash((n_byte *)value->weather->atmosphere, (MAP_AREA*sizeof(n_c_int)/2));    
    XCTAssertTrue(hash == 0x8232846f8b53c59d, @"Hash doesn't comply with prior weather hashes (%lx)", hash);
}

- (void)testBeingCheckSum
{
    n_uint hash = math_hash((n_byte *)being, sizeof(noble_being) - sizeof(n_byte *) - sizeof(social_link *) - sizeof(episodic_memory *));
    XCTAssertTrue(hash == 0x9a224c0789da96d5, @"Hash doesn't comply with prior being hashes (%lx)", hash);
}

- (void)testBrainCheckSum
{
    n_uint hash = math_hash(being->brain, DOUBLE_BRAIN);
    XCTAssertTrue(hash == 0xca440ed899ee9512, @"Hash doesn't comply with prior brain hashes (%lx)", hash);
}

- (void)testSocialCheckSum
{
    n_uint hash = math_hash((n_byte *)being->social, SOCIAL_SIZE * sizeof(social_link));
    XCTAssertTrue(hash == 0x9641d9201ee7241, @"Hash doesn't comply with prior social hashes (%lx)", hash);
}

- (void)testEpisodicCheckSum
{
    n_uint hash = math_hash((n_byte *)being->episodic, EPISODIC_SIZE * sizeof(episodic_memory));
    XCTAssertTrue(hash == 0xcc7da6d33d1f14ed, @"Hash doesn't comply with prior episodic hashes (%lx)", hash);
}


- (void)actionMouseClick
{
    n_int   map_x = APESPACE_TO_MAPSPACE(being->location[0]);
    n_int   map_y = APESPACE_TO_MAPSPACE(being->location[1]);

    shared_mouseReceived(map_x, map_y, NUM_VIEW);
}

- (void)testMouseClick
{
    XCTAssertTrue(value->select == 0, @"Selected being is non-zero");
    [self actionMouseClick];
    shared_cycle(1000, NUM_VIEW);
    shared_cycle(1000, NUM_TERRAIN);
    shared_mouseUp();
    shared_cycle(1000, NUM_VIEW);
    shared_cycle(1000, NUM_TERRAIN);
    XCTAssertTrue(value->select == 2, @"Selected being is found");
}

- (void)testMouseDrag
{
    XCTAssertTrue(being->location[0] == 10915, @"X value is wrong (%ld)", being->location[0]);
    XCTAssertTrue(being->location[1] == 20105, @"Y value is wrong (%ld)", being->location[1]);
    
    [self actionMouseClick];
    
    shared_cycle(1000, NUM_VIEW);
    shared_cycle(1000, NUM_TERRAIN);
    
    shared_mouseReceived(100, 100, NUM_VIEW);
    shared_mouseOption(1);
    
    shared_cycle(1000, NUM_VIEW);
    shared_cycle(1000, NUM_TERRAIN);
    
    shared_mouseOption(0);
    shared_mouseUp();
    
    shared_cycle(1000, NUM_VIEW);
    shared_cycle(1000, NUM_TERRAIN);
    
    NSLog(@"Location %d %d", being->location[0], being->location[1]);
    
    XCTAssertTrue(being->location[0] == 6396, @"X value is wrong (%ld)", being->location[0]);
    XCTAssertTrue(being->location[1] == 6394, @"Y value is wrong (%ld)", being->location[1]);
}

- (void)testLongtermSimulation
{
//    n_uint  time = 1;
//    n_uint hash;
//    
//    shared_flood();
//
//    while (time < ((TIME_DAY_MINUTES * 3)/2))
//    {
//        /** This will reduce the number of simulated entities, reducing total simulation time */
//        if (time == TIME_DAY_MINUTES)
//        {
//            shared_flood();
//        }
//        shared_simulate(time);
//        time++;
//    }
//
//    hash = math_hash((n_byte *)being, sizeof(noble_being) - sizeof(n_byte *) - sizeof(social_link *) - sizeof(episodic_memory *));
//    XCTAssertTrue(hash == 0xfe78f396ad6a1d9d, @"Longterm hash doesn't comply with prior being hashes (%lx)", hash);
//
//    hash = math_hash(being->brain, DOUBLE_BRAIN);
//    XCTAssertTrue(hash == 0xd16cb871361101ab, @"Longterm hash doesn't comply with prior brain hashes (%lx)", hash);
//    
//    hash = math_hash((n_byte *)being->social, SOCIAL_SIZE * sizeof(social_link));
//    XCTAssertTrue(hash == 0x4a5dd5e3b9e3fbed, @"Longterm hash doesn't comply with prior social hashes (%lx)", hash);
//
//    hash = math_hash((n_byte *)being->episodic, EPISODIC_SIZE * sizeof(episodic_memory));
//    XCTAssertTrue(hash == 0xb980d6197abe269f, @"Longterm hash doesn't comply with prior episodic hashes (%lx)", hash);
//    
//    XCTAssertTrue(value->num == 3, @"Longterm number apes doesn't comply with prior numbers (%ld)", value->num);
}

@end
