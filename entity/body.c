/****************************************************************

 body.c

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

#include "entity.h"
#include "entity_internal.h"

/**
 * @brief One being gives something to another
 * @param local The being doing the giving
 * @param other The being receiving
 * @param carrying objects being carried
 */
static void body_action_give( simulated_being *local, simulated_being *other, n_byte2 carrying )
{
    n_byte hand = BODY_RIGHT_HAND;

    if ( carrying == 0 )
    {
        hand = BODY_LEFT_HAND;
        carrying = being_carried( local, hand );
    }
    if ( ( carrying != 0 ) &&
            ( ( being_carried( other, BODY_LEFT_HAND ) == 0 ) ||
              ( being_carried( other, BODY_RIGHT_HAND ) == 0 ) ) )
    {
        being_set_attention( local, ATTENTION_BODY, BODY_RIGHT_HAND );
        being_set_attention( other, ATTENTION_BODY, BODY_RIGHT_HAND );

        episodic_interaction( local, other, EVENT_GIVEN, EPISODIC_AFFECT_ZERO, carrying );
        episodic_interaction( other, local, EVENT_GIVEN_BY, AFFECT_RECEIVE, carrying );

        being_drop( local, hand );
        if ( being_carried( other, BODY_RIGHT_HAND ) == 0 )
        {
            being_take( other, BODY_RIGHT_HAND, carrying );
        }
        else
        {
            being_take( other, BODY_LEFT_HAND, carrying );
        }
    }
}

/**
 * @brief One being bashes another
 * @param local The being doing the bashing
 * @param other The being being bashed
 * @param carrying Objects being carried
 */
static void body_action_bash( simulated_being *local, simulated_being *other, n_byte2 carrying )
{
    n_byte hand = BODY_RIGHT_HAND;
    n_int  index, hit = 0;
    simulated_isocial *graph;

    if ( carrying == 0 )
    {
        hand = BODY_LEFT_HAND;
        carrying = being_carried( local, hand );
    }
    if ( carrying != 0 )
    {
        if ( ( carrying & INVENTORY_BRANCH ) || ( carrying & INVENTORY_ROCK ) )
        {
            being_set_attention( local, ATTENTION_BODY, BODY_RIGHT_HAND );
            being_set_attention( other, ATTENTION_BODY, BODY_BACK );
            index = get_simulated_isocial( other, local );
            if ( index > -1 )
            {
                graph = being_social( other );
                if ( !graph )
                {
                    return;
                }
                if ( graph[index].friend_foe > 1 )
                {
                    graph[index].friend_foe -= 2;
                }
            }
            if ( ( carrying & INVENTORY_ROCK ) && ( being_random( other ) > THROW_ACCURACY ) )
            {
                hit = 1;
                being_energy_delta( other, 0 - SQUABBLE_ENERGY_ROCK_HURL );
            }
            if ( ( carrying & INVENTORY_BRANCH ) && ( being_random( other ) > WHACK_ACCURACY ) )
            {
                hit = 1;
                being_energy_delta( other, 0 - SQUABBLE_ENERGY_BRANCH_WHACK );
            }
        }
        if ( carrying & INVENTORY_BRANCH )
        {
            if ( hit != 0 )
            {
                episodic_interaction( local, other, EVENT_WHACKED, EPISODIC_AFFECT_ZERO, 0 );
                episodic_interaction( other, local, EVENT_WHACKED_BY, AFFECT_WHACKED, 0 );
            }
        }
        if ( carrying & INVENTORY_ROCK )
        {
            episodic_interaction( local, other, EVENT_HURLED, EPISODIC_AFFECT_ZERO, 0 );
            if ( hit != 0 )
            {
                episodic_interaction( other, local, EVENT_HURLED_BY, AFFECT_HURL, 0 );
            }
        }
    }

}

/**
 * @brief Remember interaction between two beings
 * @param local Pointer to the first being
 * @param other Pointer to the second being
 * @param local_attention Focus of attention for the first being
 * @param other_attention Focus of attention for the second being
 * @param kind The type of event
 */
