/****************************************************************

 Monkeymind - an experimental cogitive architecture

 =============================================================

 Copyright 2013-2014 Bob Mottram

 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or
 sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the followingp
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

#include "monkeymind_unittests.h"

static void test_id()
{
    mm_random_seed seed;
    mm_id id, id2;
    n_uint i, itt;

    printf("test_id...");

    /* Set a random seed */
    mm_rand_init(&seed, 0,1,2,3);

    /* create an id */
    mm_id_create(&seed, &id);

    /* test that the id exists */
    for (i = 0; i < MM_ID_LENGTH; i++) {
        assert(mm_id_get(&id, i) != 0);
    }
    assert(mm_id_exists(&id));

    /* check that the same id isn't created repeatedly */
    for (itt = 0; itt < 10; itt++) {
        mm_id_create(&seed, &id2);
        if (mm_id_equals(&id, &id2)) {
            printf("\n(itt %d)\n  ", (int)itt);
            for (i = 0; i < MM_ID_LENGTH; i++) {
                printf("%d ", (int)mm_id_get(&id, i));
            }
            printf("\n");
            for (i = 0; i < MM_ID_LENGTH; i++) {
                printf("%d ", (int)mm_id_get(&id2, i));
            }
            printf("\n");
        }
        assert(!mm_id_equals(&id, &id2));
    }

    /* clear the id */
    mm_id_clear(&id);

    /* check that it has been cleared */
    for (i = 0; i < MM_ID_LENGTH; i++) {
        assert(mm_id_get(&id, i) == 0);
    }
    assert(!mm_id_exists(&id));

    printf("Ok\n");
}

static void test_init()
{
    monkeymind mind;

    printf("test_init...");

    mm_init(&mind, MM_SEX_FEMALE, 3, 6, NULL);
    printf("Ok\n");
}

static void test_object_id()
{
    mm_object obj;
    mm_id id, test_id;
    mm_random_seed seed;
    n_uint i;

    printf("test_object_id...");

    /* Set a random seed */
    mm_rand_init(&seed, 0,1,2,3);

    /* create an id */
    mm_id_create(&seed, &id);
    mm_id_clear(&test_id);

    /* test that the id exists */
    for (i = 0; i < MM_ID_LENGTH; i++) {
        assert(mm_id_get(&id, i) != 0);
    }
    assert(mm_id_exists(&id));

    /* add the id as an object property */
    mm_obj_init(&obj);
    mm_obj_prop_add_id(&obj,
                       MM_PROPERTY_MEETER, &id);

    /* check that the id is returned */
    mm_obj_prop_get_id(&obj,
                       MM_PROPERTY_MEETER, &test_id);

    if (!mm_id_equals(&id, &test_id)) {
        printf("\nid =");
        for (i = 0; i < MM_ID_LENGTH; i++) {
            printf(" %d", (int)id.value[i]);
        }
        printf("\nreturned id =");
        for (i = 0; i < MM_ID_LENGTH; i++) {
            printf(" %d", (int)test_id.value[i]);
        }
        printf("\n");
    }
    assert(mm_id_equals(&id, &test_id));

    printf("Ok\n");
}

static void test_spatial()
{
    monkeymind mind;
    n_int i;

    printf("test_spatial...");

    mm_init(&mind, MM_SEX_FEMALE,3,6,NULL);

    for (i = 0; i < MM_SIZE_SPATIAL*MM_SIZE_SPATIAL; i++) {
        assert(mm_id_get(&mind.spatial[i].id,0) == i);
    }

    printf("Ok\n");
}

