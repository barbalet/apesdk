/****************************************************************

 Monkeymind - an experimental cogitive architecture

 =============================================================

 Copyright 2013-2015 Bob Mottram

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

#include "monkeymind_object.h"

/**
 * @brief initialise an object
 * @param obj Object to be initialised
 */
void mm_obj_init(mm_object * obj)
{
    obj->length = 0;
    obj->observations = 0;
}

/**
 * @brief returns the range of a given object property type
 * @param property_type The property type
 * @param min Returned minimum property value
 * @param max Returned maximum property value
 * @return zero on success
 */
n_int mm_obj_prop_range(n_uint property_type,
                        n_uint * min, n_uint * max)
{
    const n_uint property_range[] = {
        MM_PROPERTY_NONE, 0,0,

        /* who */
        MM_PROPERTY_MEETER, 0,0,
        MM_PROPERTY_MET, 0,0,
        MM_PROPERTY_NAME, 0,0,
        MM_PROPERTY_MEETER_NAME, 0,0,
        MM_PROPERTY_MET_NAME, 0,0,
        MM_PROPERTY_RELATION, 0,0,
        MM_PROPERTY_SOCIAL_X, 0, 64,
        MM_PROPERTY_SOCIAL_Y, 0, 64,

        /* what */
        MM_PROPERTY_OBJECT, 0,0,
        MM_PROPERTY_TALE, 0,0,
        MM_PROPERTY_EMOTION, 0,0,
        MM_PROPERTY_HEIGHT, 0,0,
        MM_PROPERTY_WEIGHT, 0,0,
        MM_PROPERTY_HAIR_LENGTH, 0,0,
        MM_PROPERTY_HAIR_COLOUR, 0,0,

        MM_PROPERTY_FRIEND_OR_FOE, 0,0,
        MM_PROPERTY_ATTRACTION, 0,0,
        MM_PROPERTY_TALE_PLOT, 0,0,
        MM_PROPERTY_TALE_ACT, 0,0,
        MM_PROPERTY_TALE_SCENE, 0,0,
        MM_PROPERTY_TALE_VIEWPOINT, 0,0,

        /* where */
        MM_PROPERTY_PLACE_X, 0,0,
        MM_PROPERTY_PLACE_Y, 0,0,
        MM_PROPERTY_PLACE_Z, 0,0,
        MM_PROPERTY_TERRITORY, 0,0,

        /* when */
        MM_PROPERTY_DAY, 0,0,
        MM_PROPERTY_SUBJECTIVE_TIME, 0,0,
    };

    *min = 0;
    *max = 0;

    if (property_type >= MM_PROPERTIES) {
        printf("Property type out of range\n");
        return -1;
    }
    *min = property_range[property_type*3 + 1];
    *max = property_range[property_type*3 + 2];

    return 0;
}


/**
 * @brief returns the array index of a given property type
 *        or -1 if not found
 * @param obj Object
 * @param property_type Type of property
 * @return Array index for the property type
 */
n_int mm_obj_prop_index(mm_object * obj,
                        n_uint property_type)
{
    n_int index = -1, start_index, end_index, curr_index;

    if (obj->length == 1) {
        if (obj->property_type[0] == property_type) {
            return 0;
        }
    }

    start_index = 0;
    end_index = obj->length-1;
    curr_index = start_index + ((end_index-start_index)/2);

    if (property_type == obj->property_type[end_index]) {
        index = end_index;
    }
    else if (property_type == obj->property_type[start_index]) {
        index = start_index;
    }

    while ((index == -1) &&
           (curr_index != start_index) &&
           (curr_index != end_index)) {

        if (property_type < obj->property_type[curr_index]) {
            end_index = curr_index;
            if (property_type == obj->property_type[end_index]) {
                index = end_index;
            }
        }
        else {
            if (property_type > obj->property_type[curr_index]) {
                start_index = curr_index;
                if (property_type == obj->property_type[start_index]) {
                    index = start_index;
                }
            }
            else {
                index = curr_index;
            }
        }
        curr_index = start_index + ((end_index-start_index)/2);
    }

    return index;
}

/**
 * @brief Adds a property to an object and returns its array index
 * @param obj Object
 * @param property_type Property type to be added
 * @param property_value Value of the property
 */
