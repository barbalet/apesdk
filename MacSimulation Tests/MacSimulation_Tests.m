//
//  MacSimulation_Tests.m
//  MacSimulation Tests
//
//  Created by Thomas Barbalet on 7/4/13.
//  Copyright (c) 2014 Thomas Barbalet. All rights reserved.
//

#import <XCTest/XCTest.h>

#include "../sim/noble/noble.h"
#include "../sim/entity/entity.h"
#include "../sim/universe/universe.h"
#include "../sim/noble/shared.h"


@interface MacSimulation_Tests : XCTestCase

@end

@implementation MacSimulation_Tests

noble_simulation * value = 0L;
noble_being      * being = 0L;

- (void)setUp
{
    [super setUp];
    
    (void)shared_new(0x1382);

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
    XCTAssertTrue(value->land->date[0] == AGE_OF_MATURITY, @"Days is not initially AGE_OF_MATURITY (%ld)", (n_int)AGE_OF_MATURITY);
    XCTAssertTrue(value->land->date[1] == 0, @"Centuries is not initially 0");
}

- (void)testMapCheckSum
{
    n_uint hash = math_hash((n_byte *)value->land->topology, MAP_AREA);
    XCTAssertTrue(hash == 0xdd4de0fa74ffc961, @"Hash doesn't comply with prior map hashes (%lx)", hash);
}

- (void)testWeatherCheckSum
{
    n_uint hash = math_hash((n_byte *)value->land->atmosphere, (MAP_AREA*sizeof(n_c_int)/2));
    XCTAssertTrue(hash == 0x6da27da42367d12f, @"Hash doesn't comply with prior weather hashes (%lx)", hash);
}

- (void)testBeingCheckSum
{

    n_int length = sizeof(noble_being) - sizeof(n_byte *) - sizeof(noble_social *) - sizeof(noble_episodic *);
    n_uint hash = math_hash((n_byte *)being, length);
    XCTAssertTrue(length == 70296, @"Length doesn't comply with prior length (%ld)", length);
    
    XCTAssertTrue(hash == 0x6ec8ca3c978c691a, @"Hash doesn't comply with prior being hashes (%lx)", hash);
}

- (void)testBrainCheckSum
{
    n_uint hash = math_hash(being->brain, DOUBLE_BRAIN);
    XCTAssertTrue(hash == 0xca440ed899ee9512, @"Hash doesn't comply with prior brain hashes (%lx)", hash);
}

- (void)testSocialCheckSum
{
    n_uint hash = math_hash((n_byte *)being->social, SOCIAL_SIZE * sizeof(noble_social));
    XCTAssertTrue(hash == 0x2e003149cb028b53, @"Hash doesn't comply with prior social hashes (%lx)", hash);
}

- (void)testEpisodicCheckSum
{
    n_uint hash = math_hash((n_byte *)being->episodic, EPISODIC_SIZE * sizeof(noble_episodic));
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
    [self actionMouseClick];
    shared_cycle(1000, NUM_VIEW, 512, 512);
    shared_cycle(1000, NUM_TERRAIN, 512, 512);
    shared_mouseUp();
    shared_cycle(1000, NUM_VIEW, 512, 512);
    shared_cycle(1000, NUM_TERRAIN, 512, 512);
    XCTAssertTrue(value->select == being, @"Selected being is found");
}

- (void)testMouseDrag
{
    XCTAssertTrue(being->location[0] == 24160, @"X value is wrong (%d)", being->location[0]);
    XCTAssertTrue(being->location[1] == 3415, @"Y value is wrong (%d)", being->location[1]);
    
    [self actionMouseClick];
    shared_cycle(1000, NUM_VIEW, 512, 512);
    shared_cycle(1000, NUM_TERRAIN, 512, 512);
    
    shared_mouseReceived(100, 100, NUM_VIEW);
    shared_mouseOption(1);
    
    shared_cycle(1000, NUM_VIEW, 512, 512);
    shared_cycle(1000, NUM_TERRAIN, 512, 512);
    shared_mouseOption(0);
    shared_mouseUp();
    
    shared_cycle(1000, NUM_VIEW, 512, 512);
    shared_cycle(1000, NUM_TERRAIN, 512, 512);
    NSLog(@"Location %d %d", being->location[0], being->location[1]);
    
    XCTAssertTrue(being->location[0] == 6394, @"X value is wrong (%d)", being->location[0]);
    XCTAssertTrue(being->location[1] == 6398, @"Y value is wrong (%d)", being->location[1]);
}