static void body_action_interactive( simulated_being *local, simulated_being *other,
                                     n_byte local_attention, n_byte other_attention, n_byte kind )
{
    being_set_attention( local, ATTENTION_BODY, local_attention );
    being_set_attention( other, ATTENTION_BODY, other_attention );
    episodic_interaction( local, other, kind, EPISODIC_AFFECT_ZERO, 0 );
    episodic_interaction( other, local, kind + 1, EPISODIC_AFFECT_ZERO, 0 );
}

/*** This block should also be the same function ***/

static void body_action_interactive_change( simulated_being *local, simulated_being *other,
        n_byte local_attention, n_byte other_attention, n_byte kind, n_byte positive, AFFECT_TYPE affect )
{
    n_int index;
    being_set_attention( local, ATTENTION_BODY, local_attention );
    being_set_attention( other, ATTENTION_BODY, other_attention );
    index = get_simulated_isocial( other, local );
    if ( index > -1 )
    {
        simulated_isocial *graph = being_social( other );
        if ( !graph )
        {
            return;
        }
        if ( positive )
        {
            if ( graph[index].friend_foe < 255 )
            {
                graph[index].friend_foe++;
            }
        }
        else
        {
            if ( graph[index].friend_foe > 0 )
            {
                graph[index].friend_foe--;
            }
        }
    }

    episodic_interaction( local, other, kind, EPISODIC_AFFECT_ZERO, 0 );
    episodic_interaction( other, local, kind + 1, affect, 0 );
}

/**
 * @brief Carry an object in a hand
 * @param local Pointer to the being
 * @param carrying Objects which are carried
 * @param hand left or right hand
 * @param kind The kind of object
 */
static void body_action_hand_object( simulated_being *local, n_byte2 carrying, n_byte hand, n_byte kind )
{
    if ( carrying == 0 )
    {
        hand = BODY_LEFT_HAND;
        carrying = being_carried( local, hand );
    }
#ifdef EPISODIC_ON
    if ( carrying != 0 )
    {
        if ( carrying & INVENTORY_BRANCH )
        {
            episodic_self( local, kind, EPISODIC_AFFECT_ZERO, INVENTORY_BRANCH );
        }
        else
        {
            if ( carrying & INVENTORY_TWIG )
            {
                episodic_self( local, kind, EPISODIC_AFFECT_ZERO, INVENTORY_TWIG );
            }
            else
            {
                if ( carrying & INVENTORY_SPEAR )
                {
                    episodic_self( local, kind, EPISODIC_AFFECT_ZERO, INVENTORY_SPEAR );
                }
            }
        }
    }
#endif
}

/**
 * @brief Performs a jabbing action
 * @param local Pointer to the being
 * @param carrying Objects which are carried
 * @param hand Left or right hand
 */
static void body_action_jab( simulated_being *local, n_byte2 carrying, n_byte hand )
{
    inventory_type carrying2 = being_carried( local, BODY_LEFT_HAND );
    if ( ( carrying & INVENTORY_SPEAR ) ||
            ( carrying2 & INVENTORY_SPEAR ) )
    {
        n_int az;
        n_vect2 location_vector, facing_vector, slope_vector;

        being_space( local, &location_vector );

        being_facing_vector( local, &facing_vector, 4 );
        land_vect2( &slope_vector, &az, &location_vector );

        if ( ( az > WATER_MAP ) && ( az < TIDE_MAX ) )
        {
            /* some probability of spearing a fish */
            if ( being_random( local ) < FISHING_PROB )
            {
                /* carry fish */
                if ( carrying & INVENTORY_SPEAR )
                {
                    being_take( local, BODY_LEFT_HAND, INVENTORY_FISH );
                }
                else
                {
                    being_take( local, hand, INVENTORY_FISH );
                }
#ifdef EPISODIC_ON
                episodic_self( local, EVENT_FISH, AFFECT_FISH, 0 );
#endif
            }
        }
    }
}

/**
 * @brief Bashes one object on another.  Both objects must be carried.
 * @param local Pointer to the being
 * @param carrying Things which are carried
 * @param hand left or right hand
 */
