/****************************************************************

 being.c

 =============================================================

 Copyright 1996-2014 Tom Barbalet. All rights reserved.

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

 This software and Noble Ape are a continuing work of Tom Barbalet,
 begun on 13 June 1996. No apes or cats were harmed in the writing
 of this software.

 ****************************************************************/

/*! \file   being.c
 *  \brief  Historically this represented the Noble Ape but moreso
    now it represents the Noble Ape's interface to something external.
    being.c also now connects to the social, brain, body and metabolim
    simulations through to external simulations.
 */

#include "entity.h"
#include "entity_internal.h"

#include <stdio.h>
#include <stdlib.h>

/* worst case 1500 + 180 per step */

#define VISIBILITY_MAXIMUM      (2000)

#define VISIBILITY_SPAN    (VISIBILITY_MAXIMUM / ((15+16) >> 1))

#define	WALK_ON_WATER(pz,w)	(((pz)<w) ? w : (pz))

/** Swim - better or worse at swimming both speed and energy use */

#define GENE_SWIM(gene)                     GENE_VAL_REG(gene, 9, 11, 13, 7)

/** Speed on land - faster v. slower */

#define GENE_SPEED(gene)                    GENE_VAL_REG(gene, 14, 5, 12, 10)

/** Control on random wander - more or less random wander */

#define GENE_STAGGER(gene)                  GENE_VAL_REG(gene, 12, 14, 3, 11)

/** Hill climbing - improved energy use for climbing */

#define GENE_HILL_CLIMB(gene)               GENE_VAL_REG(gene, 4, 6, 5, 2)

/** checks the being's line of sight to a point

	NB: As the direction of scanning can't be determined, the breaking sight point
        can't be detected through tracking the being_ground results.
*/

/** returning 0 - not in line of sight, 1 - in line of sight */

typedef	struct
{
    n_int 		start_z;
    n_int 		offset_x;
    n_int 		offset_y;
    n_int       visibility_delta;
    n_int       visibility_total;
    n_land *land;
}
being_draw;

#ifdef BRAINCODE_ON

n_byte * being_braincode_external(noble_being * value)
{
    noble_social * social_value = being_social(value);
    return social_value[0].braincode;
}

n_byte * being_braincode_internal(noble_being * value)
{
    noble_social * social_value = being_social(value);
    return social_value[value->attention[ATTENTION_ACTOR]].braincode;
}

#endif

void    being_set_state(noble_being * value, being_state_type state)
{
    value->macro_state = state;
}

void    being_add_state(noble_being * value, being_state_type state)
{
    value->macro_state |= state;

}
n_byte2 being_state(noble_being * value)
{
    return value->macro_state;
}

static void being_random3(noble_being * value)
{
    math_random3(value->seed);
}

n_byte2 being_random(noble_being * value)
{
    return math_random(value->seed);
}

static void being_set_random(noble_being * value, n_byte2 * seed)
{
    value->seed[0] = seed[0];
    value->seed[1] = seed[1];
}

static void being_set_random1(noble_being * value, n_byte2 seed1)
{
    value->seed[1] = seed1;
}

static n_byte2 * being_get_random(noble_being * value)
{
    return value->seed;
}

n_int being_memory(noble_simulation * local, n_byte * buffer, n_uint * location, n_int memory_available)
{
    n_uint  lpx = 0;
    n_uint  number_apes = 0;
    
    if (memory_available < 1)
    {
        return SHOW_ERROR("Memory not available");
    }
    
    number_apes = (memory_available / sizeof(noble_being)) - 1;
    
    if (number_apes < 1)
    {
        return SHOW_ERROR("Not enough memory for an ape");
    }
    
#ifdef LARGE_SIM
    local->max = LARGE_SIM;
#else
    local->max = (memory_available / sizeof(noble_being)) - 1;
#endif
    local->beings = (noble_being *) & buffer[  * location ];
    * location += sizeof(noble_being) * local->max ;
    
    while (lpx < local->max)
    {
        noble_being * local_being = &(local->beings[ lpx ]);
#ifdef BRAIN_ON
        io_erase(local_being->brain, DOUBLE_BRAIN);
#endif
        io_erase((n_byte *)local_being->social, (SOCIAL_SIZE * sizeof(noble_social)));
        io_erase((n_byte *)local_being->episodic, (EPISODIC_SIZE * sizeof(noble_episodic)));
        lpx ++;
    }
    return 0;
}

static void being_set_brainatates(noble_being * value, n_int asleep, n_byte2 val1, n_byte2 val2, n_byte2 val3)
{
    n_int three_offset = (asleep ? 0 : 3);

    value->brain_state[three_offset + 0] = val1;
    value->brain_state[three_offset + 1] = val2;
    value->brain_state[three_offset + 2] = val3;
}

n_int being_brainstates(noble_being * value, n_int asleep, n_byte2 * states)
{
    n_int three_offset = (asleep ? 0 : 3);
    
    states[0] = value->brain_state[three_offset + 0];
    states[1] = value->brain_state[three_offset + 1];
    states[2] = value->brain_state[three_offset + 2];
    
    return ((states[0] != 0) || (states[1] != 1024) || (states[2] != 0));
}

void being_erase(noble_being * value)
{
    io_erase((n_byte*)value, sizeof(noble_being));
}

n_byte being_honor(noble_being * value)
{
    return value->honor;
}

void being_honor_inc_dec(noble_being * inc, noble_being * dec)
{
    if (inc->honor < 255) inc->honor++;
    if (dec->honor > 0) dec->honor--;
}

void being_honor_swap(noble_being * victor, noble_being * vanquished)
{
    if (victor->honor < vanquished->honor)
    {
        /** swap social status */
        n_byte temp_hon = victor->honor;
        victor->honor = vanquished->honor;
        vanquished->honor = temp_hon;
    }
}

n_int being_honor_compare(noble_being * first, noble_being * second)
{
    if (first->honor > second->honor)
    {
        return 1;
    }
    
    if (first->honor < second->honor)
    {
        return -1;
    }
    
    return 0;
}

static n_byte being_honor_immune(noble_being * value)
{
    n_int local_honor = being_honor(value);
    if (local_honor < 250) /* ALPHA_RANK */
    {
        return (n_byte)(1 + (local_honor>>6));
    }
    return 2; /* IMMUNE_STRENGTH_ALPHA */
}

n_byte being_posture(noble_being * value)
{
    return value->posture;
}

void being_set_posture(noble_being * value, n_byte post)
{
    value->posture = post;
}

n_int being_posture_under(noble_being * value, enum posture_type post)
{
    return (value->posture < post);
}

#ifdef BRAIN_ON
n_byte * being_brain(noble_being * value)
{
    return value->brain;
}
#endif

noble_episodic * being_episodic(noble_being * value)
{
    return value->episodic;

}

noble_social * being_social(noble_being * value)
{
    return value->social;
}

n_int being_location_x(noble_being * value)
{
    return (n_int)value->location[0];
}

n_int being_location_y(noble_being * value)
{
    return (n_int)value->location[1];
}

n_byte2 * being_location(noble_being * value)
{
    return value->location;
}

void being_set_location(noble_being * value, n_byte2 * from)
{
    value->location[0] = from[0];
    value->location[1] = from[1];
}

n_byte being_speed(noble_being * value)
{
    return value->velocity;
}

void being_set_speed(noble_being * value, n_byte sp)
{
    value->velocity = sp;
}

void being_delta(noble_being * primary, noble_being * secondary, n_vect2 * delta)
{
    delta->x = primary->location[0] - secondary->location[0];
    delta->y = primary->location[1] - secondary->location[1];
}

n_int being_dob(noble_being * value)
{
    return TIME_IN_DAYS(value->date_of_birth);
}

void being_facing_towards(noble_being * value, n_vect2 * vector)
{
    value->direction_facing = math_turn_towards(vector, value->direction_facing, 0);
}

void being_wander(noble_being * value, n_int wander)
{
    value->direction_facing = (n_byte)((value->direction_facing + 256 + wander) & 255);
}

static void being_facing_init(noble_being * value)
{
    value->direction_facing = (n_byte)(being_random(value) & 255);
}

void being_facing_vector(noble_being * value, n_vect2 * vect, n_int divisor)
{
    vect2_direction(vect, value->direction_facing, divisor * 32);
}

n_byte being_facing(noble_being * value)
{
    return value->direction_facing;
}

n_genetics * being_genetics(noble_being * value)
{
    return value->genetics;
}

n_int being_pregnant(noble_being * value)
{
    return TIME_IN_DAYS(value->date_of_conception);
}

n_genetics * being_fetal_genetics(noble_being * value)
{
    return value->fetal_genetics;
}

/* TODO: Remove this kind of access eventually */
n_int   being_energy(noble_being * value)
{
    return value->stored_energy;
}

n_int   being_energy_less_than(noble_being * value, n_int less_than)
{
    return being_energy(value) < less_than;
}

void  being_dead(noble_being * value)
{
    value->stored_energy = BEING_DEAD;
}

void being_living(noble_being * value)
{
    value->stored_energy = BEING_FULL;
}

void   being_energy_delta(noble_being * value, n_int delta)
{
    n_int total = value->stored_energy + delta;

    if (total < BEING_DEAD)
    {
        total = BEING_DEAD;
    }
    
    value->stored_energy = (n_byte2) total;
}

n_int   being_drive(noble_being * value, enum drives_definition drive)
{
    return value->drives[drive];
}

void    being_inc_drive(noble_being * value, enum drives_definition drive)
{
    if (value->drives[drive] < DRIVES_MAX)
    {
        value->drives[drive]++;
    }
}

void    being_dec_drive(noble_being * value, enum drives_definition drive)
{
    if (value->drives[drive] > 0)
    {
        value->drives[drive]--;
    }
}

void    being_reset_drive(noble_being * value, enum drives_definition drive)
{
    value->drives[drive] = 0;
}

n_int   being_height(noble_being * value)
{
    return value->height;
}

n_int   being_mass(noble_being * value)
{
    return value->mass;
}

static void being_turn_away_from_water(noble_being * value, n_land * land)
{
    n_int	it_water_turn = 0;
    n_vect2 location_vector;

    vect2_byte2(&location_vector, being_location(value));

    while (it_water_turn < 7)
    {
        /* find higher land first */
        n_int    iturn = 8 - it_water_turn;
        n_int    loc_f = being_facing(value);
        n_int    iturn_plus  = loc_f + iturn;
        n_int    iturn_minus = loc_f + (256-iturn);

        n_byte   turn_plus  = (n_byte)((iturn_plus)  & 255);
        n_byte   turn_minus = (n_byte)((iturn_minus) & 255);
        n_vect2  temp_vector;

        n_int  z_plus;
        n_int  z_minus;

        vect2_direction(&temp_vector, turn_plus, 128);
        vect2_add(&temp_vector, &temp_vector, &location_vector);

        z_plus = QUICK_LAND(land, POSITIVE_LAND_COORD(APESPACE_TO_MAPSPACE(temp_vector.x)), POSITIVE_LAND_COORD(APESPACE_TO_MAPSPACE(temp_vector.y)));

        vect2_direction(&temp_vector, turn_minus, 128);
        vect2_add(&temp_vector, &temp_vector, &location_vector);

        z_minus = QUICK_LAND(land, POSITIVE_LAND_COORD(APESPACE_TO_MAPSPACE(temp_vector.x)), POSITIVE_LAND_COORD(APESPACE_TO_MAPSPACE(temp_vector.y)));

        if (z_minus > z_plus)
        {
            being_wander(value, -iturn);
        }
        else if (z_minus < z_plus)
        {
            being_wander(value, iturn);
        }
        it_water_turn++;
    }
}

enum inventory_type being_carried(noble_being * value, enum BODY_INVENTORY_TYPES location)
{
    return ((value)->inventory[location]>>3)<<3;
}

void being_drop(noble_being * value, enum BODY_INVENTORY_TYPES location)
{
    (value)->inventory[location] &= 7;
    GET_A(value,ATTENTION_BODY) = location;
}

void being_take(noble_being * value, enum BODY_INVENTORY_TYPES location, enum inventory_type object)
{
    (value)->inventory[location] |= object;
    GET_A(value,ATTENTION_BODY) = location;
}

void being_loop_no_thread(noble_simulation * sim, noble_being * being_not, being_loop_fn bf_func, void * data)
{
    n_uint loop = 0;
    while (loop < sim->num)
    {
        noble_being * output = &(sim->beings[loop]);
        if (output != being_not)
        {
            bf_func(sim, output, data);
        }
        loop++;
    }
}
/*
static void being_loop_generic(noble_simulation * sim, noble_being * being_not, being_loop_fn bf_func, void * data)
{
#ifdef EXECUTE_THREADED
    n_uint loop = 0;
    while (loop < sim->num)
    {
        noble_being * output = &(sim->beings[loop]);
        if (output != being_not)
        {            
            execute_add(((execute_function*)bf_func), (void*)sim, (void*)output, data);
        }
        loop++;
    }
#else
    being_loop_no_thread(sim, being_not, bf_func, data);
#endif
}
 */

void being_loop(noble_simulation * sim, being_loop_fn bf_func, n_int beings_per_thread)
{
#if 1
    n_uint loop  = 0;
    n_int  count = beings_per_thread;
    while (loop < sim->num)
    {
        noble_being * output = &(sim->beings[loop]);
        
        if ((beings_per_thread + loop) >= sim->num)
        {
            count = sim->num - loop;
        }
        
        execute_group(((execute_function*)bf_func), (void*)sim, (void*)output, count, sizeof(noble_being));
        
        if (count != beings_per_thread)
        {
            break;
        }
        else
        {
            loop += count;
        }
    }
#else
    being_loop_generic(sim, 0L, bf_func, 0L);
#endif
}

/**
 * @brief Check if a being is on ground or in water
 * @param px x coordinate of the being location
 * @param py y coordinate of the being location
 * @param params
 * @return 1 if on ground, 0 otherwise
 */
