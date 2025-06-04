/****************************************************************

 food.c

 =============================================================

 Copyright 1996-2025 Tom Barbalet. All rights reserved.

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

 This software is a continuing work of Tom Barbalet, begun on
 13 June 1996. No apes or cats were harmed in the writing of
 this software.

 ****************************************************************/

/*! \file   food.c
 *  \brief  Functions related to eating
 */

#include "entity.h"
#include "entity_internal.h"

/** Various energy use and addition through eating characteristics */
/** Energy from vegetables */

#define GENE_ENERGY_FROM_VEGETABLES(gene)   GENE_VAL_REG(gene, 3, 13, 15, 3)

/** Energy from fruits */

#define GENE_ENERGY_FROM_FRUITS(gene)       GENE_VAL_REG(gene, 14, 7, 6, 4)

/** Energy from shellfish */

#define GENE_ENERGY_FROM_SHELLFISH(gene)    GENE_VAL_REG(gene, 10, 12, 12, 2)

/** Energy from seaweed */

#define GENE_ENERGY_FROM_SEAWEED(gene)      GENE_VAL_REG(gene, 0, 9, 11, 12)

/** Energy from bird eggs */

#define GENE_ENERGY_FROM_BIRD_EGGS(gene)    GENE_VAL_REG(gene, 7, 1, 9, 5)

/** Energy from lizard eggs */

#define GENE_ENERGY_FROM_LIZARD_EGGS(gene)  GENE_VAL_REG(gene, 15, 3, 12, 8)


/**
 * @brief How much energy is absorbed from a given type of food
 * @param food_type The type of food
 * @param local pointer to the ape
 * @return Energy absorbed
 */
n_int food_absorption( simulated_being *local, n_int max_energy, n_byte food_type )
{
    n_genetics *genetics = being_genetics( local );

    n_int   vegetable = GENE_ENERGY_FROM_VEGETABLES( genetics );
    n_int   fruit = GENE_ENERGY_FROM_FRUITS( genetics );
    n_int   shellfish = GENE_ENERGY_FROM_SHELLFISH( genetics );
    n_int   seawood = GENE_ENERGY_FROM_SEAWEED( genetics );

    n_int   bird_eggs = GENE_ENERGY_FROM_BIRD_EGGS( genetics );
    n_int   lizard_eggs = GENE_ENERGY_FROM_LIZARD_EGGS( genetics );

    n_int        return_value = 0;
    /** note that the absorbition for different foods is normalised */
    n_int absorb_denom = 1 + vegetable + fruit + seawood + bird_eggs + lizard_eggs;

    /** ingest pathogens from certain foods */
    immune_ingest_pathogen( &local->immune_system, food_type );

    switch ( food_type )
    {
    case FOOD_VEGETABLE:
#ifdef DEBUG_LACK_OF_MOVEMENT
    {
        n_string_block information_string;
        sprintf( information_string, "vegetable %ld absorbtion %ld food", vegetable, absorb_denom );
        being_register_movement( local, information_string );
    }
#endif
    return_value = ( vegetable << 4 ) / absorb_denom;
    break;
    case FOOD_FRUIT:
#ifdef DEBUG_LACK_OF_MOVEMENT
        being_register_movement( local, "fruit food" );
#endif
        return_value = ( fruit << 4 ) / absorb_denom;
        break;
    case FOOD_SHELLFISH:
#ifdef DEBUG_LACK_OF_MOVEMENT
        being_register_movement( local, "shellfish food" );
#endif
        return_value = ( shellfish << 4 ) / absorb_denom;
        break;
    case FOOD_SEAWEED:
#ifdef DEBUG_LACK_OF_MOVEMENT
        being_register_movement( local, "seaweed food" );
#endif
        return_value = ( seawood << 4 ) / absorb_denom;
        break;
    case FOOD_BIRD_EGGS:
#ifdef DEBUG_LACK_OF_MOVEMENT
        being_register_movement( local, "bird egg food" );
#endif
        return_value = ( seawood << 4 ) / absorb_denom;
        break;
    case FOOD_LIZARD_EGGS:
#ifdef DEBUG_LACK_OF_MOVEMENT
        being_register_movement( local, "lizard egg food" );
#endif
        return_value = ( seawood << 4 ) / absorb_denom;
        break;
    default:
#ifdef DEBUG_LACK_OF_MOVEMENT
        being_register_movement( local, "no food" );
#endif
        return 0;
    }

    return_value = ( max_energy * ( 1 + return_value ) >> 3 );

    if ( return_value > 320 )
    {
        return_value = 320; /**< can only eat so much in one go */
    }
    return return_value;
}