static void body_action_bash_objects( simulated_being *local, n_byte2 carrying, n_byte hand )
{
    inventory_type carrying2 = being_carried( local, BODY_LEFT_HAND );
    if ( ( carrying & INVENTORY_ROCK ) && ( carrying2 & INVENTORY_ROCK ) )
    {
        /** bash two rocks to make a scraper */
        being_drop( local, hand );
        being_take( local, hand, INVENTORY_SCRAPER );
    }
    if ( ( ( carrying & INVENTORY_ROCK ) && ( carrying2 & INVENTORY_NUT ) ) ||
            ( ( carrying & INVENTORY_NUT ) && ( carrying2 & INVENTORY_ROCK ) ) )
    {
        /** bash nut with a rock */
        if ( carrying & INVENTORY_NUT )
        {
            being_drop( local, hand );
            being_take( local, hand, INVENTORY_NUT_CRACKED );
        }
        else
        {
            being_drop( local, BODY_LEFT_HAND );
            being_take( local, BODY_LEFT_HAND, INVENTORY_NUT_CRACKED );
        }
    }
    if ( ( ( carrying & INVENTORY_BRANCH ) && ( carrying2 & INVENTORY_SCRAPER ) ) ||
            ( ( carrying & INVENTORY_SCRAPER ) && ( carrying2 & INVENTORY_BRANCH ) ) )
    {
        /** use a scraper to make a spear */
        if ( carrying & INVENTORY_BRANCH )
        {
            being_drop( local, hand );
            being_take( local, hand, INVENTORY_SPEAR );
        }
        else
        {
            being_drop( local, BODY_LEFT_HAND );
            being_take( local, BODY_LEFT_HAND, INVENTORY_SPEAR );
        }
    }
    if ( ( ( carrying & INVENTORY_BRANCH ) && ( carrying2 & INVENTORY_NUT ) ) ||
            ( ( carrying & INVENTORY_NUT ) && ( carrying2 & INVENTORY_BRANCH ) ) )
    {
        /** whack nut with a branch */
        if ( carrying & INVENTORY_NUT )
        {
            being_drop( local, hand );
            being_take( local, hand, INVENTORY_NUT_CRACKED );
        }
        else
        {
            being_drop( local, BODY_LEFT_HAND );
            being_take( local, BODY_LEFT_HAND, INVENTORY_NUT_CRACKED );
        }
    }
}

/**
 * @brief A being chews on something
 * @param local Pointer to the being
 * @param carrying Things which are carried
 * @param hand left or right hand
 */

static void body_action_chew( simulated_being *local, n_byte2 carrying, n_byte hand )
{
    if ( !( ( carrying & INVENTORY_GRASS ) ||
            ( carrying & INVENTORY_TWIG ) ||
            ( carrying & INVENTORY_FISH ) ||
            ( carrying & INVENTORY_BIRD_EGGS ) ||
            ( carrying & INVENTORY_LIZARD_EGGS ) ||
            ( carrying & INVENTORY_NUT_CRACKED ) ) )
    {
        hand = BODY_LEFT_HAND;
        carrying = being_carried( local, hand );
    }
    if ( ( carrying & INVENTORY_GRASS ) ||
            ( carrying & INVENTORY_TWIG ) ||
            ( carrying & INVENTORY_FISH ) ||
            ( carrying & INVENTORY_BIRD_EGGS ) ||
            ( carrying & INVENTORY_LIZARD_EGGS ) ||
            ( carrying & INVENTORY_NUT_CRACKED ) )
    {
        if ( hand == BODY_RIGHT_HAND )
        {
            carrying |= 1;
        }
#ifdef EPISODIC_ON
        episodic_self( local, EVENT_CHEW, EPISODIC_AFFECT_ZERO, carrying );
#endif
    }
    if ( carrying & INVENTORY_GRASS )
    {
        /** consume grass */
        being_energy_delta( local, food_absorption( local, ENERGY_GRASS, FOOD_VEGETABLE ) );
        being_drop( local, hand );
    }
    else
    {
        if ( carrying & INVENTORY_FISH )
        {
            /** consume fish */
            being_energy_delta( local, food_absorption( local, ENERGY_FISH, FOOD_SHELLFISH ) );
            being_drop( local, hand );
        }
        else
        {
            if ( carrying & INVENTORY_NUT_CRACKED )
            {
                /** consume nut */
                being_energy_delta( local, food_absorption( local, ENERGY_NUT, FOOD_VEGETABLE ) );
                being_drop( local, hand );
            }
            else
            {
                if ( carrying & INVENTORY_BIRD_EGGS )
                {
                    /** consume nut */
                    being_energy_delta( local, food_absorption( local, ENERGY_BIRD_EGGS, FOOD_BIRD_EGGS ) );
                    being_drop( local, hand );
                }
                else
                {
                    if ( carrying & INVENTORY_LIZARD_EGGS )
                    {
                        /** consume nut */
                        being_energy_delta( local, food_absorption( local, ENERGY_LIZARD_EGGS, FOOD_LIZARD_EGGS ) );
                        being_drop( local, hand );
                    }
                }
            }
        }
    }
}
/**
 * @brief If anything is being carried in a hand this swaps it between hands
 * @param local Pointer to the being
 * @param carrying Things which are carried
 * @param hand left or right hand
 */