static void test_object_add_remove_properties()
{
    mm_object obj;
    n_uint i, prop_value;
    n_uint props[] = {
        3,6268,
        7,3568,
        1,56437,
        99,6732,
        40,1357,
        42,6267,
        23,536774,
        50,2435,
        49,23566,
        12,2356
    };

    printf("test_object_add_remove_properties...");

    /* Clear */
    obj.length = 0;
    for (i = 0; i < 10; i++) {
        obj.property_type[i] = 0;
        obj.property_value[i] = 0;
    }

    /* Add the properties */
    for (i = 0; i < 10; i++) {
        mm_obj_prop_add(&obj,props[i*2],props[i*2+1]);
        prop_value = mm_obj_prop_get(&obj,props[i*2]);
        assert(props[i*2+1] == prop_value);
    }


    /* Check that 10 properties were added */
    assert(obj.length == 10);

    /* Check that values are non-zero */
    for (i = 0; i < 10; i++) {
        assert(obj.property_type[i] > 0);
        assert(obj.property_value[i] > 0);
    }

    /* Check that the property types are in order */
    for (i = 1; i < 10; i++) {
        assert(obj.property_type[i-1] <= obj.property_type[i]);
    }

    /* Property 23 exists in the list */
    assert(mm_obj_prop_index(&obj,23) > -1);

    /* Remove it */
    mm_obj_prop_remove(&obj,23);

    /* Check that the list size is reduced */
    assert(obj.length = 9);

    /* Check that property 23 no longer exists */
    assert(mm_obj_prop_index(&obj,23) == -1);

    printf("Ok\n");
}

static void test_name()
{
    n_uint first_name = 10;
    n_uint sex = MM_SEX_FEMALE;
    n_uint surname = 125;
    n_uint name;
    n_uint extract_first_name, extract_surname, extract_sex;

    printf("test_name...");

    name = MM_NAME(sex,first_name,surname);

    extract_first_name = MM_FIRST_NAME(name);
    assert(extract_first_name == first_name);

    extract_sex = MM_SEX(name);
    assert(extract_sex == sex);

    extract_surname = MM_SURNAME(name);
    assert(extract_surname == surname);

    printf("Ok\n");
}

static void test_social_meet()
{
    monkeymind m0, m1, m2;
    n_int i;
    n_uint name;
    mm_object * g;
    mm_random_seed seed_m0, seed_m1, seed_m2;
    mm_id test_id;

    printf("test_social_meet...");

    /* Set a random seed */
    mm_rand_init(&seed_m0, 0,1,2,3);
    mm_rand_init(&seed_m1, 5,6,2,8);
    mm_rand_init(&seed_m2, 9,4,6,1);

    /* Create three agents */
    mm_init(&m0, MM_SEX_MALE, 10,20, &seed_m0);
    mm_init(&m1, MM_SEX_FEMALE, 11,31, &seed_m1);
    mm_init(&m2, MM_SEX_FEMALE, 7,8, &seed_m2);

    /* Get the name of the first agent */
    name = mm_get_property(&m0, MM_PROPERTY_NAME);
    /* Check that it's what we expect */
    assert(MM_FIRST_NAME(name) == 10);
    assert(MM_SURNAME(name) == 20);

    /* Get the name of the second agent */
    name = mm_get_property(&m1, MM_PROPERTY_NAME);
    /* Check that it's what we expect */
    assert(MM_FIRST_NAME(name) == 11);
    assert(MM_SURNAME(name) == 31);

    /* Get the name of the third agent */
    name = mm_get_property(&m2, MM_PROPERTY_NAME);
    /* Check that it's what we expect */
    assert(MM_FIRST_NAME(name) == 7);
    assert(MM_SURNAME(name) == 8);

    /* check that the IDs are different */
    assert(!mm_id_equals(&m0.id,&m1.id));
    assert(!mm_id_equals(&m0.id,&m2.id));
    assert(!mm_id_equals(&m1.id,&m2.id));

    /* First agent meets the second agent */
    mm_social_meet(&m0,&m1);
    /* First agent meets the third agent */
    mm_social_meet(&m0,&m2);
    /* First agent meets the second agent again */
    mm_social_meet(&m0,&m1);

    /* Check that two social graph entries have been made within
       the first agent, and that other entries are blank.
       Note that the first entry in the graph is the self. */
    for (i = MM_SELF+1; i < MM_SIZE_SOCIAL_GRAPH; i++) {
        if (i <= 3) {
            assert(mm_social_graph_entry_exists(&m0, i));
        }
        else {
            assert(!mm_social_graph_entry_exists(&m0, i));
        }
    }

    /* Get the first non-self social graph entry */
    g = &m0.social_graph[MM_SELF+1];
    /* Verify that the properties are what we expect */
    mm_id_clear(&test_id);
    mm_obj_prop_get_id(g,MM_PROPERTY_MET, &test_id);
    assert(mm_id_equals(&test_id,&m1.id));

    mm_id_clear(&test_id);
    mm_obj_prop_get_id(g,MM_PROPERTY_MEETER, &test_id);
    assert(mm_id_equals(&test_id,&m0.id));

    name = mm_obj_prop_get(g,MM_PROPERTY_MET_NAME);
    assert(MM_FIRST_NAME(name) == 11);
    assert(MM_SURNAME(name) == 31);
    name = mm_obj_prop_get(g,MM_PROPERTY_MEETER_NAME);
    assert(MM_FIRST_NAME(name) == 10);
    assert(MM_SURNAME(name) == 20);

    /* Get the second non-self social graph entry */
    g = &m0.social_graph[MM_SELF+2];
    /* Verify that the properties are what we expect */
    mm_id_clear(&test_id);
    mm_obj_prop_get_id(g,MM_PROPERTY_MET, &test_id);
    assert(mm_id_equals(&test_id,&m2.id));

    mm_id_clear(&test_id);
    mm_obj_prop_get_id(g,MM_PROPERTY_MEETER, &test_id);
    assert(mm_id_equals(&test_id,&m0.id));

    name = mm_obj_prop_get(g,MM_PROPERTY_MEETER_NAME);
    assert(MM_FIRST_NAME(name) == 10);
    assert(MM_SURNAME(name) == 20);
    name = mm_obj_prop_get(g,MM_PROPERTY_MET_NAME);
    assert(MM_FIRST_NAME(name) == 7);
    assert(MM_SURNAME(name) == 8);

    printf("Ok\n");
}