static n_byte	being_ground(n_int px, n_int py, n_int dx, n_int dy, void * params)
{
    n_int        abs_sum = ABS(dx) + ABS(dy);
    being_draw * being_pixel = (being_draw *) params;
    n_int        d_vis = being_pixel->visibility_delta;
    n_int	     local_z = ((px*(being_pixel->offset_x)) + (py*(being_pixel->offset_y))) >> 9;
    
    if (abs_sum)
    {
        weather_values   seven_values = weather_seven_values(being_pixel->land, MAPSPACE_TO_APESPACE(px), MAPSPACE_TO_APESPACE(py));
        n_int  span10 = ((abs_sum - 1) ? 1448 : 1024);
        
        switch (seven_values)
        {
            case WEATHER_SEVEN_SUNNY_DAY:
            case WEATHER_SEVEN_CLOUDY_DAY:
                being_pixel->visibility_total += (span10 * (d_vis + 16)) >> 11;
                break;
            case WEATHER_SEVEN_RAINY_DAY:
            case WEATHER_SEVEN_DAWN_DUSK:
                being_pixel->visibility_total += (span10 * ((2 * d_vis) + 25)) >> 11;
                break;
            case WEATHER_SEVEN_CLEAR_NIGHT:
                being_pixel->visibility_total += (span10 * ((5 * d_vis) + 65)) >> 11;
            case WEATHER_SEVEN_CLOUDY_NIGHT:
                being_pixel->visibility_total += (span10 * ((8 * d_vis) + 93)) >> 11;
            case WEATHER_SEVEN_RAINY_NIGHT:
                being_pixel->visibility_total += (span10 * ((12 * d_vis) + 145)) >> 11;
                break;
                
            case WEATHER_SEVEN_ERROR:
            default:
                return 1;
        }
        if (being_pixel->visibility_total > VISIBILITY_MAXIMUM)
            return 1;
        
        local_z += being_pixel->start_z;
        
        if (local_z < WALK_ON_WATER(QUICK_LAND(being_pixel->land, px, py),being_pixel->land->tide_level))
        {
            return 1;
        }
    }
    return 0;
}

static n_byte being_los_projection(n_land * land, noble_being * local, n_int lx, n_int ly)
{
    n_vect2    start, delta, vector_facing;

    /* TODO: Check for being awake - need a land and being based awake check */
    
    vect2_byte2(&start, being_location(local));

    delta.x = lx;
    delta.y = ly;

    vect2_subtract(&delta, &delta, &start);

    {
        n_int distance_squared = vect2_dot(&delta, &delta, 1, 1);
        if (distance_squared > (VISIBILITY_SPAN * VISIBILITY_SPAN))
            return 0;
    }
    /** check trivial case first - self aware */

    if ((delta.x == 0) && (delta.y == 0))
    {
        return 1;
    }
    being_facing_vector(local, &vector_facing, 16);

    /* if it is behind, it can't be in the line of sight */
    if (vect2_dot(&vector_facing, &delta, 1, 64) < 0)
    {
        return 0;
    }

    /** move everything from being co-ordinates to map co-ordinates */
    start.x = APESPACE_TO_MAPSPACE(start.x);
    start.y = APESPACE_TO_MAPSPACE(start.y);
    delta.x = APESPACE_TO_MAPSPACE(delta.x);
    delta.y = APESPACE_TO_MAPSPACE(delta.y);

    /* check trivial case first - self aware (after co-ord translation) */
    if ((delta.x == 0) && (delta.y == 0))
    {
        return 1;
    }

    {
        n_int	start_z = (n_int)WALK_ON_WATER(QUICK_LAND(land, start.x, start.y),land->tide_level) + 3; /* the nominal height of the Noble Ape */
        n_int	delta_z = ((n_int)WALK_ON_WATER(QUICK_LAND(land, (start.x + delta.x), (start.y + delta.y)),land->tide_level)) - start_z + 3; /* the nominal height of the Noble Ape */
        n_int	common_divisor = vect2_dot(&delta, &delta, 1, 1);
        being_draw 	  translate;

        if(common_divisor == 0)
        {
            common_divisor = 1;
        }

        {
            n_vect2 offset = {0};
            
            vect2_d(&offset, &delta, 512 * delta_z, common_divisor);

            start_z -= vect2_dot(&start, &offset, 1, 512);

            translate.start_z = start_z;
            translate.offset_x = offset.x;
            translate.offset_y = offset.y;
            
            translate.visibility_total = 100 * GENE_VISION_INITIAL(being_genetics(local));
            
            translate.visibility_delta = GENE_VISION_DELTA(being_genetics(local));
        }

        translate.land = land;
        {
            n_join		  being_point;
            being_point.information = (void *) &translate;
            being_point.pixel_draw  = &being_ground;

            if(math_join_vect2(start.x, start.y, &delta, &being_point))
            {
                return 0;
            }
        }
    }
    return 1;
}

typedef struct{
    n_string name;
    noble_being * being_from_name;
}being_from_name_loop_struct;

static void being_from_name_loop(noble_simulation * sim, noble_being * local, void * data)
{
    being_from_name_loop_struct * bfns = (being_from_name_loop_struct *)data;
    n_string_block str;

    if (bfns->being_from_name)
    {
        return;
    }
    
    being_name_simple(local, str);
    
    io_lower(str, io_length(str,STRING_BLOCK_SIZE));
    
    if (io_find(str,0,io_length(str,STRING_BLOCK_SIZE),bfns->name,io_length(bfns->name,STRING_BLOCK_SIZE))>-1)
    {
        bfns->being_from_name = local;
    }
}

/**
 * @brief return the being array index with the given name
 * @param sim Pointer to the simulation object
 * @param name Name of the being
 * @return Array index of the being within the simulation object
 */
noble_being * being_from_name(noble_simulation * sim, n_string name)
{
    being_from_name_loop_struct bfns;
    bfns.being_from_name = 0L;
    io_lower(name, io_length(name,STRING_BLOCK_SIZE));
    bfns.name = name;
    being_loop_no_thread(sim, 0L, being_from_name_loop, &bfns);
    return bfns.being_from_name;
}

void being_set_select_name(noble_simulation * sim, n_string name)
{
    noble_being * response = being_from_name(sim, name);
    if (response == 0L)
    {
        (void)SHOW_ERROR("Ape not found");
        return;
    }
    sim->select = response;
}

n_string being_get_select_name(noble_simulation * sim)
{
    static n_string_block name;
    n_int  position = 0;
    if (sim->select == 0L)
    {
        io_string_write(name,"*** ALL APES DEAD ***", &position);
    }
    else
    {
        being_name_simple(sim->select, name);
    }
    return (n_string)name;
}


/**
 This checks to see if a Noble Ape can see a particular point
 @param sim The simulation pointer
 @param reference The specific Noble Ape checked for being awake
 @param lx The x location of the point to be seen.
 @param ly The y location of the point to be seen.
 @return 1 can see, 0 can not see
 */
n_byte being_los(n_land * land, noble_being * local, n_byte2 lx, n_byte2 ly)
{
    /* There is probably a logical simplification of this
       as I can't think of it here is the brute force method.

       The Noble Ape Simulation universe wraps around in all
       directions you need to calculate the line of site off the map too. */
    n_int	local_facing = ((being_facing(local))>>5);

    /*
           6
         5   7
       4       0
         3   1
           2
    */

    if (being_los_projection(land, local,lx,ly) == 1)
        return 1;

    if ((local_facing == 6) || (local_facing == 7) || (local_facing == 0) || (local_facing == 1) || (local_facing == 2))
    {
        if (being_los_projection(land, local,lx+MAP_APE_RESOLUTION_SIZE,ly) == 1)
            return 1;
    }

    if ((local_facing == 7) || (local_facing == 0) || (local_facing == 1) || (local_facing == 2) || (local_facing == 3))
    {
        if (being_los_projection(land, local,lx+MAP_APE_RESOLUTION_SIZE,ly+MAP_APE_RESOLUTION_SIZE) == 1)
            return 1;
    }
    if ((local_facing == 0) || (local_facing == 1) || (local_facing == 2) || (local_facing == 3) || (local_facing == 4))
    {
        if (being_los_projection(land, local,lx,ly+MAP_APE_RESOLUTION_SIZE) == 1)
            return 1;
    }
    if ((local_facing == 1) || (local_facing == 2) || (local_facing == 3) || (local_facing == 4) || (local_facing == 5))
    {
        if (being_los_projection(land, local,lx-MAP_APE_RESOLUTION_SIZE,ly+MAP_APE_RESOLUTION_SIZE) == 1)
            return 1;
    }
    if ((local_facing == 2) || (local_facing == 3) || (local_facing == 4) || (local_facing == 5) || (local_facing == 6))
    {
        if (being_los_projection(land, local,lx-MAP_APE_RESOLUTION_SIZE,ly) == 1)
            return 1;
    }
    if ((local_facing == 3) || (local_facing == 4) || (local_facing == 5) || (local_facing == 6) || (local_facing == 7))
    {
        if (being_los_projection(land, local,lx-MAP_APE_RESOLUTION_SIZE,ly-MAP_APE_RESOLUTION_SIZE) == 1)
            return 1;
    }
    if ((local_facing == 4) || (local_facing == 5) || (local_facing == 6) || (local_facing == 7) || (local_facing == 0))
    {
        if (being_los_projection(land, local,lx,ly-MAP_APE_RESOLUTION_SIZE) == 1)
            return 1;
    }
    if ((local_facing == 5) || (local_facing == 6) || (local_facing == 7) || (local_facing == 0) || (local_facing == 1))
    {
        if (being_los_projection(land, local,lx+MAP_APE_RESOLUTION_SIZE,ly-MAP_APE_RESOLUTION_SIZE) == 1)
            return 1;
    }
    return 0;
}

static void being_immune_init(noble_being * local)
{
#ifdef IMMUNE_ON
    n_byte i;
    n_byte2 * local_random = being_get_random(local);
    noble_immune_system * immune = &(local->immune_system);

    for (i = 0; i < IMMUNE_ANTIGENS; i += 2)
    {
        immune->antigens[i]=0;
        immune->antigens[i+1]=0;
        math_random3(local_random);
        immune->shape_antigen[i] = (n_byte)(local_random[0]&255);
        immune->shape_antigen[i+1] = (n_byte)(local_random[1]&255);
    }
    for (i = 0; i < IMMUNE_POPULATION; i += 2)
    {
        immune->antibodies[i]=0;
        immune->antibodies[i+1]=0;
        math_random3(local_random);
        immune->shape_antibody[i] = (n_byte)(local_random[0]&255);
        immune->shape_antibody[i+1] = (n_byte)(local_random[1]&255);
    }
#endif
}

static void being_immune_seed(noble_being * mother, noble_being * child)
{
#ifdef IMMUNE_ON
    n_byte i;
    noble_immune_system * immune_mother = &(mother->immune_system);
    noble_immune_system * immune_child = &(child->immune_system);

    /** child acquires mother's antibodies */
    for (i=0; i<IMMUNE_POPULATION; i++)
    {
        immune_child->shape_antibody[i]=immune_mother->shape_antibody[i];
        immune_child->antibodies[i]=immune_mother->antibodies[i];
    }
#endif
}

static void being_acquire_pathogen(noble_being * local, n_byte transmission_type)
{
#ifdef IMMUNE_ON
    n_byte i;
    noble_immune_system * immune = &(local->immune_system);
    n_byte2 * local_random = being_get_random(local);

    math_random3(local_random);
    if (local_random[0] < PATHOGEN_ENVIRONMENT_PROB)
    {
        i = local_random[1]%IMMUNE_ANTIGENS;
        if (immune->antigens[i]==0)
        {
            math_random3(local_random);
            immune->antigens[i]=(n_byte)(local_random[0]&7);
            immune->shape_antigen[i] = (n_byte)RANDOM_PATHOGEN(local_random[1], transmission_type);
        }
    }
#endif
}

void being_ingest_pathogen(noble_being * local, n_byte food_type)
{
    n_byte transmission_type=food_type+PATHOGEN_TRANSMISSION_FOOD_VEGETABLE;
    being_acquire_pathogen(local,transmission_type);
}

void being_immune_transmit(noble_being * meeter_being, noble_being * met_being, n_byte transmission_type)
{
#ifdef IMMUNE_ON
    n_byte i,j;
    n_byte2 * local_random = being_get_random(meeter_being);
    noble_immune_system * immune0 = &(meeter_being->immune_system);
    noble_immune_system * immune1 = &(met_being->immune_system);

    /** pathogen obtained from environment */
    being_acquire_pathogen(meeter_being, transmission_type);

    /** pathogen transmitted between beings */
    math_random3(local_random);
    if (local_random[0] < PATHOGEN_TRANSMISSION_PROB)
    {
        math_random3(local_random);
        i = local_random[0]%IMMUNE_ANTIGENS;
        if ((immune0->antigens[i]>0) &&
                (PATHOGEN_TRANSMISSION(immune0->shape_antigen[i])==transmission_type))
        {
            /** does the other being already carry this pathogen ? */
            for (j = 0; j < IMMUNE_ANTIGENS; j++)
            {
                if (immune0->shape_antigen[i]==immune1->shape_antigen[j])
                {
                    if (immune1->antigens[j]<255) immune1->antigens[j]++;
                    break;
                }
            }
            if (j == IMMUNE_ANTIGENS)
            {
                j = local_random[1]%IMMUNE_ANTIGENS;
                if (immune1->antigens[j]<=MIN_ANTIGENS)
                {
                    /** spread pathogen */
                    immune1->shape_antigen[j]=immune0->shape_antigen[i];
                }
            }
        }
    }
#endif
}