static void body_action_swap_hands( simulated_being *local, n_byte2 carrying, n_byte hand )
{
    if ( ( carrying != 0 ) && ( being_carried( local, BODY_LEFT_HAND ) == 0 ) )
    {
        being_drop( local, hand );
        being_take( local, BODY_LEFT_HAND, carrying );
    }
    else
    {
        if ( ( carrying == 0 ) && ( being_carried( local, BODY_LEFT_HAND ) != 0 ) )
        {
            carrying = being_carried( local, BODY_LEFT_HAND );
            being_drop( local, BODY_LEFT_HAND );
            being_take( local, hand, carrying );
        }
    }
}

/**
 * @brief A being drops something
 * @param local Pointer to the being
 * @param carrying Things which are carried
 * @param hand left or right hand
 */
static void body_action_drop( simulated_being *local, n_byte2 carrying, n_byte hand )
{
    if ( carrying == 0 )
    {
        hand = BODY_LEFT_HAND;
        carrying = being_carried( local, hand );
    }
    if ( carrying != 0 )
    {
        being_drop( local, hand );
#ifdef EPISODIC_ON
        episodic_self( local, EVENT_DROP, EPISODIC_AFFECT_ZERO, carrying );
#endif
    }
}

/**
 * @brief A being picks something up
 * @param local Pointer to the being
 * @param carrying Things being carried by the being
 * @param hand left or right hand
 */
static void body_action_pickup( simulated_being *local, n_byte2 carrying, n_byte hand )
{
    if ( ( carrying != 0 ) && ( !being_posture_under( local, POSTURE_CROUCHING ) ) )
    {
        hand = BODY_LEFT_HAND;
        carrying = being_carried( local, hand );
    }
    if ( carrying == 0 )
    {
        n_int az;
        n_vect2 location_vector, facing_vector, slope_vector;
        being_space( local, &location_vector );
        being_facing_vector( local, &facing_vector, 4 );
        land_vect2( &slope_vector, &az, &location_vector );

        if ( az > WATER_MAP )
        {
            if ( az > TIDE_MAX )
            {
                /* TODO handle this logic centrally */

                n_int grass, trees, bush;
                food_values( being_location_x( local ), being_location_y( local ), &grass, &trees, &bush );


                if ( ( grass > bush ) && ( grass > trees ) )
                {
                    being_take( local, hand, INVENTORY_GRASS );
#ifdef EPISODIC_ON
                    episodic_self( local, EVENT_PICKUP, EPISODIC_AFFECT_ZERO, INVENTORY_GRASS );
#endif
                }
                if ( ( trees > grass ) && ( trees > bush ) )
                {
                    if ( being_posture_under( local, POSTURE_UPRIGHT ) )
                    {
                        being_take( local, hand, INVENTORY_BRANCH );
#ifdef EPISODIC_ON
                        episodic_self( local, EVENT_PICKUP, EPISODIC_AFFECT_ZERO, INVENTORY_BRANCH );
#endif
                    }
                    else
                    {
                        being_take( local, hand, INVENTORY_NUT );
#ifdef EPISODIC_ON
                        episodic_self( local, EVENT_PICKUP, EPISODIC_AFFECT_ZERO, INVENTORY_NUT );
#endif
                    }
                }
                if ( ( bush > grass ) && ( bush > trees ) )
                {
                    being_take( local, hand, INVENTORY_TWIG );
#ifdef EPISODIC_ON
                    episodic_self( local, EVENT_PICKUP, EPISODIC_AFFECT_ZERO, INVENTORY_TWIG );
#endif
                }
            }
            else
            {
                being_take( local, hand, INVENTORY_ROCK );
#ifdef EPISODIC_ON
                episodic_self( local, EVENT_PICKUP, EPISODIC_AFFECT_ZERO, INVENTORY_ROCK );
#endif
            }
        }
    }
}