static void test_som()
{
    mm_som som;
    mm_random_seed seed;
    n_int i,j;
    n_uint dimension = 32;
    n_uint vector_length = 16;
    n_uint learning_radius = 4;
    n_byte vector[16];
    n_uint x, y;

    printf("test_som...");

    /* Set a random seed */
    mm_rand_init(&seed, 0,1,2,3);

    /* Initialise the SOM */
    mm_som_init(&som,
                dimension, vector_length,
                learning_radius, &seed);

    /* SOM learns randomly */
    for (i = 0; i < 10000; i++) {
        for (j = 0; j < vector_length; j++) {
            vector[j] = mm_rand(&seed)&255;
        }
        mm_som_update(&som, vector, &x, &y);
        mm_som_learn(&som, vector, x, y);
    }

    printf("Ok\n");
}

static void test_communicate_social_categorisation()
{
    monkeymind m0, m1;
    n_int c, i, ctr;
    mm_random_seed seed_m0, seed_m1;

    printf("test_communicate_social_categorisation...");

    /* Set a random seeds */
    mm_rand_init(&seed_m0, 0,1,2,3);
    mm_rand_init(&seed_m1, 5,6,2,8);

    /* Initialise two agents */
    mm_init(&m0, MM_SEX_MALE, 10,20, &seed_m0);
    mm_init(&m1, MM_SEX_FEMALE, 11,31, &seed_m1);

    /* The two agents meet */
    mm_social_meet(&m0,&m1);

    /* Set the social categories to some known values for both agents */
    for (c = 0; c < MM_CATEGORIES; c++) {
        for (i = 0; i < MM_SOCIAL_CATEGORIES_DIMENSION*
                 MM_SOCIAL_CATEGORIES_DIMENSION; i++) {
            m0.category[c].value[i] = 10;
            m1.category[c].value[i] = 20;
        }
    }

    /* The first agent communicates its social categories to the second agent */
    mm_communicate_social_categorisation(&m0, 1, &m1, 1);

    /* Check that the social categories for the second agent have
       become more similar to those of the first agent */
    ctr = 0;
    for (c = 0; c < MM_CATEGORIES; c++) {
        for (i = 0; i < MM_SOCIAL_CATEGORIES_DIMENSION*
                 MM_SOCIAL_CATEGORIES_DIMENSION; i++) {
            /* Should have reduced by one or two */
            if (m1.category[c].value[i] < 20) {
                ctr++; /* Count the number of changed categories */
                assert(m1.category[c].value[i] >= 18);
            }
        }
    }

    /* Check the total number of changed social categories for the second agent
       is what we expect */
    assert(ctr > MM_SOCIAL_CATEGORIES_RADIUS*2*
           MM_CATEGORIES);
    assert(ctr < MM_SOCIAL_CATEGORIES_RADIUS*
           MM_SOCIAL_CATEGORIES_RADIUS*4*MM_CATEGORIES);

    /* Set the social categories to some known values for both agents,
       in the reverse order to previously.  This will check that
       alterations to categories work in both positive and negative directions. */
    for (c = 0; c < MM_CATEGORIES; c++) {
        for (i = 0; i < MM_SOCIAL_CATEGORIES_DIMENSION*
                 MM_SOCIAL_CATEGORIES_DIMENSION; i++) {
            m0.category[c].value[i] = 20;
            m1.category[c].value[i] = 10;
        }
    }

    /* The first agent communicates its social categories to the second agent
       with positive friendliness */
    mm_communicate_social_categorisation(&m0, 1, &m1, 1);

    /* Check that the social categories for the second agent have
       become more similar to those of the first agent */
    ctr = 0;
    for (c = 0; c < MM_CATEGORIES; c++) {
        for (i = 0; i < MM_SOCIAL_CATEGORIES_DIMENSION*
                 MM_SOCIAL_CATEGORIES_DIMENSION; i++) {
            /* Should have increased by one or two */
            if (m1.category[c].value[i] > 10) {
                ctr++; /* Count the number of changed categories */
                assert(m1.category[c].value[i] <= 12);
            }
        }
    }

    /* Check the total number of changed social categories for the second agent
       is what we expect */
    assert(ctr > MM_SOCIAL_CATEGORIES_RADIUS*2*
           MM_CATEGORIES);
    assert(ctr < MM_SOCIAL_CATEGORIES_RADIUS*
           MM_SOCIAL_CATEGORIES_RADIUS*4*MM_CATEGORIES);

    /* Set the social categories to some known values for both agents,
       in the reverse order to previously.  This will check that
       alterations to categories work in both positive and negative directions. */
    for (c = 0; c < MM_CATEGORIES; c++) {
        for (i = 0; i < MM_SOCIAL_CATEGORIES_DIMENSION*
                 MM_SOCIAL_CATEGORIES_DIMENSION; i++) {
            m0.category[c].value[i] = 20;
            m1.category[c].value[i] = 10;
        }
    }

    /* The first agent communicates its social categories to the second agent
       with negative friendliness*/
    mm_communicate_social_categorisation(&m0, 1, &m1, 0);

    /* Check that the social categories for the second agent have
       become more similar to those of the first agent */
    for (c = 0; c < MM_CATEGORIES; c++) {
        for (i = 0; i < MM_SOCIAL_CATEGORIES_DIMENSION*
                 MM_SOCIAL_CATEGORIES_DIMENSION; i++) {
            /* Should have decreased by one or two */
            assert(m1.category[c].value[i] >= 8);
        }
    }

    printf("Ok\n");
}