n_int mm_obj_prop_add(mm_object * obj,
                      n_uint property_type,
                      n_uint property_value)
{
    n_int i, index = 0;

    if (obj->length >= MM_MAX_OBJECT_PROPERTIES) return -1;

    index = mm_obj_prop_index(obj, property_type);
    if (index == -1) {
        /* property doesn't already exist
           so find a location for it */
        index = 0;
        while (index < obj->length) {
            if (obj->property_type[index] > property_type) {
                break;
            }
            index++;
        }
    }
    else {
        /* overwrite existing entry for this property type */
        obj->property_value[index] = property_value;
        return index;
    }

    if (index < obj->length) {
        /* insert */
        for (i = obj->length-1; i >= index; i--) {
            obj->property_type[i+1] = obj->property_type[i];
            obj->property_value[i+1] = obj->property_value[i];
            mm_id_copy(&obj->property_id[i], &obj->property_id[i+1]);
        }
    }
    obj->property_type[index] = property_type;
    obj->property_value[index] = property_value;
    mm_id_clear(&obj->property_id[index]);
    obj->length++;
    return index;
}

/**
 * @brief Adds a property with an ID to an object and returns its array index
 * @param obj Object
 * @param property_type Property type
 * @param property_id
 * @return Array index of the property within the object
 */
n_int mm_obj_prop_add_id(mm_object * obj,
                         n_uint property_type,
                         mm_id * property_id)
{
    n_int i, index = 0;

    if (obj->length >= MM_MAX_OBJECT_PROPERTIES) return -1;

    index = mm_obj_prop_index(obj, property_type);
    if (index == -1) {
        /* property doesn't already exist
           so find a location for it */
        index = 0;
        while (index < obj->length) {
            if (obj->property_type[index] > property_type) {
                break;
            }
            index++;
        }
    }
    else {
        /* overwrite existing id for this property type */
        mm_id_copy(property_id, &obj->property_id[index]);
        return index;
    }

    if (index < obj->length) {
        /* insert */
        for (i = obj->length-1; i >= index; i--) {
            obj->property_type[i+1] = obj->property_type[i];
            obj->property_value[i+1] = obj->property_value[i];
            mm_id_copy(&obj->property_id[i], &obj->property_id[i+1]);
        }
    }
    obj->property_type[index] = property_type;
    obj->property_value[index] = 1;
    mm_id_copy(property_id, &obj->property_id[index]);
    obj->length++;
    return index;
}

/**
 * @brief copies one object to another
 * @param src The source object
 * @param dest The destination object
 */
void mm_obj_copy(mm_object *src, mm_object * dest)
{
    memcpy((void*)dest,(void*)src,sizeof(mm_object));
}

/**
 * @brief Removes a property from an object
 * @param obj Object
 * @param property_type Property type to be removed
 * @return zero on success
 */
n_int mm_obj_prop_remove(mm_object * obj,
                         n_uint property_type)
{
    n_int i, index;

    if (obj->length == 0) return -1;

    index = mm_obj_prop_index(obj, property_type);
    if (index == -1) return -1;

    for (i = index+1; i < obj->length; i++) {
        obj->property_type[i-1] = obj->property_type[i];
        obj->property_value[i-1] = obj->property_value[i];
        mm_id_copy(&obj->property_id[i], &obj->property_id[i-1]);
    }
    obj->length--;
    return 0;
}

/**
 * @brief Returns the value of the given object property
 * @param obj Object
 * @param property_type The property type to obtain the value for
 * @return Property value
 */
n_uint mm_obj_prop_get(mm_object * obj,
                       n_uint property_type)
{
    n_int index = mm_obj_prop_index(obj, property_type);
    if (index > -1) return obj->property_value[index];
    return 0;
}

/**
 * @brief Returns the id for the given property type
 * @param obj Object
 * @param property_type Property type
 * @param id Returned ID value
 */
void mm_obj_prop_get_id(mm_object * obj,
                        n_uint property_type,
                        mm_id * id)
{
    n_int index = mm_obj_prop_index(obj, property_type);
    if (index > -1) {
        mm_id_copy(&obj->property_id[index], id);
        return;
    }
    mm_id_clear(id);
}

/**
 * @brief Sets an object property value
 * @param obj Object
 * @param property_type Property type
 * @param property_value Property value
 * @return zero on success
 */
