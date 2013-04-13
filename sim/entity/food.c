/****************************************************************

 food.c

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

/*! \file   food.c
 *  \brief  Functions related to eating
 */


/*NOBLEMAKE DEL=""*/

#ifndef	_WIN32
#include "../noble/noble.h"
#include "../universe/universe.h"
#else
#include "..\noble\noble.h"
#include "..\universe\universe.h"
#endif

#include "entity_internal.h"
#include "entity.h"

/*NOBLEMAKE END=""*/

/* Various energy use and addition through eating characteristics */
/* Energy from vegetables */

#define GENE_ENERGY_FROM_VEGETABLES(gene)   GENE_VAL_REG(gene, 3, 13, 15, 3)

/* Energy from fruits */

#define GENE_ENERGY_FROM_FRUITS(gene)       GENE_VAL_REG(gene, 14, 7, 6, 4)

/* Energy from shellfish */

#define GENE_ENERGY_FROM_SHELLFISH(gene)    GENE_VAL_REG(gene, 10, 12, 12, 2)

/* Energy from seaweed */

#define GENE_ENERGY_FROM_SEAWEED(gene)      GENE_VAL_REG(gene, 0, 9, 11, 12)

/**
 * How much energy is absorbed from a given type of food
 * @param food_type The type of food
 * @param local pointer to the ape
 */
n_int food_absorption(
    n_byte food_type,
    noble_being * local)
{
    n_genetics * genetics = GET_G(local);

    /** note that the absorbition for different foods is normalised */
    n_int absorb_denom =
        1 + GENE_ENERGY_FROM_VEGETABLES(genetics) +
        GENE_ENERGY_FROM_FRUITS(genetics) +
        GENE_ENERGY_FROM_SHELLFISH(genetics);

    switch (food_type)
    {
    case FOOD_VEGETABLE:
        return (GENE_ENERGY_FROM_VEGETABLES(genetics) << 4) / absorb_denom;
    case FOOD_FRUIT:
        return (GENE_ENERGY_FROM_FRUITS(genetics) << 4) / absorb_denom;
    case FOOD_SHELLFISH:
        return (GENE_ENERGY_FROM_SHELLFISH(genetics) << 4) / absorb_denom;
    case FOOD_SEAWEED:
        return (GENE_ENERGY_FROM_SEAWEED(genetics) << 4) / absorb_denom;
    }
    return 0L;
}


static n_int food_location(n_land * local_land, n_weather * local_weather,
                           n_int loc_x,
                           n_int loc_y,
                           n_int kind)
{
    return land_operator_interpolated(local_land, local_weather, loc_x, loc_y,
                                      (n_byte*)&operators[kind - VARIABLE_BIOLOGY_AREA]);
}

/**
 * Returns the values for grass, trees and bushes at the given location
 * @param local_land Pointer to the land
 * @param local_weather Pointer to the weather
 * @param loc_x X ape coordinate on the map
 * @param loc_y Y ape coordinate on the lap
 * @param grass Returned value for grass
 * @param trees Returned value for trees
 * @param bush Returned value for bushes
 */
void food_values(n_land * local_land, n_weather * local_weather,
                 n_int loc_x,
                 n_int loc_y,
                 n_int *grass, n_int *trees, n_int *bush)
{
    /** grass at this location */
    *grass =
        food_location(local_land, local_weather, loc_x, loc_y, VARIABLE_BIOLOGY_GRASS)+OFFSET_GRASS;

    /** trees at this location */
    *trees =
        food_location(local_land, local_weather, loc_x, loc_y, VARIABLE_BIOLOGY_TREE);

    /** bushes at this location */
    *bush =
        food_location(local_land, local_weather, loc_x, loc_y, VARIABLE_BIOLOGY_BUSH)+OFFSET_BUSH;

    *grass += LAND_DITHER(*grass, *trees, *bush);
}


/**
 * Returns the dominant food type on land and its associated maximum energy value
 * @param sim Pointer to the simulation
 * @param loc_x X ape coordinate
 * @param loc_y Y ape coordinate
 * @param energy Returned maximum energy value for the food type
 * @return The type of food
 */
static n_byte food_eat_land(
    n_land * local_land,
    n_weather * local_weather,
    n_int loc_x,
    n_int loc_y,
    n_int * energy)
{
    n_byte food_type = FOOD_VEGETABLE;
    n_int grass, trees, bush;

    food_values(local_land, local_weather,loc_x,loc_y,&grass, &trees, &bush);

    /** which is the dominant form of vegetation in this area? */
    if ((grass > bush) && (grass > trees))
    {
        *energy = ENERGY_GRASS;
    }
    else
    {
        if (bush > trees)
        {
            *energy = ENERGY_BUSH;
        }
        else
        {
            *energy = ENERGY_FRUIT;
            food_type = FOOD_FRUIT;
        }
    }
    return food_type;
}

/**
 * Returns the dominant food type in the intertidal zone and its associated maximum energy value
 * @param sim Pointer to the simulation
 * @param loc_x X ape coordinate
 * @param loc_y Y ape coordinate
 * @param energy Returned maximum energy value for the food type
 * @return The type of food
 */
static n_byte food_intertidal(
    n_land * local_land,
    n_weather * local_weather,
    n_int loc_x,
    n_int loc_y,
    n_int * energy)
{
    n_byte food_type = FOOD_VEGETABLE;
    n_int seaweed, rockpool, beach;

    /** seaweed at this location */
    seaweed =
        food_location(
            local_land, local_weather, loc_x, loc_y, VARIABLE_BIOLOGY_SEAWEED);

    /** rockpools at this location */
    rockpool =
        food_location(
            local_land, local_weather, loc_x, loc_y, VARIABLE_BIOLOGY_ROCKPOOL);

    /** beach at this location */
    beach =
        food_location(
            local_land, local_weather, loc_x, loc_y, VARIABLE_BIOLOGY_BEACH);

    beach += LAND_DITHER(seaweed, rockpool, beach);

    /** which is the dominant form of food in this area? */
    if ((seaweed > rockpool) && (seaweed > beach))
    {
        *energy = ENERGY_SEAWEED;
        food_type = FOOD_SEAWEED;
    }
    else
    {
        if (rockpool > beach)
        {
            *energy = ENERGY_SHELLFISH;
            food_type = FOOD_SHELLFISH;
        }
    }
    return food_type;
}

/**
 * Eat food at the given location and return the energy increase
 * @param sim Pointer to the simulation
 * @param loc_x X ape coordinate
 * @param loc_y Y ape coordinate
 * @param az Z ape coordinate
 * @param local_being Pointer to the ape
 * @return Energy obtained from the food
 */
n_int food_eat(
    n_land * local_land,
    n_weather * local_weather,
    n_int loc_x,
    n_int loc_y,
    n_int az,
    n_byte * food_type,
    noble_being * local_being)
{
    n_int energy = BEING_DEAD, max_energy = BEING_DEAD;
    *food_type = FOOD_VEGETABLE;
    if (az > TIDE_MAX)
    {
        /** above the high water mark */
        *food_type = food_eat_land(local_land, local_weather,loc_x,loc_y,&max_energy);
    }
    else
    {
        /** in the intertidal zone */
        *food_type = food_intertidal(local_land, local_weather,loc_x,loc_y,&max_energy);
    }

    /** update metabolism */
    metabolism_eat(local_being,*food_type);

    /** ingest pathogens from certain foods */
    being_ingest_pathogen(local_being, *food_type);

    energy = CONSUME_E(local_being,max_energy,*food_type);

    if (energy > 320) energy = 320; /**< can only eat so much in one go */

    return energy;
}