static void test_episodic()
{
    n_int i;
    mm_object * test1, * test2;
    mm_episodic events;
    mm_object observation1, observation2;
    /* Some properties of the first event */
    n_uint props1[] = {
        3,6268,
        7,3568,
        1,56437,
        99,6732,
        40,1357,
        42,6267,
        23,536774,
        50,2435,
        49,23566,
        12,2356
    };
    /* Some properties of the first event */
    n_uint props2[] = {
        3,2521,
        7,7548,
        1,26357,
        99,8392,
        40,9357,
        42,867,
        23,36794,
        50,2235,
        49,22566,
        12,9756
    };

    printf("test_episodic...");

    /* Create an event sequence and some observation events
       to be inserted into it */
    mm_episodic_init(&events);
    mm_obj_init(&observation1);
    mm_obj_init(&observation2);

    /* Check that there are no events in the sequence */
    assert(mm_episodic_max(&events) == 0);
    /* Check that the observation events are empty */
    assert(!mm_obj_exists(&observation1));
    assert(!mm_obj_exists(&observation2));

    /* Add properties to the observation events */
    for (i = 0; i < 10; i++) {
        mm_obj_prop_add(&observation1,props1[i*2],props1[i*2+1]);
        mm_obj_prop_add(&observation2,props2[i*2],props2[i*2+1]);
    }
    /* Check that the observation events have some contents */
    assert(mm_obj_exists(&observation1));
    assert(mm_obj_exists(&observation2));

    /* Add the first event to the sequence */
    mm_episodic_add(&events,&observation1);
    /* Check that the sequence length increases */
    assert(mm_episodic_max(&events) == 1);
    /* Add the second event to the sequence */
    mm_episodic_add(&events,&observation2);
    /* Check that the sequence length increases */
    assert(mm_episodic_max(&events) == 2);

    /* Get the first and second events from the sequence */
    test1 = mm_episodic_get(&events, 0);
    test2 = mm_episodic_get(&events, 1);
    /* Check that there is no third event */
    assert(mm_episodic_get(&events, 2)==0);

    /* Check that the returned events are the same as the original observations */
    assert(mm_obj_cmp(&observation1, test1) == 0);
    assert(mm_obj_cmp(&observation2, test2) == 0);

    printf("Ok\n");
}

