/****************************************************************

 sim_tests.c

 =============================================================

 Copyright 1996-2025 Tom Barbalet. All rights reserved.

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Include the headers we're testing
#include "toolkit.h"  // Required by sim.h
#include "sim.h"

// Simple unit test framework
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

n_int draw_error(n_constant_string error_text, n_constant_string location, n_int line_number)
{
    printf("ERROR: %s @ %s %ld\n",(const n_string) error_text, location, line_number);
    return -1;
}

#define TEST_ASSERT(test, message) do { \
    tests_run++; \
    if (test) { \
        tests_passed++; \
        printf("PASS: %s\n", message); \
    } else { \
        tests_failed++; \
        printf("FAIL: %s\n", message); \
    } \
} while(0)

#define TEST_EQUALS_INT(expected, actual, message) do { \
    tests_run++; \
    if ((expected) == (actual)) { \
        tests_passed++; \
        printf("PASS: %s (expected: %d, got: %d)\n", message, expected, actual); \
    } else { \
        tests_failed++; \
        printf("FAIL: %s (expected: %d, got: %d)\n", message, expected, actual); \
    } \
} while(0)

#define TEST_NOT_NULL(ptr, message) do { \
    tests_run++; \
    if ((ptr) != NULL) { \
        tests_passed++; \
        printf("PASS: %s\n", message); \
    } else { \
        tests_failed++; \
        printf("FAIL: %s (pointer was NULL)\n", message); \
    } \
} while(0)

// Test helper functions
void print_test_summary(void) {
    printf("\n=== TEST SUMMARY ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_failed);
    printf("Success rate: %.1f%%\n", 
           tests_run > 0 ? (100.0 * tests_passed / tests_run) : 0.0);
    printf("==================\n");
}

// Test constants and defines
void test_constants(void) {
    printf("\n--- Testing Constants ---\n");
    
    TEST_EQUALS_INT(708, VERSION_NUMBER, "Version number should be 708");
    TEST_ASSERT(SIMULATED_APE_SIGNATURE == (('N'<< 8) | 'A'), "Simulated Ape signature");
    TEST_ASSERT(SIMULATED_WAR_SIGNATURE == (('N'<< 8) | 'W'), "Simulated War signature");
    
    // Test map dimensions
    #ifdef SIMULATED_PLANET
    TEST_EQUALS_INT(8, MAP_BITS, "Map bits for planet mode");
    #else
    TEST_EQUALS_INT(9, MAP_BITS, "Map bits for regular mode");
    #endif
    
    TEST_EQUALS_INT(1<<(MAP_BITS), MAP_DIMENSION, "Map dimension calculation");
    TEST_EQUALS_INT(1<<(2*MAP_BITS), MAP_AREA, "Map area calculation");
    
    // Test window dimensions
    TEST_EQUALS_INT(4096, TERRAIN_WINDOW_WIDTH, "Terrain window width");
    TEST_EQUALS_INT(3072, TERRAIN_WINDOW_HEIGHT, "Terrain window height");
    TEST_EQUALS_INT(2048, CONTROL_WINDOW_WIDTH, "Control window width");
    TEST_EQUALS_INT(2048, CONTROL_WINDOW_HEIGHT, "Control window height");
    
    // Test time constants
    TEST_EQUALS_INT(60, TIME_HOUR_MINUTES, "Minutes per hour");
    TEST_EQUALS_INT(1440, TIME_DAY_MINUTES, "Minutes per day");
    TEST_EQUALS_INT(40320, TIME_MONTH_MINUTES, "Minutes per month");
    TEST_EQUALS_INT(524160, TIME_YEAR_MINUTES, "Minutes per year");
    
    // Test braincode constants
    TEST_EQUALS_INT(128, BRAINCODE_SIZE, "Braincode size");
    TEST_EQUALS_INT(16, BRAINCODE_PROBES, "Braincode probes");
    TEST_EQUALS_INT(3, BRAINCODE_BYTES_PER_INSTRUCTION, "Braincode bytes per instruction");
}

// Test enumerations
void test_enums(void) {
    printf("\n--- Testing Enumerations ---\n");
    
    // Test weather values
    TEST_EQUALS_INT(0, WEATHER_SEVEN_SUNNY_DAY, "Sunny day weather value");
    TEST_EQUALS_INT(1, WEATHER_SEVEN_CLOUDY_DAY, "Cloudy day weather value");
    TEST_EQUALS_INT(6, WEATHER_SEVEN_DAWN_DUSK, "Dawn/dusk weather value");
    
    // Test entity types
    TEST_EQUALS_INT(0, ET_SIMULATED_APE, "Simulated ape entity type");
    TEST_EQUALS_INT(1, ET_SIMULATED_APE_GHOST, "Ghost ape entity type");
    TEST_EQUALS_INT(2, ET_FIERCE_FELINE, "Feline entity type");
    TEST_EQUALS_INT(3, ET_FIERCE_BIRD_OF_PREY, "Bird of prey entity type");
    
    // Test braincode commands
    TEST_EQUALS_INT(0, BRAINCODE_DAT0, "Braincode DAT0");
    TEST_EQUALS_INT(1, BRAINCODE_DAT1, "Braincode DAT1");
    TEST_EQUALS_INT(2, BRAINCODE_ADD, "Braincode ADD");
    TEST_EQUALS_INT(9, BRAINCODE_JMP, "Braincode JMP");
    
    // Test color types
    TEST_EQUALS_INT(0, COLOR_BLACK, "Black color");
    TEST_EQUALS_INT(255, COLOR_RED, "Red color");
    TEST_EQUALS_INT(252, COLOR_WHITE, "White color");
}

// Test data structures
void test_data_structures(void) {
    printf("\n--- Testing Data Structures ---\n");
    
    // Test n_version structure
    n_version version;
    version.signature = SIMULATED_APE_SIGNATURE;
    version.version = VERSION_NUMBER;
    
    TEST_EQUALS_INT(SIMULATED_APE_SIGNATURE, version.signature, "Version structure signature");
    TEST_EQUALS_INT(VERSION_NUMBER, version.version, "Version structure version");
    
    // Test n_tile_coordinates structure
    n_tile_coordinates coords;
    coords.x = 100;
    coords.y = 200;
    coords.tile = 0;
    coords.facing = 45;
    
    TEST_EQUALS_INT(100, coords.x, "Tile coordinates X");
    TEST_EQUALS_INT(200, coords.y, "Tile coordinates Y");
    TEST_EQUALS_INT(0, coords.tile, "Tile coordinates tile");
    TEST_EQUALS_INT(45, coords.facing, "Tile coordinates facing");
}

// Test macro functions
void test_macros(void) {
    printf("\n--- Testing Macros ---\n");
    
    // Test coordinate conversion macros
    n_int ape_coord = 64;  // 1 << 6
    n_int map_coord = APESPACE_TO_MAPSPACE(ape_coord);
    n_int back_to_ape = MAPSPACE_TO_APESPACE(map_coord);
    
    TEST_EQUALS_INT(1, map_coord, "Apespace to mapspace conversion");
    TEST_EQUALS_INT(64, back_to_ape, "Mapspace back to apespace conversion");
    
    // Test bounds checking
    n_int bounded_positive = APESPACE_CONFINED(100);
    n_int bounded_negative = APESPACE_CONFINED(-10);
    n_int bounded_max = APESPACE_CONFINED(100000);
    
    TEST_EQUALS_INT(100, bounded_positive, "Positive coordinate confinement");
    TEST_EQUALS_INT(0, bounded_negative, "Negative coordinate confinement");
    TEST_EQUALS_INT(APESPACE_BOUNDS, bounded_max, "Maximum coordinate confinement");
    
    // Test land coordinate wrapping
    n_int wrapped = POSITIVE_LAND_COORD(-5);
    TEST_ASSERT(wrapped >= 0 && wrapped < MAP_DIMENSION, "Land coordinate wrapping");
    
    // Test night/day detection
    n_int day_time = 600;    // Should be day
    n_int night_time = 100;  // Should be night
    n_int dawn_time = 352;   // Should be dawn/dusk
    
    TEST_ASSERT(!IS_NIGHT(day_time), "Day time detection");
    TEST_ASSERT(IS_NIGHT(night_time), "Night time detection");
    TEST_ASSERT(IS_DAWNDUSK(dawn_time), "Dawn/dusk time detection");
    
    // Test water testing
    n_int land_height = 150;
    n_int water_level = 128;
    TEST_ASSERT(!WATER_TEST(land_height, water_level), "Land above water test");
    TEST_ASSERT(WATER_TEST(100, water_level), "Land below water test");
}

// Test weather system (if functions are available)
void test_weather_system(void) {
    printf("\n--- Testing Weather System ---\n");
    
    // Test weather constants
    TEST_ASSERT(WEATHER_CLOUD > 0, "Weather cloud constant");
    TEST_ASSERT(WEATHER_RAIN > WEATHER_CLOUD, "Weather rain > cloud");
    
    // Test tide constants
    TEST_EQUALS_INT(8, TIDE_AMPLITUDE_LUNAR, "Lunar tide amplitude");
    TEST_EQUALS_INT(2, TIDE_AMPLITUDE_SOLAR, "Solar tide amplitude");
    TEST_EQUALS_INT(138, TIDE_MAX, "Maximum tide level");
    
    // Test lunar orbit
    TEST_EQUALS_INT(39312, LUNAR_ORBIT_MINS, "Lunar orbit minutes");
}

// Test braincode system
void test_braincode_system(void) {
    printf("\n--- Testing Braincode System ---\n");
    
    // Test address calculation
    n_int addr1 = BRAINCODE_ADDRESS(100);
    n_int addr2 = BRAINCODE_ADDRESS(300);
    
    TEST_ASSERT(addr1 >= 0 && addr1 < BRAINCODE_MAX_ADDRESS, "Braincode address 1 bounds");
    TEST_ASSERT(addr2 >= 0 && addr2 < BRAINCODE_MAX_ADDRESS, "Braincode address 2 bounds");
    
    // Test instruction boundaries
    TEST_ASSERT(BRAINCODE_INSTRUCTIONS > BRAINCODE_ANE, "Instruction count valid");
    TEST_EQUALS_INT(16, BRAINCODE_MAX_FREQUENCY, "Max probe frequency");
    TEST_EQUALS_INT(16, BRAINCODE_BLOCK_COPY, "Block copy size");
}

// Test string constants
void test_string_constants(void) {
    printf("\n--- Testing String Constants ---\n");
    
    TEST_NOT_NULL(SHORT_VERSION_NAME, "Short version name exists");
    TEST_NOT_NULL(COPYRIGHT_NAME, "Copyright name exists");
    TEST_NOT_NULL(COPYRIGHT_FOLLOW, "Copyright follow text exists");
    TEST_NOT_NULL(FULL_VERSION_COPYRIGHT, "Full version copyright exists");
    
    // Basic string content tests
    TEST_ASSERT(strstr(SHORT_VERSION_NAME, "0.708") != NULL, "Version in short name");
    TEST_ASSERT(strstr(COPYRIGHT_NAME, "Tom Barbalet") != NULL, "Author in copyright");
    TEST_ASSERT(strstr(FULL_VERSION_COPYRIGHT, "1996-2025") != NULL, "Date range in copyright");
}

// Test drawing window flags
void test_drawing_flags(void) {
    printf("\n--- Testing Drawing Flags ---\n");
    
    TEST_EQUALS_INT(1, DRAW_WINDOW_VIEW, "Draw window view flag");
    TEST_EQUALS_INT(2, DRAW_WINDOW_TERRAIN, "Draw window terrain flag");
    TEST_EQUALS_INT(4, DRAW_WINDOW_CONTROL, "Draw window control flag");
    
    // Test flag checking
    n_int combined_flags = DRAW_WINDOW_VIEW | DRAW_WINDOW_TERRAIN;
    TEST_ASSERT(CHECK_DRAW_WINDOW(combined_flags, DRAW_WINDOW_VIEW), "View flag check");
    TEST_ASSERT(CHECK_DRAW_WINDOW(combined_flags, DRAW_WINDOW_TERRAIN), "Terrain flag check");
    TEST_ASSERT(!CHECK_DRAW_WINDOW(combined_flags, DRAW_WINDOW_CONTROL), "Control flag not set");
}

// Test high resolution map calculations
void test_hires_calculations(void) {
    printf("\n--- Testing High Resolution Calculations ---\n");
    
    TEST_EQUALS_INT(MAP_BITS + 3, HI_RES_MAP_BITS, "High res map bits");
    TEST_EQUALS_INT(1<<(HI_RES_MAP_BITS), HI_RES_MAP_DIMENSION, "High res map dimension");
    TEST_EQUALS_INT(1<<(2*HI_RES_MAP_BITS), HI_RES_MAP_AREA, "High res map area");
    
    // Test resolution calculations
    n_int map_ape_res = MAPSPACE_TO_APESPACE(MAP_DIMENSION);
    TEST_EQUALS_INT(map_ape_res, MAP_APE_RESOLUTION_SIZE, "Map ape resolution size");
    TEST_EQUALS_INT(map_ape_res - 1, APESPACE_BOUNDS, "Apespace bounds calculation");
}

// Main test runner
int main(void) {
    printf("=== Simulated Ape SDK Unit Tests ===\n");
    printf("Testing sim.h functionality\n");
    
    // Run all test suites
    test_constants();
    test_enums();
    test_data_structures();
    test_macros();
    test_weather_system();
    test_braincode_system();
    test_string_constants();
    test_drawing_flags();
    test_hires_calculations();
    
    // Print final summary
    print_test_summary();
    
    // Return appropriate exit code
    return (tests_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
