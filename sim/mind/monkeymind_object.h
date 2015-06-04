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

#ifndef MONKEYMIND_OBJECT_H
#define MONKEYMIND_OBJECT_H

#include <stdio.h>
#include <string.h>
#include "noble.h"
#include "monkeymind_id.h"

/* the maximum number of properties of an object */
#define MM_MAX_OBJECT_PROPERTIES  16

enum {
    MM_CATEGORY_FOF = 0,
    MM_CATEGORY_ATTRACTION,
    MM_CATEGORIES
};

enum relation_types
{
    MM_RELATION_SELF,
    MM_RELATION_BROTHER,
    MM_RELATION_SISTER,
    MM_RELATION_FATHER,
    MM_RELATION_MOTHER,
    MM_RELATION_GRANDFATHER,
    MM_RELATION_GRANDMOTHER,
    MM_RELATION_UNCLE,
    MM_RELATION_AUNT,
    MM_RELATIONS
};

enum property_types
{
    MM_PROPERTY_NONE = 0,

    /* who */
    MM_PROPERTY_MEETER,
    MM_PROPERTY_MET,
    MM_PROPERTY_NAME,
    MM_PROPERTY_MEETER_NAME,
    MM_PROPERTY_MET_NAME,
    MM_PROPERTY_RELATION,
    MM_PROPERTY_SOCIAL_X,
    MM_PROPERTY_SOCIAL_Y,

    /* what */
    MM_PROPERTY_OBJECT,
    MM_PROPERTY_TALE,
    MM_PROPERTY_EMOTION,
    MM_PROPERTY_HEIGHT,
    MM_PROPERTY_WEIGHT,
    MM_PROPERTY_HAIR_LENGTH,
    MM_PROPERTY_HAIR_COLOUR,
    MM_PROPERTY_FRIEND_OR_FOE,
    MM_PROPERTY_ATTRACTION,
    MM_PROPERTY_TALE_PLOT,
    MM_PROPERTY_TALE_ACT,
    MM_PROPERTY_TALE_SCENE,
    MM_PROPERTY_TALE_VIEWPOINT,
    MM_PROPERTY_AWAKE_PERCENT,

    /* where */
    MM_PROPERTY_PLACE_X,
    MM_PROPERTY_PLACE_Y,
    MM_PROPERTY_PLACE_Z,
    MM_PROPERTY_TERRITORY,

    /* when */
    MM_PROPERTY_DAY,
    MM_PROPERTY_SUBJECTIVE_TIME,

    MM_PROPERTIES
};


typedef struct
{
    mm_id id;

    /* a number of properties of the object */
    n_uint length;
    n_uint property_type[MM_MAX_OBJECT_PROPERTIES];
    n_uint property_value[MM_MAX_OBJECT_PROPERTIES];
    mm_id property_id[MM_MAX_OBJECT_PROPERTIES];

    /* how often has this property been observed or known */
    n_uint property_frequency[MM_MAX_OBJECT_PROPERTIES];

    /* the number of observations of this object */
    n_uint observations;
} mm_object;


void mm_obj_init(mm_object * obj);
n_int mm_obj_exists(mm_object * obj);
n_int mm_obj_prop_index(mm_object * obj,
                        n_uint property_type);
n_int mm_obj_prop_range(n_uint property_type,
                        n_uint * min, n_uint * max);
n_int mm_obj_prop_add(mm_object * obj,
                      n_uint property_type,
                      n_uint property_value);
n_int mm_obj_prop_add_id(mm_object * obj,
                         n_uint property_type,
                         mm_id * property_id);
n_int mm_obj_prop_remove(mm_object * obj,
                         n_uint property_type);
n_uint mm_obj_prop_get(mm_object * obj,
                       n_uint property_type);
void mm_obj_prop_get_id(mm_object * obj,
                        n_uint property_type,
                        mm_id * id);
n_int mm_obj_prop_set(mm_object * obj,
                      n_uint property_type,
                      n_uint property_value);
n_int mm_obj_prop_set_id(mm_object * obj,
                         n_uint property_type,
                         mm_id * property_id);
void mm_obj_copy(mm_object *src, mm_object * dest);
void mm_obj_to_vect(mm_object * obj,
                    n_byte * vect);
n_int mm_obj_cmp(mm_object * obj1, mm_object * obj2);
n_int mm_obj_match(mm_object * obj1, mm_object * obj2);
void mm_obj_change_perspective(mm_object * obj,
                               mm_id * from_id, n_uint from_name,
                               mm_id * to_id, n_uint to_name);

#endif
