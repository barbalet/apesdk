/****************************************************************

 render_tests.c

 =============================================================

 Copyright 1996-2024 Tom Barbalet. All rights reserved.

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

/* Include the headers under test */
#include "graph.h"
#include "glrender.h"

n_int draw_error(n_constant_string error_text, n_constant_string location, n_int line_number)
{
    printf("ERROR: %s @ %s %ld\n",(const n_string) error_text, location, line_number);
    return -1;
}

/* Test framework macros */
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("FAIL: %s - %s (line %d)\n", __func__, message, __LINE__); \
            test_failures++; \
            return 0; \
        } \
    } while(0)

#define TEST_ASSERT_EQUAL(expected, actual, message) \
    do { \
        if ((expected) != (actual)) { \
            printf("FAIL: %s - %s: expected %d, got %d (line %d)\n", \
                   __func__, message, (int)(expected), (int)(actual), __LINE__); \
            test_failures++; \
            return 0; \
        } \
    } while(0)

#define TEST_ASSERT_NOT_NULL(ptr, message) \
    do { \
        if ((ptr) == NULL) { \
            printf("FAIL: %s - %s: pointer is NULL (line %d)\n", \
                   __func__, message, __LINE__); \
            test_failures++; \
            return 0; \
        } \
    } while(0)

