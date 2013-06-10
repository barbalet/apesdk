/****************************************************************

 being.c

 =============================================================

 Copyright 1996-2013 Tom Barbalet. All rights reserved.

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

/*NOBLEMAKE DEL=""*/

#include "entity.h"
#include "entity_internal.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*NOBLEMAKE END=""*/

/*NOBLEMAKE VAR=""*/

/*NOBLEMAKE END=""*/

#define GENEALOGY_XML_FILENAME "tmp_genealogy.xml"
#define GENEALOGY_GEDCOM_FILENAME "tmp_genealogy.ged"

#define GENEALOGY_EVENT_BIRTH "E1"
#define GENEALOGY_EVENT_DEATH "E2"
#define GENEALOGY_YEAR_ZERO   1900

/** Genealogy format GenXML 2.0
 http://cosoft.org/genxml */

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
    n_land *land;
}
being_draw;

n_int being_location_x(noble_being * value)
{
    return value->location[0];
}

n_int being_location_y(noble_being * value)
{
    return value->location[1];
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

n_int being_speed(noble_being * value)
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
    value->direction_facing = (n_byte)(math_random(value->seed) & 255);
}

void being_facing_vector(noble_being * value, n_vect2 * vect, n_int divisor)
{
    vect2_direction(vect, value->direction_facing, divisor);
}

n_int being_facing(noble_being * value)
{
    return value->direction_facing;
}

n_genetics * being_genetics(noble_being * value)
{
    return value->genes;
}

n_int   being_energy(noble_being * value)
{
    return value->stored_energy;
}

void   being_set_energy(noble_being * value, n_int energy)
{
    value->stored_energy = energy;
}