static void being_immune_response(noble_being * local)
{
#ifdef IMMUNE_ON
    n_int min_antibodies;
    n_int max_bits_matched;
    n_byte2 total_antigens,max_severity;
    n_byte i,j,k,match,best_match,bits_matched,bit;
    n_byte2 * local_random = being_get_random(local);
    noble_immune_system * immune = &(local->immune_system);

    /** antibodies die at some fixed rate */
    math_random3(local_random);
    if (local_random[0]<ANTIBODY_DEPLETION_PROB)
    {
        i = local_random[1]%IMMUNE_POPULATION;
        if (immune->antibodies[i]>0)
        {
            immune->antibodies[i]--;
        }
    }

    /** pick an antigen */
    math_random3(local_random);
    i = local_random[0]%IMMUNE_ANTIGENS;
    if (immune->antigens[i] != 0)
    {
        /** mutate with some probability */
        if (local_random[1]<PATHOGEN_MUTATION_PROB)
        {
            math_random3(local_random);
            if ((immune->shape_antigen[i] & (1<<(local_random[0]&7))) != 0)
            {
                immune->shape_antigen[i] ^= (local_random[0]&7);
            }
            else
            {
                immune->shape_antigen[i] |= (local_random[0]&7);
            }
        }

        /** try to find a matching antibody */
        max_bits_matched=0;
        best_match=0;
        for (j = 0; j < IMMUNE_POPULATION; j++)
        {
            match = (immune->shape_antibody[j] & immune->shape_antigen[i]) |
                    ((~immune->shape_antibody[j]) & (~immune->shape_antigen[i]));
            if (match!=0)
            {
                /** how good is the fit ? */
                bits_matched=0;
                for (bit=0; bit<8; bit++)
                {
                    if ((match & (1<<bit)) != 0)
                    {
                        bits_matched++;
                    }
                }
                /** record best fit */
                if (bits_matched>max_bits_matched)
                {
                    max_bits_matched=bits_matched;
                    best_match = j;
                }
            }
        }

        /** select the antibody with the smallest population */
        min_antibodies=immune->antibodies[0];
        j=0;
        for (k=1; k<IMMUNE_POPULATION; k++)
        {
            if (immune->antibodies[k]<min_antibodies)
            {
                min_antibodies = immune->antibodies[k];
                j = k;
            }
        }

        /** match antigen and antibody */
        if (max_bits_matched>IMMUNE_FIT)
        {
            /** Antibodies multiply
             A better fit results in more antibodies */
            if (immune->antibodies[best_match]<255-max_bits_matched)
            {
                immune->antibodies[best_match]+=(n_byte)max_bits_matched;
                /** apply a minimum threshold so that generated
                 antibodies don't overwrite known good fits */
                if (immune->antibodies[best_match]<MIN_ANTIBODIES)
                {
                    immune->antibodies[best_match]=MIN_ANTIBODIES;
                }
            }
            /** antigens are depleted according to the immune system strength */
            if (immune->antigens[i]>being_honor_immune(local))
            {
                immune->antigens[i]-=being_honor_immune(local);
            }
            else
            {
                immune->antigens[i]=0;
            }
            /** clone antibody with mutation */
            if (j!=best_match)
            {
                immune->antibodies[j]=1;
                match = immune->shape_antibody[best_match];
                math_random3(local_random);
                if ((match & (1<<(local_random[0]&7))) != 0)
                {
                    match ^= (local_random[0]&7);
                }
                else
                {
                    match |= (local_random[0]&7);
                }
                immune->shape_antibody[j] = match;
            }
        }
        else
        {
            /** If pathogens are not challenged they multiply */
            if (immune->antigens[i]<255)
            {
                immune->antigens[i]++;
            }

            /** produce differently shaped antibodies */
            math_random3(local_random);
            if (local_random[0]<ANTIBODY_GENERATION_PROB(local))
            {
                math_random3(local_random);
                immune->shape_antibody[j]=(n_byte)(local_random[0]&255);
                immune->antibodies[j]=(n_byte)(local_random[1]&7);
            }
        }
    }

    /** Energy level is reduced based upon pathogens.
     Note that not all pathogens have the same energy cost. */
    total_antigens=0;
    max_severity=0;
    for (i=0; i<IMMUNE_ANTIGENS; i++)
    {
        /* total quantity of pathogen */
        total_antigens+=immune->antigens[i];
        /* record the maximum pathogen severity */
        if (immune->shape_antigen[i]>max_severity)
        {
            max_severity=immune->shape_antigen[i];
        }
    }
    math_random3(local_random);
    if ((local_random[0] < (total_antigens>>2)) && (being_energy_less_than(local, BEING_DEAD + 1) == 0))
    {
        being_energy_delta(local, 0-PATHOGEN_SEVERITY(max_severity));
    }
#endif
}

static n_uint being_genetic_count_zeros(n_genetics count)
{
    n_uint loop = 0;
    n_uint addition = 0;
    while (loop < sizeof(n_genetics)*8)
    {
        if (((count>>loop) & 1) == 0)
        {
            addition++;
        }
        loop++;
    }
    return addition;
}

n_uint being_genetic_comparison(n_genetics * primary, n_genetics * secondary, n_int parse_requirements)
{
    n_int   loop = 0;
    n_uint  addition = 0;
    
    if (FIND_SEX(secondary[CHROMOSOME_Y]) != SEX_FEMALE)
    {
        if (parse_requirements == 1) return 0;
    }
    else
    {
        if (parse_requirements == 0) return 0;
    }
    
    while (loop < CHROMOSOMES)
    {
        n_genetics comparison = primary[loop] ^ secondary[loop];
        addition += being_genetic_count_zeros(comparison);
        loop++;
    }
    return addition;
}

typedef struct
{
    n_int         parse_requirements;
    n_int         older;
    n_uint        comparison_best;
    n_int         actual_age;
    n_genetics  * genetics;
    noble_being * return_value;
} being_find_closest_struct;

static void being_find_closest_loop(noble_simulation * sim, noble_being * local, void * data)
{
    being_find_closest_struct * bfcs = (being_find_closest_struct *)data;

    n_byte success = 0;
    n_int  local_dob = being_dob(local);
    
    if (bfcs->older == 0) success = 1;
    
    if ((bfcs->older == 1) && ((local_dob - AGE_OF_MATURITY) > bfcs->actual_age))
    {
        success = 1;
    }
    
    if ((bfcs->older == -1) && ((bfcs->actual_age - AGE_OF_MATURITY) > local_dob))
    {
        success = 1;
    }
    
    if (success)
    {
        n_uint comparison = being_genetic_comparison(bfcs->genetics, being_genetics(local), bfcs->parse_requirements);
        if (comparison > bfcs->comparison_best)
        {
            bfcs->comparison_best = comparison;
            bfcs->return_value = local;
        }
    }

}

static noble_being * being_find_closest(noble_simulation * sim, noble_being * actual, n_int parse_requirements, n_int older)
{
    being_find_closest_struct bfcs;
    bfcs.parse_requirements = parse_requirements;
    bfcs.older = older;
    /** comparison must be better than average */
    bfcs.comparison_best = 3 * sizeof(n_genetics) * CHROMOSOMES;
    bfcs.return_value = 0L;
    bfcs.genetics = being_genetics(actual);
    bfcs.actual_age = being_dob(actual);
    
    being_loop_no_thread(sim, actual, being_find_closest_loop, &bfcs);
    
    return bfcs.return_value;
}

typedef struct
{
    n_int         today;
    n_uint        comparison_best;
    n_int         max_age;
    n_genetics  * genetics;
    noble_being * return_value;
} being_find_child_struct;

static void being_find_child_loop(noble_simulation * sim, noble_being * local, void * data)
{
    being_find_child_struct * bfcs = (being_find_child_struct *) data;
    n_uint        comparison = being_genetic_comparison(bfcs->genetics, being_genetics(local), -1);
    if ((comparison > bfcs->comparison_best) &&  ((bfcs->today - being_dob(local)) < bfcs->max_age))
    {
        bfcs->comparison_best = comparison;
        bfcs->return_value = local;
    }
}

static noble_being * being_find_child(noble_simulation * sim, n_genetics * genetics, n_int max_age)
{
    being_find_child_struct bfcs;
    
    bfcs.today = TIME_IN_DAYS(sim->land->date);
    bfcs.comparison_best = 0;
    bfcs.max_age = max_age;
    bfcs.genetics = genetics;
    bfcs.return_value = 0L;
    being_loop_no_thread(sim, 0L, being_find_child_loop, &bfcs);
    return bfcs.return_value;
}

typedef struct
{
    n_byte2 first_gender;
    n_byte2 family;
    noble_being * local;
} being_find_name_struct;

static void being_find_name_loop(noble_simulation * sim, noble_being * local, void * data)
{
    being_find_name_struct * bfns = (being_find_name_struct *)data;
    if (bfns->local == 0L)
    {
        if (being_name_comparison(local, bfns->first_gender, bfns->family))
        {
            bfns->local = local;
        }
    }
}

noble_being * being_find_name(noble_simulation * sim, n_byte2 first_gender, n_byte2 family)
{
    being_find_name_struct bfns;
    bfns.first_gender = first_gender;
    bfns.family = family;
    bfns.local = 0L;
    being_loop_no_thread(sim, 0L, being_find_name_loop, &bfns);
    return bfns.local;
}

/** returns the total positive and negative affect within memory */
n_uint being_affect(noble_simulation * local_sim, noble_being * local, n_byte is_positive)
{
    n_uint affect = 0;
#ifdef EPISODIC_ON
    n_uint i;
    noble_episodic * local_episodic = being_episodic(local);
    if (!local_episodic) return affect;

    for (i=0; i<EPISODIC_SIZE; i++)
    {
        if (is_positive!=0)
        {
            if (local_episodic[i].affect>EPISODIC_AFFECT_ZERO)
            {
                affect += (n_uint)(local_episodic[i].affect) - EPISODIC_AFFECT_ZERO;
            }
        }
        else
        {
            if (local_episodic[i].affect<EPISODIC_AFFECT_ZERO)
            {
                affect += EPISODIC_AFFECT_ZERO - (n_uint)(local_episodic[i].affect);
            }
        }
    }
#endif
    return affect;
}

const n_string body_inventory_description[INVENTORY_SIZE] =
{
    "Head","Teeth","Back","Front","Left hand","Right hand","Left foot","Right foot"
};

n_string being_body_inventory_description(n_int index)
{
    return body_inventory_description[index];
}

const n_string relationship_description[RELATIONSHIPS] =
{
    "Associate","Self","Mother","Father","Daughter",
    "Son","Granddaughter","Grandson","Sister","Brother",
    "Maternal Grandmother","Maternal Grandfather","Paternal Grandmother","Paternal Grandson", "Mother",
    "Father","Daughter","Son","Granddaughter","Grandson",
    "Sister","Brother","Maternal Grandmother","Maternal Grandfather","Paternal Grandmother",
    "Paternal Grandson"
};

void being_relationship_description(n_int index, n_string description)
{
    n_int position = 0;
    if (index >= RELATIONSHIPS)
    {
        sprintf(description,"ERROR: relationship out of range %d\n",(int)index);
        return;
    }
    io_string_write(description, relationship_description[index], &position);
}

static void being_inventory_string(n_string string, n_int * location, n_int item)
{
    switch (item)
    {
    case INVENTORY_BRANCH:
        io_string_write(string,"branch",location);
        break;
    case INVENTORY_ROCK:
        io_string_write(string,"rock",location);
        break;
    case INVENTORY_SHELL:
        io_string_write(string,"shell",location);
        break;
    case INVENTORY_TWIG:
        io_string_write(string,"twig",location);
        break;
    case INVENTORY_NUT_CRACKED:
        io_string_write(string,"cracked nut",location);
        break;
    case INVENTORY_GRASS:
        io_string_write(string,"piece of grass",location);
        break;
    case INVENTORY_SCRAPER:
        io_string_write(string,"scraper",location);
        break;
    case INVENTORY_SPEAR:
        io_string_write(string,"spear",location);
        break;
    case INVENTORY_FISH:
        io_string_write(string,"fish",location);
        break;
    case INVENTORY_BIRD_EGGS:
        io_string_write(string,"bird eggs",location);
        break;
    case INVENTORY_LIZARD_EGGS:
        io_string_write(string,"lizard eggs",location);
        break;
    case INVENTORY_CHILD:
    case INVENTORY_WOUND:
    case INVENTORY_GROOMED:
    default:
        io_string_write(string,"thing being carried",location);
        break;
    }
}

static void being_social_event_string(n_string string, n_int * location, n_int event_type, n_string name_str)
{
    switch (event_type)
    {
    case EVENT_MATE:
        io_string_write(string,"Mated with ",location);
        break;
    case EVENT_SEEK_MATE:
        io_string_write(string,"Searched for mate ",location);
        break;
    case EVENT_GROOM:
        io_string_write(string,"Groomed ",location);
        break;
    case EVENT_GROOMED:
        io_string_write(string,"Groomed by ",location);
        break;
    case EVENT_CHAT:
        io_string_write(string,"Chatted with ",location);
        break;
    case EVENT_BIRTH:
        io_string_write(string,"Gave birth to ",location);
        break;
    case EVENT_HURLED:
        io_string_write(string,"Hurled a rock at ",location);
        break;
    case EVENT_HURLED_BY:
        io_string_write(string,"Was hit by a rock hurled by ",location);
        break;

    case EVENT_HIT:
        io_string_write(string,"Hit ",location);
        break;
    case EVENT_HIT_BY:
        io_string_write(string,"Hit by ",location);
        break;
    case EVENT_CARRIED:
        io_string_write(string,"Carried ",location);
        break;
    case EVENT_CARRIED_BY:
        io_string_write(string,"Carried by ",location);
        break;
    case EVENT_SUCKLED:
        io_string_write(string,"Suckled ",location);
        break;
    case EVENT_SUCKLED_BY:
        io_string_write(string,"Suckled by ",location);
        break;
    case EVENT_WHACKED:
        io_string_write(string,"Whacked ",location);
        break;
    case EVENT_WHACKED_BY:
        io_string_write(string,"Whacked by ",location);
        break;
    case EVENT_HUGGED:
        io_string_write(string,"Hugged ",location);
        break;
    case EVENT_HUGGED_BY:
        io_string_write(string,"Hugged by ",location);
        break;
    case EVENT_PRODDED:
        io_string_write(string,"Prodded ",location);
        break;
    case EVENT_PRODDED_BY:
        io_string_write(string,"Prodded by ",location);
        break;
    case EVENT_GIVEN:
        io_string_write(string,"Given ",location);
        break;
    case EVENT_GIVEN_BY:
        io_string_write(string,"Given by ",location);
        break;
    case EVENT_POINT:
        io_string_write(string,"Pointed to ",location);
        break;
    case EVENT_POINTED:
        io_string_write(string,"Pointed to by ",location);
        break;
    case EVENT_SMILED:
        io_string_write(string,"Smiled at ",location);
        break;
    case EVENT_SMILED_BY:
        io_string_write(string,"Smiled at by ",location);
        break;
    case EVENT_TICKLED:
        io_string_write(string,"Tickled ",location);
        break;
    case EVENT_TICKLED_BY:
        io_string_write(string,"Tickled by ",location);
        break;
    case EVENT_GLOWERED:
        io_string_write(string,"Glowered at ",location);
        break;
    case EVENT_GLOWERED_BY:
        io_string_write(string,"Glowered at by ",location);
        break;
    case EVENT_PATTED:
        io_string_write(string,"Patted ",location);
        break;
    case EVENT_PATTED_BY:
        io_string_write(string,"Patted by ",location);
        break;
    default:
    {
        n_string_block  number_str;
        sprintf(number_str,"%ld", event_type);
        io_string_write(string,"Some erroneous action (",location);
        io_string_write(string,number_str,location);
        io_string_write(string,") with ",location);
        break;
    }
    }
    io_string_write(string,name_str,location);
}

void being_remains_init(noble_simulation * sim)
{
    noble_remains * remains = sim->remains;
    
    remains->count = 0;
    remains->location = 0;
}

void being_remains(noble_simulation * sim, noble_being * dead)
{
    noble_remains * remains  = sim->remains;
    n_byte2         location = remains->location;

    remains->bodies[location].location[0] = dead->location[0];
    remains->bodies[location].location[1] = dead->location[1];
    remains->location = (remains->location + 1) % NUMBER_OF_BODIES;

    if (remains->count < NUMBER_OF_BODIES)
    {
        remains->count++;
    }
}

