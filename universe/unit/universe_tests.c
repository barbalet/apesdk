/****************************************************************

 universe_tests.c

 =============================================================

 Copyright 1996-2026 Tom Barbalet. All rights reserved.

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

 ****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../toolkit/toolkit.h"
#include "../../sim/sim.h"
#include "../../entity/entity.h"
#include "../universe.h"

static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

n_int draw_error(n_constant_string error_text, n_constant_string location, n_int line_number)
{
    printf("ERROR: %s @ %s %ld\n", (const n_string) error_text, location, line_number);
    return -1;
}

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

static n_uint group_digest(void)
{
    simulated_group *group = sim_group();
    n_uint digest = group->num;
    n_uint loop;
    n_uint max = group->num < 8 ? group->num : 8;

    for (loop = 0; loop < max; loop++)
    {
        simulated_being *being = &group->beings[loop];
        digest ^= math_hash((n_byte *)&being->delta, sizeof(simulated_being_delta)) + (digest << 6) + (digest >> 2);
        digest ^= math_hash((n_byte *)&being->constant, sizeof(simulated_being_constant)) + (digest << 6) + (digest >> 2);
    }
    return digest;
}

static n_uint run_deterministic_digest(void)
{
    n_int loop;

    if (sim_init(KIND_START_UP, 0x12738291, MAP_AREA, 0) == 0L)
    {
        return 0;
    }

    for (loop = 0; loop < 4; loop++)
    {
        sim_cycle();
    }

    {
        n_uint digest = group_digest();
        sim_close();
        return digest;
    }
}

static void test_sim_cycle_is_deterministic(void)
{
    n_uint first = run_deterministic_digest();
    n_uint second = run_deterministic_digest();

    TEST_ASSERT(first != 0, "deterministic digest has simulation data");
    TEST_ASSERT(first == second, "sim_cycle is deterministic for a fixed seed");
}

static void test_save_load_and_transfer(void)
{
    simulated_group *group;
    n_uint before_count;
    n_file *binary;
    n_file *json;

    TEST_ASSERT(sim_init(KIND_START_UP, 0x12738291, MAP_AREA, 0) != 0L, "sim_init allocates simulation memory");
    sim_cycle();
    group = sim_group();
    before_count = group->num;

    binary = tranfer_out();
    TEST_ASSERT(binary != 0L, "tranfer_out returns a file");
    TEST_ASSERT(binary->location > 0, "tranfer_out writes bytes");

    json = tranfer_out_json();
    TEST_ASSERT(json != 0L, "tranfer_out_json returns a file");
    TEST_ASSERT(json->location > 0, "tranfer_out_json writes bytes");

    TEST_ASSERT(tranfer_in(binary) == 0, "tranfer_in reloads binary transfer");
    TEST_ASSERT(sim_group()->num == before_count, "transfer round trip preserves being count");
    TEST_ASSERT(sim_group()->select == sim_group()->beings, "transfer round trip selects first being");

    io_file_free(&binary);
    io_file_free(&json);
    sim_close();
}

static void test_command_parsing(void)
{
    n_int number = 0;
    n_int interval = -1;

    TEST_ASSERT(get_time_interval("30 mins", &number, &interval) == 0, "get_time_interval accepts minutes");
    TEST_ASSERT(number == 30 && interval == INTERVAL_MINS, "get_time_interval parses minutes");

    number = 0;
    interval = -1;
    TEST_ASSERT(get_time_interval("2 hours", &number, &interval) == 0, "get_time_interval accepts hours");
    TEST_ASSERT(number == 2 && interval == INTERVAL_HOURS, "get_time_interval parses hours");

    number = 0;
    interval = -1;
    TEST_ASSERT(get_time_interval("1 M", &number, &interval) == 0, "get_time_interval accepts month shorthand");
    TEST_ASSERT(number == 1 && interval == INTERVAL_MONTHS, "get_time_interval distinguishes month shorthand");
}

static void test_selection_and_control(void)
{
    simulated_group *group;
    n_byte2 before_y;

    TEST_ASSERT(sim_init(KIND_START_UP, 0x12738291, MAP_AREA, 0) != 0L, "sim_init starts selection test");
    sim_cycle();
    group = sim_group();

    TEST_ASSERT(group->num > 1, "simulation starts with multiple beings");
    sim_set_select(&group->beings[0]);
    TEST_ASSERT(group->select == &group->beings[0], "sim_set_select updates selected being");
    sim_change_selected(1);
    TEST_ASSERT(group->select == &group->beings[1], "sim_change_selected advances selected being");
    sim_change_selected(0);
    TEST_ASSERT(group->select == &group->beings[0], "sim_change_selected reverses selected being");

    sim_view_options(11, 17);
    TEST_ASSERT(being_location_x(group->select) == APESPACE_CONFINED(MAPSPACE_TO_APESPACE(11)), "sim_view_options updates selected x");
    TEST_ASSERT(being_location_y(group->select) == APESPACE_CONFINED(MAPSPACE_TO_APESPACE(17)), "sim_view_options updates selected y");

    before_y = (n_byte2)being_location_y(group->select);
    sim_move(1, 2);
    TEST_ASSERT(being_location_y(group->select) != before_y, "sim_move updates selected location");

    sim_control_erase(256, 256, 16);
    sim_control_set(16, 16, 1, 'A');
    sim_control_regular(16, 16);
    TEST_ASSERT(group->select != 0L, "sim_control APIs keep a valid selection");

    sim_close();
}

int main(void)
{
    printf("Universe unit tests\n");

    test_sim_cycle_is_deterministic();
    test_save_load_and_transfer();
    test_command_parsing();
    test_selection_and_control();

    printf("\nTests run: %d\n", tests_run);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);

    return tests_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