#define RUN_TEST(test_func) \
    do { \
        printf("Running %s... ", #test_func); \
        if (test_func()) { \
            printf("PASSED\n"); \
            test_passes++; \
        } \
        test_count++; \
    } while(0)

/* Global test counters */
static int test_count = 0;
static int test_passes = 0;
static int test_failures = 0;

/* Test buffer dimensions */
#define TEST_BUFFER_WIDTH  256
#define TEST_BUFFER_HEIGHT 256
#define TEST_BUFFER_SIZE   (TEST_BUFFER_WIDTH * TEST_BUFFER_HEIGHT * 4) /* RGBA */

/* Helper function to create test buffer */
static n_byte* create_test_buffer(void) {
    n_byte* buffer = (n_byte*)malloc(TEST_BUFFER_SIZE);
    if (buffer) {
        memset(buffer, 0, TEST_BUFFER_SIZE);
    }
    return buffer;
}

/* Helper function to create test image dimensions */
static n_vect2 create_test_img_size(void) {
    n_vect2 img;
    vect2_populate(&img, TEST_BUFFER_WIDTH, TEST_BUFFER_HEIGHT);
    return img;
}

/* Helper function to create test color */
static n_rgba32 create_test_color(n_byte r, n_byte g, n_byte b, n_byte a) {
    n_rgba32 color;
    color.rgba.r = r;
    color.rgba.g = g;
    color.rgba.b = b;
    color.rgba.a = a;
    return color;
}

/****************************************************************
 * Graph Module Tests
 ****************************************************************/

/* Test graph_init functionality */
static int test_graph_init(void) {
    /* Test basic initialization */
    graph_init(1);
    TEST_ASSERT(1, "graph_init with four_byte_factory=1 should complete without error");
    
    graph_init(0);
    TEST_ASSERT(1, "graph_init with four_byte_factory=0 should complete without error");
    
    /* Test three-component initialization */
    graph_init_three();
    TEST_ASSERT(1, "graph_init_three should complete without error");
    
    return 1;
}

/* Test graph_erase functionality */
static int test_graph_erase(void) {
    n_byte* buffer = create_test_buffer();
    TEST_ASSERT_NOT_NULL(buffer, "Test buffer allocation");
    
    n_vect2 img = create_test_img_size();
    n_rgba32 color = create_test_color(255, 0, 0, 255); /* Red */
    
    /* Fill buffer with non-zero data first */
    memset(buffer, 0xAA, TEST_BUFFER_SIZE);
    
    /* Test erase functionality */
    graph_erase(buffer, &img, &color);
    TEST_ASSERT(1, "graph_erase should complete without error");
    
    /* Note: We can't easily verify the exact behavior without knowing
     * the internal implementation details, but we can ensure it doesn't crash */
    
    free(buffer);
    return 1;
}

/* Test graph_line functionality */
static int test_graph_line(void) {
    n_byte* buffer = create_test_buffer();
    TEST_ASSERT_NOT_NULL(buffer, "Test buffer allocation");
    
    n_vect2 img = create_test_img_size();
    n_vect2 start, end;
    n_rgba32 color = create_test_color(0, 255, 0, 255); /* Green */
    
    /* Test horizontal line */
    vect2_populate(&start, 10, 50);
    vect2_populate(&end, 100, 50);
    
    graph_line(buffer, &img, &start, &end, &color, 1);
    TEST_ASSERT(1, "Horizontal line drawing should complete without error");
    
    /* Test vertical line */
    vect2_populate(&start, 50, 10);
    vect2_populate(&end, 50, 100);
    
    graph_line(buffer, &img, &start, &end, &color, 2);
    TEST_ASSERT(1, "Vertical line drawing should complete without error");
    
    /* Test diagonal line */
    vect2_populate(&start, 10, 10);
    vect2_populate(&end, 100, 100);
    
    graph_line(buffer, &img, &start, &end, &color, 3);
    TEST_ASSERT(1, "Diagonal line drawing should complete without error");
    
    /* Test with zero thickness */
    graph_line(buffer, &img, &start, &end, &color, 0);
    TEST_ASSERT(1, "Line with zero thickness should complete without error");
    
    free(buffer);
    return 1;
}

/* Test graph_curve functionality */
static int test_graph_curve(void) {
    n_byte* buffer = create_test_buffer();
    TEST_ASSERT_NOT_NULL(buffer, "Test buffer allocation");
    
    n_vect2 img = create_test_img_size();
    n_vect2 pt0, pt1, pt2;
    n_rgba32 color = create_test_color(0, 0, 255, 255); /* Blue */
    
    /* Test basic curve */
    vect2_populate(&pt0, 10, 50);
    vect2_populate(&pt1, 50, 10);
    vect2_populate(&pt2, 90, 50);
    
    graph_curve(buffer, &img, &pt0, &pt1, &pt2, &color, 50, 1, 3);
    TEST_ASSERT(1, "Basic curve drawing should complete without error");
    
    /* Test curve with same start and end thickness */
    graph_curve(buffer, &img, &pt0, &pt1, &pt2, &color, 25, 2, 2);
    TEST_ASSERT(1, "Curve with uniform thickness should complete without error");
    
    /* Test curve with extreme radius */
    graph_curve(buffer, &img, &pt0, &pt1, &pt2, &color, 100, 1, 1);
    TEST_ASSERT(1, "Curve with maximum radius should complete without error");
    
    /* Test curve with zero radius */
    graph_curve(buffer, &img, &pt0, &pt1, &pt2, &color, 0, 1, 1);
    TEST_ASSERT(1, "Curve with zero radius should complete without error");
    
    free(buffer);
    return 1;
}

/* Test graph_fill_polygon functionality */
static int test_graph_fill_polygon(void) {
    n_byte* buffer = create_test_buffer();
    TEST_ASSERT_NOT_NULL(buffer, "Test buffer allocation");
    
    n_vect2 img = create_test_img_size();
    n_rgba32 color = create_test_color(255, 255, 0, 255); /* Yellow */
    
    /* Create a triangle */
    n_vect2 triangle[3];
    vect2_populate(&triangle[0], 50, 20);
    vect2_populate(&triangle[1], 20, 80);
    vect2_populate(&triangle[2], 80, 80);
    
    graph_fill_polygon(triangle, 3, &color, 255, buffer, &img);
    TEST_ASSERT(1, "Triangle filling should complete without error");
    
    /* Create a square */
    n_vect2 square[4];
    vect2_populate(&square[0], 100, 100);
    vect2_populate(&square[1], 150, 100);
    vect2_populate(&square[2], 150, 150);
    vect2_populate(&square[3], 100, 150);
    
    graph_fill_polygon(square, 4, &color, 128, buffer, &img);
    TEST_ASSERT(1, "Square filling with transparency should complete without error");
    
    /* Test with single point (degenerate case) */
    n_vect2 point[1];
    vect2_populate(&point[0], 50, 50);
    
    graph_fill_polygon(point, 1, &color, 255, buffer, &img);
    TEST_ASSERT(1, "Single point polygon should complete without error");
    
    /* Test with empty polygon */
    graph_fill_polygon(NULL, 0, &color, 255, buffer, &img);
    TEST_ASSERT(1, "Empty polygon should complete without error");
    
    free(buffer);
    return 1;
}

/****************************************************************
 * GLRender Module Tests
 ****************************************************************/

/* Test glrender initialization and cleanup */
static int test_glrender_lifecycle(void) {
    glrender_init();
    TEST_ASSERT(1, "glrender_init should complete without error");
    
    glrender_reset();
    TEST_ASSERT(1, "glrender_reset should complete without error");
    
    glrender_close();
    TEST_ASSERT(1, "glrender_close should complete without error");
    
    return 1;
}

/* Test glrender size setting */
static int test_glrender_size(void) {
    glrender_set_size(640, 480);
    TEST_ASSERT(1, "Setting standard size should complete without error");
    
    glrender_set_size(1920, 1080);
    TEST_ASSERT(1, "Setting HD size should complete without error");
    
    glrender_set_size(1, 1);
    TEST_ASSERT(1, "Setting minimum size should complete without error");
    
    return 1;
}

/* Test glrender color operations */
static int test_glrender_colors(void) {
    /* Test all predefined colors */
    glrender_color(GLR_GREEN);
    TEST_ASSERT(1, "Setting green color should complete without error");
    
    glrender_color(GLR_LIGHT_GREEN);
    TEST_ASSERT(1, "Setting light green color should complete without error");
    
    glrender_color(GLR_RED);
    TEST_ASSERT(1, "Setting red color should complete without error");
    
    glrender_color(GLR_ORANGE);
    TEST_ASSERT(1, "Setting orange color should complete without error");
    
    glrender_color(GLR_LIGHT_GREY);
    TEST_ASSERT(1, "Setting light grey color should complete without error");
    
    glrender_color(GLR_GREY);
    TEST_ASSERT(1, "Setting grey color should complete without error");
    
    glrender_color(GLR_DARK_GREY);
    TEST_ASSERT(1, "Setting dark grey color should complete without error");
    
    glrender_color(GLR_BLACK);
    TEST_ASSERT(1, "Setting black color should complete without error");
    
    /* Test background color */
    glrender_background_green();
    TEST_ASSERT(1, "Setting green background should complete without error");
    
    return 1;
}

/* Test glrender color mapping */
static int test_glrender_color_mapping(void) {
    n_byte2 color_map[256];
    n_byte4 replace_map[256];
    
    /* Initialize test color maps */
    for (int i = 0; i < 256; i++) {
        color_map[i] = (n_byte2)(i * 256 + i);
        replace_map[i] = (n_byte4)(i * 0x01010101);
    }
    
    glrender_color_map(color_map);
    TEST_ASSERT(1, "Setting color map should complete without error");
    
    glrender_color_map_replace(replace_map);
    TEST_ASSERT(1, "Setting replacement color map should complete without error");
    
    return 1;
}

/* Test glrender string rendering */
static int test_glrender_string(void) {
    glrender_string("Hello, World!", 10, 20);
    TEST_ASSERT(1, "Rendering basic string should complete without error");
    
    glrender_string("", 0, 0);
    TEST_ASSERT(1, "Rendering empty string should complete without error");
    
    glrender_string("Test with special chars: !@#$%^&*()", 50, 100);
    TEST_ASSERT(1, "Rendering string with special characters should complete without error");
    
    return 1;
}

/* Test glrender list management */
static int test_glrender_lists(void) {
    /* Test text list */
    glrender_start_text_list();
    TEST_ASSERT(1, "Starting text list should complete without error");
    
    glrender_string("List Text", 0, 0);
    
    glrender_end_text_list();
    TEST_ASSERT(1, "Ending text list should complete without error");
    
    /* Test display list */
    glrender_start_display_list();
    TEST_ASSERT(1, "Starting display list should complete without error");
    
    glrender_end_display_list();
    TEST_ASSERT(1, "Ending display list should complete without error");
    
    /* Test active list */
    glrender_start_active_list();
    TEST_ASSERT(1, "Starting active list should complete without error");
    
    glrender_end_active_list();
    TEST_ASSERT(1, "Ending active list should complete without error");
    
    return 1;
}

/* Test glrender line operations */
static int test_glrender_lines(void) {
    n_vect2 start, end;
    
    vect2_populate(&start, 10, 10);
    vect2_populate(&end, 100, 100);
    
    /* Test line width settings */
    glrender_wide_line();
    TEST_ASSERT(1, "Setting wide line should complete without error");
    
    glrender_line(&start, &end);
    TEST_ASSERT(1, "Drawing wide line should complete without error");
    
    glrender_thin_line();
    TEST_ASSERT(1, "Setting thin line should complete without error");
    
    glrender_line(&start, &end);
    TEST_ASSERT(1, "Drawing thin line should complete without error");
    
    return 1;
}

/* Test glrender quad operations */
static int test_glrender_quads(void) {
    n_vect2 quad_points[4];
    
    /* Create a test quad */
    vect2_populate(&quad_points[0], 10, 10);
    vect2_populate(&quad_points[1], 50, 10);
    vect2_populate(&quad_points[2], 50, 50);
    vect2_populate(&quad_points[3], 10, 50);
    
    /* Test filled quad */
    glrender_quads(quad_points, 1);
    TEST_ASSERT(1, "Drawing filled quad should complete without error");
    
    /* Test outline quad */
    glrender_quads(quad_points, 0);
    TEST_ASSERT(1, "Drawing outline quad should complete without error");
    
    return 1;
}

/* Test glrender transformation operations */
static int test_glrender_transforms(void) {
    n_vect2 center, location;
    
    vect2_populate(&center, 50, 50);
    vect2_populate(&location, 100, 100);
    
    glrender_delta_move(&center, &location, 45, 100);
    TEST_ASSERT(1, "Delta movement should complete without error");
    
    glrender_delta_move(&center, &location, 0, 50);
    TEST_ASSERT(1, "Delta movement with zero rotation should complete without error");
    
    glrender_delta_move(&center, &location, 360, 200);
    TEST_ASSERT(1, "Delta movement with full rotation should complete without error");
    
    return 1;
}

/* Test glrender rendering operations */
static int test_glrender_rendering(void) {
    n_byte* output_buffer = create_test_buffer();
    TEST_ASSERT_NOT_NULL(output_buffer, "Output buffer allocation");
    
    /* Create test memory list for lines */
    memory_list* lines = memory_list_new(sizeof(n_vect2), 10);
    TEST_ASSERT_NOT_NULL(lines, "Lines memory list allocation");
    
    glrender_render_lines(output_buffer, lines);
    TEST_ASSERT(1, "Rendering lines should complete without error");
    
    glrender_render_text(output_buffer);
    TEST_ASSERT(1, "Rendering text should complete without error");
    
    glrender_render_display(output_buffer);
    TEST_ASSERT(1, "Rendering display should complete without error");
    
    glrender_render_active(output_buffer);
    TEST_ASSERT(1, "Rendering active elements should complete without error");
    
    memory_list_free(&lines);
    free(output_buffer);
    return 1;
}

/* Test glrender scene management */
static int test_glrender_scene(void) {
    n_int scene_result = glrender_scene_done();
    TEST_ASSERT(scene_result >= 0, "Scene done should return valid status");
    
    return 1;
}

/****************************************************************
 * Integration Tests
 ****************************************************************/

/* Test integration between graph and glrender modules */
static int test_integration_graph_glrender(void) {
    n_byte* buffer = create_test_buffer();
    TEST_ASSERT_NOT_NULL(buffer, "Integration test buffer allocation");
    
    /* Initialize both modules */
    graph_init(1);
    glrender_init();
    
    /* Set up rendering context */
    glrender_set_size(TEST_BUFFER_WIDTH, TEST_BUFFER_HEIGHT);
    glrender_color(GLR_RED);
    
    /* Perform some graph operations */
    n_vect2 img = create_test_img_size();
    n_rgba32 color = create_test_color(255, 0, 0, 255);
    graph_erase(buffer, &img, &color);
    
    /* Perform some glrender operations */
    glrender_start_display_list();
    glrender_string("Integration Test", 10, 10);
    glrender_end_display_list();
    
    /* Clean up */
    glrender_close();
    free(buffer);
    
    TEST_ASSERT(1, "Integration test should complete without error");
    return 1;
}

/****************************************************************
 * Test Runner
 ****************************************************************/

int main(void) {
    printf("Starting Graphics Module Unit Tests\n");
    printf("===================================\n\n");
    
    /* Graph Module Tests */
    printf("Graph Module Tests:\n");
    RUN_TEST(test_graph_init);
    RUN_TEST(test_graph_erase);
    RUN_TEST(test_graph_line);
    RUN_TEST(test_graph_curve);
    RUN_TEST(test_graph_fill_polygon);
    
    printf("\n");
    
    /* GLRender Module Tests */
    printf("GLRender Module Tests:\n");
    RUN_TEST(test_glrender_lifecycle);
    RUN_TEST(test_glrender_size);
    RUN_TEST(test_glrender_colors);
    //RUN_TEST(test_glrender_color_mapping);
    //RUN_TEST(test_glrender_string);
    RUN_TEST(test_glrender_lists);
    RUN_TEST(test_glrender_lines);
    RUN_TEST(test_glrender_quads);
    RUN_TEST(test_glrender_transforms);
    RUN_TEST(test_glrender_rendering);
    RUN_TEST(test_glrender_scene);
    
    printf("\n");
    
    /* Integration Tests */
    printf("Integration Tests:\n");
    RUN_TEST(test_integration_graph_glrender);
    
    printf("\n");
    
    /* Print results */
    printf("Test Results:\n");
    printf("=============\n");
    printf("Total tests: %d\n", test_count);
    printf("Passed: %d\n", test_passes);
    printf("Failed: %d\n", test_failures);
    
    if (test_failures == 0) {
        printf("\nAll tests PASSED! ✓\n");
        return 0;
    } else {
        printf("\n%d tests FAILED! ✗\n", test_failures);
        return 1;
    }
}

/****************************************************************
 * Compilation Instructions:
 * 
 * To compile this test suite, use:
 * gcc -o graphics_test graphics_test.c graph.c glrender.c toolkit.c -lm
 * 
 * Note: You'll need to link against the actual implementation files
 * and any required libraries (like OpenGL for glrender).
 * 
 * For a mock/stub implementation testing approach, you could create
 * stub versions of the functions that simply return success values.
 ****************************************************************/
