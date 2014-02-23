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
#include "../sim/gui/shared.h"


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
    n_uint hash = math_hash((n_byte *)value->land->map, MAP_AREA);    
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

static n_uint hash_comparison[4] =
{
    0xe06fcbeb27ffaeb5,
    0x673dff5fc201e1c0,
    0xca3f68d986265aa5,
    0xb03d82bb435825a4
};

- (void)testLongtermSimulation
{
    n_uint  time = 3;
    n_uint hash = math_hash((n_byte *)being, sizeof(noble_being) - sizeof(n_byte *) - sizeof(noble_social *) - sizeof(noble_episodic *));
    XCTAssertTrue(hash == 0xcf57c761d1f3db58, @"Starting hash doesn't comply with prior being hashes (%lx)", hash);
    
    shared_flood();

    shared_cycle(1000, NUM_VIEW, 512, 512);
    shared_cycle(1000, NUM_TERRAIN, 512, 512);
    
    hash = math_hash((n_byte *)being, sizeof(noble_being) - sizeof(n_byte *) - sizeof(noble_social *) - sizeof(noble_episodic *));
    XCTAssertTrue(hash == 0xf07db769957bb69a, @"Longterm hash doesn't comply with prior being hashes (%lx)", hash);
 
    while (time < ((TIME_DAY_MINUTES * 1)/2))
    {
        shared_cycle(1000 * time, NUM_VIEW, 512, 512);
        shared_cycle(1000 * time, NUM_TERRAIN, 512, 512);
        time++;
    }
    
    XCTAssertTrue(sizeof(noble_being) == 2728, @"noble_being (%ld)", sizeof(noble_being));

    
    hash = math_hash((n_byte *)being, sizeof(noble_being) - sizeof(n_byte *) - sizeof(noble_social *) - sizeof(noble_episodic *));
    XCTAssertTrue(hash == hash_comparison[0], @"Longterm hash doesn't comply with prior being hashes (%lx)", hash);
    
    hash = math_hash(being->brain, DOUBLE_BRAIN);
    XCTAssertTrue(hash == hash_comparison[1], @"Longterm hash doesn't comply with prior brain hashes (%lx)", hash);
    
    hash = math_hash((n_byte *)being->social, SOCIAL_SIZE * sizeof(noble_social));
    XCTAssertTrue(hash == hash_comparison[2], @"Longterm hash doesn't comply with prior social hashes (%lx)", hash);
    
    hash = math_hash((n_byte *)being->episodic, EPISODIC_SIZE * sizeof(noble_episodic));
    XCTAssertTrue(hash == hash_comparison[3], @"Longterm hash doesn't comply with prior episodic hashes (%lx)", hash);
        
    while (time < ((TIME_DAY_MINUTES * 3)/2))
    {
        shared_cycle(1000 * time, NUM_VIEW, 512, 512);
        shared_cycle(1000 * time, NUM_TERRAIN, 512, 512);
        time++;
    }

    hash = math_hash((n_byte *)being, sizeof(noble_being) - sizeof(n_byte *) - sizeof(noble_social *) - sizeof(noble_episodic *));
    XCTAssertTrue(hash == 0xc25ddddb92cddd5d, @"Longterm hash doesn't comply with prior being hashes (%lx)", hash);

    hash = math_hash(being->brain, DOUBLE_BRAIN);
    XCTAssertTrue(hash == 0xebfe99c1165b8dc7, @"Longterm hash doesn't comply with prior brain hashes (%lx)", hash);
    
    hash = math_hash((n_byte *)being->social, SOCIAL_SIZE * sizeof(noble_social));
    XCTAssertTrue(hash == 0xd4d67c210fedfb9b, @"Longterm hash doesn't comply with prior social hashes (%lx)", hash);

    hash = math_hash((n_byte *)being->episodic, EPISODIC_SIZE * sizeof(noble_episodic));
    XCTAssertTrue(hash == 0x7f8bdd2b1269b13b, @"Longterm hash doesn't comply with prior episodic hashes (%lx)", hash);
    
    XCTAssertTrue(value->num == 21, @"Longterm number apes doesn't comply with prior numbers (%ld)", value->num);
    
    hash = math_hash((n_byte *)value->land->atmosphere, (MAP_AREA*sizeof(n_c_int)/2));
    XCTAssertTrue(hash == 0x891f251be1b8af8, @"Longterm hash doesn't comply with prior weather hashes (%lx)", hash);
    
    hash = math_hash((n_byte *)value->land->map, MAP_AREA);
    XCTAssertTrue(hash == 0xdd4de0fa74ffc961, @"Longterm hash doesn't comply with prior map hashes (%lx)", hash);
}

- (void)testLongtermSimulationCompare
{
    n_uint  time = 3;
    n_uint hash = math_hash((n_byte *)being, sizeof(noble_being) - sizeof(n_byte *) - sizeof(noble_social *) - sizeof(noble_episodic *));
    XCTAssertTrue(hash == 0xcf57c761d1f3db58, @"Starting hash doesn't comply with prior being hashes (%lx)", hash);
    
    shared_flood();
    
    shared_cycle(1000, NUM_VIEW, 512, 512);
    shared_cycle(1000, NUM_TERRAIN, 512, 512);
    
    hash = math_hash((n_byte *)being, sizeof(noble_being) - sizeof(n_byte *) - sizeof(noble_social *) - sizeof(noble_episodic *));
    XCTAssertTrue(hash == 0xf07db769957bb69a, @"Longterm hash doesn't comply with prior being hashes (%lx)", hash);
    
    while (time < ((TIME_DAY_MINUTES * 1)/2))
    {
        shared_cycle(1000 * time, NUM_VIEW, 512, 512);
        shared_cycle(1000 * time, NUM_TERRAIN, 512, 512);
        time++;
    }
    
    hash = math_hash((n_byte *)being, sizeof(noble_being) - sizeof(n_byte *) - sizeof(noble_social *) - sizeof(noble_episodic *));
    XCTAssertTrue(hash == hash_comparison[0], @"Longterm hash doesn't comply with prior being hashes (%lx)", hash);
    
    hash = math_hash(being->brain, DOUBLE_BRAIN);
    XCTAssertTrue(hash == hash_comparison[1], @"Longterm hash doesn't comply with prior brain hashes (%lx)", hash);
    
    hash = math_hash((n_byte *)being->social, SOCIAL_SIZE * sizeof(noble_social));
    XCTAssertTrue(hash == hash_comparison[2], @"Longterm hash doesn't comply with prior social hashes (%lx)", hash);
    
    hash = math_hash((n_byte *)being->episodic, EPISODIC_SIZE * sizeof(noble_episodic));
    XCTAssertTrue(hash == hash_comparison[3], @"Longterm hash doesn't comply with prior episodic hashes (%lx)", hash);
}

@end