/**
 * @brief Returns the amount of food of the given type at the given location
 * @param loc_x x coordinate
 * @param loc_y y coordinate
 * @param kind The type of food
 * @return The amount of food of the given type at this location
 */
static n_int food_location( n_int loc_x,
                            n_int loc_y,
                            n_int kind )
{
    return land_operator_interpolated( loc_x, loc_y,
                                       ( n_byte * )&operators[kind - VARIABLE_BIOLOGY_AREA] );
}

/**
 * @brief Returns the values for grass, trees and bushes at the given location
 * @param loc_x X ape coordinate on the map
 * @param loc_y Y ape coordinate on the lap
 * @param grass Returned value for grass
 * @param trees Returned value for trees
 * @param bush Returned value for bushes
 */
void food_values( n_int loc_x,
                  n_int loc_y,
                  n_int *grass, n_int *trees, n_int *bush )
{
    /* TODO include bird and lizard eggs */

    /** grass at this location */
    *grass =
        food_location( loc_x, loc_y, VARIABLE_BIOLOGY_GRASS ) + OFFSET_GRASS;

    /** trees at this location */
    *trees =
        food_location( loc_x, loc_y, VARIABLE_BIOLOGY_TREE );

    /** bushes at this location */
    *bush =
        food_location( loc_x, loc_y, VARIABLE_BIOLOGY_BUSH ) + OFFSET_BUSH;

    *grass += LAND_DITHER( *grass, *trees, *bush );
}

/**
 * @brief Returns the dominant food type on land and its associated maximum energy value
 * @param loc_x X ape coordinate
 * @param loc_y Y ape coordinate
 * @param energy Returned maximum energy value for the food type
 * @return The type of food
 */
static n_byte food_eat_land(
    n_int loc_x,
    n_int loc_y,
    n_int *energy )
{
    n_byte food_type = FOOD_VEGETABLE;
    n_int grass, trees, bush;

    /* TODO Handle this logic centrally - including the int values in the function not outside */

    food_values( loc_x, loc_y, &grass, &trees, &bush );

    /** which is the dominant form of vegetation in this area? */
    if ( ( grass > bush ) && ( grass > trees ) )
    {
        *energy = ENERGY_GRASS;
    }
    else
    {
        if ( bush > trees )
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
 * @brief Returns the dominant food type in the intertidal zone and its associated maximum energy value
 * @param loc_x X ape coordinate
 * @param loc_y Y ape coordinate
 * @param energy Returned maximum energy value for the food type
 * @return The type of food
 */
static n_byte food_intertidal(
    n_int loc_x,
    n_int loc_y,
    n_int *energy )
{
    n_byte food_type = FOOD_VEGETABLE;
    n_int seaweed, rockpool, beach;

    /** seaweed at this location */
    seaweed =
        food_location( loc_x, loc_y, VARIABLE_BIOLOGY_SEAWEED );

    /** rockpools at this location */
    rockpool =
        food_location( loc_x, loc_y, VARIABLE_BIOLOGY_ROCKPOOL );

    /** beach at this location */
    beach =
        food_location( loc_x, loc_y, VARIABLE_BIOLOGY_BEACH );

    beach += LAND_DITHER( seaweed, rockpool, beach );

    /** which is the dominant form of food in this area? */
    if ( ( seaweed > rockpool ) && ( seaweed > beach ) )
    {
        *energy = ENERGY_SEAWEED;
        food_type = FOOD_SEAWEED;
    }
    else
    {
        if ( rockpool > beach )
        {
            *energy = ENERGY_SHELLFISH;
            food_type = FOOD_SHELLFISH;
        }
    }
    return food_type;
}

/**
 * @brief Eat food at the given location and return the energy increase
 * @param loc_x X ape coordinate
 * @param loc_y Y ape coordinate
 * @param az Z ape coordinate
 * @param local_being Pointer to the ape
 * @return Energy obtained from the food
 */
n_int food_eat(
    n_int loc_x,
    n_int loc_y,
    n_int az,
    n_byte *food_type,
    simulated_being *local_being )
{
    n_int max_energy = BEING_DEAD;

    *food_type = FOOD_VEGETABLE;

    if ( az > TIDE_MAX )
    {
        /** above the high water mark */
        *food_type = food_eat_land( loc_x, loc_y, &max_energy );
    }
    else
    {
        /** in the intertidal zone */
        *food_type = food_intertidal( loc_x, loc_y, &max_energy );
    }

    return food_absorption( local_being, max_energy, *food_type );
}