n_int mm_obj_prop_set(mm_object * obj,
                      n_uint property_type,
                      n_uint property_value)
{
    n_int index = mm_obj_prop_index(obj, property_type);
    if (index > -1) {
        obj->property_value[index] = property_value;
        return 0;
    }
    return -1;
}

/**
 * @brief sets an object property id
 * @param obj Object
 * @param property_type Property type
 * @param property_id ID to set
 * @return zero on success
 */
n_int mm_obj_prop_set_id(mm_object * obj,
                         n_uint property_type,
                         mm_id * property_id)
{
    n_int index = mm_obj_prop_index(obj, property_type);
    if (index > -1) {
        mm_id_copy(property_id, &obj->property_id[index]);
        return 0;
    }
    return -1;
}

/**
 * @brief Normalise property values into a single byte range
 *        This is typically used to create vectors to be processed by a SOM
 * @param obj Object
 * @param vect The returned array of bytes
 */
void mm_obj_to_vect(mm_object * obj,
                    n_byte * vect)
{
    n_uint i, p, min, max, v;

    memset((void*)vect, '\0',
           MM_PROPERTIES*sizeof(n_byte));
    for (i = 0; i < obj->length; i++) {
        p = obj->property_type[i];
        min = max = 0;
        if (mm_obj_prop_range(p, &min, &max) == 0) {
            if (min + max > 0) {
                v = obj->property_value[i];
                vect[p] =
                    (n_byte)((v - min) * 255 / max);
            }
        }
    }
}

/**
 * @brief Returns true of the given object has been initialised
 * @param obj Object
 * @return True if the object has been initialised
 */
n_int mm_obj_exists(mm_object * obj)
{
    return (obj->length > 0);
}

/**
 * @brief compares two objects and returns 0 if they are the same, -1 otherwise
 * @param obj1 First object
 * @param obj2 Second object
 * @return zero if the two objects are the same
 */
n_int mm_obj_cmp(mm_object * obj1, mm_object * obj2)
{
    n_uint i;

    if (obj1->length != obj2->length) return -1;

    for (i = 0; i < obj1->length; i++) {
        if (obj1->property_type[i] !=
            obj2->property_type[i]) {
            return -1;
        }
        if (obj1->property_value[i] !=
            obj2->property_value[i]) {
            return -1;
        }
        if (!mm_id_equals(&obj1->property_id[i],
                          &obj2->property_id[i])) {
            return -1;
        }
    }
    return 0;
}

/**
 * @brief Returns a score giving a rough indication of how
 *        similar two objects are
 * @param obj1 First object
 * @param obj2 Second object
 * @return Similarity score
 */
n_int mm_obj_match(mm_object * obj1, mm_object * obj2)
{
    n_int similarity;
    n_uint i, value, same_types = 0;
    n_uint same_values = 0;
    mm_id test_id;

    for (i = 0; i < obj1->length; i++) {
        value = mm_obj_prop_get(obj2, obj1->property_type[i]);
        if (value > 0) {
            same_types++;
            if (value == obj1->property_value[i]) {
                same_values++;
            }
        }
        if (mm_id_exists(&obj1->property_id[i])) {
            mm_obj_prop_get_id(obj2, obj1->property_type[i], &test_id);
            if (mm_id_equals(&test_id,
                             &obj1->property_id[i])) {
                same_values++;
            }
        }
    }
    similarity = same_types + (2*same_values);
    return similarity;
}

/**
 * @brief Change the perspective of the meeter to a given being
 *        For example "I" becomes "you"
 * @param obj Object
 * @param from_id ID of the from being
 * @param from_name Name of the from being
 * @param to_id ID of the to being
 * @param to_name Name of the to being
 */
void mm_obj_change_perspective(mm_object * obj,
                               mm_id * from_id, n_uint from_name,
                               mm_id * to_id, n_uint to_name)
{
    n_uint i;

    for (i = 0; i < obj->length; i++) {
        if (((obj->property_type[i] == MM_PROPERTY_MEETER) ||
             (obj->property_type[i] == MM_PROPERTY_MET)) &&
            (mm_id_equals(&obj->property_id[i], from_id))) {
            mm_id_copy(to_id, &obj->property_id[i]);
        }
        if ((obj->property_type[i] == MM_PROPERTY_MEETER_NAME) &&
            (obj->property_value[i] == from_name)) {
            obj->property_value[i] = to_name;
        }
    }
}
