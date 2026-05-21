/****************************************************************

 entity_tests.c

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
#include "../../universe/universe.h"
#include "../entity.h"

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

static void make_being(simulated_being *being, n_byte first, n_byte family0, n_byte family1, n_int female)
{
    n_byte2 location[2] = { 1234, 5678 };

    memory_erase((n_byte *)being, sizeof(simulated_being));
    being->constant.name[0] = first;
    being->constant.genetics[CHROMOSOME_Y] = female ? SEX_FEMALE : 0;
    being_set_family_name(being, family0, family1);
    being_set_location(being, location);
    being_living(being);
    being_set_height(being, BIRTH_HEIGHT);
    being->delta.mass = BIRTH_MASS;
    being->delta.awake = FULLY_AWAKE;
    being->delta.crowding = MIN_CROWDING;
}

static void test_being_state_energy_and_names(void)
{
    simulated_being being;
    n_string_block name;
    n_string_block state;
    n_byte2 location[2] = { 42, 84 };

    make_being(&being, 2, 3, 4, 1);

    TEST_ASSERT(being_energy(&being) == BEING_FULL, "being_living sets full energy");
    being_energy_delta(&being, 0 - (BEING_FULL * 2));
    TEST_ASSERT(being_energy(&being) == BEING_DEAD, "being_energy_delta clamps at dead");
    being_living(&being);
    TEST_ASSERT(being_energy_less_than(&being, BEING_HUNGRY) == 0, "being_energy_less_than reports full energy");

    being_set_state(&being, BEING_STATE_AWAKE);
    being_add_state(&being, BEING_STATE_MOVING);
    TEST_ASSERT((being_state(&being) & BEING_STATE_MOVING) != 0, "being_add_state preserves new state bits");
    memory_erase((n_byte *)state, sizeof(state));
    being_state_description(being_state(&being), state);
    TEST_ASSERT(strstr((char *)state, "Moving") != 0L, "being_state_description names moving state");

    being_set_location(&being, location);
    TEST_ASSERT(being_location_x(&being) == 42, "being_location_x returns set x");
    TEST_ASSERT(being_location_y(&being) == 84, "being_location_y returns set y");

    being_name_simple(&being, name);
    TEST_ASSERT(strlen((char *)name) > 0, "being_name_simple returns a display name");
    TEST_ASSERT(being_family_first_name(&being) == 3, "being_family_first_name returns packed first family");
    TEST_ASSERT(being_family_second_name(&being) == 4, "being_family_second_name returns packed second family");
    TEST_ASSERT(being_name_comparison(&being, being_gender_name(&being), being_family_name(&being)) != 0, "being_name_comparison matches own name");
}

static void test_body_drives_and_attention(void)
{
    simulated_being being;

    make_being(&being, 4, 5, 6, 0);

    being_take(&being, BODY_LEFT_HAND, INVENTORY_BRANCH);
    TEST_ASSERT(being_carried(&being, BODY_LEFT_HAND) == INVENTORY_BRANCH, "being_take stores body inventory");
    TEST_ASSERT(being_attention(&being, ATTENTION_BODY) == BODY_LEFT_HAND, "being_take updates body attention");
    being_drop(&being, BODY_LEFT_HAND);
    TEST_ASSERT(being_carried(&being, BODY_LEFT_HAND) == 0, "being_drop clears body inventory");

    being_reset_drive(&being, DRIVE_HUNGER);
    being_inc_drive(&being, DRIVE_HUNGER);
    TEST_ASSERT(being_drive(&being, DRIVE_HUNGER) == 1, "being_inc_drive increments hunger");
    being_dec_drive(&being, DRIVE_HUNGER);
    TEST_ASSERT(being_drive(&being, DRIVE_HUNGER) == 0, "being_dec_drive decrements hunger");

    being_dead(&being);
    drives_hunger(&being);
    TEST_ASSERT(being_drive(&being, DRIVE_HUNGER) == 1, "drives_hunger increases hunger when energy is low");

    being_living(&being);
    drives_hunger(&being);
    TEST_ASSERT(being_drive(&being, DRIVE_HUNGER) == 0, "drives_hunger decreases hunger when energy is high");

    being_set_speed(&being, FATIGUE_SPEED_THRESHOLD + 1);
    drives_fatigue(&being);
    TEST_ASSERT(being_drive(&being, DRIVE_FATIGUE) == 1, "drives_fatigue increases fatigue above speed threshold");
}

static void test_brain_body_genome_and_immune(void)
{
    simulated_being being;
    n_byte instruction[3] = { BRAINCODE_ADD, 1, 2 };
    n_string_block command;
    n_string_block sentence;
    n_genetics genome[CHROMOSOMES] = {0, 0, 0, 0};
    n_byte genome_string[64];
    simulated_immune_system immune;
    n_byte2 random[2] = { 0x6727, 0xfd31 };
    n_int loop;

    make_being(&being, 7, 8, 9, 1);

    memory_erase((n_byte *)command, sizeof(command));
    memory_erase((n_byte *)sentence, sizeof(sentence));
    brain_three_byte_command(command, instruction);
    brain_sentence(sentence, instruction);
    TEST_ASSERT(strstr((char *)command, "ADD") != 0L, "brain_three_byte_command formats ADD mnemonic");
    TEST_ASSERT(strlen((char *)sentence) > 0, "brain_sentence returns spoken text");

    body_genome(1, genome, genome_string);
    TEST_ASSERT(strlen((char *)genome_string) == (CHROMOSOMES * 8), "body_genome returns one letter per gene pair");
    for (loop = 0; loop < (CHROMOSOMES * 8); loop++)
    {
        TEST_ASSERT(strchr("ATCG", genome_string[loop]) != 0L, "body_genome only emits nucleotide letters");
    }

    immune_init(&immune, random);
    TEST_ASSERT(immune_response(&immune, being_honor_immune(&being), being_energy(&being)) == 0, "immune_response is clear without pathogens");
}

static void test_episodic_and_social(void)
{
    simulated_group *group;
    simulated_being *first;
    simulated_being *second;
    n_int relationship_index;
    simulated_iepisodic *memory;

    group = (simulated_group *)memory_new(sizeof(simulated_group));
    TEST_ASSERT(group != 0L, "simulated_group test fixture allocates");
    if (group == 0L)
    {
        return;
    }

    memory_erase((n_byte *)group, sizeof(simulated_group));
    group->num = 2;
    group->max = LARGE_SIM;
    first = &group->beings[0];
    second = &group->beings[1];

    make_being(first, 10, 11, 12, 0);
    make_being(second, 13, 14, 15, 1);

    episodic_self(first, EVENT_EAT, AFFECT_CHAT, INVENTORY_GRASS);
    memory = being_episodic(first);
    TEST_ASSERT(memory[0].event == EVENT_EAT, "episodic_self stores an event");
    TEST_ASSERT(memory[0].arg == INVENTORY_GRASS, "episodic_self stores the event argument");
    TEST_ASSERT(episodic_first_person_memories_percent(first, 0) == 100, "episodic first-person percentage sees self event");

    relationship_index = social_set_relationship(group, first, RELATIONSHIP_SISTER, second);
    TEST_ASSERT(relationship_index > 0, "social_set_relationship creates a social graph entry");
    TEST_ASSERT(social_get_relationship(first, RELATIONSHIP_SISTER) == relationship_index, "social_get_relationship finds stored relationship");
    TEST_ASSERT(social_respect_mean(first) > 0, "social_respect_mean reports populated graph");

    memory_free((void **)&group);
}

int main(void)
{
    printf("Entity unit tests\n");

    test_being_state_energy_and_names();
    test_body_drives_and_attention();
    test_brain_body_genome_and_immune();
    test_episodic_and_social();

    printf("\nTests run: %d\n", tests_run);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);

    return tests_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