void   being_energy_delta(noble_being * value, n_int delta)
{
    being_set_energy(value, being_energy(value) + delta);
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

    while (it_water_turn < 4)
    {
        /* find higher land first */
        n_int    iturn = 5 - it_water_turn;
        n_int    loc_f = being_facing(value);
        n_int    iturn_plus  = loc_f + iturn;
        n_int    iturn_minus = loc_f + (256-iturn);

        n_byte   turn_plus  = (n_byte)((iturn_plus)  & 255);
        n_byte   turn_minus = (n_byte)((iturn_minus) & 255);
        n_vect2  temp_vector;

        n_int  z_plus;
        n_int  z_minus;

        vect2_direction(&temp_vector, turn_plus, 4);
        vect2_add(&temp_vector, &temp_vector, &location_vector);

        z_plus = QUICK_LAND(land, POSITIVE_LAND_COORD(APESPACE_TO_MAPSPACE(temp_vector.x)), POSITIVE_LAND_COORD(APESPACE_TO_MAPSPACE(temp_vector.y)));

        vect2_direction(&temp_vector, turn_minus, 4);
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

/**
 * @brief Applies a function to each being in the simulation
 * @param sim Pointer to the simulation object
 * @param bnr_func The function to be applied
 */

void being_loop_no_return(noble_simulation * sim, being_no_return bnr_func)
{
    n_uint loop = 0;
    while (loop < sim->num)
    {
        noble_being * output = &(sim->beings[loop]);
        (bnr_func)(sim, output);
        loop++;
    }
}

/**
 * @brief Check if a being is on ground or in water
 * @param px x coordinate of the being location
 * @param py y coordinate of the being location
 * @param params
 * @return 1 if on ground, 0 otherwise
 */
static n_byte	being_ground(n_int px, n_int py, void * params)
{
    being_draw * being_pixel = (being_draw *) params;
    n_int	local_z = ((px*(being_pixel->offset_x)) + (py*(being_pixel->offset_y))) >> 9;
    local_z += being_pixel->start_z;
    if (local_z < WALK_ON_WATER(QUICK_LAND(being_pixel->land, px, py),being_pixel->land->tide_level))
    {
        return 1;
    }
    return 0;
}

static n_byte being_los_projection(n_land * land, noble_being * local, n_int lx, n_int ly)
{
    n_vect2    start, delta, vector_facing;

    vect2_byte2(&start, being_location(local));

    delta.x = lx;
    delta.y = ly;

    vect2_subtract(&delta, &delta, &start);

    {
        n_int distance_squared = vect2_dot(&delta, &delta, 1, 1);
        /* TODO: This should also include weather conditions (eg seeing through the rain) */
        if (distance_squared > VISUAL_DISTANCE_SQUARED)
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
        }

        translate.land = land;
        {
            n_join		  being_point;
            being_point.information = (void *) &translate;
            being_point.pixel_draw  = &being_ground;

            if(math_join(start.x, start.y, delta.x, delta.y, &being_point))
            {
                return 0;
            }
        }
    }
    return 1;
}

/**
 * @brief return the being array index with the given name
 * @param sim Pointer to the simulation object
 * @param name Name of the being
 * @return Array index of the being within the simulation object
 */
static n_uint being_num_from_name(noble_simulation * sim, n_string name)
{
    n_int i;
    noble_being * b;

    if (io_length(name,STRING_BLOCK_SIZE)<10) return NO_BEINGS_FOUND;

    io_lower(name, io_length(name,STRING_BLOCK_SIZE));

    for (i = 0; i < (n_int)(sim->num); i++)
    {
        n_string_block str;
        b = &sim->beings[i];
        being_name((FIND_SEX(GET_I(b)) == SEX_FEMALE), GET_NAME(sim,b), GET_FAMILY_FIRST_NAME(sim,b), GET_FAMILY_SECOND_NAME(sim,b), str);

        io_lower(str, io_length(str,STRING_BLOCK_SIZE));

        if (io_find(str,0,io_length(str,STRING_BLOCK_SIZE),name,io_length(name,STRING_BLOCK_SIZE))>-1)
        {
            return i;
        }
    }
    return NO_BEINGS_FOUND;
}


void being_set_select_name(noble_simulation * sim, n_string name)
{
    n_uint response = being_num_from_name(sim, name);
    if ((response == NO_BEINGS_FOUND) && sim->num)
    {
        (void)SHOW_ERROR("Ape not found");
        return;
    }
    sim->select = response;
}

n_string being_get_select_name(noble_simulation * sim)
{
    static n_string_block name;
    noble_being *b;

    if (sim->select == NO_BEINGS_FOUND)
    {
        sprintf(name,"*** ALL APES DEAD ***");
    }
    else
    {
        b = &sim->beings[sim->select];
        being_name((FIND_SEX(GET_I(b)) == SEX_FEMALE), GET_NAME(sim,b), GET_FAMILY_FIRST_NAME(sim,b), GET_FAMILY_SECOND_NAME(sim,b), name);
    }
    return (n_string)name;
}

/**
 * @brief return the being object with the given name
 * @param sim Pointer to the simulation object
 * @param name Name of the being
 * @return Pointer to the being, or NO_BEINGS_FOUND
 */
noble_being * being_from_name(noble_simulation * sim, n_string name)
{
    n_uint response = being_num_from_name(sim, name);

    if (response == NO_BEINGS_FOUND)
    {
        return 0L;
    }
    return &(sim->beings[sim->select]);
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

/*NOBLEMAKE END=""*/

static void being_immune_init(noble_being * local)
{
#ifdef IMMUNE_ON
    n_byte i;
    n_byte2 * local_random = local->seed;
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
    n_byte2 * local_random = local->seed;

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
    n_byte2 * local_random = meeter_being->seed;
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
    n_byte2 * local_random = local->seed;
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
            if (immune->antigens[i]>IMMUNE_STRENGTH(local))
            {
                immune->antigens[i]-=IMMUNE_STRENGTH(local);
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
    if ((local_random[0] < (total_antigens>>2)) && (being_energy(local) > BEING_DEAD))
    {
        if (being_energy(local)>PATHOGEN_SEVERITY(max_severity))
        {
            being_energy_delta(local, 0-PATHOGEN_SEVERITY(max_severity));
        }
        else
        {
            being_set_energy(local, BEING_DEAD);
        }
    }
#endif
}

static noble_being * being_find_child(noble_simulation * sim, n_genetics * genetics, n_uint max_age)
{
    n_int today = TIME_IN_DAYS(sim->land->date);
    n_uint loop = 0;
    while ( loop < sim->num )
    {
        noble_being * local = &(sim->beings[loop]);
        if (genetics_compare(local->mother_genetics, genetics))
        {
            if (max_age == 0)
            {
                return local;
            }
            else
            {
                if ((today - being_dob(local)) < max_age)
                    return local;
            }
        }
        loop++;
    }
    return 0L;
}

noble_being * being_find_female(noble_simulation * sim, n_genetics * genetics)
{
    n_uint loop = 0;
    while ( loop < sim->num )
    {
        noble_being * local = &(sim->beings[loop]);

        if (genetics_compare(being_genetics(local), genetics))
        {
            if (FIND_SEX(GET_I(local)) == SEX_FEMALE)
            {
                return local;
            }
        }
        loop++;
    }
    return 0L;
}

noble_being * being_find_name(noble_simulation * sim, n_byte2 first_gender, n_byte2 family)
{
    n_uint loop = 0;
    while ( loop < sim->num )
    {
        noble_being * local = &(sim->beings[loop]);

        if ( (GET_NAME_GENDER(sim,local) == first_gender) &&
                (GET_NAME_FAMILY2(sim,local) == family) )
        {
            return local;
        }
        loop++;
    }
    return 0L;
}

/** returns the total positive and negative affect within memory */
n_uint being_affect(noble_simulation * local_sim, noble_being * local, n_byte is_positive)
{
    n_uint affect = 0;
#ifdef EPISODIC_ON
    n_uint i;
    episodic_memory * local_episodic = (episodic_memory*)GET_EPI(local_sim, local);
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
    if (index >= RELATIONSHIPS)
    {
        sprintf(description,"ERROR: relationship out of range %d\n",(int)index);
        return;
    }
    sprintf(description,"%s",relationship_description[index]);
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
    episodic_memory * local_episodic;
    n_uint days_elapsed,time_elapsed;
    n_uint current_date;

    current_date = TIME_IN_DAYS(sim->land->date);
    local_episodic = GET_EPI(sim, local_being);

    if(local_episodic == 0L)
    {
        return SHOW_ERROR("No episodic description");
    }

    if ((local_episodic[index].event>0) &&
            (local_episodic[index].first_name[0]==GET_NAME_GENDER(sim,local_being)) &&
            (local_episodic[index].family_name[0]==GET_NAME_FAMILY2(sim,local_being)))
    {
        being_name(((local_episodic[index].first_name[BEING_MET]>>8) == SEX_FEMALE),
                   local_episodic[index].first_name[BEING_MET]&255,
                   UNPACK_FAMILY_FIRST_NAME(local_episodic[index].family_name[BEING_MET]),
                   UNPACK_FAMILY_SECOND_NAME(local_episodic[index].family_name[BEING_MET]),
                   name_str);

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
    str[string_index]='\0';
    sprintf(description,"%s",str);
    return social;
}

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

void  being_name(n_byte female, n_int first, n_byte family0, n_byte family1, n_string name)
{
    if (first != -1)
    {
        if (female)
        {
            sprintf(name,"%s %s-%s",EnglishNames[ NAMES_SURNAMES + first ], EnglishNames[ family0 ], EnglishNames[ family1 ]);
            return;
        }
        else
        {
            sprintf(name,"%s %s-%s",EnglishNames[ NAMES_SURNAMES + 256 + first ], EnglishNames[ family0 ], EnglishNames[ family1 ]);
            return;
        }
    }
    sprintf(name,"%s","Unknown");
}

void being_state_description(n_byte2 state, n_string result)
{
    const n_string state_description[] =
    {
        "Sleeping","Awake","Foraging","Swimming","Eating","Moving",
        "Chatting","Calling","Reproducing","Grooming","Suckling",
        "Displaying","Fighting"
    };
    n_int string_length=0;
    n_int n=2;

    if (state == BEING_STATE_ASLEEP)
    {
        io_string_write(result, state_description[0], &string_length);
        result[string_length]=0;
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
    result[string_length]=0;
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

/**
 This checks to see if the Noble Ape is awake
 @param sim The simulation pointer
 @param reference The specific Noble Ape checked for being awake
 @return 2 is fully awake, 1 is slightly awake to eat, 0 is asleep
 */
n_byte being_awake(noble_simulation * sim, noble_being * local)
{
    n_land  * land  =   sim->land;
    n_int     local_energy = being_energy(local);


    if(local_energy == BEING_DEAD)
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

    if(local_energy < BEING_HUNGRY)
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

#ifdef BRAINCODE_ON

n_int brain_probe_to_location(n_int position)
{
    /** could have a more interesting translation */
    return ((position * (SINGLE_BRAIN>>8))) % SINGLE_BRAIN;
}

static void update_brain_probes(noble_simulation * sim, noble_being * local)
{
    n_byte * brain_point = GET_B(sim, local);
    n_int    i, inputs = 0, outputs = 0;
    /** count the inputs and outputs */
    for (i=0; i<BRAINCODE_PROBES; i++)
    {
        if (local->brainprobe[i].type == INPUT_SENSOR)
        {
            inputs++;
        }
        else
        {
            outputs++;
        }
    }
    /** check to ensure that there are a minimum number of sensors and actuators */
    if (inputs < (BRAINCODE_PROBES>>2))
    {
        local->brainprobe[0].type = INPUT_SENSOR;
    }
    else
    {
        if (outputs < (BRAINCODE_PROBES>>2))
        {
            local->brainprobe[0].type = OUTPUT_ACTUATOR;
        }
    }
    /** update each probe */
    for (i=0; i<BRAINCODE_PROBES; i++)
    {
        local->brainprobe[i].state++;
        if (local->brainprobe[i].state >= local->brainprobe[i].frequency)
        {
            /** position within the brain */
            n_int n1 = brain_probe_to_location(local->brainprobe[i].position);

            local->brainprobe[i].state = 0;

            if (local->brainprobe[i].type == INPUT_SENSOR)
            {
                n_byte * local_braincode = GET_BRAINCODE_INTERNAL(sim,local);
                /** address within braincode */
                n_int n2 = local->brainprobe[i].address % BRAINCODE_SIZE;
                n_int n3 = (brain_point[n1] + local->brainprobe[i].offset)&255;
                /** read from brain */
                local_braincode[n2] = (n_byte)n3;
            }
            else
            {
                /** write to brain */
                brain_point[n1] = 255;
            }
        }
    }
}

#endif

/** stuff still goes on during sleep */
void being_cycle_universal(noble_simulation * sim, noble_being * local, n_byte awake)
{
    /* By default return towards a resting state */
#ifdef METABOLISM_ON
    metabolism_cycle(sim, local);
#endif

    being_immune_response(local);

#ifdef BRAINCODE_ON
    /** may need to add external probe linking too */
    if (GET_B(sim,local))
    {
        update_brain_probes(sim, local);
    }
#endif

    if (awake == 0)
    {
        local->state = BEING_STATE_ASLEEP;

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
    social_link * parent_social_graph;

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
            parent_social_graph = GET_SOC(sim, parent[j]);
            if (parent_social_graph)
            {
                for (i = 0; i < 2; i++) /** grandmother or grandfather */
                {
                    parent[2+(j*2)+i] = 0L;
                    /** graph index for parent's mother or father */
                    index = social_get_relationship(parent[j],RELATIONSHIP_MOTHER+i,sim);
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
            parent_social_graph = GET_SOC(sim, parent[j]);
            if (parent_social_graph)
            {
                for (i=1; i<SOCIAL_SIZE; i++)
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

/**
 * Follow a being to which we are paying attention
 * @param sim Pointer to the simulation
 * @param current_being_index Array index of the current being
 * @param opposite_sex Array index of the closest being of the opposite sex
 * @param same_sex Array index of the closest being of the same sex
 * @param opposite_sex_distance Returned distance to the closest being of the opposite sex
 * @param same_sex_distance Returned distance to the closest being of the same sex
 * @return 1 if the attended to being is seen, 0 otherwise
 */
static int being_follow(noble_simulation * sim,
                        n_uint current_being_index,
                        n_uint * opposite_sex, n_uint * same_sex,
                        n_uint * opposite_sex_distance, n_uint * same_sex_distance)
{
    noble_being * being_buffer = sim->beings;
    noble_being * local        = &being_buffer[current_being_index];
    n_vect2       difference_vector;
    social_link * local_social_graph;
    n_int social_graph_index;
    n_uint i;
    n_int result_los;

    *opposite_sex_distance = 0xffffffff;
    *same_sex_distance = 0xffffffff;
    *opposite_sex = NO_BEINGS_FOUND;
    *same_sex = NO_BEINGS_FOUND;

    /** is a mate in view? */
    if (local->goal[0]==GOAL_MATE)
    {
        for (i = 0; i < sim->num; i++)
        {
            if (i != current_being_index)
            {
                /** get the name of the other being */
                noble_being * other = &being_buffer[i];
                n_byte2 other_first_name = GET_NAME_GENDER(sim,other);
                n_byte2 other_family_name = GET_NAME_FAMILY2(sim,other);
                /** is this the same as the name of the being to which we are paying attention? */
                if ((FIND_SEX(GET_I(other))!=FIND_SEX(GET_I(local))) &&
                        (local->goal[1]==other_first_name) &&
                        (local->goal[2]==other_family_name))
                {
                    being_delta(local, other, &difference_vector);
                    result_los = being_los(sim->land, local, (n_byte2)difference_vector.x, (n_byte2)difference_vector.y);
                    if (result_los)
                    {
                        n_uint compare_distance = vect2_dot(&difference_vector, &difference_vector, 1, 1);
                        *opposite_sex = i;
                        *opposite_sex_distance = compare_distance;
                        return 1;
                    }
                }
            }
        }
    }

    local_social_graph = GET_SOC(sim, local);
    if (local_social_graph == 0L) return 0;

    /** which entry in the social graph are we paying attention to? */
    social_graph_index = GET_A(local,ATTENTION_ACTOR);
    /** Does this entry correspond to another being? */
    if ((social_graph_index>0) &&
            (local_social_graph[social_graph_index].entity_type==ENTITY_BEING) &&
            (!SOCIAL_GRAPH_ENTRY_EMPTY(local_social_graph, social_graph_index)))
    {
        /** search for the other being */
        for (i = 0; i < sim->num; i++)
        {
            if (i != current_being_index)
            {
                /** get the name of the other being */
                noble_being * other = &being_buffer[i];
                n_byte2 other_first_name = GET_NAME_GENDER(sim,other);
                n_byte2 other_family_name = GET_NAME_FAMILY2(sim,other);
                /** is this the same as the name of the being to which we are paying attention? */
                if ((local_social_graph[social_graph_index].first_name[BEING_MET]==other_first_name) &&
                        (local_social_graph[social_graph_index].family_name[BEING_MET]==other_family_name))
                {
                    /** Is this being within sight? */
                    being_delta(local, other, &difference_vector);
                    result_los = being_los(sim->land, local, (n_byte2)difference_vector.x, (n_byte2)difference_vector.y);
                    if (result_los)
                    {
                        n_uint compare_distance = vect2_dot(&difference_vector, &difference_vector, 1, 1);
                        if (FIND_SEX(GET_I(other))!=FIND_SEX(GET_I(local)))
                        {
                            *opposite_sex = i;
                            *opposite_sex_distance = compare_distance;
                        }
                        else
                        {
                            *same_sex = i;
                            *same_sex_distance = compare_distance;
                        }
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

/**
 * Listen for shouts
 * @param sim Pointer to the simulation
 * @param current_being_index Array index of the current being
 */
static void being_listen(noble_simulation * sim,
                         n_uint current_being_index)
{
    n_uint i;
    noble_being * being_buffer = sim->beings;
    noble_being * local        = &being_buffer[current_being_index];
    n_int         max_shout_volume = 127;
    n_vect2       difference_vector;
    n_uint        compare_distance;

    /** clear shout values */
    local->shout[SHOUT_CONTENT] = 0;
    local->shout[SHOUT_HEARD] = 0;
    if (local->shout[SHOUT_CTR] > 0)
    {
        local->shout[SHOUT_CTR]--;
    }
    for (i = 0; i < sim->num; i++)
    {
        if (i != current_being_index)
        {
            noble_being	* other = &being_buffer[i];

            being_delta(local, other, &difference_vector);
            compare_distance = vect2_dot(&difference_vector, &difference_vector, 1, 1);
            /** listen for the nearest shout out */
            if ((other->state&BEING_STATE_SHOUTING) &&
                    (compare_distance < SHOUT_RANGE) &&
                    (other->shout[SHOUT_VOLUME] > max_shout_volume))
            {
                max_shout_volume = other->shout[SHOUT_VOLUME];
                local->shout[SHOUT_HEARD] = other->shout[SHOUT_CONTENT];
                local->shout[SHOUT_FAMILY0] = GET_FAMILY_FIRST_NAME(sim,other);
                local->shout[SHOUT_FAMILY1] = GET_FAMILY_SECOND_NAME(sim,other);
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
static int being_closest(noble_simulation * sim,
                         n_uint current_being_index,
                         n_uint	* opposite_sex, n_uint * same_sex,
                         n_uint * opposite_sex_distance, n_uint * same_sex_distance)
{
    n_byte        opposite_sex_seen = 0;
    n_uint	      loop = 0;
    noble_being * being_buffer = sim->beings;
    n_uint        number       = sim->num;
    noble_being * local        = &being_buffer[current_being_index];
    n_byte        beings_in_vicinity = 0;
    n_uint        local_is_female = FIND_SEX(GET_I(local));

    *opposite_sex_distance = 0xffffffff;
    *same_sex_distance = 0xffffffff;
    *opposite_sex = NO_BEINGS_FOUND;
    *same_sex = NO_BEINGS_FOUND;

    while (loop < number)
    {
        if (loop != current_being_index)
        {
            noble_being	* test_being = &being_buffer[loop];
            n_int         los_previously_calculated = 0;
            n_int         result_los = 0;
            /** check distance before line of sight */
            n_vect2       difference_vector;
            n_uint         compare_distance;

            being_delta(local, test_being, &difference_vector);

            compare_distance = vect2_dot(&difference_vector, &difference_vector, 1, 1);

            if ( compare_distance < SOCIAL_RANGE )
            {
                beings_in_vicinity++;
            }
            /* '<' : signed/unsigned mismatch */
            if ((opposite_sex_seen==0) && ( compare_distance < *opposite_sex_distance ))
            {
                if (FIND_SEX(GET_I(test_being)) != local_is_female)
                {
                    /* 'function' : conversion from 'n_int' to 'n_byte2', possible loss of data x 2 */
                    los_previously_calculated = 1;
                    result_los = being_los(sim->land, local, (n_byte2)difference_vector.x, (n_byte2)difference_vector.y);
                    if (result_los)
                    {
                        *opposite_sex_distance = compare_distance;
                        *opposite_sex = loop;
                    }
                }
            }
            /** '<' : signed/unsigned mismatch */
            if ( compare_distance < *same_sex_distance )
            {
                if (FIND_SEX(GET_I(test_being)) == local_is_female)
                {
                    if (los_previously_calculated == 0)
                    {
                        result_los = being_los(sim->land, local, (n_byte2)difference_vector.x,  (n_byte2)difference_vector.y);
                    }
                    if (result_los)
                    {
                        *same_sex_distance = compare_distance;
                        *same_sex = loop;
                    }
                }
            }
        }
        loop++;
    }
    return beings_in_vicinity;
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
                           n_uint   being_index,
                           n_uint   other_being_index,
                           n_uint   other_being_distance,
                           n_int  * awake,
                           n_byte * state,
                           n_int  * speed,
                           n_int  * energy,
                           n_byte   opposite_sex)
{
    if (other_being_index != NO_BEINGS_FOUND)
    {
        noble_being * being_buffer = sim->beings;
        noble_being * local        = &being_buffer[being_index];
        n_land      * land         = sim->land;
        n_byte2     * today        = land->date;
        n_int         today_days   = TIME_IN_DAYS(today);
        n_int         birth_days   = being_dob(local);
        n_uint        local_is_female = FIND_SEX(GET_I(local));

        noble_being	* other_being = &being_buffer[other_being_index];

        n_vect2 delta_vector;

        /** social networking */
        n_byte2 familiarity=0;
        n_int   being_index = social_network(local, other_being, other_being_distance, sim);

        being_delta(local, other_being, &delta_vector);

        if (being_index > -1)
        {
            social_link * local_social_graph = GET_SOC(sim, local);
            if (local_social_graph)
            {
                familiarity = local_social_graph[being_index].familiarity;
            }
        }

        being_facing_towards(local, &delta_vector);

        if ((genetics_compare(local->mother_genetics, 0L)) || ((birth_days+AGE_OF_MATURITY)<today_days))
        {
#ifdef PARASITES_ON
            if (social_groom(local, other_being, other_being_distance, *awake, familiarity, sim))
            {
                *state |= BEING_STATE_GROOMING;

                /* both beings stop */
                *speed = 0;
                being_set_speed(other_being, 0);
            }
            else
            {
#endif
                /* squabbling between adults */
                if ((other_being_distance < SQUABBLE_RANGE) && ((being_dob(other_being)+AGE_OF_MATURITY) < today_days))
                {
                    n_byte2 squabble_val;
                    being_set_energy(local, *energy);
                    being_set_speed(local, *speed);
                    squabble_val = social_squabble(local, other_being, other_being_distance, local_is_female, sim);
                    if (squabble_val != 0)
                    {
                        *state |= squabble_val;
                        *energy = being_energy(local);
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
                *state |= social_mate(local, other_being, today, being_index, other_being_distance, sim);
            }

            /* chat */
            *state |= social_chat(local, other_being, being_index, sim);
        }
    }
}

void being_cycle_awake(noble_simulation * sim, n_uint current_being_index)
{
    n_uint	      loop;
    n_land      * land               = sim->land;
    n_byte2     * today              = land->date;
    noble_being * local              = &(sim->beings[current_being_index]);
    n_uint        local_is_female    = FIND_SEX(GET_I(local));
    n_int         today_days         = TIME_IN_DAYS(today);
    n_int         birth_days         = being_dob(local);
    n_byte        beings_in_vicinity = 0;

    n_int	      loc_s              = being_speed(local);
    n_int	      loc_e              = being_energy(local);
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
    n_int	delta_z;

    n_vect2 location_vector;
    n_vect2 facing_vector;
    n_vect2 slope_vector;
    n_vect2 looking_vector;
    n_uint territory_index;

    /** Listen for any shouts */
    being_listen(sim,current_being_index);

#ifdef EPISODIC_ON
    episodic_cycle(sim,local);
#endif

    vect2_byte2(&location_vector, being_location(local));

    being_facing_vector(local, &facing_vector, 4);

    land_vect2(&slope_vector,&az,land,&location_vector);

    vect2_add(&looking_vector, &location_vector, &facing_vector);

    delta_z = vect2_dot(&slope_vector,&facing_vector,1,24);

    tmp_speed = ((delta_z + 280) >> 4);

#ifdef LAND_ON
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

        n_int   hungry = (loc_e < BEING_HUNGRY);

        if ((loc_state & (BEING_STATE_AWAKE | BEING_STATE_SWIMMING | BEING_STATE_MOVING)) == BEING_STATE_AWAKE)
        {
            hungry = (loc_e < BEING_FULL);
        }

        if (hungry != 0)
        {
            loc_state |= BEING_STATE_HUNGRY;
        }
    }
    local->speak = 0;

    /** amount of body fat in kg */
    fat_mass = GET_BODY_FAT(local);
    if (fat_mass > BEING_MAX_MASS_FAT_G)
    {
        fat_mass = BEING_MAX_MASS_FAT_G;
    }

    /** If it sees water in the distance then turn */
#ifdef LAND_ON
    if (((loc_state & BEING_STATE_SWIMMING) != 0) ||
            (MAP_WATERTEST(land, POSITIVE_LAND_COORD(APESPACE_TO_MAPSPACE(looking_vector.x)),
                           POSITIVE_LAND_COORD(APESPACE_TO_MAPSPACE(looking_vector.y))) != 0))

#else
    if ((loc_state & BEING_STATE_SWIMMING) != 0)
#endif
    {
        being_turn_away_from_water(local, land);

        /** horizontally oriented posture */
        GET_PS(local) = 0;
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

#ifdef PARASITES_ON
        /** bathing removes parasites */
        if (local->parasites > 0) local->parasites--;
#endif

    }
    else
    {

        n_uint	opposite_sex = NO_BEINGS_FOUND;
        n_uint opposite_sex_distance = 0xffffffff;
        n_uint	same_sex = NO_BEINGS_FOUND;
        n_uint same_sex_distance = 0xffffffff;

        /** adjust speed using genetics */
        tmp_speed = (tmp_speed * (GENE_SPEED(genetics)+8)) >> 3;

        /** is the being to which we are paying attention within view? */
        beings_in_vicinity = being_follow(sim,current_being_index,
                                          &opposite_sex, &same_sex,
                                          &opposite_sex_distance, &same_sex_distance);
        if (beings_in_vicinity==0)
        {
            /** Find the closest beings */
            beings_in_vicinity =
                being_closest(sim, current_being_index,
                              &opposite_sex, &same_sex,
                              &opposite_sex_distance, &same_sex_distance);
        }

        if (being_drive(local, DRIVE_SOCIAL) > SOCIAL_THRESHOLD(local))
        {
            being_interact(sim,
                           current_being_index,
                           same_sex, same_sex_distance,
                           &awake, &loc_state,
                           &loc_s, &loc_e, 0);

            being_interact(sim,
                           current_being_index,
                           opposite_sex, opposite_sex_distance,
                           &awake, &loc_state,
                           &loc_s, &loc_e, 1);
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
                n_int energy = food_eat(sim->land, sim->weather, location_vector.x, location_vector.y, az, &food_type, local);

                INDICATOR_INC(sim, IT_FOOD+food_type);

                /** remember eating */
                episodic_food(sim, local, energy, food_type);

                if (energy > BEING_DEAD)
                {
#ifdef METABOLISM_ON
                    metabolism_vascular_response(sim, local, VASCULAR_PARASYMPATHETIC);
#endif
                    loc_e += energy;

                    being_reset_drive(local, DRIVE_HUNGER);

                    INDICATOR_ADD(sim, IT_AVERAGE_ENERGY_INPUT, energy);

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
                else
                {
                    loc_s = 10;
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
            tmp_speed = (loc_e >> 7);
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

    /** Create a wander based on the brain value */
    if ((local->goal[0]==GOAL_NONE) &&
            (beings_in_vicinity==0) &&
            (math_random(local->seed) < 1000 + 3600*GENE_STAGGER(genetics)))
    {
        n_byte * local_brain = GET_B(sim, local);
        n_int	 wander = 0;

        if (local_brain != 0L)
        {
            wander =
                (math_spread_byte(local_brain[BRAIN_OFFSET(22+(15*32)+(15*32*32))]>>4)) -
                (math_spread_byte(local_brain[BRAIN_OFFSET(11+(15*32)+(15*32*32))]>>4));
        }
        else
        {
            wander = math_spread_byte(math_random(local->seed) & 7);
        }

        being_wander(local, wander);
    }

    /** a certain time after giving birth females become receptive again */
    if ((TIME_IN_DAYS(local->date_of_conception) != 0) &&
            ((TIME_IN_DAYS(local->date_of_conception) + GESTATION_DAYS + CONCEPTION_INHIBITION_DAYS) < today_days))
    {
        /** zero value indicates ready to conceive */
        local->date_of_conception[0] = 0;
        local->date_of_conception[1] = 0;
    }

    if ((loc_state & (BEING_STATE_AWAKE | BEING_STATE_SWIMMING)) == BEING_STATE_AWAKE)
    {
        n_uint conception_days = TIME_IN_DAYS(local->date_of_conception) ;

        if (conception_days > 0)
        {
            n_uint gestation_days = conception_days + GESTATION_DAYS;
            if (today_days > gestation_days)
            {
                /** A mother could have multiple children, so only find the youngest */
                noble_being * being_child = being_find_child(sim, genetics, CARRYING_DAYS);

                /** Birth */
                if (being_child == 0L)
                {
                    (void)being_init(sim, local, -1, 0);
                    loc_state |= BEING_STATE_REPRODUCING;
                    being_child = &(sim->beings[sim->num-1]);
                    episodic_close(sim, local, being_child, EVENT_BIRTH, AFFECT_BIRTH, 0);
                    being_create_family_links(local,being_child,sim);
                }
                else
                {
                    /** mother carries the child */
                    n_uint carrying_days = conception_days + GESTATION_DAYS + CARRYING_DAYS;
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
        if ((genetics_compare(local->mother_genetics, 0L)) &&
                ((birth_days + WEANING_DAYS) > today_days))
        {
            noble_being * mother = being_find_female(sim,local->mother_genetics);
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
                        if (being_energy(mother) > BEING_HUNGRY)
                        {
                            /** suckling induces relaxation */
#ifdef METABOLISM_ON
                            metabolism_suckle(sim,local,mother);
#endif
                            /** mother loses energy */
                            being_energy_delta(mother, 0 - SUCKLING_ENERGY);
                            /** child gains energy */
                            loc_e += SUCKLING_ENERGY;
                            /** update indicators */

                            INDICATOR_ADD(sim, IT_AVERAGE_ENERGY_INPUT, SUCKLING_ENERGY);

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

    /** update biological drives */
    drives_cycle(local, beings_in_vicinity, awake, sim);

    being_set_energy(local, loc_e);
    being_set_speed(local, loc_s);
    GET_H(local) = (n_byte2) loc_h;
    GET_M(local) = (n_byte2)((BEING_MAX_MASS_G*loc_h/BEING_MAX_HEIGHT)+fat_mass+child_mass);
    local->state = loc_state;

    INDICATOR_ADD(sim, IT_AVERAGE_MOBILITY, loc_s);
}


/** initialise inner or outer braincode */
void being_init_braincode(noble_simulation * sim,
                          noble_being * local,
                          noble_being * other,
                          n_byte2* local_random,
                          n_byte friend_foe,
                          n_byte internal)
{
    n_uint ch,i,most_similar_index,diff,min,actor_index;
    social_link * graph;

    if (other==0L)
    {
        /** initially seed the brain with instructions which are random but genetically biased */
        for (ch = 0; ch < BRAINCODE_SIZE; ch+=3)
        {
            math_random3(local_random);
            if (internal!=0)
            {
#ifdef RANDOM_INITIAL_BRAINCODE
                GET_BRAINCODE_INTERNAL(sim,local)[ch] = math_random(local_random) & 255;
#else
                GET_BRAINCODE_INTERNAL(sim,local)[ch] = (math_random(local_random) & 192) | get_braincode_instruction(local);
#endif
                GET_BRAINCODE_INTERNAL(sim,local)[ch+1] = math_random(local_random) & 255;
                GET_BRAINCODE_INTERNAL(sim,local)[ch+2] = math_random(local_random) & 255;
            }
            else
            {
#ifdef RANDOM_INITIAL_BRAINCODE
                GET_BRAINCODE_INTERNAL(sim,local)[ch] = math_random(local_random) & 255;
#else
                GET_BRAINCODE_EXTERNAL(sim,local)[ch] = (math_random(local_random) & 192) | get_braincode_instruction(local);
#endif
                GET_BRAINCODE_EXTERNAL(sim,local)[ch+1] = math_random(local_random) & 255;
                GET_BRAINCODE_EXTERNAL(sim,local)[ch+2] = math_random(local_random) & 255;
            }
        }
    }
    else
    {
        /** initialise based upon a similar being */
        graph = GET_SOC(sim, local);

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

/** Assign a unique name to the given being, based upon the given family names */

static n_int being_set_unique_name(noble_simulation * sim,
                                   noble_being * local_being,
                                   n_int random_factor,
                                   n_byte2 mother_family_name,
                                   n_byte2 father_family_name)
{
    n_uint i;
    n_int samples=0,found=0;
    n_byte2 possible_family_name;
    n_byte2 possible_first_name;
    n_byte2 local_random[2];

    /** random number initialization */
    local_random[0] = (n_byte2)(random_factor & 0xffff);
    local_random[1] = (n_byte2)(random_factor & 0xffff);
    math_random3(local_random);

    /** if no mother and father are specified then randomly create names */
    if ((mother_family_name==0) &&
            (father_family_name==0))
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

    /** avoid the same two family names */
    if (UNPACK_FAMILY_FIRST_NAME(mother_family_name) ==
            UNPACK_FAMILY_SECOND_NAME(father_family_name))
    {
        possible_family_name =
            GET_NAME_FAMILY(UNPACK_FAMILY_FIRST_NAME(mother_family_name),
                            (math_random(local_random) & FAMILY_NAME_AND_MOD));
    }

    while ((found == 0) && (samples < 2048))
    {
        /** choose a first_name at random */
        possible_first_name = (math_random(local_random) & 255) | (FIND_SEX(GET_I(local_being))<<8);

        if (samples == 1024)
        {
            /** switch naming order */
            possible_family_name =
                GET_NAME_FAMILY(UNPACK_FAMILY_SECOND_NAME(mother_family_name),
                                UNPACK_FAMILY_FIRST_NAME(father_family_name));

            /** avoid the same two family names */
            if (UNPACK_FAMILY_SECOND_NAME(mother_family_name) ==
                    UNPACK_FAMILY_FIRST_NAME(father_family_name))
            {
                possible_family_name =
                    GET_NAME_FAMILY(UNPACK_FAMILY_SECOND_NAME(mother_family_name),
                                    (math_random(local_random) & FAMILY_NAME_AND_MOD));
            }
        }

        /** does the name already exist in the population */
        found = 1;
        for (i = 0; i < sim->num; i++)
        {
            noble_being * other_being = &sim->beings[i];
            if (other_being == local_being) continue;
            if ((GET_NAME_GENDER(sim,other_being) == possible_first_name) &&
                    (GET_NAME_FAMILY2(sim,other_being) == possible_family_name))
            {
                found = 0;
                break;
            }
        }
        if (found == 1)
        {
            SET_FIRST_NAME(sim,local_being,possible_first_name);
            SET_FAMILY_NAME(sim,local_being,
                            UNPACK_FAMILY_FIRST_NAME(possible_family_name),
                            UNPACK_FAMILY_SECOND_NAME(possible_family_name));
        }
        samples++;
    }
    return found;
}

/**
 * Initialise the ape's variables and clear its brain
 * @param sim Pointer to the simulation object
 * @param mother Pointer to the mother
 * @param random_factor Random seed
 * @param first_generation If non zero this is the first generation
 * @return 0
 */
n_int being_init(noble_simulation * sim, noble_being * mother,
                 n_int random_factor, n_byte first_generation)
{

    if((sim->num + 1) >= sim->max)
        return 0;

    {
        /** this is the being to be born */
        noble_being * local = &(sim->beings[sim->num]);
        n_land  * land  = sim->land;
        n_byte        ch;
        n_byte2		  numerical_brain_location =
            local->brain_memory_location;
        n_byte      * brain_memory;
#ifdef EPISODIC_ON
        social_link * local_social_graph = GET_SOC(sim, local);
        episodic_memory * local_episodic = GET_EPI(sim, local);
#endif
        n_genetics * mother_genetics = 0L;


        /***********************************************
         Please note: This clears the being structure to
                      0. No further zeroing is required.
         ***********************************************/
        io_erase((n_byte *) local, sizeof(noble_being));

        local->brain_memory_location = numerical_brain_location;

        brain_memory = GET_B(sim, local);

        if (brain_memory != 0L)
        {
            io_erase(brain_memory, DOUBLE_BRAIN);
        }

        local->goal[0]=GOAL_NONE;

        /** Set learned preferences to 0.5 (no preference in
        	either direction.
        	This may seem like tabla rasa, but there are genetic
        	biases */
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
        for (ch = 0; ch < 3; ch++)
        {
            local->generation[ch] = 0;
        }

#ifdef BRAINCODE_ON

#undef BARBALET_VERSION

#ifdef BARBALET_VERSION
        for (ch = 0; ch < BRAINCODE_SIZE; ch+=3)
        {
            math_random3(local_random);
            GET_BRAINCODE_INTERNAL(sim,local)[ch] =
                (math_random(local_random) & 192) |
                (math_random(local_random) % (BRAINCODE_DAT+1));
            GET_BRAINCODE_INTERNAL(sim,local)[ch+1] =
                math_random(local_random) & 255;
            GET_BRAINCODE_INTERNAL(sim,local)[ch+2] =
                math_random(local_random) & 255;
            math_random3(local_random);
            GET_BRAINCODE_EXTERNAL(sim,local)[ch] =
                (math_random(local_random) & 192) |
                (math_random(local_random) % (BRAINCODE_DAT+1));
            GET_BRAINCODE_EXTERNAL(sim,local)[ch+1] =
                math_random(local_random) & 255;
            GET_BRAINCODE_EXTERNAL(sim,local)[ch+2] =
                math_random(local_random) & 255;
        }
#else
        /** initially seed the brain with instructions which
        	are genetically biased */


        if (random_factor > -1)
        {
            local->seed[0] = (n_byte2)(random_factor & 0xffff);
            local->seed[1] = (n_byte2)(random_factor & 0xffff);
        }
        else
        {
            mother_genetics = being_genetics(mother);
            local->seed[0] = mother->seed[1];
            local->seed[1] = mother->seed[0];
            math_random(mother->seed);

            math_random3(local->seed);

            local->seed[1] = mother_genetics[0];

            math_random3(local->seed);

            local->seed[1] = sim->land->time;

            math_random3(local->seed);

        }

        math_random3(local->seed);

        being_init_braincode(sim,local,0L,local->seed,0,
                             BRAINCODE_INTERNAL);
        being_init_braincode(sim,local,0L,local->seed,0,
                             BRAINCODE_EXTERNAL);

#endif

        /** randomly initialize registers */
        for (ch = 0; ch < BRAINCODE_PSPACE_REGISTERS; ch++)
        {
            math_random3(local->seed);
            local->braincode_register[ch]=(n_byte)local->seed[0];
        }

        /** initialize brainprobes */
        for (ch = 0; ch < BRAINCODE_PROBES; ch++)
        {
            math_random3(local->seed);
            if ((n_byte)local->seed[0]&1)
            {
                local->brainprobe[ch].type = INPUT_SENSOR;
            }
            else
            {
                local->brainprobe[ch].type = OUTPUT_ACTUATOR;
            }
            local->brainprobe[ch].frequency =
                (n_byte)1 + (local->seed[1]%BRAINCODE_MAX_FREQUENCY);
            math_random3(local->seed);
            local->brainprobe[ch].address = (n_byte)local->seed[0];
            local->brainprobe[ch].position = (n_byte)local->seed[1];
            math_random3(local->seed);
            local->brainprobe[ch].offset = (n_byte)local->seed[0];
        }


#endif
#ifdef EPISODIC_ON
        for (ch=0; ch<EPISODIC_SIZE; ch++)
        {
            local_episodic[ch].affect=EPISODIC_AFFECT_ZERO;
        }

        /** has no social connections initially */
        for (ch=0; ch<SOCIAL_SIZE; ch++)
        {
            /** default type of entity */
            local_social_graph[ch].entity_type = ENTITY_BEING;
            /** friend_or_foe can be positive or negative,
            	with SOCIAL_RESPECT_NORMAL as the zero point */
            local_social_graph[ch].friend_foe = SOCIAL_RESPECT_NORMAL;
            /** clear names */
            local_social_graph[ch].first_name[BEING_MEETER]=0;
            local_social_graph[ch].first_name[BEING_MET]=0;
            local_social_graph[ch].family_name[BEING_MEETER]=0;
            local_social_graph[ch].family_name[BEING_MET]=0;
            if (ch > 0)
            {
                local_social_graph[ch].relationship=0;
            }
            else
            {
                local_social_graph[ch].relationship=RELATIONSHIP_SELF;
            }
        }
#endif

        being_facing_init(local);

        being_set_unique_name(sim,local,random_factor,0,0);
        if(random_factor > -1)
        {
            n_byte2  location[2];

            n_int loop = 0;

            math_random3(local->seed);

            do
            {
                location[0] = (n_byte2)(math_random(local->seed) & APESPACE_BOUNDS);
                location[1] = (n_byte2)(math_random(local->seed) & APESPACE_BOUNDS);
                loop ++;
            }
            while ((loop < 20) && (MAP_WATERTEST(land, APESPACE_TO_MAPSPACE(location[0]), APESPACE_TO_MAPSPACE(location[1]))));

            being_set_location(local, location);

            body_genome_random(sim, local, local->seed);

            local->social_x = local->social_nx =
                                  (math_random(local->seed) & 32767)+16384;
            local->social_y = local->social_ny =
                                  (math_random(local->seed) & 32767)+16384;
        }
        else
        {


            being_set_location(local, being_location(mother));

            /** this is the same as equals */
            being_wander(local, being_facing(mother) - being_facing(local));

            (void) math_random(local->seed);
            local->social_x = local->social_nx = mother->social_x;
            local->social_y = local->social_ny = mother->social_y;
            body_genetics(sim,local,mother,local->seed);

#ifdef PARASITES_ON
            /** ascribed social status */
            local->honor = (mother->honor + mother->father_honor) >> 2;
#endif

            genetics_set(local->mother_genetics, mother_genetics);
            genetics_set(local->father_genetics,
                         mother->father_genetics);

            being_set_unique_name(sim,local,random_factor,
                                  GET_NAME_FAMILY2(sim,mother),
                                  mother->father_name[1]);

            /** set the maternal generation number */
            if (mother->generation[GENERATION_MATERNAL] >
                    mother->generation[GENERATION_PATERNAL])
            {
                if (mother->generation[GENERATION_MATERNAL] <
                        MAX_GENERATION)
                {
                    local->generation[GENERATION_MATERNAL] =
                        mother->generation[GENERATION_MATERNAL]+1;
                }
            }
            else
            {
                if (mother->generation[GENERATION_PATERNAL] <
                        MAX_GENERATION)
                {
                    local->generation[GENERATION_MATERNAL] =
                        mother->generation[GENERATION_PATERNAL]+1;
                }
            }
            /** set the paternal generation number */
            if (mother->generation[GENERATION_FATHER] < MAX_GENERATION)
            {
                local->generation[GENERATION_PATERNAL] =
                    mother->generation[GENERATION_FATHER]+1;
            }
        }

        being_set_energy(local, BEING_FULL + 15);

        local->date_of_birth[0] = land->date[0];
        local->date_of_birth[1] = land->date[1];
        if (first_generation == 0)
        {
            GET_H(local) = BIRTH_HEIGHT;
            GET_M(local) = BIRTH_MASS;
        }
        else
        {
            /** produce an initial distribution of heights and masses*/
            math_random3(local->seed);
            GET_H(local) = BIRTH_HEIGHT +
                           (local->seed[0]%(BEING_MAX_HEIGHT-BIRTH_HEIGHT));
            GET_M(local) = BIRTH_MASS +
                           (local->seed[1]%(BEING_MAX_MASS_G-BIRTH_MASS));
        }
        local->crowding = MIN_CROWDING;

        if (GET_B(sim,local))
        {
            /** These magic numbers were found in March 2001 -
            	feel free to change them! */

#ifdef SOFT_BRAIN_ON
            GET_BS(local, 0) = 171;
            GET_BS(local, 1) = 0;
            GET_BS(local, 2) = 146;
            GET_BS(local, 3) = 86;
            GET_BS(local, 4) = 501;
            GET_BS(local, 5) = 73;
#else
            GET_BS(local, 0) = 0;
            GET_BS(local, 1) = 1024;
            GET_BS(local, 2) = 0;
            GET_BS(local, 3) = 0;
            GET_BS(local, 4) = 1024;
            GET_BS(local, 5) = 0;
#endif
        }
        if (sim->ext_birth != 0)
        {
            sim->ext_birth(local,mother,sim);
        }
#ifdef METABOLISM_ON
        metabolism_init(local);
#endif
    }
    sim->num++;

    return 0;
}



void being_tidy(noble_simulation * local_sim)
{
    noble_being *local  = local_sim->beings;
    n_land	    *land   = local_sim->land;
    n_uint       number = local_sim->num;
    n_uint	     loop   = 0;
    n_int	     fat_mass, insulation=0, bulk, conductance, delta_e;
#ifdef PARASITES_ON
    n_byte       max_honor = 1;
#endif
    while (loop < number)
    {
        noble_being *local_being = &local[loop];
        n_int	     local_e = being_energy(local_being);
        n_genetics  *genetics = being_genetics(local_being);
        delta_e = 0;
        conductance = 5;
#ifdef PARASITES_ON
        if (local_being->honor > max_honor)
        {
            max_honor = local_being->honor;
        }
#endif
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
                    delta_energy = ((delta_energy * delta_energy) >> 9);
                    /** the more body fat, the less energy is lost whilst swimming */
                    fat_mass = GET_BODY_FAT(local_being);
                    if (fat_mass > BEING_MAX_MASS_FAT_G)
                    {
                        fat_mass = BEING_MAX_MASS_FAT_G;
                    }
                    insulation = fat_mass * 5 / BEING_MAX_MASS_FAT_G;
                    delta_e += (delta_energy + 10 - insulation) >> 3;
                    conductance = 4;
#ifdef METABOLISM_ON
                    metabolism_vascular_response(local_sim, local_being, VASCULAR_SYMPATHETIC*(1+GENE_SWIM(genetics)));
#endif
                }
                else
                {
                    if (delta_z > 0)
                    {
                        /** going uphill */
                        delta_energy += GENE_HILL_CLIMB(genetics);
#ifdef METABOLISM_ON
                        metabolism_vascular_response(local_sim, local_being, VASCULAR_SYMPATHETIC*(8+(GENE_HILL_CLIMB(genetics)>>1)));
#endif
                    }
                    else
                    {
#ifdef METABOLISM_ON
                        metabolism_vascular_response(local_sim, local_being, VASCULAR_SYMPATHETIC*8);
#endif
                    }
                    delta_energy = ((delta_energy * delta_energy) >> 9);

                    /* the more massive the more energy consumed when moving */
                    bulk = GET_M(local_being)*5/BEING_MAX_MASS_G;
                    delta_e += (delta_energy + 4 + bulk) >> 2;
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

        local_e -= delta_e;

        INDICATOR_ADD(local_sim, IT_AVERAGE_ENERGY_OUTPUT, delta_e);

        if (land->time == 0)
        {
            n_int age_in_years = AGE_IN_YEARS(local_sim,local_being);
            /** this simulates natural death or at least some trauma the ape may or may not be able to recover from */
            if (age_in_years > 29)
            {
                if(math_random(local_being->seed) < (age_in_years - 29))
                {
                    local_e -= BEING_HUNGRY;
                }
            }
        }

        if (local_e < BEING_DEAD)
        {
            local_e = BEING_DEAD;
        }

        being_set_energy(local_being, local_e);
        loop++;
    }
#ifdef PARASITES_ON
    /** normalize honor values */
    if (max_honor>=254)
    {
        for(loop=0; loop < number ; loop++)
        {
            local[loop].honor = (n_byte)((n_int)local[loop].honor*220/255);
        }
    }
#endif
}

n_int being_remove_internal = 0;
n_int being_remove_external = 0;

void being_remove(noble_simulation * local_sim)
{
    noble_being * local = local_sim->beings;
    n_uint	reference = local_sim->select;
    n_uint  end_loop =  local_sim->num;
    n_uint  last_reference = reference;
    n_uint	count = 0;
    n_uint	possible = NO_BEINGS_FOUND;
    n_uint  loop=0;
    n_int   selected_died = 0;

    if (being_remove_external)
        do {}
        while(being_remove_external);

    being_remove_internal = 1;

    while (loop < end_loop)
    {
        if (being_energy(&(local[loop])) == BEING_DEAD)
        {
            noble_being * b = &local[loop];
            noble_being * child;
            n_genetics  * genetics = being_genetics(b);
            n_uint i = 0;
            n_byte2 name, family_name, met_name, met_family_name;

            if (local_sim->ext_death != 0L)
            {
                local_sim->ext_death(b,local_sim);
            }

            /** Did the being drown? */
            if (b->state & BEING_STATE_SWIMMING)
            {
                INDICATOR_INC(local_sim, IT_DROWNINGS);
            }

            /** remove all children's maternal links if the mother dies */
            if (GET_I(b) > 0)
            {
                do
                {
                    child = being_find_child(local_sim, genetics, 0);
                    if (child != 0L)
                    {
                        genetics_zero(child->mother_genetics);
                    }
                }
                while (child != 0L);
            }
            /** set familiarity to zero so that the entry for the removed being will eventually be overwritten */
            name = GET_NAME_GENDER(local_sim,b);
            family_name = GET_NAME_FAMILY2(local_sim,b);
            while (i < end_loop)
            {
                if (being_energy(&(local[i])) != BEING_DEAD)
                {
                    noble_being * b2 = &local[i];
                    social_link * b2_social_graph = GET_SOC(local_sim, b2);
                    if (b2_social_graph)
                    {
                        n_uint j = 1;
                        while (j < SOCIAL_SIZE)
                        {
                            met_name = b2_social_graph[j].first_name[BEING_MET];
                            if (met_name==name)
                            {
                                met_family_name = b2_social_graph[j].family_name[BEING_MET];
                                if (met_family_name==family_name)
                                {
                                    b2_social_graph[j].familiarity=0;
                                    break;
                                }
                            }
                            j++;
                        }
                    }
                }
                i++;
            }
        }
        loop++;
    }

    loop=0;
    while (loop < end_loop)
    {
        if ( loop == reference )
        {
            possible = count;
        }

        if (being_energy(&(local[loop])) != BEING_DEAD)
        {
            if ( count != loop )
            {
                /** the logic associated with copying th brsin memory location doesn't make sense */
                n_byte2        new_brain_memory_location = local[ count ].brain_memory_location;
                n_byte       * new_brain = GET_B(local_sim, &local[ count ]);
                n_byte       * old_brain = GET_B(local_sim, &local[ loop ]);
                social_link * new_event = GET_SOC(local_sim, &local[ count ]);
                social_link * old_event = GET_SOC(local_sim, &local[ loop ]);

                io_copy((n_byte *)&local[ loop ], (n_byte *)&local[ count ], sizeof(noble_being));

                if ((new_brain != 0L) && (old_brain != 0L))
                {
                    io_copy(old_brain, new_brain, DOUBLE_BRAIN);
                    io_erase(old_brain, DOUBLE_BRAIN);

                    io_copy((n_byte *)old_event, (n_byte *)new_event, (SOCIAL_SIZE * sizeof(social_link)));
                    io_erase((n_byte *)old_event, (SOCIAL_SIZE * sizeof(social_link)));

                    local[ count ].brain_memory_location = new_brain_memory_location;
                }
            }

            count++;
        }
        else
        {
            if (loop == last_reference)
            {
                selected_died = 1;
            }
        }
        loop++;
    }

    if (count == 0)
    {
        possible = NO_BEINGS_FOUND;
    }
    else
    {
        if (possible == NO_BEINGS_FOUND)
        {
            possible = 0;
        }
        else if (count < possible)
        {
            possible = 0;
        }
    }

    local_sim->num    = count;

    if (selected_died)
    {
        sim_set_select(possible);
    }

    if ((possible == NO_BEINGS_FOUND) && (last_reference != NO_BEINGS_FOUND))
    {
        (void)SHOW_ERROR("No Apes remain start new run");
    }

    being_remove_internal = 0;
}

static n_byte log_genealogy = 0;


void genealogy_log(noble_simulation * sim,n_byte value)
{
    log_genealogy = value;
    if (value == 0)
    {
        sim->ext_birth = 0L;
        sim->ext_death = 0L;
    }
    else
    {
        sim->ext_birth = &genealogy_birth;
        sim->ext_death = &genealogy_death;
    }
}

#ifdef GENEALOGY_ON

static n_byte empty_genetics(n_genetics * genetics)
{
    n_int i;
    for (i=0; i<CHROMOSOMES; i++)
    {
        if (genetics[i]!=0) return 0;
    }
    return 1;
}

static void genealogy_being_id(noble_being * local_being, n_file * fp, int parent)
{
    n_genetics * genome;
    n_int ch;

    switch(parent)
    {
    case 1:
        genome = local_being->mother_genetics;
        break;
    case 2:
        genome = local_being->father_genetics;
        break;
    default:
        genome = being_genetics(local_being);
    }

    for (ch = 0; ch < CHROMOSOMES; ch++)
    {
        io_writenumber(fp, genome[ch],1,0);
    }
}

static void genealogy_today(noble_simulation * sim, n_file * fp)
{
    n_int local_dob=0;

    n_int current_date = TIME_IN_DAYS(sim->land->date);

    n_int age_in_years = (current_date - local_dob) / TIME_YEAR_DAYS;
    n_int age_in_months = ((current_date - local_dob) - (age_in_years * TIME_YEAR_DAYS)) / (TIME_YEAR_DAYS/12);
    n_int age_in_days = (current_date - local_dob) - ((TIME_YEAR_DAYS/12) * age_in_months) - (age_in_years * TIME_YEAR_DAYS);

    if (age_in_months>11) age_in_months=11;

    io_writenumber(fp, (age_in_years+GENEALOGY_YEAR_ZERO), 1, 0);
    io_file_write(fp,'-');
    io_writenumber(fp, age_in_months, 1, 0);
    io_file_write(fp,'-');
    io_writenumber(fp, (age_in_days+1), 1, 0);
}

static void genealogy_birth_genxml(noble_being * child, noble_being * mother, noble_simulation * sim)
{
    n_file * fp;
    n_string_block str, first_name, surnames;
    n_int n;
    n_uint i;

    if (log_genealogy!=0)
    {
        if (!io_disk_check(GENEALOGY_XML_FILENAME))
        {
            fp = io_file_new();

            if (fp == 0L) return;

            io_file_xml_open(fp, "?xml version='1.0' encoding='ISO-8859-1'?");
            io_file_xml_open(fp, "genxml");

            io_file_xml_open(fp, "file");
            io_file_xml_int(fp, "version", 200);
            io_file_xml_int(fp, "level", 2);
            io_file_xml_close(fp, "file");

            io_file_xml_open(fp, "header");
            io_file_xml_string(fp, "exportingsystem", "Noble Ape");
            io_file_xml_string(fp, "version", "0.0");
            io_file_xml_close(fp, "header");

            io_write(fp,"<eventtype id=\"",0);
            io_write(fp,GENEALOGY_EVENT_BIRTH,0);
            io_write(fp,"\" class=\"birth\">",1);


            io_file_xml_string(fp, "description", "born");
            io_file_xml_string(fp, "gedcomtag", "BIRT");

            io_file_xml_int(fp, "roles", 1);
            io_file_xml_close(fp,  "eventtype");

            io_write(fp, "<eventtype id=\"",0);
            io_write(fp, GENEALOGY_EVENT_BIRTH,0);
            io_write(fp, "\" class=\"death\">",1);

            io_file_xml_string(fp, "description", "died");
            io_file_xml_string(fp, "gedcomtag", "DEAT");

            io_file_xml_int(fp, "roles", 1);
            io_file_xml_close(fp,  "eventtype");

            io_disk_write(fp,GENEALOGY_XML_FILENAME);

            io_file_free(fp);
            fp = 0L;
        }

        fp = io_file_new();

        if (fp != 0L)
        {

            io_write(fp, "<person id=\"", 0);
            genealogy_being_id(child,fp,0);
            io_write(fp, "\" sex=\"", 0);
            if (FIND_SEX(GET_I(child))!=SEX_FEMALE)
            {
                io_write(fp,"male",0);
            }
            else
            {
                io_write(fp,"female",0);
            }
            io_write(fp, "\">", 1);

            io_file_xml_open(fp, "personalname");


            being_name((FIND_SEX(GET_I(child)) == SEX_FEMALE), GET_NAME(sim,child), GET_FAMILY_FIRST_NAME(sim,child), GET_FAMILY_SECOND_NAME(sim,child), (n_string)str);

            for (i=0; i<strlen((char*)str); i++)
            {
                if (str[i]==' ') break;
                first_name[i]=str[i];
            }
            first_name[i]='\0';
            i++;
            n=0;
            while (i<strlen((char*)str))
            {
                surnames[n++]=str[i];
                i++;
            }
            surnames[n]='\0';

            io_write(fp,"<np tp=\"unkw\">",0);
            io_write(fp,(n_string)first_name,0);
            io_write(fp,"</np>",1);

            io_write(fp,"<np tp=\"surn\">",0);
            io_write(fp,(n_string)surnames,0);
            io_write(fp,"</np>",1);

            io_file_xml_close(fp, "personalname");
            io_file_xml_close(fp, "person");


            io_write(fp, "<assertion id=\"BIRTH",0);

            genealogy_being_id(child,fp,0);

            io_write(fp, "\">",1);


            io_write(fp, "<event type=\"",0);

            io_write(fp, GENEALOGY_EVENT_BIRTH,0);

            io_write(fp, "\">",1);


            io_write(fp, "<principal>",0);
            genealogy_being_id(child,fp,0);
            io_file_xml_close(fp, "principal");



            io_file_xml_open(fp, "date");
            io_write(fp, "<exact>",0);
            genealogy_today(sim,fp);
            io_file_xml_close(fp, "exact");
            io_file_xml_close(fp, "date");


            io_file_xml_open(fp, "place");
            io_write(fp, "<pnp>",0);


            io_writenumber(fp, being_location_x(child), 1, 0);
            io_file_write(fp, ' ');
            io_writenumber(fp, being_location_y(child), 1, 0);

            io_file_xml_close(fp, "pnp");
            io_file_xml_close(fp, "place");


            io_file_xml_close(fp, "event");
            io_file_xml_close(fp, "assertion");



            io_write(fp, "<assertion id=\"REL",0);
            genealogy_being_id(child,fp,0);
            io_write(fp, "\">",1);

            io_file_xml_open(fp, "relationship");

            io_file_xml_string(fp, "relation", "biological");

            io_write(fp, "<child>",0);
            genealogy_being_id(child,fp,0);
            io_file_xml_close(fp, "child");
            io_write(fp, "<mother>",0);
            genealogy_being_id(child,fp,1);
            io_file_xml_close(fp, "mother");
            io_write(fp, "<father>",0);
            genealogy_being_id(child,fp,2);


            io_file_xml_close(fp, "father");
            io_file_xml_close(fp, "relationship");
            io_file_xml_close(fp, "assertion");

            io_disk_append(fp,GENEALOGY_XML_FILENAME);

            io_file_free(fp);
        }
    }
}

static void genealogy_death_genxml(noble_being * local_being, noble_simulation * sim)
{
    n_file * fp;

    if (log_genealogy!=0)
    {
        fp = io_file_new();
        if (fp != 0L)
        {

            io_write(fp, "<assertion id=\"DEATH",0);
            genealogy_being_id(local_being,fp,0);
            io_write(fp, "\">",1);


            io_write(fp, "<event type=\"",0);

            io_write(fp, GENEALOGY_EVENT_DEATH,0);

            io_write(fp, "\">",1);

            io_write(fp, "<principal>",0);

            genealogy_being_id(local_being,fp,0);

            io_file_xml_close(fp, "principal");

            io_file_xml_open(fp, "date");
            io_write(fp, "<exact>",0);

            genealogy_today(sim,fp);

            io_file_xml_close(fp, "exact");
            io_file_xml_close(fp, "date");

            io_file_xml_close(fp, "event");
            io_file_xml_close(fp, "assertion");


            io_file_xml_open(fp, "place");
            io_write(fp, "<pnp>",0);

            io_writenumber(fp, being_location_x(local_being), 1, 0);
            io_file_write(fp, ' ');
            io_writenumber(fp, being_location_y(local_being), 1, 0);

            io_file_xml_close(fp, "pnp");
            io_file_xml_close(fp, "place");

            io_file_xml_close(fp, "event");
            io_file_xml_close(fp, "assertion");

            io_disk_append(fp,GENEALOGY_XML_FILENAME);
            io_file_free(fp);
        }
    }
}

static n_int genealogy_save_genxml(noble_simulation * sim, n_string filename)
{
    n_file * fp;

    if (log_genealogy!=0)
    {
        fp = io_file_new();
        if (io_disk_check(GENEALOGY_XML_FILENAME))
        {
            io_disk_read(fp, GENEALOGY_XML_FILENAME);
        }

        io_file_xml_close(fp, "genxml");
        io_disk_write(fp, filename);
        io_file_free(fp);
        return 1;
    }
    return 0;
}

/** GEDCOM 5.5 http://en.wikipedia.org/wiki/GEDCOM */

static void genealogy_today_gedcom(noble_simulation * sim, n_file * fp)
{
    n_string month[] =
    {
        "JAN","FEB","MAR",
        "APR","MAY","JUN",
        "JUL","AUG","SEP",
        "OCT","NOV","DEC"
    };
    n_int	current_date = TIME_IN_DAYS(sim->land->date);
    n_int days_month[12] = {31, 28, 31, 30,
                            31, 30, 31, 31,
                            30, 31, 30, 31
                           };

    n_int years = current_date / TIME_YEAR_DAYS;
    n_int days = current_date % TIME_YEAR_DAYS;
    n_int loop = 0;
    while (loop < 12)
    {
        n_int days_in_month = days_month[ loop ];
        if (days >= days_in_month)
        {
            days -= days_in_month;
        }
        else
        {
            io_writenumber(fp, (years+GENEALOGY_YEAR_ZERO), 1, 0);
            io_file_write(fp,' ');

            io_write(fp, month[loop], 0);

            io_file_write(fp,' ');
            io_writenumber(fp, (days+1), 1, 0);
            return;
        }
        loop++;
    }
}

static void genealogy_birth_gedcom(noble_being * child, noble_being * mother, noble_simulation * sim)
{
    n_string_block str,first_name,surnames;
    n_int n;
    n_uint i;
    n_file * fp;

    if (log_genealogy!=0)
    {
        if (!io_disk_check(GENEALOGY_GEDCOM_FILENAME))
        {
            fp = io_file_new();
            if (fp == 0L) return;

            io_write(fp, "0 HEAD", 1);
            io_write(fp, "1 SOUR Noble Ape", 1);
            io_write(fp, "2 VERS V0.0", 1);
            io_write(fp, "1 DEST Noble Ape", 1);
            io_write(fp, "1 FILE ", 0);
            io_write(fp, GENEALOGY_GEDCOM_FILENAME, 1);
            io_write(fp, "1 GEDC", 1);
            io_write(fp, "2 VERS 5.5", 1);
            io_write(fp, "1 CHAR ASCII", 1);

            io_disk_write(fp,GENEALOGY_GEDCOM_FILENAME);

            io_file_free(fp);
            fp = 0L;
        }

        fp = io_file_new();

        if (fp != 0L)
        {

            being_name((FIND_SEX(GET_I(child)) == SEX_FEMALE), GET_NAME(sim,child), GET_FAMILY_FIRST_NAME(sim,child), GET_FAMILY_SECOND_NAME(sim,child), (n_string)str);

            for (i=0; i<strlen((char*)str); i++)
            {
                if (str[i]==' ') break;
                first_name[i]=str[i];
            }
            first_name[i]='\0';
            i++;
            n=0;
            while (i<strlen((char*)str))
            {
                surnames[n++]=str[i];
                i++;
            }
            surnames[n]='\0';

            io_write(fp, "0 @I", 0);
            genealogy_being_id(child,fp,0);

            io_write(fp, "@ INDI", 1);


            io_write(fp, "1 NAME ", 0);
            io_write(fp, (n_string)first_name, 0);
            io_write(fp, " /", 0);
            io_write(fp, (n_string)surnames, 0);
            io_write(fp, "/", 1);

            io_write(fp, "1 SEX ", 0);

            if (FIND_SEX(GET_I(child))!=SEX_FEMALE)
            {
                io_write(fp, "M", 1);
            }
            else
            {
                io_write(fp, "F", 1);
            }
            io_write(fp, "1 BIRT", 1);
            io_write(fp, "2 DATE ", 0);
            genealogy_today_gedcom(sim,fp);
            io_write(fp, "", 1);
            /*
             io_write(fp, "2 PLAC ", 0);
             genealogy_place_name(child,fp);
             io_write(fp, "", 1);
             */

            if (!empty_genetics(child->father_genetics))
            {
                io_write(fp, "1 FAMC @F", 0);
                genealogy_being_id(child,fp,0);
                io_write(fp, "@", 1);

                io_write(fp, "0 @F", 0);
                genealogy_being_id(child,fp,0);
                io_write(fp, "@ FAM", 1);

                io_write(fp, "1 HUSB @I", 0);
                genealogy_being_id(child,fp,2);
                io_write(fp, "@", 1);
                io_write(fp, "1 WIFE @I", 0);
                genealogy_being_id(child,fp,1);
                io_write(fp, "@", 1);

                io_write(fp, "1 MARR", 1);
                io_write(fp, "2 DATE ", 0);
                genealogy_today_gedcom(sim,fp);
                io_write(fp, "", 1);

                io_write(fp, "1 CHIL @I", 0);
                genealogy_being_id(child,fp,0);
                io_write(fp, "@", 1);
            }

            io_disk_append(fp,GENEALOGY_GEDCOM_FILENAME);

            io_file_free(fp);
        }
    }
}

static void genealogy_death_gedcom(noble_being * local_being, noble_simulation * sim)
{
    if (log_genealogy!=0)
    {
        n_file * fp = io_file_new();

        if (fp == 0L) return;

        io_write(fp, "0 @I", 0);
        genealogy_being_id(local_being,fp,0);
        io_write(fp, "@ INDI", 1);

        io_write(fp, "1 DEAT", 1);

        io_write(fp, "2 DATE ", 0);
        genealogy_today_gedcom(sim,fp);
        io_write(fp, "", 1);
        /*
         io_write(fp, "2 PLAC ", 0);
         genealogy_place_name(local_being,fp);
         io_write(fp, "", 1);
         */
        io_disk_append(fp,GENEALOGY_GEDCOM_FILENAME);

        io_file_free(fp);
    }
}

static n_int genealogy_save_gedcom(noble_simulation * sim, n_string filename)
{
    n_file * fp;

    if (log_genealogy!=0)
    {
        fp = io_file_new();
        if (io_disk_check(GENEALOGY_GEDCOM_FILENAME))
        {
            io_disk_read(fp, GENEALOGY_GEDCOM_FILENAME);
        }
        io_write(fp, "0 TRLR",1);
        io_disk_write(fp, filename);
        io_file_free(fp);
        return 1;
    }
    return 0;
}

void genealogy_birth(noble_being * child, noble_being * mother, void * sim)
{
    switch(log_genealogy)
    {
    case GENEALOGY_GENXML:
        genealogy_birth_genxml(child, mother, (noble_simulation *)sim);
        break;
    case GENEALOGY_GEDCOM:
        genealogy_birth_gedcom(child, mother, (noble_simulation *)sim);
        break;
    }
}

void genealogy_death(noble_being * local_being, void * sim)
{
    switch(log_genealogy)
    {
    case GENEALOGY_GENXML:
        genealogy_death_genxml(local_being, (noble_simulation *)sim);
        break;
    case GENEALOGY_GEDCOM:
        genealogy_death_gedcom(local_being, (noble_simulation *)sim);
        break;
    }
}

n_int genealogy_save(noble_simulation * sim, n_string filename)
{
    n_int retval=0;

    switch(log_genealogy)
    {
    case GENEALOGY_GENXML:
        retval = genealogy_save_genxml(sim, filename);
        break;
    case GENEALOGY_GEDCOM:
        retval = genealogy_save_gedcom(sim, filename);
        break;
    }
    return retval;
}

#endif


