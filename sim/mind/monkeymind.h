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

#ifndef MONKEYMIND_H
#define MONKEYMIND_H

#include <stdio.h>
#include <string.h>
#include "monkeymind_id.h"
#include "monkeymind_rand.h"
#include "monkeymind_time.h"
#include "monkeymind_emotion.h"
#include "monkeymind_object.h"
#include "monkeymind_som.h"
#include "monkeymind_narratives.h"
#include "monkeymind_episodic.h"
#include "monkeymind_motives.h"

/* social graph array index of the self */
#define MM_SELF                    0

/* maximum size of the social graph */
#define MM_SIZE_SOCIAL_GRAPH       32

/* dimension of a 2D map within which the agent is located */
#define MM_SIZE_SPATIAL            32

/* the maximum number of abstract social categories */
#define MM_SIZE_SOCIAL_STEREOTYPES 32

#define MM_SEX_MALE                0
#define MM_SEX_FEMALE              1

/* dimension of the SOM used to classify social properties */
#define MM_SOCIAL_CATEGORIES_DIMENSION 64

/* radius used to update the social classification SOM */
#define MM_SOCIAL_CATEGORIES_RADIUS    4

/* name is stored as a single 32bit number,
   and contains sex, first name and last name */
#define MM_NAME(sex, first_name, surname)                       \
    ( ((((sex)&255)<<8)|((first_name)&255)) | ((surname)<<16) )
#define MM_SEX(name)             (((name)>>8)&255)
#define MM_FIRST_NAME(name)      ((name)&255)
#define MM_SURNAME(name)         ((name)>>16)

/* different types of attention */
enum {
    MM_ATTENTION_SOCIAL_GRAPH = 0,
    MM_ATTENTION_NARRATIVE,
    MM_ATTENTION_SIZE
};

typedef struct
{
    n_int value[MM_SOCIAL_CATEGORIES_DIMENSION*
                MM_SOCIAL_CATEGORIES_DIMENSION];
} mm_som_categories;

typedef struct
{
    mm_id id;

    mm_random_seed seed;

    /* intrinsic properties of the individual */
    mm_object * properties;

    /* self-organising map for classifying met individuals */
    mm_som social_categories;

    /* friend or foe categories */
    mm_som_categories category[MM_CATEGORIES];

    /* Memory of recent sequences of events */
    mm_episodic episodic_buffer;

    /* memory which may contain a number of narratives
       which have been created from the sequence of
       events */
    mm_narratives narratives;

    /* details of each known agent */
    mm_object social_graph[MM_SIZE_SOCIAL_GRAPH];

    /* different types of attention */
    n_uint attention[MM_ATTENTION_SIZE];

    /* abstracted individuals */
    mm_object social_stereotype[MM_SIZE_SOCIAL_STEREOTYPES];

    /* spatial memory */
    mm_object spatial[MM_SIZE_SPATIAL*MM_SIZE_SPATIAL];

	/* Here "cognitive system" refers to something like
       the reaction-diffusion system in Noble Ape */

    /* pointer to the state of the cognitive system */
    n_byte * cognitive_system_state;
    /* cognitive system state size in bytes */
    n_uint cognitive_system_state_size;
} monkeymind;

void mm_observe(monkeymind * mind, mm_object * observation);
void mm_add_property(monkeymind * mind,
                     n_uint property_type,
                     n_uint property_value);
void mm_set_property(monkeymind * mind,
                     n_uint property_type,
                     n_uint property_value);
n_uint mm_get_property(monkeymind * mind,
                       n_uint property_type);
void mm_remove_property(monkeymind * mind,
                        n_uint property_type);
void mm_init(monkeymind * mind,
             n_byte sex,
             n_byte first_name,
             n_byte surname,
             mm_random_seed * seed);
void mm_dialogue(monkeymind * mind0, monkeymind * mind1);
void mm_dialogue_internal(monkeymind * mind);
n_int mm_dialogue_narrative(monkeymind * speaker, monkeymind * listener);

#endif