/**
 * @brief Performs a given action
 * @param local Pointer to the being performing the action
 * @param other Pointer to another being involved in the action
 * @param action The type of action
 */
void social_action(
    simulated_being *local,
    simulated_being *other,
    n_byte action )
{
    n_byte2 carrying;
    n_byte hand = BODY_RIGHT_HAND;

    if ( local->delta.awake == FULLY_ASLEEP )
    {
        return;
    }

    carrying = being_carried( local, hand );
    if ( other == 0L )
    {
        /** individual action */
        switch ( action % INDIVIDUAL_ACTIONS )
        {
        case ACTION_JAB:
            body_action_jab( local, carrying, hand );
            break;
        case ACTION_BASH_OBJECTS:
            body_action_bash_objects( local, carrying, hand );
            break;
        case ACTION_CHEW:
            body_action_chew( local, carrying, hand );
            break;
        case ACTION_BRANDISH:
            body_action_hand_object( local, carrying, hand, EVENT_BRANDISH );
            break;
        case ACTION_DRAG:
            body_action_hand_object( local, carrying, hand, EVENT_DRAG );
            break;
        case ACTION_SWAP_HANDS:
            body_action_swap_hands( local, carrying, hand );
            break;
        case ACTION_DROP:
            body_action_drop( local, carrying, hand );
            break;
        case ACTION_PICKUP:
            body_action_pickup( local, carrying, hand );
            break;
        }
    }
    else
    {
        /** social action */
        switch ( action % SOCIAL_ACTIONS )
        {
        case ACTION_PROD:
            body_action_interactive_change( local, other, BODY_RIGHT_HAND, BODY_FRONT,
                                            EVENT_PRODDED, 0, AFFECT_PRODDED );
            break;
        case ACTION_HUG:
            body_action_interactive_change( local, other, BODY_FRONT, BODY_FRONT,
                                            EVENT_HUGGED, 1, AFFECT_HUGGED );
            break;
        case ACTION_SMILE:
            body_action_interactive_change( local, other, BODY_TEETH, BODY_TEETH,
                                            EVENT_SMILED, 1, AFFECT_SMILED );
            break;
        case ACTION_GLOWER:
            body_action_interactive_change( local, other, BODY_HEAD, BODY_HEAD,
                                            EVENT_GLOWERED, 0, AFFECT_GLOWER );
            break;
        case ACTION_TICKLE:
            body_action_interactive( local, other, BODY_RIGHT_HAND, BODY_FRONT, EVENT_TICKLED );
            break;
        case ACTION_POINT:
            body_action_interactive( local, other, BODY_RIGHT_HAND, BODY_RIGHT_HAND, EVENT_POINT );
            break;
        case ACTION_PAT:
            body_action_interactive( local, other, BODY_RIGHT_HAND, BODY_BACK, EVENT_PATTED );
            break;
        case ACTION_BASH:
            body_action_bash( local, other, carrying );
            break;
        case ACTION_GIVE:
            body_action_give( local, other, carrying );
            break;
        }
    }
}

