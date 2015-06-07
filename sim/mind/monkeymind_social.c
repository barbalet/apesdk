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

#include "monkeymind_social.h"

/**
 * @brief Returns true if a social graph entry exists at the given index
 * @param mind Monkeymind object
 * @param index Social graph array index
 * @return True if a social graph entry exists
 */
n_int mm_social_graph_entry_exists(monkeymind *mind, n_int index)
{
    mm_id meeter_id, met_id;

    mm_obj_prop_get_id(&mind->social_graph[index],
                       MM_PROPERTY_MEETER, &meeter_id);

    if (!mm_id_exists(&meeter_id)) {
        return (1 == 0);
    }

    mm_obj_prop_get_id(&mind->social_graph[index],
                       MM_PROPERTY_MET, &met_id);

    return mm_id_exists(&met_id);
}

/**
 * @brief Returns the social graph array index of the individual having the
 *        given id
 * @param mind Monkeymind object
 * @param met_id ID of the individual to search for within the social graph
 * @return Social graph array index, or -1 if not found
 */
n_int mm_social_index_from_id(monkeymind * mind, mm_id * met_id)
{
    n_int i;
    mm_id test_id;

    for (i = MM_SELF+1; i < MM_SIZE_SOCIAL_GRAPH; i++) {
        if (!mm_social_graph_entry_exists(mind, i)) break;

        mm_obj_prop_get_id(&mind->social_graph[i],
                           MM_PROPERTY_MEETER, &test_id);
        if (mm_id_equals(&test_id, met_id)) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief Returns the social graph array index of the individual having the
 *        given name
 * @param mind Monkeymind object
 * @param met_name The name of the individual to search for
 * @return Array index within the social graph, or -1 if not found
 */
n_int mm_social_index_from_name(monkeymind * mind, n_uint met_name)
{
    n_int i;

    for (i = MM_SELF+1; i < MM_SIZE_SOCIAL_GRAPH; i++) {
        if (!mm_social_graph_entry_exists(mind, i)) break;

        if (mm_obj_prop_get(&mind->social_graph[i],
                            MM_PROPERTY_MET_NAME) == met_name) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief Returns the social graph array index of a forgettable individual
 * @param mind Monkeymind object
 * @return Social graph array index
 */
static n_int mm_social_forget(monkeymind * mind)
{
    n_int i, index = -1;
    n_uint min_observations = mind->social_graph[0].observations;

    /* pick the individual with the fewest observations */
    for (i = 1; i < MM_SIZE_SOCIAL_GRAPH; i++) {
        if (!mm_social_graph_entry_exists(mind, i)) break;

        if (mind->social_graph[i].observations < min_observations) {
            min_observations = mind->social_graph[i].observations;
            index = i;
        }
    }
    return index;
}

/**
 * @brief The prejudice function
 * @param meeter The individual doing the meeting
 * @param met The individual being met
 * @param social_graph_entry Social graph entry for the met individual
 */
static void mm_social_evaluate(monkeymind * meeter,
                               monkeymind * met,
                               mm_object * social_graph_entry)
{
    n_uint fof = MM_NEUTRAL;
    n_uint attraction = MM_NEUTRAL;

    /* TODO: calculate or look up friend or foe value */

    /* TODO: calculate or look up attraction value */

    mm_obj_prop_add(social_graph_entry,
                    MM_PROPERTY_FRIEND_OR_FOE, fof);
    mm_obj_prop_add(social_graph_entry,
                    MM_PROPERTY_ATTRACTION, attraction);
}

/**
 * @brief get the existing categorisation at this location
 * @param categories Array storing social categorisations
 * @param social_x X coordinate in the topological social categorisation space
 * @param social_y Y coordinate in the topological social categorisation space
 * @return Social category value
 */
static n_int mm_social_get_category(n_int categories[],
                                    n_uint social_x,
                                    n_uint social_y)
{
    n_uint n = social_y*MM_SOCIAL_CATEGORIES_DIMENSION + social_x;
    if (categories[n] > 0) return SOCIAL_POSITIVE;
    if (categories[n] < 0) return SOCIAL_NEGATIVE;
    return SOCIAL_MEH;
}

/**
 * @brief Rebalances categories such that positive and negative values
 *        have an average of zero.
 *        This helps to avoid situtaions such as liking everyone
 *        or hating everyone
 * @param categories A 2D array of category values corresponding to the
 *        topological social generalisations map
 */
static void mm_social_category_rebalance(n_int categories[])
{
    n_uint i;
    n_int average = 0, hits = 0;

    for (i = 0;
         i < MM_SOCIAL_CATEGORIES_DIMENSION*
             MM_SOCIAL_CATEGORIES_DIMENSION; i++) {
        if (categories[i] != 0) {
            average += categories[i];
            hits++;
        }
    }

    if (hits == 0) return;

    average /= hits;

    for (i = 0;
         i < MM_SOCIAL_CATEGORIES_DIMENSION*
             MM_SOCIAL_CATEGORIES_DIMENSION; i++) {
        if (categories[i] != 0) {
            categories[i] -= average;
        }
    }
}

/**
 * @brief Update categories using the given increment
 * @param categories 2D map of social categories/dispositions corresponding to a SOM
 * @param social_x X coordinate in social categorisation space
 * @param social_y Y coordinate in social categorisation space
 * @param increment Increment to be applied
 */
static void mm_social_category_update(n_int categories[],
                                      n_uint social_x,
                                      n_uint social_y,
                                      n_int increment)
{
    n_int x, y, dx, dy, max_radius, inner_radius, r, n;
    n_byte normalise = 0;
    const n_int max_response = 256;

    if (increment == 0) return;

    max_radius =
        MM_SOCIAL_CATEGORIES_RADIUS*MM_SOCIAL_CATEGORIES_RADIUS;
    inner_radius =
        MM_SOCIAL_CATEGORIES_RADIUS*MM_SOCIAL_CATEGORIES_RADIUS/4;

    for (x = (n_int)social_x - MM_SOCIAL_CATEGORIES_RADIUS;
         x <= (n_int)social_x + MM_SOCIAL_CATEGORIES_RADIUS;
         x++) {
        if ((x < 0) || (x >= MM_SOCIAL_CATEGORIES_DIMENSION)) {
            continue;
        }
        dx = x - (n_int)social_x;
        for (y = (n_int)social_y - MM_SOCIAL_CATEGORIES_RADIUS;
             y <= (n_int)social_y + MM_SOCIAL_CATEGORIES_RADIUS;
             y++) {
            if ((y < 0) || (y >= MM_SOCIAL_CATEGORIES_DIMENSION)) {
                continue;
            }
            dy = y - (n_int)social_y;
            r = dx*dx + dy*dy;
            if (r > max_radius) continue;

            /* location within the map */
            n = y*MM_SOCIAL_CATEGORIES_DIMENSION + x;

            if (r < inner_radius) {
                categories[n] += increment*2;
            }
            else {
                categories[n] += increment;
            }
            /* range checking */
            if ((categories[n] > max_response) ||
                (categories[n] < -max_response)) {
                normalise = 1;
            }
        }
    }

    /* rebalance */
    mm_social_category_rebalance(categories);

    /* normalise if necessary to keep values in range */
    if (normalise == 1) {
        for (n = 0;
             n < MM_SOCIAL_CATEGORIES_DIMENSION*
                 MM_SOCIAL_CATEGORIES_DIMENSION; n++) {
            categories[n] /= 2;
        }
    }
}

/**
 * @brief Align the som categories of another agent with those of the current agent
 *        Friendly individuals tend to align their categories, unfriendly ones
 *        dissociate their categories
 * @param mind Monkeymind object
 * @param social_x X coordinate within the social space of the other mind
 * @param social_y Y coordinate within the social space of the other mind
 * @param other Other monkeymind object
 * @param is_friendly Either 1 if the two minds are friendly, or 0 otherwise
 */
static void mm_align_categories(monkeymind * mind,
                                n_uint social_x,
                                n_uint social_y,
                                monkeymind * other,
                                n_byte is_friendly)
{
    n_int max_radius, inner_radius, i, x, y;
    n_int r, n, dx, dy, dcat;

    /* positive or negative disposition */
    n_int disposition = 1;
    if (is_friendly <= 0) disposition = -1;

    max_radius =
        MM_SOCIAL_CATEGORIES_RADIUS*MM_SOCIAL_CATEGORIES_RADIUS;
    inner_radius =
        MM_SOCIAL_CATEGORIES_RADIUS*MM_SOCIAL_CATEGORIES_RADIUS/4;

    for (i = 0; i < MM_CATEGORIES; i++) {
        for (x = (n_int)social_x - MM_SOCIAL_CATEGORIES_RADIUS;
             x <= (n_int)social_x + MM_SOCIAL_CATEGORIES_RADIUS;
             x++) {
            if ((x < 0) || (x >= MM_SOCIAL_CATEGORIES_DIMENSION)) {
                continue;
            }
            dx = x - (n_int)social_x;
            for (y = (n_int)social_y - MM_SOCIAL_CATEGORIES_RADIUS;
                 y <= (n_int)social_y + MM_SOCIAL_CATEGORIES_RADIUS;
                 y++) {
                if ((y < 0) || (y >= MM_SOCIAL_CATEGORIES_DIMENSION)) {
                    continue;
                }
                dy = y - (n_int)social_y;
                r = dx*dx + dy*dy;
                if (r > max_radius) continue;

                /* location within the social categorisation map */
                n = y*MM_SOCIAL_CATEGORIES_DIMENSION + x;

                /* difference in categories */
                dcat = mind->category[i].value[n] -
                    other->category[i].value[n];

                /* categories are already within a similarity tollerance */
                if ((dcat < 2) && (dcat > -2)) continue;

                if (r < inner_radius) {
                    if (dcat > 0) {
                        other->category[i].value[n] += 2*disposition;
                    }
                    else {
                        other->category[i].value[n] -= 2*disposition;
                    }
                }
                else {
                    if (dcat > 0) {
                        other->category[i].value[n] += disposition;
                    }
                    else {
                        other->category[i].value[n] -= disposition;
                    }
                }
            }
        }
    }
}

/**
 * @brief Communicates the social categorisation of a given social
 *        graph entry to another individual
 * @param mind Monkeymind object
 * @param index Social graph array index of the other within the mind
 * @param other Another monkeymind object
 * @param is_friendly Either 1 if the two minds are friendly, or 0 otherwise
 */
void mm_communicate_social_categorisation(monkeymind * mind,
                                          n_int index,
                                          monkeymind * other,
                                          n_byte is_friendly)
{
    mm_object * individual;
    n_uint social_x, social_y;
    n_byte normalised_properties[MM_PROPERTIES];

    /* if this has already been communicated then do nothing */
    if (!mm_social_graph_entry_exists(mind, index)) return;

    individual = &mind->social_graph[index];

    /* get the coordinates of the other individual within the
       topological social categorisation space */
    social_x = mm_obj_prop_get(individual, MM_PROPERTY_SOCIAL_X);
    social_y = mm_obj_prop_get(individual, MM_PROPERTY_SOCIAL_Y);

    /* normalise property values into a single byte range, so that
       they can be efficiently used by the SOM */
    mm_obj_to_vect(individual, normalised_properties);

    /* adjust weights within the social categorisation SOM
       of the other individual */
    mm_som_learn(&other->social_categories,
                 normalised_properties,
                 social_x, social_y);

    /* align the categories which are associalted with the
       topological social space */
    mm_align_categories(mind, social_x, social_y, other, is_friendly);
}

/**
 * @brief categorise an entry within the social graph
 * @param mind Monkeymind object
 * @param index Array index within the social graph
 * @returns zero on success
 */
static n_int mm_social_categorisation(monkeymind * mind,
                                      n_int index)
{
    mm_object * individual;
    n_int fof_increment = 0, attraction_increment = 0;
    n_byte normalised_properties[MM_PROPERTIES];
    n_uint fof, attraction, social_x=0, social_y=0;

    if (index >= MM_SIZE_SOCIAL_GRAPH) return -1;

    if (!mm_social_graph_entry_exists(mind, index)) return -2;

    individual = &mind->social_graph[index];

    /* normalise property values into a single byte range */
    mm_obj_to_vect(individual, normalised_properties);

    /* friendly or unfriendly? */
    fof = mm_obj_prop_get(individual, MM_PROPERTY_FRIEND_OR_FOE);
    if (fof > MM_NEUTRAL) {
        fof_increment = 1;
    }
    if (fof < MM_NEUTRAL) {
        fof_increment = -1;
    }

    /* attractive or unattractive? */
    attraction = mm_obj_prop_get(individual, MM_PROPERTY_ATTRACTION);
    if (attraction > MM_NEUTRAL) {
        attraction_increment = 1;
    }
    if (attraction < MM_NEUTRAL) {
        attraction_increment = -1;
    }

    /* find the peak response within the SOM,
       corresponding to the minimum Euclidean distance */
    mm_som_update(&mind->social_categories,
                  normalised_properties,
                  &social_x, &social_y);

    /* store the categorisation as properties */
    mm_obj_prop_add(individual, MM_PROPERTY_SOCIAL_X, social_x);
    mm_obj_prop_add(individual, MM_PROPERTY_SOCIAL_Y, social_y);

    /* adjust weights within the social categorisation SOM */
    mm_som_learn(&mind->social_categories,
                 normalised_properties,
                 social_x, social_y);

    /* alter the friend of foe status depending upon the existing
       classification */
    mm_obj_prop_set(individual, MM_PROPERTY_FRIEND_OR_FOE,
                    mm_social_get_category(mind->category[MM_CATEGORY_FOF].value,
                                           social_x, social_y));

    /* alter the friend or foe values within the classifier */
    mm_social_category_update(mind->category[MM_CATEGORY_FOF].value,
                              social_x, social_y, fof_increment);

    /* alter the attraction status depending upon the existing
       classification */
    mm_obj_prop_set(individual, MM_PROPERTY_ATTRACTION,
                    mm_social_get_category(mind->category[MM_CATEGORY_ATTRACTION].value,
                                           social_x, social_y));

    /* alter the attraction values within the classifier */
    mm_social_category_update(mind->category[MM_CATEGORY_ATTRACTION].value,
                              social_x, social_y, attraction_increment);

    return 0;
}

/**
 * @brief adds a social graph enry at the given index
 * @param meeter The being doing the meeting
 * @param met The being who was met
 * @param index Meeter social graph index for the met individual
 * @param familiar non-zero if already met
 * @return zero on success
 */
static n_int mm_social_add(monkeymind * meeter, monkeymind * met,
                           n_int index, n_byte familiar)
{
    n_int i;
    mm_object * individual;

    if (index >= MM_SIZE_SOCIAL_GRAPH) return -1;

    individual = &meeter->social_graph[index];

    /* remember properties of the met individual */
    mm_obj_copy(met->properties, individual);

    mm_obj_prop_add_id(individual,
                       MM_PROPERTY_MEETER, &meeter->id);

    /* add an id for the social graph entry */
    mm_id_create(&meeter->seed, &individual->id);

    mm_obj_prop_add(individual,
                    MM_PROPERTY_MEETER_NAME,
                    mm_get_property(meeter, MM_PROPERTY_NAME));

    mm_obj_prop_add_id(individual,
                       MM_PROPERTY_MET, &met->id);

    mm_obj_prop_add(individual,
                    MM_PROPERTY_MET_NAME,
                    mm_get_property(met, MM_PROPERTY_NAME));

    if (familiar == 0) {
        /* first meeting */
        individual->observations = 1;
        mm_social_evaluate(meeter, met, individual);
    }
    else {
        /* subsequent meetings */
        individual->observations++;
        /* if the limit of observations has been reached then rescale */
        if (meeter->social_graph[index].observations >=
            MM_SOCIAL_MAX_OBSERVATIONS) {
            for (i = 0; i < MM_SIZE_SOCIAL_GRAPH; i++) {
                meeter->social_graph[i].observations >>= 1;
            }
        }
    }

    /* make generalisations about the met individual */
    if (mm_social_categorisation(meeter, index) != 0) return -2;

    /* set attention to the met being */
    meeter->attention[MM_ATTENTION_SOCIAL_GRAPH] = index;

    return 0;
}

/**
 * @brief Returns the social graph array index for a met being
 * @param meeter The being doing the meeting
 * @param met The being who was met
 * @oaram familiar Returns whether this is a previously met individual
 * @return Meeter social graph array index for the met individual
 */
static int mm_social_next_graph_index(monkeymind * meeter, monkeymind * met,
                                      n_byte * familiar)
{
    n_int index = mm_social_index_from_id(meeter, &met->id);

    *familiar = 0;

    if (index == -1) {
        /* are all array entries occupied? */
        if (mm_social_graph_entry_exists(meeter, MM_SIZE_SOCIAL_GRAPH-1)) {
            index = mm_social_forget(meeter);
        }
        else {
            /* find the last entry */
            for (index = MM_SIZE_SOCIAL_GRAPH-1; index >= 0; index--) {
                if (mm_social_graph_entry_exists(meeter, index)) {
                    index++;
                    break;
                }
            }
        }
    }
    else {
        *familiar = 1;
    }
    return index;
}

/**
 * @brief two individuals meet
 * @param meeter The being doing the meeting
 * @param met The being who was met
 * @return zero on success
 */
n_int mm_social_meet(monkeymind * meeter, monkeymind * met)
{
    n_byte familiar;
    n_int index = mm_social_next_graph_index(meeter, met, &familiar);

    if (index == -1) return -1;

    return mm_social_add(meeter, met, index, familiar);
}

/**
 * @brief
 * @param speaker
 * @param listener
 */
void mm_social_speak(monkeymind * speaker, monkeymind * listener)
{
    /* TODO */
}