static void test_tale()
{
    n_int i;
    mm_object * test1, * test2;
    mm_tale tale;
    mm_object observation1, observation2;
    mm_random_seed seed;
    /* Some properties of the first observation */
    n_uint props1[] = {
        3,6268,
        7,3568,
        1,56437,
        99,6732,
        40,1357,
        42,6267,
        23,536774,
        50,2435,
        49,23566,
        12,2356
    };
    /* Some properties of the second observation */
    n_uint props2[] = {
        3,2521,
        7,7548,
        1,26357,
        99,8392,
        40,9357,
        42,867,
        23,36794,
        50,2235,
        49,22566,
        12,9756
    };

    printf("test_tale...");

    /* Set a random seed */
    mm_rand_init(&seed, 0,1,2,3);

    /* Create a tale and some observation steps to insert into it */
    mm_tale_init(&tale, &seed);
    mm_obj_init(&observation1);
    mm_obj_init(&observation2);

    /* Add properties to the observation steps */
    for (i = 0; i < 10; i++) {
        mm_obj_prop_add(&observation1, props1[i*2], props1[i*2+1]);
        mm_obj_prop_add(&observation2, props2[i*2], props2[i*2+1]);
    }
    /* Check that the observations have non-zero properties */
    assert(mm_obj_exists(&observation1));
    assert(mm_obj_exists(&observation2));

    /* Add a first step to the tale */
    assert(mm_tale_add(&tale, &observation1) == 0);
    assert(tale.length == 1);
    /* Add a second step to the tale */
    assert(mm_tale_add(&tale, &observation2) == 0);
    assert(tale.length == 2);

    /* Retrieve the first and second steps from the tale */
    test1 = mm_tale_get(&tale, 0);
    test2 = mm_tale_get(&tale, 1);
    /* Check that they're the same as the original observations */
    assert(mm_obj_cmp(&observation1, test1) == 0);
    assert(mm_obj_cmp(&observation2, test2) == 0);

    /* Remove the first step from the tale */
    assert(mm_tale_remove(&tale, 0) == 0);
    /* Check that the length is reduced */
    assert(tale.length == 1);
    /* Get the first step in the tale */
    test1 = mm_tale_get(&tale, 0);
    /* Check that it corresponds to the second original observation */
    assert(mm_obj_cmp(&observation2, test1) == 0);

    printf("Ok\n");
}

