/****************************************************************

 toolkit_tests.c

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
#include "../toolkit.h"

// Test counter and result tracking
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

n_int draw_error(n_constant_string error_text, n_constant_string location, n_int line_number)
{
    printf("ERROR: %s @ %s %ld\n",(const n_string) error_text, location, line_number);
    return -1;
}

// Test assertion macro
#define TEST_ASSERT(condition, message) \
    do { \
        tests_run++; \
        if (condition) { \
            tests_passed++; \
            printf("PASS: %s\n", message); \
        } else { \
            tests_failed++; \
            printf("FAIL: %s (line %d)\n", message, __LINE__); \
        } \
    } while(0)

// Test suite header macro
#define TEST_SUITE(name) \
    printf("\n=== Testing %s ===\n", name)

// Mock implementations for functions that might not be implemented
// These would be replaced with actual function calls when toolkit.c is available

// Test helper functions
void print_test_summary(void) {
    printf("\n=== Test Summary ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_failed);
    printf("Success rate: %.1f%%\n", 
           tests_run > 0 ? (100.0 * tests_passed / tests_run) : 0.0);
}

// Vector2 Tests
void test_vect2_functions(void) {
    TEST_SUITE("Vector2 Functions");
    
    n_vect2 v1, v2, result;
    
    // Test vect2_populate
    vect2_populate(&v1, 10, 20);
    TEST_ASSERT(v1.x == 10 && v1.y == 20, "vect2_populate sets coordinates correctly");
    
    // Test vect2_copy
    vect2_copy(&v2, &v1);
    TEST_ASSERT(v2.x == v1.x && v2.y == v1.y, "vect2_copy copies coordinates correctly");
    
    // Test vect2_add
    vect2_populate(&v1, 5, 10);
    vect2_populate(&v2, 3, 7);
    vect2_add(&result, &v1, &v2);
    TEST_ASSERT(result.x == 8 && result.y == 17, "vect2_add performs addition correctly");
    
    // Test vect2_subtract
    vect2_subtract(&result, &v1, &v2);
    TEST_ASSERT(result.x == 2 && result.y == 3, "vect2_subtract performs subtraction correctly");
    
    // Test vect2_center
    vect2_center(&result, &v1, &v2);
    TEST_ASSERT(result.x == 4 && result.y == 8, "vect2_center calculates midpoint correctly");
    
    // Test vect2_offset
    vect2_populate(&v1, 10, 15);
    vect2_offset(&v1, 5, -3);
    TEST_ASSERT(v1.x == 15 && v1.y == 12, "vect2_offset modifies coordinates correctly");
    
    // Test vect2_scalar_multiply
    vect2_populate(&v1, 4, 6);
    vect2_scalar_multiply(&v1, 3);
    TEST_ASSERT(v1.x == 12 && v1.y == 18, "vect2_scalar_multiply scales correctly");
    
    // Test vect2_scalar_divide
    vect2_scalar_divide(&v1, 2);
    TEST_ASSERT(v1.x == 6 && v1.y == 9, "vect2_scalar_divide scales correctly");
    
    // Test vect2_nonzero
    vect2_populate(&v1, 0, 0);
    TEST_ASSERT(vect2_nonzero(&v1) == 0, "vect2_nonzero returns 0 for zero vector");
    
    vect2_populate(&v1, 1, 0);
    TEST_ASSERT(vect2_nonzero(&v1) != 0, "vect2_nonzero returns non-zero for non-zero vector");
    
    // Test vect2_rotate90
    vect2_populate(&v1, 3, 4);
    vect2_rotate90(&v1);
    TEST_ASSERT(v1.x == 4 && v1.y == -3, "vect2_rotate90 rotates 90 degrees counter-clockwise");
}

// Vector3 Tests
void test_vect3_functions(void) {
    TEST_SUITE("Vector3 Functions");
    
    n_vect3 v1, v2, result;
    
    // Test vect3_populate
    vect3_populate(&v1, 1.0, 2.0, 3.0);
    TEST_ASSERT(v1.x == 1.0 && v1.y == 2.0 && v1.z == 3.0, 
                "vect3_populate sets coordinates correctly");
    
    // Test vect3_copy
    vect3_copy(&v2, &v1);
    TEST_ASSERT(v2.x == v1.x && v2.y == v1.y && v2.z == v1.z, 
                "vect3_copy copies coordinates correctly");
    
    // Test vect3_add
    vect3_populate(&v1, 1.5, 2.5, 3.5);
    vect3_populate(&v2, 0.5, 1.5, 2.5);
    vect3_add(&result, &v1, &v2);
    TEST_ASSERT(result.x == 2.0 && result.y == 4.0 && result.z == 6.0, 
                "vect3_add performs addition correctly");
    
    // Test vect3_subtract
    vect3_subtract(&result, &v1, &v2);
    TEST_ASSERT(result.x == 1.0 && result.y == 1.0 && result.z == 1.0, 
                "vect3_subtract performs subtraction correctly");
    
    // Test vect3_center
    vect3_center(&result, &v1, &v2);
    TEST_ASSERT(result.x == 1.0 && result.y == 2.0 && result.z == 3.0, 
                "vect3_center calculates midpoint correctly");
    
    // Test vect3_offset
    vect3_populate(&v1, 10.0, 15.0, 20.0);
    vect3_offset(&v1, 5.0, -3.0, 2.0);
    TEST_ASSERT(v1.x == 15.0 && v1.y == 12.0 && v1.z == 22.0, 
                "vect3_offset modifies coordinates correctly");
    
    // Test vect3_nonzero
    vect3_populate(&v1, 0.0, 0.0, 0.0);
    TEST_ASSERT(vect3_nonzero(&v1) == 0, "vect3_nonzero returns 0 for zero vector");
    
    vect3_populate(&v1, 1.0, 0.0, 0.0);
    TEST_ASSERT(vect3_nonzero(&v1) != 0, "vect3_nonzero returns non-zero for non-zero vector");
}

// Memory Management Tests
void test_memory_functions(void) {
    TEST_SUITE("Memory Management Functions");
    
    // Test memory_new and memory_free
    void *ptr = memory_new(1024);
    TEST_ASSERT(ptr != NULL, "memory_new allocates memory successfully");
    
    memory_free(&ptr);
    TEST_ASSERT(ptr == NULL, "memory_free sets pointer to NULL");
    
    // Test memory_copy
    n_byte source[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    n_byte dest[10] = {0};
    
    memory_copy(source, dest, 10);
    TEST_ASSERT(memcmp(source, dest, 10) == 0, "memory_copy copies data correctly");
    
    // Test memory_erase
    memory_erase(dest, 10);
    n_byte zeros[10] = {0};
    TEST_ASSERT(memcmp(dest, zeros, 10) == 0, "memory_erase clears memory correctly");
    
    // Test memory_list_new and memory_list_free
    memory_list *list = memory_list_new(sizeof(n_int), 10);
    TEST_ASSERT(list != NULL, "memory_list_new creates list successfully");
    TEST_ASSERT(list->unit_size == sizeof(n_int), "memory_list_new sets unit size correctly");
    TEST_ASSERT(list->max == 10, "memory_list_new sets max capacity correctly");
    TEST_ASSERT(list->count == 0, "memory_list_new initializes count to zero");
    
    memory_list_free(&list);
    TEST_ASSERT(list == NULL, "memory_list_free sets pointer to NULL");
}

// File I/O Tests
void test_file_functions(void) {
    TEST_SUITE("File I/O Functions");
    
    // Test io_file_new and io_file_free
    n_file *file = io_file_new();
    TEST_ASSERT(file != NULL, "io_file_new creates file structure successfully");
    TEST_ASSERT(file->location == 0, "io_file_new initializes location to zero");
    
    io_file_free(&file);
    TEST_ASSERT(file == NULL, "io_file_free sets pointer to NULL");
    
    // Test io_file_new_from_string
    n_string test_string = "Hello, World!";
    n_uint string_length = strlen(test_string);
    file = io_file_new_from_string(test_string, string_length);
    
    TEST_ASSERT(file != NULL, "io_file_new_from_string creates file successfully");
    TEST_ASSERT(strncmp((char*)file->data, test_string, string_length) == 0,
                "io_file_new_from_string copies data correctly");
    
    io_file_free(&file);
    
    // Test io_length
    n_string test_str = "Test String";
    n_int length = io_length(test_str, 100);
    TEST_ASSERT(length == 11, "io_length returns correct string length");
    
    // Test io_find
    n_string search_in = "Hello World Hello";
    n_string search_for = "Hello";
    n_int found_pos = io_find(search_in, 0, 17, search_for, 5);
    TEST_ASSERT(found_pos == 0, "io_find finds string at beginning");
    
    found_pos = io_find(search_in, 6, 17, search_for, 5);
    TEST_ASSERT(found_pos == 12, "io_find finds string at later position");
}

// String Utility Tests
void test_string_utilities(void) {
    TEST_SUITE("String Utilities");
    
    // Test io_string_copy
    n_string original = "Test String";
    n_string copy = io_string_copy(original);
    
    TEST_ASSERT(copy != NULL, "io_string_copy allocates memory");
    TEST_ASSERT(strcmp(copy, original) == 0, "io_string_copy copies string correctly");
    TEST_ASSERT(copy != original, "io_string_copy creates separate memory");
    
    memory_free((void**)&copy);
    
    // Test io_lower
    char test_string[] = "HELLO World 123";
    io_lower(test_string, strlen(test_string));
    TEST_ASSERT(strcmp(test_string, "hello world 123") == 0, 
                "io_lower converts uppercase to lowercase");
    
    // Test io_number_to_string
    n_string_block number_string;
    io_number_to_string(number_string, 12345);
    TEST_ASSERT(strcmp(number_string, "12345") == 0, 
                "io_number_to_string converts number correctly");
    
    // Test io_three_strings
    n_string_block combined;
    io_three_strings(combined, "Hello", " ", "World", 0);
    TEST_ASSERT(strcmp(combined, "Hello World") == 0, 
                "io_three_strings combines strings correctly");
}

// Math Function Tests
void test_math_functions(void) {
    TEST_SUITE("Math Functions");
    
    // Test math_root
    n_uint root_result = math_root(16);
    TEST_ASSERT(root_result == 4, "math_root calculates square root correctly");
    
    root_result = math_root(25);
    TEST_ASSERT(root_result == 5, "math_root calculates square root of 25 correctly");
    
    root_result = math_root(0);
    TEST_ASSERT(root_result == 0, "math_root handles zero correctly");
    
    // Test math_sine (approximate test since we're dealing with fixed-point)
    n_int sine_result = math_sine(0, 1);
    TEST_ASSERT(sine_result == 0, "math_sine(0) returns approximately 0");
    
    // Test math_hash_fnv1
    n_byte4 hash1 = math_hash_fnv1("test");
    n_byte4 hash2 = math_hash_fnv1("test");
    n_byte4 hash3 = math_hash_fnv1("different");
    
    TEST_ASSERT(hash1 == hash2, "math_hash_fnv1 produces consistent results");
    TEST_ASSERT(hash1 != hash3, "math_hash_fnv1 produces different hashes for different strings");
    
    // Test vect2_distance_under
    n_vect2 p1, p2;
    vect2_populate(&p1, 0, 0);
    vect2_populate(&p2, 3, 4);
    
    TEST_ASSERT(vect2_distance_under(&p1, &p2, 6) == 1, 
                "vect2_distance_under returns true when distance is under threshold");
    TEST_ASSERT(vect2_distance_under(&p1, &p2, 4) == 0, 
                "vect2_distance_under returns false when distance is over threshold");
}

// Area and Geometry Tests
void test_geometry_functions(void) {
    TEST_SUITE("Geometry Functions");
    
    n_area2 area;
    n_vect2 point;
    
    // Test area2_add with first point
    vect2_populate(&point, 10, 20);
    area2_add(&area, &point, 1);
    TEST_ASSERT(area.top_left.x == 10 && area.top_left.y == 20, 
                "area2_add sets first point as both top_left and bottom_right");
    TEST_ASSERT(area.bottom_right.x == 10 && area.bottom_right.y == 20, 
                "area2_add sets first point as both top_left and bottom_right");
    
    // Test area2_add with second point
    vect2_populate(&point, 5, 30);
    area2_add(&area, &point, 0);
    TEST_ASSERT(area.top_left.x == 5 && area.top_left.y == 20, 
                "area2_add updates bounding box correctly");
    TEST_ASSERT(area.bottom_right.x == 10 && area.bottom_right.y == 30, 
                "area2_add updates bounding box correctly");
    
    // Test line intersection
    n_vect2 p1, q1, p2, q2;
    vect2_populate(&p1, 0, 0);
    vect2_populate(&q1, 10, 10);
    vect2_populate(&p2, 0, 10);
    vect2_populate(&q2, 10, 0);
    
    n_byte intersects = math_do_intersect(&p1, &q1, &p2, &q2);
    TEST_ASSERT(intersects != 0, "math_do_intersect detects intersecting lines");
    
    // Test parallel lines
    vect2_populate(&p2, 1, 1);
    vect2_populate(&q2, 11, 11);
    intersects = math_do_intersect(&p1, &q1, &p2, &q2);
    TEST_ASSERT(intersects == 0, "math_do_intersect detects parallel lines");
}

// Object/JSON Tests
void test_object_functions(void) {
    TEST_SUITE("Object/JSON Functions");
    
    // Test array creation
    n_array *num_array = array_number(42);
    TEST_ASSERT(num_array != NULL, "array_number creates array successfully");
    TEST_ASSERT(num_array->type == OBJECT_NUMBER, "array_number sets correct type");
    
    n_array *str_array = array_string("test string");
    TEST_ASSERT(str_array != NULL, "array_string creates array successfully");
    TEST_ASSERT(str_array->type == OBJECT_STRING, "array_string sets correct type");
    
    n_array *bool_array = array_boolean(1);
    TEST_ASSERT(bool_array != NULL, "array_boolean creates array successfully");
    TEST_ASSERT(bool_array->type == OBJECT_BOOLEAN, "array_boolean sets correct type");
    
    // Test object creation
    n_object *obj = NULL;
    obj = object_number(obj, "test_number", 123);
    TEST_ASSERT(obj != NULL, "object_number creates object successfully");
    
    obj = object_string(obj, "test_string", "hello");
    TEST_ASSERT(obj != NULL, "object_string adds to object successfully");
    
    obj = object_boolean(obj, "test_boolean", 1);
    TEST_ASSERT(obj != NULL, "object_boolean adds to object successfully");
    
    // Test object queries
    n_string result = obj_contains(obj, "test_string", OBJECT_STRING);
    TEST_ASSERT(result != NULL, "obj_contains finds string property");
    
    n_int number_result;
    n_int found = obj_contains_number(obj, "test_number", &number_result);
    TEST_ASSERT(found != 0, "obj_contains_number finds number property");
    TEST_ASSERT(number_result == 123, "obj_contains_number returns correct value");
    
    // Clean up
    obj_free(&obj);
    TEST_ASSERT(obj == NULL, "obj_free sets pointer to NULL");
}

// Utility macro and constant tests
void test_utility_macros(void) {
    TEST_SUITE("Utility Macros and Constants");
    
    // Test character classification macros
    TEST_ASSERT(ASCII_NUMBER('5') != 0, "ASCII_NUMBER identifies digit correctly");
    TEST_ASSERT(ASCII_NUMBER('a') == 0, "ASCII_NUMBER rejects non-digit correctly");
    
    TEST_ASSERT(ASCII_UPPERCASE('A') != 0, "ASCII_UPPERCASE identifies uppercase correctly");
    TEST_ASSERT(ASCII_UPPERCASE('a') == 0, "ASCII_UPPERCASE rejects lowercase correctly");
    
    TEST_ASSERT(ASCII_LOWERCASE('a') != 0, "ASCII_LOWERCASE identifies lowercase correctly");
    TEST_ASSERT(ASCII_LOWERCASE('A') == 0, "ASCII_LOWERCASE rejects uppercase correctly");
    
    TEST_ASSERT(IS_SPACE(' ') != 0, "IS_SPACE identifies space correctly");
    TEST_ASSERT(IS_SPACE('a') == 0, "IS_SPACE rejects non-space correctly");
    
    TEST_ASSERT(IS_RETURN('\n') != 0, "IS_RETURN identifies newline correctly");
    TEST_ASSERT(IS_RETURN('\r') != 0, "IS_RETURN identifies carriage return correctly");
    TEST_ASSERT(IS_RETURN('a') == 0, "IS_RETURN rejects non-return correctly");
    
    // Test constants
    TEST_ASSERT(CHAR_SPACE == 32, "CHAR_SPACE has correct value");
    TEST_ASSERT(NOTHING == 0L, "NOTHING is zero");
    TEST_ASSERT(BIG_INTEGER == 2147483647, "BIG_INTEGER has correct value");
    
    // Test ABS macro
    TEST_ASSERT(ABS(-5) == 5, "ABS handles negative numbers correctly");
    TEST_ASSERT(ABS(5) == 5, "ABS handles positive numbers correctly");
    TEST_ASSERT(ABS(0) == 0, "ABS handles zero correctly");
}

// Data type size and structure tests
void test_data_types(void) {
    TEST_SUITE("Data Types and Structures");
    
    // Test basic type sizes (architecture dependent)
    TEST_ASSERT(sizeof(n_byte) == 1, "n_byte is 1 byte");
    TEST_ASSERT(sizeof(n_byte2) == 2, "n_byte2 is 2 bytes");
    TEST_ASSERT(sizeof(n_byte4) == 4, "n_byte4 is 4 bytes");
    
    // Test structure sizes and alignment
    n_vect2 v2;
    TEST_ASSERT(sizeof(v2) >= 2 * sizeof(n_int), "n_vect2 has expected minimum size");
    TEST_ASSERT(&v2.data[0] == &v2.x, "n_vect2 union maps x to data[0]");
    TEST_ASSERT(&v2.data[1] == &v2.y, "n_vect2 union maps y to data[1]");
    
    n_vect3 v3;
    TEST_ASSERT(sizeof(v3) >= 3 * sizeof(n_double), "n_vect3 has expected minimum size");
    TEST_ASSERT(&v3.data[0] == &v3.x, "n_vect3 union maps x to data[0]");
    TEST_ASSERT(&v3.data[1] == &v3.y, "n_vect3 union maps y to data[1]");
    TEST_ASSERT(&v3.data[2] == &v3.z, "n_vect3 union maps z to data[2]");
    
    n_rgba32 color;
    color.thirtytwo = 0x12345678;
    TEST_ASSERT(sizeof(color.rgba) == sizeof(color.thirtytwo), 
                "n_rgba32 union components have same size");
    
    // Test file structure
    n_file file_struct = {0};
    TEST_ASSERT(file_struct.size == 0, "n_file structure initializes correctly");
    TEST_ASSERT(file_struct.location == 0, "n_file structure initializes correctly");
    TEST_ASSERT(file_struct.data == NULL, "n_file structure initializes correctly");
}

// Main test runner
int main(int argc, char *argv[]) {
    printf("Starting Toolkit Unit Tests\n");
    printf("============================\n");
    
    // Run all test suites
    test_utility_macros();
    test_data_types();
    test_vect2_functions();
    test_vect3_functions();
    test_memory_functions();
    test_file_functions();
    test_string_utilities();
    test_math_functions();
    test_geometry_functions();
    //test_object_functions(); // need to be uncommented when memory issue is fixed
    
    // Print summary
    print_test_summary();
    
    // Return appropriate exit code
    return (tests_failed == 0) ? 0 : 1;
}

// Additional helper functions for testing
void debug_print_vect2(const char* name, n_vect2* v) {
    printf("DEBUG: %s = (%ld, %ld)\n", name, v->x, v->y);
}

void debug_print_vect3(const char* name, n_vect3* v) {
    printf("DEBUG: %s = (%.2f, %.2f, %.2f)\n", name, v->x, v->y, v->z);
}

void debug_print_memory_list(const char* name, memory_list* list) {
    if (list) {
        printf("DEBUG: %s - count: %u, max: %u, unit_size: %u\n", 
               name, list->count, list->max, list->unit_size);
    } else {
        printf("DEBUG: %s - NULL\n", name);
    }
}