/**
 * @brief Compares two genetics and returns 1 if they are the same
 * @param genetics_a First genetics
 * @param genetics_b Second genetics
 * @return 1 if the two genetics are the same, 0 otherwise
 */
n_int genetics_compare( n_genetics *genetics_a, n_genetics *genetics_b )
{
    n_int loop = 0;

    while ( loop < CHROMOSOMES )
    {
        if ( genetics_a[loop] != genetics_b[loop] )
        {
            return 0;
        }
        loop++;
    }
    return 1;
}

/**
 * @brief Sets genetics A to be the same as genetics B
 * @param genetics_a The destination genetics
 * @param genetics_b The source genetics
 */
void genetics_set( n_genetics *genetics_a, n_genetics *genetics_b )
{
    n_int loop = 0;
    while ( loop < CHROMOSOMES )
    {
        genetics_a[loop] = genetics_b[loop];
        loop++;
    }
}

/**
 * @brief Creates a blank genome
 * @param genetics_a The genetics to be cleared
 */
void genetics_zero( n_genetics *genetics_a )
{
    n_int loop = 0;
    while ( loop < CHROMOSOMES )
    {
        genetics_a[loop] = 0;
        loop++;
    }
}

/**
 * @brief Returns 1 if the given genetics are unique within the population
 * @param local Pointer to the simulation object
 * @param genetics The new genetics to be compared
 * @return 1 if the given genetics are unique in the population
 */
static n_int genetics_unique( simulated_being *local, n_int number, n_genetics *genetics )
{
    n_int         loop = 0;
    if ( number == 0 )
    {
        return 1;
    }
    while ( loop < number )
    {
        simulated_being	 *local_being = &( local[loop] );
        if ( genetics_compare( being_genetics( local_being ), genetics ) )
        {
            return 0;
        }
        loop++;
    }
    return 1;
}

/**
 * @brief Returns the 2 bit gene value from the given point within a diploid chromosome.
   This includes mutations
 * @param chromosome The chromosome as a 32 bit value.  Lower 16 bits are from father, upper 16 bits from mother
 * @param point The index within the chromosome
 * @param mutation_prob The probability of mutation
 * @param local Random number generator seed
 * @return 2 bit gene value
 */
static n_int genetics_child_gene( n_genetics chromosome, n_int point, n_byte2 mutation_prob, n_byte2 *local )
{
    n_byte2 mutation_type;
    n_int child_gene = 0;

    math_random3( local );
    if ( math_random( local ) < mutation_prob )
    {
        mutation_type = ( math_random( local ) & 7 );
        switch ( mutation_type )
        {
        /** mutation on the maternal chromosome */
        case MUTATION_MATERNAL:
            child_gene = DIPLOID(
                             ( math_random( local ) & 3 ),
                             ( ( CHROMOSOME_FROM_FATHER( chromosome ) >> point ) & 3 ) );
            break;
        /** mutation on the paternal chromosome */
        case MUTATION_PATERNAL:
            child_gene = DIPLOID(
                             ( ( CHROMOSOME_FROM_MOTHER( chromosome ) >> point ) & 3 ),
                             ( math_random( local ) & 3 ) );
            break;
        /** duplicate of the maternal gene on both sides */
        case MUTATION_MATERNAL_DUPLICATE:
            child_gene = DIPLOID(
                             ( ( CHROMOSOME_FROM_MOTHER( chromosome ) >> point ) & 3 ),
                             ( ( CHROMOSOME_FROM_MOTHER( chromosome ) >> point ) & 3 ) );
            break;
        /** duplicate of the paternal gene on both sides */
        case MUTATION_PATERNAL_DUPLICATE:
            child_gene = DIPLOID(
                             ( ( CHROMOSOME_FROM_FATHER( chromosome ) >> point ) & 3 ),
                             ( ( CHROMOSOME_FROM_FATHER( chromosome ) >> point ) & 3 ) );
            break;
        default:
            math_random3( local );

            child_gene = DIPLOID(
                             ( math_random( local ) & 3 ), ( math_random( local ) & 3 ) );
        }
    }
    else
    {
        /** normal gene.  What grandparent genes get into the current generation is randomly chosen */
        if ( ( math_random( local ) & 1 ) != 0 )
        {
            child_gene = DIPLOID(
                             ( ( CHROMOSOME_FROM_MOTHER( chromosome ) >> point ) & 3 ),
                             ( ( CHROMOSOME_FROM_FATHER( chromosome ) >> point ) & 3 ) );
        }
        else
        {
            child_gene = DIPLOID(
                             ( ( CHROMOSOME_FROM_FATHER( chromosome ) >> point ) & 3 ),
                             ( ( CHROMOSOME_FROM_MOTHER( chromosome ) >> point ) & 3 ) );
        }
    }
    return child_gene;
}

