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

#include "monkeymind.h"

/* make an observation */
void mm_observe(monkeymind * mind, mm_object * observation)
{
    mm_episodic_add(&mind->episodic_buffer, observation);
}

/* adds a property to the individual */
void mm_add_property(monkeymind * mind,
                     n_uint property_type,
                     n_uint property_value)
{
    mm_obj_prop_add(mind->properties, property_type, property_value);
}

/* sets a property of the individual */
void mm_set_property(monkeymind * mind,
                     n_uint property_type,
                     n_uint property_value)
{
    mm_obj_prop_set(mind->properties, property_type, property_value);
}

/* gets a property of the individual */
n_uint mm_get_property(monkeymind * mind,
                       n_uint property_type)
{
    return mm_obj_prop_get(mind->properties, property_type);
}

/* remove a property type from an individual */
void mm_remove_property(monkeymind * mind,
                        n_uint property_type)
{
    mm_obj_prop_remove(mind->properties, property_type);
}

/* initialise the spatial memory */
static void mm_init_spatial(monkeymind * mind)
{
    n_int i;

    for (i = 0; i < MM_SIZE_SPATIAL*MM_SIZE_SPATIAL; i++) {
		mm_id_set(&mind->spatial[i].id, i);
    }
}

/* initially randomly assigned categories */
static void random_categories(mm_random_seed * seed,
                              n_int * categories)
{
    n_uint i;

    for (i = 0;
         i < MM_SOCIAL_CATEGORIES_DIMENSION*
             MM_SOCIAL_CATEGORIES_DIMENSION;
         i++) {
        categories[i] = (n_int)(mm_rand(seed)%3) - 1;
    }
}

/* initialises a mind */
void mm_init(monkeymind * mind,
             n_byte sex,
             n_byte first_name,
             n_byte surname,
             mm_random_seed * seed)
{
    mm_object * individual;
    n_uint name, i;

    if (seed != NULL) {
        mm_rand_copy(seed, &mind->seed);
    }
    else {
        /* a default random seed */
		mm_rand_init(&mind->seed, 0,1,2,3);
    }

    mm_id_create(&mind->seed, &mind->id);

    name = MM_NAME(sex,first_name,surname);

    memset((void*)&mind->narratives, '\0',
           sizeof(mm_narratives));
    memset((void*)mind->social_graph, '\0',
           MM_SIZE_SOCIAL_GRAPH * sizeof(mm_object));
    memset((void*)mind->social_stereotype, '\0',
           MM_SIZE_SOCIAL_STEREOTYPES * sizeof(mm_object));
    memset((void*)&mind->properties, '\0', sizeof(mm_object));
    memset((void*)mind->spatial, '\0',
           MM_SIZE_SPATIAL * MM_SIZE_SPATIAL * sizeof(mm_object));
    memset((void*)mind->attention, '\0',
           MM_ATTENTION_SIZE*sizeof(n_uint));

    for (i = 0; i < MM_CATEGORIES; i++) {
        random_categories(&mind->seed, mind->category[i].value);
    }

    mm_episodic_init(&mind->episodic_buffer);
    mm_init_spatial(mind);
    mm_som_init(&mind->social_categories,
                MM_SOCIAL_CATEGORIES_DIMENSION,
                MM_PROPERTIES,
                MM_SOCIAL_CATEGORIES_RADIUS,
                &mind->seed);

    mind->properties = &mind->social_graph[MM_SELF];

    individual = &mind->social_graph[MM_SELF];
    individual->length = 0;

    mm_obj_prop_add_id(individual,
					   MM_PROPERTY_MEETER, &mind->id);

    mm_obj_prop_add(individual,
                    MM_PROPERTY_MEETER_NAME, name);

    mm_obj_prop_add_id(individual,
					   MM_PROPERTY_MET, &mind->id);

    mm_obj_prop_add(individual,
                    MM_PROPERTY_MET_NAME, name);

    mm_add_property(mind, MM_PROPERTY_NAME, name);

    mm_add_property(mind, MM_PROPERTY_AWAKE_PERCENT, 100);

    /* no cognitive system specified */
    mind->cognitive_system_state = 0;
    mind->cognitive_system_state_size = 0;
}

/* external dialogue between two agents */
void mm_dialogue(monkeymind * mind0, monkeymind * mind1)
{
	/*
    n_uint attention0 = mind0->attention[MM_ATTENTION_SOCIAL_GRAPH];
    n_uint attention1 = mind1->attention[MM_ATTENTION_SOCIAL_GRAPH];
	*/

	/* TODO narrative based dialogue */
}

/* internal dialogue within an agent */
void mm_dialogue_internal(monkeymind * mind)
{
	/* IDEA: when sleeping narrative memories can be transfered
	   to the episodic buffer and vice versa */
	/*n_uint awake_percent =
		mm_obj_prop_get(mind->properties,
		MM_PROPERTY_AWAKE_PERCENT);*/

	/*mm_episodic_confabulate(&mind->episodic_buffer, mm_tale * tale,
	  awake_percent, mind->seed);*/

}

/* transmits a narrative from a speaker to a listener */
n_int mm_dialogue_narrative(monkeymind * speaker, monkeymind * listener)
{
    n_uint speaker_attention = speaker->attention[MM_ATTENTION_NARRATIVE];
	mm_tale * speaker_tale;
	mm_tale * listener_tale;
    n_int listener_narrative_index;
	n_int min_similarity = 0;
	n_int offset = 0;

	/* TODO this should vary depending upon social and/or physiological factors */
	n_int listener_interest_percent = 20;

	/* does the speaker have nothing to say? */
    if (speaker->narratives.length == 0) {
		return -1;
	}

    /* get the tale which is the speaker's current focus of attention */
    speaker_tale =
        &speaker->narratives.tale[speaker_attention %
                                  speaker->narratives.length];

    speaker_tale->times_told++;

	/* get the array index of the closest matching tale */
	listener_narrative_index =
		mm_narratives_match_tale(&listener->narratives, speaker_tale,
								 min_similarity, &offset);

	if (listener_narrative_index == -1) {
		/* if there are no matches then overwrite the least heard */
		listener_narrative_index = mm_narratives_least_heard(&listener->narratives);
        mm_narratives_insert(&listener->narratives,
                             listener_narrative_index, speaker_tale);
		listener_tale = &listener->narratives.tale[listener_narrative_index];
	}
	else {
		listener_tale = &listener->narratives.tale[listener_narrative_index];
		/* make the remembered tale more similar to the one just told */
		mm_tale_confabulate(speaker_tale, listener_tale,
							listener_interest_percent, &listener->seed);
	}

	/* increment the number of times heard */
	listener_tale->times_heard++;

	/* listener's attention is on the current narrative */
	listener->attention[MM_ATTENTION_NARRATIVE] =
		(n_uint)listener_narrative_index;

    return 0;
}