static void test_narratives()
{
    n_uint i, j;
    mm_narratives narratives;
    mm_random_seed seed;
    mm_id id[10];

    printf("test_narratives...");

    /* Set a random seed */
    mm_rand_init(&seed, 0,1,2,3);

    mm_narratives_init(&narratives);

    /* create some tales */
    for (i = 0; i < 10; i++) {
        mm_tale tale;
        mm_tale_init(&tale, &seed);
        mm_id_copy(&tale.id, &id[i]);
        for (j = 0; j < 11; j++) {
            mm_object scene;
            mm_obj_init(&scene);
            mm_obj_prop_add(&scene, j, i*2);
            mm_tale_add(&tale, &scene);
        }
        assert(tale.length == 11);
        assert(mm_narratives_add(&narratives, &tale) == 0);
        assert(narratives.length == i+1);
    }

    /* check IDs */
    assert(mm_narratives_get(&narratives, &id[2]) == 2);
    assert(mm_narratives_get(&narratives, &id[9]) == 9);

    /* remove a tale */
    assert(mm_narratives_remove(&narratives, 3) == 0);
    assert(narratives.length == 9);
    assert(mm_narratives_get(&narratives, &id[2]) == 2);
    assert(mm_narratives_get(&narratives, &id[4]) == 3);

    printf("Ok\n");
}

static void test_confabulation_with_narratives()
{
    n_uint i, j;
    n_int offset=0, similarity, index;
    mm_narratives narratives;
    mm_random_seed seed;

    printf("test_confabulation_with_narratives...");

    /* Set a random seed */
    mm_rand_init(&seed, 0,1,2,3);

    mm_narratives_init(&narratives);

    /* create some tales and add them to the narratives */
    for (i = 0; i < 10; i++) {
        mm_tale tale;
        mm_tale_init(&tale, &seed);
        for (j = 0; j < 11; j++) {
            mm_object scene;
            mm_obj_init(&scene);
            mm_obj_prop_add(&scene, (j%5)+i, (j*2)%10);
            mm_tale_add(&tale, &scene);
        }
        assert(tale.length == 11);
        assert(mm_narratives_add(&narratives, &tale) == 0);
        assert(narratives.length == i+1);
    }

    /* create a source tale */
    mm_tale tale_source;
    mm_tale_init(&tale_source, &seed);
    for (j = 0; j < 4; j++) {
        mm_object scene;
        mm_obj_init(&scene);
        mm_obj_prop_add(&scene, (j%5)+2, (j*3)%10);
        mm_tale_add(&tale_source, &scene);
    }

    /* test tale matching */
    n_int target_similarity[] = {
        2,5,3,3,2,1,0,0,0,0
    };
    n_int target_offset[] = {
        2,1,0,4,3,2,-1,-1,-1,-1
    };
    for (i = 0; i < 10; i++) {
        similarity = mm_tale_match(&tale_source, &narratives.tale[i], &offset);
        if ((similarity != target_similarity[i]) ||
            (offset != target_offset[i])) {
            printf("%d similarity = %d  %d\n",
                   (int)i, (int)similarity, (int)offset);
        }
        assert(similarity == target_similarity[i]);
        assert(offset == target_offset[i]);
    }

    /* test locating the closest tale within a set of narratives */
    index = mm_narratives_match_tale(&narratives, &tale_source,
                                     (n_int)-1, &offset);
    if (index != 1) {
        printf("index = %d\n",(int)index);
    }
    assert(index == 1);

    /* make the narrative closer to the source */
    mm_tale_confabulate(&tale_source, &narratives.tale[index],
                        100, &seed);

    /* test the match again to ensure that it is more similar
       than before */
    similarity = mm_tale_match(&tale_source, &narratives.tale[index], &offset);
    if ((similarity != 9) ||
        (offset != target_offset[index])) {
        printf("%d similarity = %d  %d\n",
               (int)index, (int)similarity, (int)offset);
    }

    assert(similarity == 9);
    assert(offset == target_offset[index]);

    printf("Ok\n");
}