- (void)testLongtermSimulationCompare
{
    n_uint  time = 3;
    n_uint hash = math_hash((n_byte *)being, sizeof(noble_being));
    XCTAssertTrue(hash == 0x52d78f67b881df87, @"Starting hash doesn't comply with prior being hashes (%lx)", hash);
        
    shared_cycle(1000, NUM_VIEW, 512, 512);
    shared_cycle(1000, NUM_TERRAIN, 512, 512);
    
    hash = math_hash((n_byte *)being, sizeof(noble_being));
    XCTAssertTrue(hash == 0xbcd10c1d12434be, @"Longterm hash doesn't comply with prior being hashes (%lx)", hash);
    
    while (time < ((TIME_DAY_MINUTES * 1)/2))
    {
        shared_cycle(1000 * time, NUM_VIEW, 512, 512);
        shared_cycle(1000 * time, NUM_TERRAIN, 512, 512);
        time++;
    }
    
    hash = math_hash((n_byte *)being, sizeof(noble_being));
    XCTAssertTrue(hash == 0x6eab4ce50ade4e10, @"Longterm hash doesn't comply with prior being hashes (%lx)", hash);
    
    hash = math_hash(being->brain, DOUBLE_BRAIN);
    XCTAssertTrue(hash == 0xe85788cb36856bb1, @"Longterm hash doesn't comply with prior brain hashes (%lx)", hash);
    
    hash = math_hash((n_byte *)being->social, SOCIAL_SIZE * sizeof(noble_social));
    XCTAssertTrue(hash == 0x94350bd7a489387f, @"Longterm hash doesn't comply with prior social hashes (%lx)", hash);
    
    hash = math_hash((n_byte *)being->episodic, EPISODIC_SIZE * sizeof(noble_episodic));
    XCTAssertTrue(hash == 0x078f54f2a019f71e, @"Longterm hash doesn't comply with prior episodic hashes (%lx)", hash);
}

- (void)testLongtermSimulation
{
    n_uint  time = 720;
    n_uint  hash;
    
    [self testLongtermSimulationCompare];
    
    while (time < ((TIME_DAY_MINUTES * 3)/2))
    {
        shared_cycle(1000 * time, NUM_VIEW, 512, 512);
        shared_cycle(1000 * time, NUM_TERRAIN, 512, 512);
        time++;
    }
    
    hash = math_hash((n_byte *)being->brain, DOUBLE_BRAIN);
    XCTAssertTrue(hash == 0xf393bd67f603e03f, @"Longterm hash doesn't comply with prior brain hashes (%lx)", hash);
    
    hash = math_hash((n_byte *)being->social, SOCIAL_SIZE * sizeof(noble_social));
    XCTAssertTrue(hash == 0x82d10806734d4a2a, @"Longterm hash doesn't comply with prior social hashes (%lx)", hash);
    
    hash = math_hash((n_byte *)being->episodic, EPISODIC_SIZE * sizeof(noble_episodic));
    XCTAssertTrue(hash == 0x598365b3e749f462, @"Longterm hash doesn't comply with prior episodic hashes (%lx)", hash);
    
    XCTAssertTrue(value->num == 59, @"Longterm number apes doesn't comply with prior numbers (%ld)", value->num);
    /*
     hash = math_hash((n_byte *)value->land->atmosphere, (MAP_AREA*sizeof(n_c_int)/2));
     XCTAssertTrue(hash == 0x106da9dae6533ea1, @"Longterm hash doesn't comply with prior weather hashes (%lx)", hash);
     */
    hash = math_hash((n_byte *)value->land->topology, MAP_AREA);
    XCTAssertTrue(hash == 0xdd4de0fa74ffc961, @"Longterm hash doesn't comply with prior map hashes (%lx)", hash);
}


@end