n_int episode_description(
    noble_simulation * sim,
    noble_being * local_being,
    n_int index,
    n_string description)
{
    n_string_block str;
    n_int string_index = 0;
    n_int social = 0;
#ifdef EPISODIC_ON
    n_string_block str2, name_str;
    noble_episodic * local_episodic;
    n_uint days_elapsed,time_elapsed;
    n_uint current_date;

    current_date = TIME_IN_DAYS(sim->land->date);
    local_episodic = being_episodic(local_being);

    if(local_episodic == 0L)
    {
        return SHOW_ERROR("No episodic description");
    }

    if ((local_episodic[index].event>0) &&
            being_name_comparison(local_being, local_episodic[index].first_name[0], local_episodic[index].family_name[0]))
    {
        being_name_byte2(local_episodic[index].first_name[BEING_MET], local_episodic[index].family_name[BEING_MET], name_str);

        switch(local_episodic[index].event)
        {
        case EVENT_EAT:
        {
            io_string_write(str,"Was eating ",&string_index);
            switch(local_episodic[index].food)
            {
            case FOOD_VEGETABLE:
            {
                io_string_write(str,"vegetation",&string_index);
                break;
            }
            case FOOD_FRUIT:
            {
                io_string_write(str,"fruit",&string_index);
                break;
            }
            case FOOD_SHELLFISH:
            {
                io_string_write(str,"shellfish",&string_index);
                break;
            }
            case FOOD_SEAWEED:
            {
                io_string_write(str,"seaweed",&string_index);
                break;
            }
            case FOOD_BIRD_EGGS:
            {
                io_string_write(str,"bird eggs",&string_index);
                break;
            }
            case FOOD_LIZARD_EGGS:
            {
                io_string_write(str,"lizard eggs",&string_index);
                break;
            }
            }
            break;
        }
        case EVENT_SWIM:
        {
            io_string_write(str,"Went swimming",&string_index);
            break;
        }
        case EVENT_GROOM:
        {
            io_string_write(str,"Groomed ",&string_index);
            io_string_write(str,name_str,&string_index);
            io_string_write(str,"'s ",&string_index);
            io_string_write(str,being_body_inventory_description(local_episodic[index].arg),&string_index);

            social = 1;
            break;
        }
        case EVENT_SHOUT:
        {
            io_string_write(str,"Shouted ",&string_index);
            break;
        }
        case EVENT_FISH:
        {
            io_string_write(str,"Caught a fish ",&string_index);
            break;
        }
        case EVENT_CHEW:
        {
            io_string_write(str,"Chewing ",&string_index);
            if (local_episodic[index].arg & INVENTORY_GRASS)
            {
                io_string_write(str,"grass ",&string_index);
            }
            else
            {
                if (local_episodic[index].arg & INVENTORY_TWIG)
                {
                    io_string_write(str,"twig ",&string_index);
                }
                else
                {
                    if (local_episodic[index].arg & INVENTORY_FISH)
                    {
                        io_string_write(str,"fish ",&string_index);
                    }
                    else
                    {
                        if (local_episodic[index].arg & INVENTORY_NUT_CRACKED)
                        {
                            io_string_write(str,"a cracked nut ",&string_index);
                        }
                        else
                        {
                            {
                                if (local_episodic[index].arg & INVENTORY_BIRD_EGGS)
                                {
                                    io_string_write(str,"birds eggs ",&string_index);
                                }
                                else
                                {
                                    if (local_episodic[index].arg & INVENTORY_LIZARD_EGGS)
                                    {
                                        io_string_write(str,"lizard eggs ",&string_index);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            if (local_episodic[index].arg & 1)
            {
                io_string_write(str,"held in right hand ",&string_index);
            }
            else
            {
                io_string_write(str,"held in left hand ",&string_index);
            }
            break;
        }
        case EVENT_DRAG:
            io_string_write(str,"Dragged a ",&string_index);
            being_inventory_string(str, &string_index, local_episodic[index].arg);
            break;
        case EVENT_BRANDISH:
            io_string_write(str,"Waved a ",&string_index);
            being_inventory_string(str, &string_index, local_episodic[index].arg);
            break;
        case EVENT_DROP:
            io_string_write(str,"Dropped a ",&string_index);
            being_inventory_string(str, &string_index, local_episodic[index].arg);
            break;
        case EVENT_PICKUP:
            io_string_write(str,"Picked up a ",&string_index);
            being_inventory_string(str, &string_index, local_episodic[index].arg);
            break;
        default:
            being_social_event_string(str, &string_index, local_episodic[index].event, name_str);
            social = 1;
            break;
        }

        if (string_index == 0)
        {
            return SHOW_ERROR("No string in episodic description");
        }

        days_elapsed = current_date - TIME_IN_DAYS(&(local_episodic[index].date[0]));
        if (days_elapsed==0)
        {
            time_elapsed = sim->land->time - local_episodic[index].time;

            if (time_elapsed<60)
            {
                if (time_elapsed == 0)
                {
                    io_string_write(str," now",&string_index);
                }
                else if (time_elapsed == 1)
                {
                    io_string_write(str," a minute ago",&string_index);
                }
                else if (time_elapsed<5)
                {
                    io_string_write(str," a few minutes ago",&string_index);
                }
                else
                {
                    sprintf((char*)str2," %d minutes ago", (int)time_elapsed);
                    io_string_write(str,str2, &string_index);
                }
            }
            else
            {
                if (time_elapsed<120)
                {
                    io_string_write(str," an hour ago",&string_index);
                }
                else
                {
                    sprintf((char*)str2," %d hours ago", (int)time_elapsed/60);
                    io_string_write(str,str2,&string_index);
                }
            }
        }
        else
        {
            if (days_elapsed==1)
            {
                io_string_write(str, " yesterday",&string_index);
            }
            else
            {
                sprintf((char*)str2," %d days ago", (int)days_elapsed);
                io_string_write(str, str2,&string_index);
            }
        }
    }
#endif
    str[string_index] = 0;
    
    string_index = 0;
    
    io_string_write(description, str, &string_index);
    
    return social;
}

#ifdef SHORT_NAMES

/* ape names */
#define NAMES_SURNAMES              16
#define NAMES_FIRST                 64

/** Surname = 16, Female = 64, Male = 64,  */
const n_string EnglishNames[144] =
{
    "Adams","Baker","Brown","Davis","Evans","Green","Jones","Mason",
    "Moore","Myers","Perry","Price","Quinn","Smith","White","Young",
    
    "Agnes","Aimee","Alice","Amber","Anita","April","Becky","Beryl",
    "Carla","Chloe","Donna","Doris","Edith","Elena","Elise","Ellen",
    "Emily","Erika","Ethel","Faith","Fiona","Flora","Gilda","Grace",
    "Hazel","Helen","Hilda","Holly","Irene","Janet","Jewel","Kacey",
    
    "Kerri","Lacey","Linda","Mabel","Madge","Mandy","Maude","Mavis",
    "Megan","Mercy","Misty","Molly","Nancy","Naomi","Norma","Nydia",
    "Pansy","Patty","Pearl","Polly","Rhoda","Robin","Sadie","Sally",
    "Sarah","Tammy","Tanya","Tilda","Tracy","Vania","Wanda","Wilda",

    "Aaron","Angus","Barry","Basil","Blair","Blake","Boris","Brent",
    "Brian","Brock","Bruce","Bruno","Byron","Casey","Cecil","Clive",
    "Clyde","Colin","Craig","Cyril","Damon","Darcy","David","Derek",
    "Edgar","Edwin","Elmer","Elroy","Elton","Errol","Felix","Floyd",
    
    "Frank","Garth","Gavin","Giles","Glenn","Grant","Henry","Homer",
    "Isaac","Jacob","Jason","Jesse","Keith","Kevin","Leroy","Lloyd",
    "Logan","Miles","Nigel","Oscar","Peter","Quinn","Ralph","Roger",
    "Scott","Shawn","Simon","Trent","Tyler","Unwin","Vance","Wayne"
};

#else

/* ape names */
#define NAMES_SURNAMES              64
#define NAMES_FIRST                 256

/** Surname = 64, Male = 256, Female = 256 */
const n_string EnglishNames[576] =
{
    "Adams","Allen","Bailey","Baker","Barnes","Bell","Brooks","Brown","Butler","Clark","Cook","Cooper","Davies","Davis","Evans",
    "Fisher","Foster","Graham","Gray","Green","Hall","Harris","Hill","Howard","Hughes","James","Jones","Kelly","King","Lewis",
    "Long","Mason","Matine","Miller","Moore","Morgan","Munroe","Murphy","Mutz","Myers","Nelson","Owen","Parker","Perry",
    "Powell","Price","Quinn","Reed","Reid","Rogers","Rose","Reis","Scrim","Smith","Taylor","Thiel","Turner","Walker","Ward",
    "Watson","White","Wilson","Wood","Young",
    
    "Ada","Agatha","Agnes","Aileen","Aimee","Alanna","Alda","Alice","Alina","Alison","Alma","Amanda","Amber","Andrea","Angela",
    "Anita","Anthea","April","Ariana","Arleen","Astrid","Audrey","Beata","Becky","Beryl","Bess","Bianca","Blair","Blythe",
    "Bonnie","Brenda","Briana","Brooke","Carla","Carly","Carmen","Cheryl","Chloe","Coral","Daphne","Davida","Dawn","Denise",
    "Donna","Dora","Doris","Echo","Eda","Edana","Edith","Edlyn","Edna","Edwina","Effie","Eileen","Elaine","Elena","Elga",
    "Elise","Eliza","Ella","Ellen","Eloise","Elsie","Elvira","Emily","Emma","Erika","Erin","Estra","Ethel","Eudora","Eunice",
    "Faith","Fannie","Fawn","Faye","Fedora","Fern","Fiona","Flora","Gale","Gaye","Geneva","Gilda","Gladys","Gloria","Grace",
    "Gwynne","Harley","Hattie","Hazel","Hetty","Hilda","Holly","Honey","Hope","Ingrid","Irene","Iris","Ivory","Ivy","Jade",
    "Jane","Janet","Janice","Jeanne","Jemima","Jewel","Joan","Joanna","Joy","June","Kacey","Kara","Kate","Kay","Keely","Kelsey",
    "Kendra","Kerri","Kyla","Lacey","Lane","Lara","Larina","Leanne","Leslie","Linda","Livia","Lizzie","Lois","Lorena","Lulu",
    "Luna","Lynn","Mabel","Madge","Maggie","Maia","Maisie","Mandy","Marcia","Margot","Marnia","Mary","Maude","Maura","Mavis",
    "Maxine","Megan","Melody","Mercy","Meris","Merle","Miriam","Misty","Moira","Molly","Mona","Monica","Mora","Morgan","Muriel",
    "Myra","Myrtle","Nancy","Naomi","Nell","Nerita","Nina","Noelle","Nola","Norma","Nydia","Odette","Olga","Opal","Oprah","Orva",
    "Page","Pamela","Pansy","Patty","Pearl","Phoebe","Polly","Quenna","Questa","Rachel","Ramona","Regina","Rhea","Rhoda","Rita",
    "Robin","Rosa","Rowena","Ruby","Ruth","Sacha","Sadie","Salena","Sally","Salome","Sandra","Sarah","Serena","Shana","Sharon",
    "Sheila","Sibley","Silver","Sirena","Talia","Tamara","Tammy","Tanya","Tara","Tasha","Tatum","Tess","Thalia","Thea","Thelma",
    "Thora","Tilda","Tina","Tracy","Trina","Trista","Tyne","Udele","Ula","Ulrica","Ulva","Una","Unity","Ursa","Ursula","Valda",
    "Vania","Veleda","Vera","Verda","Violet","Vita","Wanda","Wilda","Willa","Willow","Wynne","Zea","Zelda","Zera","Zoe",
    
    "Aaron","Abbott","Abel","Adam","Albern","Albert","Alfie","Alfred","Alvin","Amery","Amos","Andrew","Angus","Ansel","Arlen",
    "Arnold","Arvel","Austin","Axel","Baird","Barry","Basil","Bert","Blair","Blake","Boris","Brent","Brian","Brice","Brock",
    "Bruce","Bruno","Bryant","Buck","Bud","Burton","Byron","Calvin","Carl","Carter","Carver","Cary","Casey","Casper","Cecil",
    "Cedric","Claude","Clive","Clyde","Colin","Conan","Connor","Conrad","Conroy","Conway","Corwin","Craig","Crosby","Culver",
    "Curt","Curtis","Cyril","Damon","Daniel","Darcy","David","Dean","Declan","Dennis","Derek","Dermot","Derwin","Dexter",
    "Dillon","Dion","Dirk","Donald","Dorian","Drew","Dudley","Duncan","Dwayne","Dwight","Dylan","Earl","Edgar","Edwin","Efrain",
    "Egbert","Elbert","Elmer","Elroy","Elton","Elvis","Emmett","Emrick","Ernest","Errol","Esmond","Eugene","Evan","Ezra","Fabian",
    "Farley","Felix","Fenton","Ferris","Finbar","Floyd","Foster","Fox","Frank","Gale","Galvin","Garret","Garth","Gavin","George",
    "Gideon","Giles","Gilroy","Glenn","Godwin","Graham","Grant","Guy","Hadden","Hadley","Hadwin","Hale","Hall","Hamlin","Hardy",
    "Harley","Hector","Henry","Herman","Homer","Howard","Hubert","Hunter","Ian","Isaac","Isaiah","Ivan","Ives","Jack","Jacob",
    "Jarvis","Jason","Jasper","Jed","Jerome","Jesse","John","Joshua","Justin","Keaton","Keith","Kelsey","Kelvin","Kent","Kerry",
    "Kevin","Kirby","Kirk","Kit","Kody","Konrad","Kurt","Kyle","Lamont","Landon","Lane","Lars","Lee","Leroy","Leslie","Lester",
    "Lionel","Lloyd","Logan","Lowell","Lyndon","Marcus","Marlon","Martin","Marvin","Medwin","Melvin","Merlin","Miles","Morgan",
    "Morris","Morton","Murray","Neal","Nigel","Noel","Norman","Olaf","Olin","Oliver","Oscar","Oswald","Otis","Owen","Paul",
    "Perry","Peter","Philip","Pierce","Quincy","Quinn","Ralph","Rex","Riley","Rodney","Roger","Roland","Rolf","Ronald","Rory",
    "Ross","Roy","Rufus","Rupert","Ryan","Samson","Samuel","Scott","Sean","Seth","Shawn","Sidney","Simon","Sloane","Stacy",
    "Thomas","Toby","Todd","Tony","Trent","Trevor","Troy","Tyler","Unwin","Vance","Victor","Walter","Warren","Wayne","Wilbur",
    "Willis","Wyatt","Wylie"
};

#endif

#define FAMILY_NAME_AND_MOD (NAMES_SURNAMES - 1)
#define FIRST_NAME_AND_MOD  (NAMES_FIRST - 1)

#define UNPACK_FAMILY_FIRST_NAME(packed_family_name)  (packed_family_name & FAMILY_NAME_AND_MOD)
#define UNPACK_FAMILY_SECOND_NAME(packed_family_name) ((packed_family_name / NAMES_SURNAMES)&FAMILY_NAME_AND_MOD)
#define GET_NAME_FAMILY(f0,f1) ((n_byte2)((f0&FAMILY_NAME_AND_MOD)|((f1&FAMILY_NAME_AND_MOD)*NAMES_SURNAMES)))


void being_unpack_family(n_byte2 name, n_byte * values)
{
    values[0] = UNPACK_FAMILY_FIRST_NAME(name);
    values[1] = UNPACK_FAMILY_SECOND_NAME(name);
}

n_byte being_first_name(noble_being * value)
{
    if (value == 0L)
    {
        return 0;
    }
    {
        noble_social * local_social = being_social(value);
        return local_social->first_name[BEING_MET] & FIRST_NAME_AND_MOD;
    }
}

static void being_set_first_name(noble_being * value, n_byte2 name)
{
    noble_social * local_social = being_social(value);
    local_social->first_name[BEING_MET] = name & FIRST_NAME_AND_MOD;
}

void being_set_family_name(noble_being * value, n_byte first, n_byte last)
{
    noble_social * local_social = being_social(value);
    local_social->family_name[BEING_MET] = GET_NAME_FAMILY(first,last);
}

n_byte2 being_gender_name(noble_being * value)
{
    if (value == 0L)
    {
        return 0;
    }
    return (n_byte2)((being_first_name(value) | (FIND_SEX(GET_I(value))<<8)));
}

n_byte2 being_family_name(noble_being * value)
{
    if (value == 0L)
    {
        return 0;
    }
    return (GET_NAME_FAMILY(being_family_first_name(value),being_family_second_name(value)));
}

n_int being_name_comparison(noble_being * value, n_byte2 gender_name, n_byte2 family_name)
{
    return ((being_gender_name(value) == gender_name) && (being_family_name(value) == family_name));
}

n_byte being_family_first_name(noble_being * value)
{
    if (value == 0L)
    {
        return 0;
    }
    {
        noble_social * local_social = being_social(value);
        return UNPACK_FAMILY_FIRST_NAME(local_social->family_name[BEING_MET]);
    }
}

n_byte being_family_second_name(noble_being * value)
{
    if (value == 0L)
    {
        return 0;
    }
    {
        noble_social * local_social = being_social(value);
        return UNPACK_FAMILY_SECOND_NAME(local_social->family_name[BEING_MET]);
    }
}

static void  being_name(n_byte female, n_int first, n_byte family0, n_byte family1, n_string name)
{
    n_int  position = 0;
    if (first != -1)
    {
        if (female)
        {
            io_string_write(name, EnglishNames[ NAMES_SURNAMES + first ], &position);
        }
        else
        {
            io_string_write(name, EnglishNames[ NAMES_SURNAMES + NAMES_FIRST + first ], &position);
        }
        io_string_write(name, " ", &position);
        io_string_write(name, EnglishNames[ family0 ], &position);
        io_string_write(name, "-", &position);
        io_string_write(name, EnglishNames[ family1 ], &position);
    }
    else
    {
        io_string_write(name, "Unknown", &position);
    }
}

void being_name_simple(noble_being * value, n_string str)
{
    being_name((FIND_SEX(GET_I(value)) == SEX_FEMALE), being_first_name(value), being_family_first_name(value), being_family_second_name(value), str);
}

void being_name_byte2(n_byte2 first, n_byte2 family, n_string name)
{
    being_name((n_byte)((first>>8)==SEX_FEMALE),
               (n_int)(first&255),
               (n_byte)UNPACK_FAMILY_FIRST_NAME(family),
               (n_byte)UNPACK_FAMILY_SECOND_NAME(family),
               name);
}

void being_state_description(n_byte2 state, n_string result)
{
    /* if you change this you need to change the corresponding definitions in entity.h */
    const n_string state_description[] =
    {
        "Sleeping", "Awake", "Hungry", "Swimming", "Eating", "Moving",
        "Speaking", "Shouting", "Grooming", "Suckling",
        "Showing Force", "Attacking"
    };
    n_int string_length=0;
    n_int n=2;

    if (state == BEING_STATE_ASLEEP)
    {
        io_string_write(result, state_description[0], &string_length);
        return;
    }

    while (n < BEING_STATES)
    {
        if (state & (1<<(n-1)))
        {
            if (string_length > 0)
            {
                io_string_write(result, ", ", &string_length);
            }
            io_string_write(result, state_description[n], &string_length);
        }
        n++;
    }
}

/**
 This moves the specified Noble Ape outside the core simulation movement
 interface - think mouse movement or keyboard movement.
 @param local The pointer to the noble_being being moved.
 @param rel_vel The movement variable used - historically the velocity hence the
 variable name.
 @param kind The kind of movement used - 0 = turning around pivoting on the
 z-axis (ie a standing turn), 1 = move forwards or backwards based on the
 direction facing, 2 = cursor like movements along the X and Y axis useful
 when operating the arrow keys (but not much else).
 */
void being_move(noble_being * local, n_int rel_vel, n_byte kind)
{
    if (kind > 0)
    {
        n_vect2 location_vector;
        n_byte2 loc[2];
        vect2_byte2(&location_vector, being_location(local));
        if (kind == 1)
        {
            n_vect2 facing_vector;
            being_facing_vector(local, &facing_vector, 1);
            vect2_d(&location_vector, &facing_vector, rel_vel, 512);
        }
        else
        {
            if (rel_vel < 2)
                location_vector.y -= (rel_vel * 200)-100;
            else
                location_vector.x += 500-(rel_vel * 200);
        }
        loc[0] = APESPACE_WRAP(location_vector.x);
        loc[1] = APESPACE_WRAP(location_vector.y);
        being_set_location(local, loc);
    }
    else
    {
        being_wander(local, -rel_vel);
    }
}

void being_change_selected(noble_simulation * sim, n_byte forwards)
{
    noble_being * local_select = sim->select;
    noble_being * first = sim->beings;
    noble_being * last = &(sim->beings[sim->num - 1]);
    if (forwards)
    {
        if (sim->select != last)
        {
            local_select++;
        }
        else
        {
            local_select = first;
        }
    }
    else
    {
        if (sim->select != first)
        {
            local_select--;
        }
        else
        {
            local_select = last;
        }
    }
    sim_set_select(local_select);
}

/**
 This checks to see if the Noble Ape is awake
 @param sim The simulation pointer
 @param reference The specific Noble Ape checked for being awake
 @return 2 is fully awake, 1 is slightly awake to eat, 0 is asleep
 */
n_byte being_awake(noble_simulation * sim, noble_being * local)
{
    n_land  * land  =   sim->land;
    
    if (being_energy_less_than(local, BEING_DEAD + 1))
    {
        return FULLY_ASLEEP;
    }

    /** if it is not night, the being is fully awake */
    if(IS_NIGHT(land->time) == 0)
    {
        return FULLY_AWAKE;
    }

    /** if it  night the being is... */

    /** ... fully awake to swim */

    if(WATER_TEST(QUICK_LAND(land, APESPACE_TO_MAPSPACE(being_location_x(local)), APESPACE_TO_MAPSPACE(being_location_y(local))),land->tide_level))
    {
        return FULLY_AWAKE;
    }

    /** ... slightly awake to eat */

    if (being_energy_less_than(local, BEING_HUNGRY + 1))
    {
        return SLIGHTLY_AWAKE;
    }

    /** ... slightly awake to slow down */

    if(being_speed(local) > 0)
    {
        return SLIGHTLY_AWAKE;
    }

    /** ... asleep */

    return FULLY_ASLEEP;
}

/*
 
 Rough sketch on brain compression method
 
 Hypthesis:
 
     From a zeroed brain,
     
     After n OUTPUT_ACTUATOR firings into the brain, and,
     
     After m brain_cycle occurrences with the states recorded,
     
     There should be a near identical brain to a standard run brain.
     
     n and m are independent but an acceptable level needs to be recorded.
 
 Data set:
 
     Type (either output actuator or brain cycle)
     
     For output actuator, record position in brain (n_byte2) and set value (n_byte)
     
     For brain cycle, record awake or asleep.
     
     Position in brain is on the 32k boundry, the delimiting could be:
     
     Position in brain - if 32k-1 then actuator set. If 64k-1, then brain cycle
     
     Set value is set value if actuator set, 0 is asleep, 1 is awake if brain cycle
 
 */


#ifdef BRAINCODE_ON
#ifdef BRAIN_ON

static void being_brain_probe(noble_being * local)
{
    n_byte * local_brain = being_brain(local);
    n_int    i = 0;
    n_int    count[NUMBER_BRAINPROBE_TYPES] = {0};
    
    if (local_brain == 0L) return;
    
    while (i < BRAINCODE_PROBES)
    {
        count[local->brainprobe[i++].type]++;
    }
    
    /** check to ensure that there are a minimum number of sensors and actuators */
    if (count[INPUT_SENSOR] < (BRAINCODE_PROBES>>2))
    {
        local->brainprobe[0].type = INPUT_SENSOR;
    }
    else if (count[OUTPUT_ACTUATOR] < (BRAINCODE_PROBES>>2))
    {
        local->brainprobe[0].type = OUTPUT_ACTUATOR;
    }
    
    /** update each probe */
    i = 0;
    
    while (i < BRAINCODE_PROBES)
    {
        local->brainprobe[i].state++;
        if (local->brainprobe[i].state >= local->brainprobe[i].frequency)
        {
            n_byte * local_braincode = being_braincode_internal(local);
            /** position within the brain */
            n_int position_in_brain = ((local->brainprobe[i].position * (SINGLE_BRAIN>>8))) & (SINGLE_BRAIN-1);
            n_int position_in_braincode = local->brainprobe[i].address % BRAINCODE_SIZE;

            local->brainprobe[i].state = 0;

            if (local->brainprobe[i].type == INPUT_SENSOR)
            {
                /** address within braincode */
                n_int set_value = (local_brain[position_in_brain] + local->brainprobe[i].offset)&255;
                /** read from brain */
                local_braincode[position_in_braincode] = (n_byte)set_value;
            }
            else
            {
                /** address within braincode */
                n_int set_value = (local_braincode[position_in_braincode] + local->brainprobe[i].offset)&255;
                /** write to brain */
                local_brain[position_in_brain] = (n_byte)set_value;
            }
        }
        i++;
    }
}
#endif
#endif

/** stuff still goes on during sleep */
void being_cycle_universal(noble_simulation * sim, noble_being * local, n_byte awake)
{
    being_immune_response(local);
    
#ifdef BRAINCODE_ON
#ifdef BRAIN_ON

    /** may need to add external probe linking too */
    being_brain_probe(local);
#endif
#endif

    if ((awake == 0) && local)
    {
        being_set_state(local, BEING_STATE_ASLEEP);

        being_reset_drive(local, DRIVE_FATIGUE);
    }
}

/* For a new child this populates the social graph with family relationships */
static void being_create_family_links(noble_being * mother,
                                      noble_being * child,
                                      noble_simulation * sim)
{
    n_int i,j,index;
    noble_being * parent[6]= {0L};
    noble_being * sibling;
    n_byte parent_relation[6];
    n_byte child_relation[6];
    n_byte sibling_relation;
    noble_social * parent_social_graph;

    if (mother==0L) return;

    /** First tow entries in the array are parents.
       Subsequent entries are grandparents */
    parent[0] = mother;
    parent[1] = being_find_name(sim, mother->father_name[0], mother->father_name[1]);

    parent_relation[0] = RELATIONSHIP_DAUGHTER;
    parent_relation[1] = RELATIONSHIP_DAUGHTER;
    parent_relation[2] = RELATIONSHIP_GRANDDAUGHTER;
    parent_relation[3] = RELATIONSHIP_GRANDDAUGHTER;
    parent_relation[4] = RELATIONSHIP_GRANDDAUGHTER;
    parent_relation[5] = RELATIONSHIP_GRANDDAUGHTER;

    child_relation[0] = RELATIONSHIP_MOTHER;
    child_relation[1] = RELATIONSHIP_MOTHER;
    child_relation[2] = RELATIONSHIP_MATERNAL_GRANDMOTHER;
    child_relation[3] = RELATIONSHIP_MATERNAL_GRANDMOTHER;
    child_relation[4] = RELATIONSHIP_PATERNAL_GRANDMOTHER;
    child_relation[5] = RELATIONSHIP_PATERNAL_GRANDMOTHER;

    /** grandparents */
    for (j = 0; j < 2; j++) /** maternal or paternal */
    {
        if (parent[j])
        {
            /** social graph for mother or father */
            parent_social_graph = being_social(parent[j]);
            if (parent_social_graph)
            {
                for (i = 0; i < 2; i++) /** grandmother or grandfather */
                {
                    parent[2+(j*2)+i] = 0L;
                    /** graph index for parent's mother or father */
                    index = social_get_relationship(parent[j], (n_byte)(RELATIONSHIP_MOTHER+i),sim);
                    if ((index > -1) && (parent_social_graph != 0L))
                    {
                        /** store the grandparent reference if still living */
                        parent[2+(j*2)+i] =
                            being_find_name(sim,
                                            parent_social_graph[index].first_name[BEING_MET],
                                            parent_social_graph[index].family_name[BEING_MET]);
                    }
                }
            }
        }
    }

    /** brothers and sisters */
    sibling_relation = RELATIONSHIP_BROTHER;
    if (FIND_SEX(GET_I(child)) == SEX_FEMALE)
    {
        sibling_relation = RELATIONSHIP_SISTER;
    }

    for (j = 0; j < 2; j++)
    {
        /** social graph for mother or father */
        if (parent[j])
        {
            parent_social_graph = being_social(parent[j]);
            if (parent_social_graph)
            {
                for (i=1; i<SOCIAL_SIZE_BEINGS; i++)
                {
                    if ((parent_social_graph[i].relationship==RELATIONSHIP_SON) ||
                            (parent_social_graph[i].relationship==RELATIONSHIP_DAUGHTER))
                    {
                        sibling = being_find_name(sim, parent_social_graph[i].first_name[BEING_MET], parent_social_graph[i].family_name[BEING_MET]);
                        if (sibling!=0L)
                        {
                            if (parent_social_graph[i].relationship==RELATIONSHIP_SON)
                            {
                                social_set_relationship(child, RELATIONSHIP_BROTHER, sibling, sim);
                            }
                            else
                            {
                                social_set_relationship(child, RELATIONSHIP_SISTER, sibling, sim);
                            }
                            social_set_relationship(sibling, sibling_relation, child, sim);
                        }
                    }
                }
            }
        }
    }

    /** set relationships */
    for (i = 0; i < 6; i++)
    {
        if (parent[i]==0L) continue;

        /** create the parent/child social graph relation */
        if (FIND_SEX(GET_I(child)) == SEX_FEMALE)
        {
            social_set_relationship(parent[i], parent_relation[i], child, sim);
        }
        else
        {
            social_set_relationship(parent[i], parent_relation[i]+1, child, sim);
        }

        if (i%2==0)
        {
            social_set_relationship(child, child_relation[i], parent[i], sim);
        }
        else
        {
            social_set_relationship(child, child_relation[i]+1, parent[i], sim);
        }
    }

}

static void being_follow_loop1(noble_simulation * sim, noble_being * other, void * data)
{
    being_nearest * nearest = (being_nearest *)data;
    n_vect2        difference_vector;

    /** is this the same as the name of the being to which we are paying attention? */
    if ((FIND_SEX(GET_I(other))!=FIND_SEX(GET_I(nearest->local))) &&
        being_name_comparison(other, nearest->local->goal[1], nearest->local->goal[2]))
    {
        being_delta(nearest->local, other, &difference_vector);
        if (being_los(sim->land, nearest->local, (n_byte2)difference_vector.x, (n_byte2)difference_vector.y))
        {
            n_uint compare_distance = vect2_dot(&difference_vector, &difference_vector, 1, 1);
            if (compare_distance < nearest->opposite_sex_distance)
            {
                nearest->opposite_sex = other;
                nearest->opposite_sex_distance = compare_distance;
            }
        }
    }
}

static void being_follow_loop2(noble_simulation * sim, noble_being * other, void * data)
{
    being_nearest * nearest = (being_nearest *)data;
    n_vect2        difference_vector;

    /** is this the same as the name of the being to which we are paying attention? */
    if (being_name_comparison(other,
            nearest->local_social->first_name[BEING_MET],
            nearest->local_social->family_name[BEING_MET]))
    {
        /** Is this being within sight? */
        being_delta(nearest->local, other, &difference_vector);
        if (being_los(sim->land, nearest->local, (n_byte2)difference_vector.x, (n_byte2)difference_vector.y))
        {
            n_uint compare_distance = vect2_dot(&difference_vector, &difference_vector, 1, 1);
            if (FIND_SEX(GET_I(other))!=FIND_SEX(GET_I(nearest->local)))
            {
                if (compare_distance < nearest->opposite_sex_distance)
                {
                    nearest->opposite_sex = other;
                    nearest->opposite_sex_distance = compare_distance;
                }
            }
            else
            {
                if (compare_distance < nearest->same_sex_distance)
                {
                    nearest->same_sex = other;
                    nearest->same_sex_distance = compare_distance;
                }
            }
        }
    }

}

/**
 * Follow a being to which we are paying attention
 * @param sim Pointer to the simulation
 * @param current_being_index Array index of the current being
 * @param opposite_sex Array index of the closest being of the opposite sex
 * @param same_sex Array index of the closest being of the same sex
 * @param opposite_sex_distance Returned distance to the closest being of the opposite sex
 * @param same_sex_distance Returned distance to the closest being of the same sex
 */
static void being_follow(noble_simulation * sim,
                           noble_being * local,
                           being_nearest * nearest)
{
    noble_social * local_social_graph;
    n_int          social_graph_index;

    nearest->local = local;
    nearest->opposite_sex_distance = 0xffffffff;
    nearest->same_sex_distance = 0xffffffff;
    nearest->opposite_sex = 0L;
    nearest->same_sex = 0L;

    /** is a mate in view? */
    if (local->goal[0]==GOAL_MATE)
    {
        being_loop_no_thread(sim, local, being_follow_loop1, nearest);
        if (nearest->opposite_sex != 0L)
        {
            return;
        }
    }
    
    local_social_graph = being_social(local);
    if (local_social_graph == 0L) return;
    
    /** which entry in the social graph are we paying attention to? */
    social_graph_index = GET_A(local,ATTENTION_ACTOR);
    
    nearest->local_social = &local_social_graph[social_graph_index];
    
    /** Does this entry correspond to another being? */
    if ((social_graph_index>0) &&
        (local_social_graph[social_graph_index].entity_type == ENTITY_BEING) &&
        (!SOCIAL_GRAPH_ENTRY_EMPTY(local_social_graph, social_graph_index)))
    {
        being_loop_no_thread(sim, local, being_follow_loop2, nearest);
    }
}

typedef struct
{
    n_int         max_shout_volume;
    noble_being * local;
} being_listen_struct;

static void being_listen_loop(noble_simulation * sim, noble_being * other, void * data)
{
    being_listen_struct * bls = (being_listen_struct *)data;
    n_vect2       difference_vector;
    n_uint        compare_distance;
    
    being_delta(bls->local, other, &difference_vector);
    compare_distance = vect2_dot(&difference_vector, &difference_vector, 1, 1);
    /** listen for the nearest shout out */
    if ((being_state(other)&BEING_STATE_SHOUTING) &&
        (compare_distance < SHOUT_RANGE) &&
        (other->shout[SHOUT_VOLUME] > bls->max_shout_volume))
    {
        bls->max_shout_volume = other->shout[SHOUT_VOLUME];
        bls->local->shout[SHOUT_HEARD] = other->shout[SHOUT_CONTENT];
        bls->local->shout[SHOUT_FAMILY0] = being_family_first_name(other);
        bls->local->shout[SHOUT_FAMILY1] = being_family_second_name(other);
    }
}

/**
 * Listen for shouts
 * @param sim Pointer to the simulation
 * @param current_being_index Array index of the current being
 */
void being_listen(noble_simulation * local_sim, noble_being * local_being, void * data)
{
    being_listen_struct bls;
    
    if (being_awake(local_sim, local_being) == 0) return;
    
    bls.max_shout_volume = 127;
    bls.local = local_being;
    /** clear shout values */
    if (local_being->shout[SHOUT_CTR] > 0)
    {
        local_being->shout[SHOUT_CTR]--;
    }
    being_loop_no_thread(local_sim, local_being, being_listen_loop, &bls);
}

static void being_closest_loop(noble_simulation * sim, noble_being * test_being, void * data)
{
    being_nearest * nearest = (being_nearest *)data;
    n_vect2       difference_vector;
    n_uint        compare_distance;
    being_delta(nearest->local, test_being, &difference_vector);
    compare_distance = vect2_dot(&difference_vector, &difference_vector, 1, 1);
    if (FIND_SEX(GET_I(test_being)) != FIND_SEX(GET_I(nearest->local)))
    {
        if (compare_distance < nearest->opposite_sex_distance)
        {
            /* 'function' : conversion from 'n_int' to 'n_byte2', possible loss of data x 2 */
            if (being_los(sim->land, nearest->local, (n_byte2)difference_vector.x, (n_byte2)difference_vector.y))
            {
                nearest->opposite_sex_distance = compare_distance;
                nearest->opposite_sex = test_being;
            }
        }
    }
    else
    {
        if ( compare_distance < nearest->same_sex_distance )
        {
            if (FIND_SEX(GET_I(test_being)) == FIND_SEX(GET_I(nearest->local)))
            {
                if (being_los(sim->land, nearest->local, (n_byte2)difference_vector.x, (n_byte2)difference_vector.y))
                {
                    nearest->same_sex_distance = compare_distance;
                    nearest->same_sex = test_being;
                }
            }
        }
    }
}

/**
 * Returns the closest beings
 * @param sim Pointer to the simulation
 * @param current_being_index Array index of the current being
 * @param opposite_sex Array index of the closest being of the opposite sex
 * @param same_sex Array index of the closest being of the same sex
 * @param opposite_sex_distance Returned distance to the closest being of the opposite sex
 * @param same_sex_distance Returned distance to the closest being of the same sex
 * @return The number of beings in the vicinity
 */
static void being_closest(noble_simulation * sim,
                            noble_being * local,
                            being_nearest * nearest)
{
    nearest->local = local;
    nearest->opposite_sex_distance = 0xffffffff;
    nearest->same_sex_distance = 0xffffffff;
    nearest->opposite_sex = 0L;
    nearest->same_sex = 0L;
    being_loop_no_thread(sim, local, being_closest_loop, nearest);
}

/**
 * One being interacts with another
 * @param sim Pointer to the simulation
 * @param being_index Array index of the being
 * @param other_being_index Array index of the other being
 * @param other_being_distance Distance to the other being
 * @param awake Whether the being is awake
 * @param state The state of the being
 * @param speed The speed of the being
 * @param energy The energy of the being
 * @param opposite_sex Non zero if the other being is the opposite sex
 */
static void being_interact(noble_simulation * sim,
                           noble_being * local,
                           noble_being	* other_being,
                           n_uint   other_being_distance,
                           n_int  * awake,
                           n_byte * state,
                           n_int  * speed,
                           n_byte   opposite_sex)
{
    if (other_being != 0L)
    {
        n_land      * land         = sim->land;
        n_int         today_days   = TIME_IN_DAYS(land->date);
        n_int         birth_days   = being_dob(local);
        n_uint        local_is_female = FIND_SEX(GET_I(local));

        n_vect2 delta_vector;

        /** social networking */
        n_byte2 familiarity=0;
        n_int   being_index = social_network(local, other_being, other_being_distance, sim);

        being_delta(local, other_being, &delta_vector);

        if (being_index > -1)
        {
            noble_social * local_social_graph = being_social(local);
            if (local_social_graph)
            {
                familiarity = local_social_graph[being_index].familiarity;
            }
        }

        being_facing_towards(local, &delta_vector);

        if ((birth_days+AGE_OF_MATURITY)<today_days)
        {
            if (social_groom(local, other_being, other_being_distance, *awake, familiarity, sim))
            {
                *state |= BEING_STATE_GROOMING;

                /* both beings stop */
                *speed = 0;
                being_set_speed(other_being, 0);
            }
            else
            {
                /* squabbling between adults */
                if ((other_being_distance < SQUABBLE_RANGE) && ((being_dob(other_being)+AGE_OF_MATURITY) < today_days))
                {
                    n_byte2 squabble_val;
                    being_set_speed(local, (n_byte)*speed);
                    squabble_val = social_squabble(local, other_being, other_being_distance, local_is_female, sim);
                    if (squabble_val != 0)
                    {
                        *state |= squabble_val;
                        *speed = being_speed(local);
                    }
                }
            }

        }
        if ((other_being_distance < SOCIAL_RANGE) && (being_index>-1))
        {
            /* attraction and mating */
            if (opposite_sex != 0)
            {
                *state |= social_mate(local, other_being, being_index, other_being_distance, sim);
            }

            /* chat */
            *state |= social_chat(local, other_being, being_index, sim);
        }
    }
}

typedef struct{
    n_int counter;
    n_int return_value;
    noble_being * being;
}being_index_loop_struct;

void being_index_loop(noble_simulation * local_sim, noble_being * local_being, void * data)
{
    being_index_loop_struct * bils = (being_index_loop_struct *) data;
    
    if (bils->return_value != -1)
    {
        return;
    }
    
    if (local_being == bils->being)
    {
        bils->return_value = bils->counter;
    }
    else
    {
        bils->counter++;
    }
}

n_int being_index(noble_simulation * sim, noble_being * local)
{
    being_index_loop_struct value;
    
    value.return_value = -1;
    value.being = local;
    value.counter = 0;
    
    being_loop_no_thread(sim, 0L, being_index_loop, &value);
    return value.return_value;
}

void being_cycle_awake(noble_simulation * sim, noble_being * local)
{
    n_land      * land               = sim->land;
    n_uint        local_is_female    = FIND_SEX(GET_I(local));
    n_int         today_days         = TIME_IN_DAYS(land->date);
    n_int         birth_days         = being_dob(local);

    n_int	      loc_s              = being_speed(local);
    n_int	      loc_h              = GET_H(local);

    n_byte        loc_state          = BEING_STATE_ASLEEP;
    n_int         fat_mass, child_mass = 0;
    n_int         awake = being_awake(sim, local);

    n_int         carrying_child = 0;

    n_genetics *  genetics = being_genetics(local);

    /** tmp_speed is the optimum speed based on the gradient */
    n_int	tmp_speed;
    /** delta_energy is the energy required for movement */
    n_int	az;

#ifdef TERRITORY_ON
    n_uint territory_index;
#endif
    being_nearest nearest;
    n_int         test_land = 1;

    nearest.opposite_sex = 0L;
    nearest.same_sex = 0L;

    {
        n_vect2 location_vector;
        n_vect2 facing_vector;
        n_vect2 slope_vector;
        n_vect2 looking_vector;

        vect2_byte2(&location_vector, being_location(local));

        being_facing_vector(local, &facing_vector, 4);

        land_vect2(&slope_vector, &az, land, &location_vector);

        vect2_add(&looking_vector, &location_vector, &facing_vector);
        
        test_land = (MAP_WATERTEST(land, POSITIVE_LAND_COORD(APESPACE_TO_MAPSPACE(looking_vector.x)),
                                   POSITIVE_LAND_COORD(APESPACE_TO_MAPSPACE(looking_vector.y))) != 0);
        
        {
            n_int delta_z = vect2_dot(&slope_vector,&facing_vector,1,24);

            tmp_speed = ((delta_z + 280) >> 4);
        }
    }
#ifdef LAND_ON
    /* TODO why not test_land here? */
    
    if (WATER_TEST(az,land->tide_level) != 0)
    {
        loc_state |= BEING_STATE_SWIMMING;
    }
#endif

    if (awake != FULLY_ASLEEP)
    {
        loc_state |= BEING_STATE_AWAKE;
    }

    if (loc_s != 0)
    {
        loc_state |= BEING_STATE_MOVING;
    }

    {
        n_int   hungry = being_energy_less_than(local, BEING_HUNGRY);

        if ((loc_state & (BEING_STATE_AWAKE | BEING_STATE_SWIMMING | BEING_STATE_MOVING)) == BEING_STATE_AWAKE)
        {
            hungry = being_energy_less_than(local, BEING_FULL);
        }

        if (hungry != 0)
        {
            loc_state |= BEING_STATE_HUNGRY;
        }
    }

    /** amount of body fat in kg */
    fat_mass = GET_BODY_FAT(local);
    if (fat_mass > BEING_MAX_MASS_FAT_G)
    {
        fat_mass = BEING_MAX_MASS_FAT_G;
    }

    /** If it sees water in the distance then turn */
    if (((loc_state & BEING_STATE_SWIMMING) != 0) || test_land)
    {
        n_uint	      loop;

        being_turn_away_from_water(local, land);

        /** horizontally oriented posture */
        being_set_posture(local, 0);
        
        /** When swimming drop everything except what's on your head or back.
           Note that the groomed flag is also cleared */

        for (loop=0; loop<INVENTORY_SIZE; loop++)
        {
            if (!((loop==BODY_HEAD) || (loop==BODY_BACK)))
            {
                local->inventory[loop] = 0;
            }
        }
        /** swimming proficiency */
        tmp_speed = (tmp_speed * (GENE_SWIM(genetics)+8)) >> 4;

        episodic_self(sim, local, EVENT_SWIM, being_energy(local), 0);

        /** bathing removes parasites */
        if (local->parasites > 0) local->parasites--;
    }
    else
    {
        /** adjust speed using genetics */
        tmp_speed = (tmp_speed * (GENE_SPEED(genetics)+8)) >> 3;

        /** is the being to which we are paying attention within view? */
        being_follow(sim, local, &nearest);
        if (nearest.opposite_sex == 0L)
        {
            /** Find the closest beings */
            being_closest(sim, local, &nearest);
        }

		/* TODO: SOCIAL_THRESHOLD should not be a macro, it should be a function returning n_int */

        if (being_drive(local, DRIVE_SOCIAL) > SOCIAL_THRESHOLD(local))
        {
            being_interact(sim,
                           local,
                           nearest.same_sex, nearest.same_sex_distance,
                           &awake, &loc_state,
                           &loc_s, 0);

            being_interact(sim,
                           local,
                           nearest.opposite_sex, nearest.opposite_sex_distance,
                           &awake, &loc_state,
                           &loc_s, 1);
        }
    }

    if ((loc_state & (BEING_STATE_SWIMMING | BEING_STATE_GROOMING | BEING_STATE_ATTACK | BEING_STATE_SHOWFORCE)) == 0)
    {
        if ((loc_state & BEING_STATE_HUNGRY) != 0)
        {
            if (loc_s == 0)
            {
                /** eating when stopped */
                n_byte  food_type;
                n_int energy = food_eat(sim->land, being_location_x(local), being_location_y(local), az, &food_type, local);
                
                /** remember eating */
                episodic_food(sim, local, energy, food_type);

                being_energy_delta(local, energy);

                being_reset_drive(local, DRIVE_HUNGER);

                loc_state |= BEING_STATE_EATING;
                /** grow */
                if (loc_h < BEING_MAX_HEIGHT)
                {
                    if ((birth_days+AGE_OF_MATURITY) > today_days)
                    {
                        loc_h += ENERGY_TO_GROWTH(local,energy);
                    }
                }
            }
        }
        else
        {
            /** orient towards a goal */
            social_goals(local);
            if (loc_s==0)
            {
                loc_s = 10;
            }
        }
    }

    if (tmp_speed > 39) tmp_speed = 39;
    if (tmp_speed < 0) tmp_speed = 0;

    if ((awake != FULLY_AWAKE) || (loc_state & BEING_STATE_HUNGRY))
    {
        if ((loc_state & BEING_STATE_SWIMMING) != 0)
        {
            tmp_speed = (being_energy(local) >> 7);
        }
        else
        {
            tmp_speed = 0;
        }
    }

    if (tmp_speed > loc_s) loc_s++;
    if (tmp_speed < loc_s) loc_s--;
    if (tmp_speed < loc_s) loc_s--;
    if (tmp_speed < loc_s) loc_s--;

    if ((local->goal[0]==GOAL_NONE) &&
        (nearest.opposite_sex == 0L) &&
        (nearest.same_sex == 0L) &&
            (being_random(local) < 1000 + 3600*GENE_STAGGER(genetics)))
    {
        n_int	 wander = math_spread_byte(being_random(local) & 7);
        being_wander(local, wander);
    }

    /** a certain time after giving birth females become receptive again */
    if ((being_pregnant(local) != 0) &&
            ((being_pregnant(local) + GESTATION_DAYS + CONCEPTION_INHIBITION_DAYS) < today_days))
    {
        /** zero value indicates ready to conceive */
        local->date_of_conception[0] = 0;
        local->date_of_conception[1] = 0;
    }

    if ((loc_state & (BEING_STATE_AWAKE | BEING_STATE_SWIMMING)) == BEING_STATE_AWAKE)
    {
        n_uint conception_days = being_pregnant(local) ;
        if (conception_days > 0)
        {
            n_int gestation_days = conception_days + GESTATION_DAYS;
            if (today_days > gestation_days)
            {
                /** A mother could have multiple children, so only find the youngest */
                noble_being * being_child = being_find_child(sim, genetics, CARRYING_DAYS);

                /** Birth */
                if (being_child == 0L)
                {
                    if((sim->num + 1) < sim->max)
                    {
                        being_child = &(sim->beings[sim->num]);

                        if (being_init(sim->land, sim->beings, sim->num, being_child, local, 0L) == 0)
                        {
                            episodic_close(sim, local, being_child, EVENT_BIRTH, AFFECT_BIRTH, 0);
                            being_create_family_links(local,being_child,sim);
                            if (sim->ext_birth != 0)
                            {
                                sim->ext_birth(being_child,local,sim);
                            }
                            sim->num++;
                        }
                    }
                }
                else
                {
                    /** mother carries the child */
                    n_int carrying_days = conception_days + GESTATION_DAYS + CARRYING_DAYS;
                    if (today_days < carrying_days)
                    {
                        if (!((local->inventory[BODY_FRONT] & INVENTORY_CHILD) ||
                                (local->inventory[BODY_BACK] & INVENTORY_CHILD)))
                        {
                            local->inventory[BODY_BACK] |= INVENTORY_CHILD;
                            GET_A(local,ATTENTION_BODY) = BODY_BACK;
                        }
                        carrying_child = 1;

                        being_set_location(being_child, being_location(local));

                        child_mass = GET_M(being_child);
                        episodic_close(sim, local, being_child, EVENT_CARRIED, AFFECT_CARRYING, 0);
                        episodic_close(sim, being_child, local, EVENT_CARRIED_BY, AFFECT_CARRIED, 0);
                    }
                }
            }
            else
            {
                /** Compute the mass of the unborn child.
                   This will be added to the mass of the mother */
                child_mass = (today_days - conception_days) * BIRTH_MASS / GESTATION_DAYS;
            }
        }

        /** child follows the mother */
        if ((birth_days + WEANING_DAYS) > today_days)
        {
            noble_being * mother = being_find_closest(sim, local, 1, 1);
            if (mother != 0L)
            {
                /** orient towards the mother */
                n_vect2    mother_vector;

                being_delta(mother, local, &mother_vector);

                being_facing_towards(local, &mother_vector);

                /** suckling */
                if ((loc_state & BEING_STATE_HUNGRY) != 0)
                {
                    n_int distance = vect2_dot(&mother_vector, &mother_vector, 1, 1);
                    if (distance < SUCKLING_MAX_SEPARATION)
                    {
                        /** child moves from back to front */
                        if (mother->inventory[BODY_BACK] & INVENTORY_CHILD)
                        {
                            mother->inventory[BODY_BACK] -= INVENTORY_CHILD;
                        }
                        mother->inventory[BODY_FRONT] |= INVENTORY_CHILD;
                        GET_A(mother,ATTENTION_BODY) = BODY_FRONT;
                        /** sucking causes loss of grooming */
                        if (mother->inventory[BODY_FRONT] & INVENTORY_GROOMED)
                        {
                            mother->inventory[BODY_FRONT] -= INVENTORY_GROOMED;
                        }
                        /** hungry mothers stop producing milk */
                        if (being_energy_less_than(mother, BEING_HUNGRY) == 0)
                        {
                            /** mother loses energy */
                            being_energy_delta(mother, 0 - SUCKLING_ENERGY);
                            /** child gains energy */
                            
                            being_energy_delta(local, SUCKLING_ENERGY);
                            
                            /** set child state to suckling */
                            loc_state |= BEING_STATE_SUCKLING;
                            /** child acquires immunity from mother */
                            being_immune_seed(mother, local);

                            episodic_close(sim, mother, local, EVENT_SUCKLED, AFFECT_SUCKLING, 0);
                            episodic_close(sim, local, mother, EVENT_SUCKLED_BY, AFFECT_SUCKLING, 0);
                        }
                    }
                }
            }
        }
    }

    /** no longer carrying the child */
    if ((carrying_child==0) && (local_is_female == SEX_FEMALE))
    {
        if (local->inventory[BODY_FRONT] & INVENTORY_CHILD)
        {
            local->inventory[BODY_FRONT] -= INVENTORY_CHILD;
        }
        if (local->inventory[BODY_BACK] & INVENTORY_CHILD)
        {
            local->inventory[BODY_BACK] -= INVENTORY_CHILD;
        }
    }

#ifdef TERRITORY_ON
    territory_index =
        APESPACE_TO_TERRITORY(being_location_y(local))*TERRITORY_DIMENSION +
        APESPACE_TO_TERRITORY(being_location_x(local));

    if (local->territory[territory_index].familiarity<65534)
    {
        local->territory[territory_index].familiarity++;
    }
    else
    {
        /** rescale familiarity values */
        for (territory_index=0; territory_index<TERRITORY_AREA; territory_index++)
        {
            local->territory[territory_index].familiarity>>=2;
        }
    }
#endif

    being_set_speed(local, (n_byte)loc_s);
    GET_H(local) = (n_byte2) loc_h;
    GET_M(local) = (n_byte2)((BEING_MAX_MASS_G*loc_h/BEING_MAX_HEIGHT)+fat_mass+child_mass);
    being_set_state(local, loc_state);
}

#ifdef BRAINCODE_ON
/** initialise inner or outer braincode */
void being_init_braincode(noble_being * local,
                          noble_being * other,
                          n_byte friend_foe,
                          n_byte internal)
{
    n_byte2 * local_random = being_get_random(local);
    n_uint ch,i,most_similar_index,diff,min,actor_index;
    noble_social * graph;    
    if (other==0L)
    {
        /** initially seed the brain with instructions which are random but genetically biased */
        for (ch = 0; ch < BRAINCODE_SIZE; ch+=3)
        {
            math_random3(local_random);
            if (internal != 0)
            {
#ifdef RANDOM_INITIAL_BRAINCODE
                being_braincode_internal(local)[ch] = math_random(local_random) & 255;
#else
                being_random3(local);
                being_braincode_internal(local)[ch] = (math_random(local_random) & 192) | get_braincode_instruction(local);
#endif
                being_braincode_internal(local)[ch+1] = math_random(local_random) & 255;
                being_braincode_internal(local)[ch+2] = math_random(local_random) & 255;
            }
            else
            {
#ifdef RANDOM_INITIAL_BRAINCODE
                being_braincode_external(local)[ch] = math_random(local_random) & 255;
#else
                being_random3(local);
                being_braincode_external(local)[ch] = (math_random(local_random) & 192) | get_braincode_instruction(local);
#endif
                being_braincode_external(local)[ch+1] = math_random(local_random) & 255;
                being_braincode_external(local)[ch+2] = math_random(local_random) & 255;
            }
        }
    }
    else
    {
        /** initialise based upon a similar being */
        graph = being_social(local);

        if (graph == 0L)
        {
            return;
        }

        most_similar_index=0;
        min=99999;
        actor_index = GET_A(local,ATTENTION_ACTOR);

        /** Find the entry in the social graph with the most similar friend or foe value.
           The FOF value is used because when two beings meet for the first time this
           value is calculated based upon a variety of genetic and learned dispositions.
           Notice also that the search includes index zero, which is the self. */
        for (i=0; i<SOCIAL_SIZE; i++)
        {
            if ((i!=actor_index) && (!SOCIAL_GRAPH_ENTRY_EMPTY(graph,i)))
            {
                n_int signed_diff = (n_int)graph[i].friend_foe - (n_int)friend_foe;
                if (signed_diff < 0)
                {
                    diff = -signed_diff;
                }
                else
                {
                    diff = signed_diff;
                }
                if (diff < min)
                {
                    min = diff;
                    most_similar_index = i;
                }
            }
        }
        /** Copy braincode for the most similar individual */
        io_copy(graph[most_similar_index].braincode, graph[actor_index].braincode, BRAINCODE_SIZE);
    }
}
#endif

/** Assign a unique name to the given being, based upon the given family names */
static n_int being_set_unique_name(noble_being * beings,
                                   n_int number,
                                   noble_being * local_being,
                                   n_byte2   mother_family_name,
                                   n_byte2   father_family_name)
{
    n_int     i;
    n_int     samples=0,found=0;
    n_byte2   possible_family_name;
    n_byte2   possible_first_name;
    n_byte2 * random_factor = being_get_random(local_being);
    n_byte2   local_random[2];

    local_random[0] = random_factor[0];
    local_random[1] = random_factor[1];
    
    /** random number initialization */
    math_random3(local_random);
    math_random3(local_random);

    /** if no mother and father are specified then randomly create names */
    if ((mother_family_name==0) && (father_family_name==0))
    {
        mother_family_name =
            GET_NAME_FAMILY((math_random(local_random) & FAMILY_NAME_AND_MOD),
                            (math_random(local_random) & FAMILY_NAME_AND_MOD));
        father_family_name =
            GET_NAME_FAMILY((math_random(local_random) & FAMILY_NAME_AND_MOD),
                            (math_random(local_random) & FAMILY_NAME_AND_MOD));
    }

    /** conventional family name */
    possible_family_name =
        GET_NAME_FAMILY(UNPACK_FAMILY_FIRST_NAME(mother_family_name),
                        UNPACK_FAMILY_SECOND_NAME(father_family_name));

    while ((found == 0) && (samples < 2048))
    {
        /** choose a first_name at random */
        possible_first_name = (n_byte2)((math_random(local_random) & 255) | (FIND_SEX(GET_I(local_being))<<8));

        /** avoid the same two family names */
        if (UNPACK_FAMILY_FIRST_NAME(mother_family_name) ==
            UNPACK_FAMILY_SECOND_NAME(father_family_name))
        {
            possible_family_name =
            GET_NAME_FAMILY(UNPACK_FAMILY_FIRST_NAME(mother_family_name),
                            (math_random(local_random) & FAMILY_NAME_AND_MOD));
        }
        
        if (samples == 1024)
        {
            /** switch naming order */
            possible_family_name =
                GET_NAME_FAMILY(UNPACK_FAMILY_SECOND_NAME(mother_family_name),
                                UNPACK_FAMILY_FIRST_NAME(father_family_name));
        }
        
        /** avoid the same two family names */
        if (UNPACK_FAMILY_SECOND_NAME(mother_family_name) ==
            UNPACK_FAMILY_FIRST_NAME(father_family_name))
        {
            possible_family_name =
            GET_NAME_FAMILY(UNPACK_FAMILY_SECOND_NAME(mother_family_name),
                            (math_random(local_random) & FAMILY_NAME_AND_MOD));
        }

        being_set_first_name(local_being,possible_first_name);
        being_set_family_name(local_being,
                              UNPACK_FAMILY_FIRST_NAME(possible_family_name),
                              UNPACK_FAMILY_SECOND_NAME(possible_family_name));
        
        /** does the name already exist in the population */
        found = 1;
        for (i = 0; i < number; i++)
        {
            noble_being * other_being = &beings[i];
            if (being_name_comparison(local_being, being_gender_name(other_being), being_family_name(other_being)))
            {
                found = 0;
                break;
            }
        }
        samples++;
    }

    return found;
}

static void being_random_genetics(n_genetics * value, n_byte2 * random, n_int male)
{
    n_int loop = 0;
    math_random3(random);
    while (loop < CHROMOSOMES)
    {
        n_uint loop2 = 0;
        
        value[loop] = 0;
        
        while (loop2 < (sizeof(n_genetics)*8))
        {
            if (math_random(random)&1)
            {
                value[loop] |= 1 << loop2;
            }
            loop2++;
        }
        loop++;
    }
    value[CHROMOSOME_Y] &= ~1;
    value[CHROMOSOME_Y] |= (male ? 2 : 3);
}

/**
 * Initialise the ape's variables and clear its brain
 * @param sim Pointer to the simulation object
 * @param mother Pointer to the mother
 * @param random_factor Random seed
 * @return 0
 */
n_int being_init(n_land * land, noble_being * beings, n_int number,
                 noble_being * local, noble_being * mother,
                 n_byte2* random_factor)
{
    /** this is the being to be born */
    n_byte        ch;
#ifdef EPISODIC_ON
    noble_social * local_social_graph = being_social(local);
    noble_episodic * local_episodic = being_episodic(local);
#endif
    if (local_social_graph == 0L)
    {
        return SHOW_ERROR("Social memory not available");
    }
    if (local_episodic == 0L)
    {
        return SHOW_ERROR("Episodic memory not available");
    }
    
    being_erase(local);

#ifdef BRAIN_ON
    {
        n_byte      * brain_memory = being_brain(local);
        if (brain_memory != 0L)
        {
            io_erase(brain_memory, DOUBLE_BRAIN);
        }
        else
        {
            return SHOW_ERROR("Brain memory not available");
        }
    }
#endif
    
    local->goal[0]=GOAL_NONE;

    /** Set learned preferences to 0.5 (no preference in either direction.
        This may seem like tabla rasa, but there are genetic biases */
    for (ch = 0; ch < PREFERENCES; ch++)
    {
        local->learned_preference[ch]=127;
    }

    being_immune_init(local);

    for (ch = 0; ch < ATTENTION_SIZE; ch++)
    {
        local->attention[ch]=0;
    }

    /** clear the generation numbers for mother and father */
    if (mother)
    {
        local->generation_max = mother->child_generation_max + 1;
        local->generation_min = mother->child_generation_min + 1;
    }
    else
    {
        local->generation_max = 0;
        local->generation_min = 0;
    }
    local->child_generation_max = 0;
    local->child_generation_min = 0;

    /** initially seed the brain with instructions which
        are genetically biased */


    if (random_factor)
    {
        being_set_random(local, random_factor);
        being_random3(local);
        being_random3(local);
        
    }
    else if (mother)
    {
        (void)being_random(mother);
        
        being_set_random(local, being_get_random(mother));

        being_random3(local);
        
        being_set_random1(local, being_get_random(mother)[0]);

        being_random3(local);

        being_set_random1(local, land->time);

        being_random3(local);
    }
    else
    {
        NA_ASSERT(random_factor, "Random factor not set");
        NA_ASSERT(mother, "Mother not set");
        return SHOW_ERROR("No correct being interface provided");
    }
#ifdef BRAINCODE_ON

    being_random3(local);
    
#ifdef EPISODIC_ON
    /** has no social connections initially */
    io_erase((n_byte*)local_social_graph,sizeof(noble_social)*SOCIAL_SIZE);
    
    for (ch=0; ch<EPISODIC_SIZE; ch++)
    {
        local_episodic[ch].affect=EPISODIC_AFFECT_ZERO;
    }
    
    local_social_graph[0].relationship=RELATIONSHIP_SELF;
    for (ch=0; ch<SOCIAL_SIZE; ch++)
    {
        /** default type of entity */
        local_social_graph[ch].entity_type = ENTITY_BEING;
        /** friend_or_foe can be positive or negative,
         with SOCIAL_RESPECT_NORMAL as the zero point */
        local_social_graph[ch].friend_foe = SOCIAL_RESPECT_NORMAL;
    }
#endif
    
    being_init_braincode(local,0L,0,
                         BRAINCODE_INTERNAL);
    being_init_braincode(local,0L,0,
                         BRAINCODE_EXTERNAL);

    /** randomly initialize registers */
    for (ch = 0; ch < BRAINCODE_PSPACE_REGISTERS; ch++)
    {
        being_random3(local);
        local->braincode_register[ch] = (n_byte)being_random(local)&255;
    }

    /** initialize brainprobes */
    for (ch = 0; ch < BRAINCODE_PROBES; ch++)
    {
        being_random3(local);
        if (being_random(local)&1)
        {
            local->brainprobe[ch].type = INPUT_SENSOR;
        }
        else
        {
            local->brainprobe[ch].type = OUTPUT_ACTUATOR;
        }
        local->brainprobe[ch].frequency = (n_byte)1 + (being_random(local)%BRAINCODE_MAX_FREQUENCY);
        being_random3(local);
        local->brainprobe[ch].address = (n_byte)being_random(local)&255;
        local->brainprobe[ch].position = (n_byte)being_random(local)&255;
        being_random3(local);
        local->brainprobe[ch].offset = (n_byte)being_random(local)&255;
    }
#endif
    
    being_facing_init(local);

    if (random_factor)
    {
        n_byte2  location[2];

        n_int loop = 0;

        being_random3(local);

        do
        {
            location[0] = (n_byte2)(being_random(local) & APESPACE_BOUNDS);
            location[1] = (n_byte2)(being_random(local) & APESPACE_BOUNDS);
            loop ++;
        }
        while ((loop < 20) && (MAP_WATERTEST(land, APESPACE_TO_MAPSPACE(location[0]), APESPACE_TO_MAPSPACE(location[1]))));

        being_set_location(local, location);

        {
            n_genetics mother_genetics[CHROMOSOMES];
            n_genetics father_genetics[CHROMOSOMES];
            n_byte2    gene_random[2];
            
            being_random3(local);
            
            gene_random[0] = being_random(local);
            being_random3(local);
            being_random3(local);

            gene_random[1] = being_random(local);
            
            being_random_genetics(mother_genetics, gene_random, 0);
            
            being_random3(local);
            
            gene_random[0] = being_random(local);
            being_random3(local);
            being_random3(local);
            being_random3(local);
            
            gene_random[1] = being_random(local);
            
            being_random_genetics(father_genetics, gene_random, 1);
            being_random3(local);
            
            body_genetics(beings, number, being_genetics(local), mother_genetics, father_genetics, gene_random);
            
            being_set_unique_name(beings, number, local, 0L, 0L);
        }
        local->social_x = local->social_nx =
                              (math_random(local->seed) & 32767)+16384;
        local->social_y = local->social_ny =
                              (math_random(local->seed) & 32767)+16384;
        
        local->date_of_birth[0] = 0;
        local->date_of_birth[1] = 0;
    }
    else
    {
        being_set_location(local, being_location(mother));

        /** this is the same as equals */
        being_wander(local, being_facing(mother) - being_facing(local));

        (void) being_random(local);
        local->social_x = local->social_nx = mother->social_x;
        local->social_y = local->social_ny = mother->social_y;
 
        genetics_set(being_genetics(local), being_fetal_genetics(mother));
        
        /** ascribed social status */
        local->honor = (n_byte)being_honor(mother);

        being_set_unique_name(beings, number, local,
                              being_family_name(mother),
                              mother->father_name[1]);
        
        local->date_of_birth[0] = land->date[0];
        local->date_of_birth[1] = land->date[1];
    }
    
    being_living(local);

    if (random_factor)
    {
        GET_H(local) = BIRTH_HEIGHT;
        GET_M(local) = BIRTH_MASS;
    }
    else
    {
        /** produce an initial distribution of heights and masses*/
        being_random3(local);
        GET_H(local) = BIRTH_HEIGHT +
                       (local->seed[0]%(BEING_MAX_HEIGHT-BIRTH_HEIGHT));
        GET_M(local) = BIRTH_MASS +
                       (local->seed[1]%(BEING_MAX_MASS_G-BIRTH_MASS));
    }
    
    local->crowding = MIN_CROWDING;
#ifdef BRAIN_ON
    if (being_brain(local))
    {
        /** These magic numbers were found in March 2001 -
            feel free to change them! */
        being_set_brainatates(local, 0, 171, 0, 146);
        being_set_brainatates(local, 1, 86, 501, 73);

    }
#endif
    return 0;
}

void being_tidy_loop(noble_simulation * local_sim, noble_being * local_being, void * data)
{
    n_genetics  *genetics = being_genetics(local_being);
    n_int        local_honor = being_honor(local_being);
    n_land	    *land   = local_sim->land;
    n_int        delta_e = 0;
    n_int        conductance = 5;
    n_int       *max_honor = data;
    if (local_honor >= 254)
    {
        max_honor[0] = 1;
    }
    if(being_awake(local_sim, local_being))
    {
        n_int	local_s  = being_speed(local_being);
        
        n_vect2	location_vector;
        n_vect2	facing_vector;
        
        vect2_byte2(&location_vector, being_location(local_being));
        
        being_facing_vector(local_being, &facing_vector, 1);
        
        if (local_s > 0)
        {
            n_byte2 location[2];
            vect2_d(&location_vector, &facing_vector, local_s, 512);
            
            /* vector to n_byte2 may do incorrect wrap around MUST be improved */
            location[0] = (n_byte2)APESPACE_WRAP(location_vector.x);
            location[1] = (n_byte2)APESPACE_WRAP(location_vector.y);
            
            being_set_location(local_being, location);
        }
        
        {
            n_int delta_z;
            n_int delta_energy;
            n_int local_z;
            n_vect2 slope_vector;
            
            land_vect2(&slope_vector, &local_z,land, &location_vector);
            
            delta_z = vect2_dot(&slope_vector,&facing_vector,1,96);
            delta_energy = ((512 - delta_z) * local_s)/80;
            
            if (WATER_TEST(local_z,land->tide_level))
            {
                n_int insulation=0;
                /** the more body fat, the less energy is lost whilst swimming */
                n_int fat_mass = GET_BODY_FAT(local_being);
                delta_energy = ((delta_energy * delta_energy) >> 9);
                if (fat_mass > BEING_MAX_MASS_FAT_G)
                {
                    fat_mass = BEING_MAX_MASS_FAT_G;
                }
                insulation = fat_mass * 5 / BEING_MAX_MASS_FAT_G;
                delta_e += (delta_energy + 10 - insulation) >> 3;
                conductance = 4;
            }
            else
            {
                if (delta_z > 0)
                {
                    /** going uphill */
                    delta_energy += GENE_HILL_CLIMB(genetics);
                }

                delta_energy = ((delta_energy * delta_energy) >> 9);
                
                /* the more massive the more energy consumed when moving */
                delta_e += (delta_energy + 4 + (GET_M(local_being)*5/BEING_MAX_MASS_G)) >> 2;
            }
        }
    }
    else
    {
        being_set_speed(local_being, 0);
        delta_e += (7) >> 2;
    }
    
    if (delta_e > 0)
    {
        /** hairy creatures are better insulated */
        delta_e -= ((GENE_HAIR(genetics)*delta_e)>>conductance);
        if (delta_e < 1) delta_e = 1;
    }
    
    being_energy_delta(local_being, 0 - delta_e);
    
    
    if (land->time == 0)
    {
        n_int age_in_years = AGE_IN_YEARS(local_sim,local_being);
        /** this simulates natural death or at least some trauma the ape may or may not be able to recover from */
        if (age_in_years > 29)
        {
            if(being_random(local_being) < (age_in_years - 29))
            {
                being_energy_delta(local_being, 0 - BEING_HUNGRY);
            }
        }
    }
}

void  being_recalibrate_honor_loop(noble_simulation * local, noble_being * value, void * data)
{
    value->honor = (n_byte)(((n_int)value->honor*220)/255);
}

n_int being_remove_internal = 0;
n_int being_remove_external = 0;

void being_remove_loop1(noble_simulation * local_sim, noble_being * local_being, void * data)
{
    if (being_energy_less_than(local_being, BEING_DEAD + 1))
    {
        local_sim->ext_death(local_being,local_sim);
    }
}

void being_remove_loop2(noble_simulation * local_sim, noble_being * local, void * data)
{
    being_remove_loop2_struct * brls = (being_remove_loop2_struct *)data;
    
    if (being_energy_less_than(local, BEING_DEAD + 1) == 0)
    {
        if ( local != brls->being_count )
        {
            io_copy((n_byte *)local, (n_byte *)brls->being_count, sizeof(noble_being));
        }
        brls->being_count++;
        brls->count++;
    }
    else
    {
        being_remains(local_sim, local);
        if (local == brls->reference)
        {
            brls->selected_died = 1;
        }
    }
}

being_remove_loop2_struct * being_remove_initial(noble_simulation * local_sim)
{
    being_remove_loop2_struct * brls = (being_remove_loop2_struct *)io_new(sizeof(being_remove_loop2_struct));
    
    brls->reference = local_sim->select;
    brls->being_count = local_sim->beings;
    brls->selected_died = 0;
    brls->count = 0;
    
    if (being_remove_external)
    {
        do {}
        while(being_remove_external);
    }
    
    being_remove_internal = 1;
    return brls;
}

void being_remove_final(noble_simulation * local_sim, being_remove_loop2_struct ** brls)
{
    local_sim->num = (*brls)->count;
    if ((*brls)->selected_died)
    {
        if ((*brls)->count)
        {
            sim_set_select(local_sim->beings);
        }
        else
        {
            sim_set_select(0L);
        }
    }
    
    if ((*brls)->count == 0)
    {
        (void)SHOW_ERROR("No Apes remain start new run");
    }
    
    being_remove_internal = 0;
    io_free((void **)brls);
}