static void test_confabulation_with_episodic()
{
    n_uint i, j;
    n_int offset=0, similarity, index;
    mm_narratives narratives;
    mm_random_seed seed;

    printf("test_confabulation_with_episodic...");

    /* Set a random seed */
    mm_rand_init(&seed, 0,1,2,3);

    mm_narratives_init(&narratives);

    /* create some tales and add them to the narratives */
    for (i = 0; i < 10; i++) {
        mm_tale tale;
        mm_tale_init(&tale, &seed);
        for (j = 0; j < MM_EVENT_MEMORY_SIZE/2; j++) {
            mm_object scene;
            mm_obj_init(&scene);
            mm_obj_prop_add(&scene, (j%5)+i, (j*2)%10);
            mm_tale_add(&tale, &scene);
        }
        assert(tale.length == j);
        assert(mm_narratives_add(&narratives, &tale) == 0);
        assert(narratives.length == i+1);
    }

    /* create an episodic memory and fill it with events*/
    mm_episodic events;
    mm_episodic_init(&events);
    for (i = 0; i < MM_EVENT_MEMORY_SIZE; i++) {
        mm_object scene;
        mm_obj_init(&scene);
        for (j = 0; j < 8 + (i%20); j++) {
            mm_obj_prop_add(&scene, (j%6)+i, (j%7));
        }
        mm_episodic_add(&events, &scene);
        if (mm_episodic_max(&events) != i+1) {
            printf("\n%d mm_episodic_max(&events) %d\n",
                   (int)i, (int)mm_episodic_max(&events));
        }
        assert(mm_episodic_max(&events) == i+1);
    }

    /* Check that the sequence length increases */
    if (mm_episodic_max(&events) != MM_EVENT_MEMORY_SIZE) {
        printf("\nmm_episodic_max(&events) %d/%d\n",
               (int)mm_episodic_max(&events),
               MM_EVENT_MEMORY_SIZE);
    }
    assert(mm_episodic_max(&events) == MM_EVENT_MEMORY_SIZE);

    /* test episodic matching */
    n_int target_similarity[] = {
        6,6,4,4,6,6,6,8,8,10
    };
    n_int target_offset[] = {
        0,1,0,0,0,0,1,2,3,4
    };
    for (i = 0; i < 10; i++) {
        /* how similar are the events to the tale ? */
        similarity = mm_tale_match_events(&narratives.tale[i], &events, &offset);

        /*printf("%d,", similarity);*/
        /*printf("%d,", offset);*/

        if ((similarity != target_similarity[i]) ||
            (offset != target_offset[i])) {
            printf("%d similarity = %d  %d\n",
                   (int)i, (int)similarity, (int)offset);
        }
        assert(similarity == target_similarity[i]);
        assert(offset == target_offset[i]);
    }

    /* test locating the closest tale to the episodic memory
       within a set of narratives */
    index = mm_narratives_match_episodic(&narratives, &events,
                                         (n_int)-1, &offset);
    if (index != 9) {
        printf("index = %d\n",(int)index);
    }
    assert(index == 9);

    /* make the events closer to the narrative */
    mm_episodic_confabulate(&events, &narratives.tale[index],
                            100, &seed);

    /* test the match again to ensure that it is more similar
       than before */
    similarity = mm_tale_match_events(&narratives.tale[index], &events, &offset);
    if ((similarity != 18) ||
        (offset != target_offset[index])) {
        printf("%d similarity = %d  %d\n",
               (int)index, (int)similarity, (int)offset);
    }
    assert(similarity == 18);
    assert(offset == target_offset[index]);

    printf("Ok\n");
}

void mm_run_tests()
{
    test_id();
    test_object_id();
    test_init();
    test_spatial();
    test_object_add_remove_properties();
    test_name();
    test_social_meet();
    test_som();
    test_communicate_social_categorisation();
    test_episodic();
    test_tale();
    test_narratives();
    test_confabulation_with_narratives();
    test_confabulation_with_episodic();

    printf("All tests passed\n");
}