/**
 * @brief Performs crossover and mutation
 * @param mother Chromosome of the mother (first 16 bits from maternal grandfather, second 16 bits from maternal grandmother)
 * @param father Chromosome of the father (first 16 bits from paternal grandfather, second 16 bits from paternal grandmother)
 * @param local Random number generator seed
 * @return Child chromosome
 */
static n_genetics	genetics_crossover( n_genetics mother, n_genetics father, n_byte2 *local )
{
    n_int loop = 0;
    n_genetics result = 0;
    n_int point, point2;
    n_int deletion_point = 16;
    n_byte2 prob;
    n_genetics parent;

    /** randomly select a crossover point */
    n_int crossover_point = ( math_random( local ) >> 13 ) << 1;

    /** gene insertion/deletion */
    if ( math_random( local ) < MUTATION_DELETION_PROB )
    {
        deletion_point = ( math_random( local ) >> 13 ) << 1;
    }

    point = point2 = crossover_point - 8;
    /** for every 2 bit gene in the 16 bit chromosome */
    while ( loop < 16 )
    {
        if ( loop == deletion_point )
        {
            point2 -= 2;
        }

        /** equal genetic contribution from mother and father */
        if ( point2 < 0 )
        {
            point2 += 16;
        }
        else
        {
            if ( point2 > 15 )
            {
                point2 -= 16;
            }
        }

        if ( loop < 8 )
        {
            parent = father;
            /** higher mutation probability in males */
            prob = MUTATION_CROSSOVER_PROB * 50;
        }
        else
        {
            parent = mother;
            prob = MUTATION_CROSSOVER_PROB;
        }

        result |= ( genetics_child_gene( parent, point2, prob, local ) << point );
        loop += 2;
        point += 2;
        point2 += 2;
    }
    return result;
}

/**
 * @brief Mutates a single chromosome, without crossover
 * @param chromosome The chromosome to be mutated
 * @param local Random number generator seed
 * @return The mutated chromosome
 */
static n_genetics	genetics_mutate( n_genetics chromosome, n_byte2 *local )
{
    n_genetics result = 0;
    n_int point = 0;
    n_int loop = 0;
    n_int deletion_point = 16;

    /** gene insertion/deletion */
    if ( math_random( local ) < MUTATION_DELETION_PROB )
    {
        deletion_point = ( math_random( local ) >> 13 ) << 1;
    }

    /** for every 2 bit gene in the 16 bit chromosome */
    point = 0;
    while ( loop < 16 )
    {
        if ( loop == deletion_point )
        {
            point -= 2;
            if ( point < 0 )
            {
                point += 16;
            }
        }
        if ( point > 15 )
        {
            point -= 16;
        }

        result |= ( genetics_child_gene( chromosome, point, MUTATION_CROSSOVER_PROB, local ) << point );
        loop += 2;
        point += 2;
    }
    return result;
}

/**
 * @brief Transposes segments of the genome between chromosomes or within the same chromosome.
 This is the main cause of variation between siblings.
 * @param genetics Pointer to the genetics
 * @param local Random number generator seed
 */
static void genetics_transpose( n_genetics *genetics, n_byte2 *local )
{
    math_random3( local );

    if ( math_random( local ) < MUTATION_TRANSPOSE_PROB )
    {
        /** number of bits to transpose */
        /** chromosome numbers */
        /** locations within the chromosomes */
        n_byte2 local_random0 = math_random( local );
        n_byte2 local_random1 = math_random( local );

        n_byte source_offset = ( local_random0 >> 8 ) & 31;
        n_byte dest_offset   = local_random1 & 31;
        /** whether to invert the sequence */
        n_byte inversion     = ( local_random0 >> 13 ) & 1;
        n_byte source_ch     = ( local_random1 >> 5 ) % CHROMOSOMES;
        n_byte dest_ch       = ( local_random1 >> 7 ) % CHROMOSOMES;
        n_int  ctr1          = source_offset;
        n_byte p             = 0;
        math_random3( local );

        while ( p < ( math_random( local ) & 15 ) )
        {
            n_int ctr2;
            ctr1 = ( ctr1 & 31 );

            if ( inversion == 0 )
            {
                ctr2 = ( n_int )( dest_offset + p );
            }
            else
            {
                /** inverted sequence */
                ctr2 = ( n_int )dest_offset - p + 32;
            }
            ctr2 = ( ctr2 & 31 );
            /** clear destination bit */
            if ( ( genetics[dest_ch] & ( 1 << ctr2 ) ) != 0 )
            {
                genetics[dest_ch] ^= ( 1 << ctr2 );
            }
            /** set destination bit */
            if ( ( genetics[source_ch] & ( 1 << ctr1 ) ) != 0 )
            {
                genetics[dest_ch] |= ( 1 << ctr2 );
            }
            p++;
            ctr1++;
        }
    }
}


void body_genetics( simulated_being *beings, n_int number, n_genetics *genetics, n_genetics *mother_genetics, n_genetics *father_genetics, n_byte2 *local )
{
    n_int ch;
    n_byte sex = 2;
    math_random3( local );
    sex |= ( math_random( local ) & 1 );
    do
    {
        math_random3( local );

        /** crossover and mutation */
        for ( ch = 0; ch < CHROMOSOMES; ch++ )
        {
            if ( ch != CHROMOSOME_Y )
            {
                genetics[ch] = genetics_crossover( mother_genetics[ch], father_genetics[ch], local );
            }
        }

        /** Y chromosome does not undergo crossover and passes from father to son */
        if ( sex != SEX_FEMALE )
        {
            genetics[CHROMOSOME_Y] = genetics_mutate( father_genetics[CHROMOSOME_Y], local );
        }
        else
        {
            genetics[CHROMOSOME_Y] = genetics_mutate( mother_genetics[CHROMOSOME_Y], local );
        }
        /** transpose genes between chromosomes */
        genetics_transpose( genetics, local );
        /** align the sex genetics */
        genetics[CHROMOSOME_Y] &= ~1;
        genetics[CHROMOSOME_Y] |= sex;
    }
    while ( genetics_unique( beings, number, genetics ) == 0 );
}

/**
 * @brief Returns a string of letters representing the genome
 * @param maternal Show either the maternal or paternal side of each chromosome
 * @param genome The genome to be shown
 * @param genome_str The returned string
 */
void body_genome( n_byte maternal, n_genetics *genome, n_byte *genome_str )
{
    n_byte string_point = 0;
    n_int ch, value;

    n_byte nucleotide[] = { 'A', 'T', 'C', 'G' };
    /** for every chromosome */
    for ( ch = 0; ch < CHROMOSOMES; ch++ )
    {
        /** for each 2 bit gene in the chromosome.
            Each chromosome is 16 bits long with the full
            32 bit value containing the chromosome pair */
        n_byte gene_point = 0;
        while ( gene_point < 16 )
        {
            if ( maternal != 0 )
            {
                /** the maternal part of the diplod */
                value = ( CHROMOSOME_FROM_MOTHER( genome[ch] ) >> gene_point ) & 3;
            }
            else
            {
                /** the paternal part of the diploid */
                value = ( CHROMOSOME_FROM_FATHER( genome[ch] ) >> gene_point ) & 3;
            }
            genome_str[string_point++] = nucleotide[value];
            gene_point += 2;
        }
    }
    genome_str[string_point] = 0;
}

