/****************************************************************

 immune.c

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

#include "../toolkit/toolkit.h"
#include "../universe/universe.h"

#define IMMUNE_FIT                            5
#define MIN_ANTIBODIES                        16
#define MIN_ANTIGENS                          8
#define PATHOGEN_TRANSMISSION_PROB            1000
#define PATHOGEN_ENVIRONMENT_PROB             100
#define PATHOGEN_MUTATION_PROB                100
#define ANTIBODY_DEPLETION_PROB               100

#define RANDOM_PATHOGEN(seed,pathogen_type)   (((seed%(255/PATHOGEN_TRANSMISSION_TOTAL))*PATHOGEN_TRANSMISSION_TOTAL)+pathogen_type)
#define PATHOGEN_SEVERITY(pathogen)           (((pathogen)*(pathogen))>>11)
#define PATHOGEN_TRANSMISSION(pathogen)       ((pathogen)&7)

void immune_init( simulated_immune_system *immune, n_byte2 *local_random )
{
#ifdef IMMUNE_ON
    n_byte i;
    immune->random_seed[0] = local_random[0];
    immune->random_seed[1] = local_random[1];

    for ( i = 0; i < IMMUNE_ANTIGENS; i += 2 )
    {
        immune->antigens[i] = 0;
        immune->antigens[i + 1] = 0;
        math_random3( immune->random_seed );
        immune->shape_antigen[i] = ( n_byte )( immune->random_seed[0] & 255 );
        immune->shape_antigen[i + 1] = ( n_byte )( immune->random_seed[1] & 255 );
    }
    for ( i = 0; i < IMMUNE_POPULATION; i += 2 )
    {
        immune->antibodies[i] = 0;
        immune->antibodies[i + 1] = 0;
        math_random3( immune->random_seed );
        immune->shape_antibody[i] = ( n_byte )( immune->random_seed[0] & 255 );
        immune->shape_antibody[i + 1] = ( n_byte )( immune->random_seed[1] & 255 );
    }
#endif
}

void immune_seed( simulated_immune_system *immune_mother, simulated_immune_system *immune_child )
{
#ifdef IMMUNE_ON
    n_byte i;
    /** child acquires mother's antibodies */
    for ( i = 0; i < IMMUNE_POPULATION; i++ )
    {
        immune_child->shape_antibody[i] = immune_mother->shape_antibody[i];
        immune_child->antibodies[i] = immune_mother->antibodies[i];
    }
#endif
}

static void immune_acquire_pathogen( simulated_immune_system *immune, n_byte transmission_type )
{
#ifdef IMMUNE_ON
    n_byte i;
    math_random3( ( n_byte2 * )&immune->random_seed );
    if ( immune->random_seed[0] < PATHOGEN_ENVIRONMENT_PROB )
    {
        i = immune->random_seed[1] % IMMUNE_ANTIGENS;
        if ( immune->antigens[i] == 0 )
        {
            math_random3( ( n_byte2 * )&immune->random_seed );
            immune->antigens[i] = ( n_byte )( immune->random_seed[0] & 7 );
            immune->shape_antigen[i] = ( n_byte )RANDOM_PATHOGEN( immune->random_seed[1], transmission_type );
        }
    }
#endif
}

void immune_ingest_pathogen( simulated_immune_system *immune, n_byte food_type )
{
    n_byte transmission_type = food_type + PATHOGEN_TRANSMISSION_FOOD_VEGETABLE;
    immune_acquire_pathogen( immune, transmission_type );
}

void immune_transmit( simulated_immune_system *immune0, simulated_immune_system *immune1, n_byte transmission_type )
{
#ifdef IMMUNE_ON
    n_byte i, j;
    /** pathogen obtained from environment */
    immune_acquire_pathogen( immune0, transmission_type );

    /** pathogen transmitted between beings */
    math_random3( ( n_byte2 * )&immune0->random_seed );
    if ( immune0->random_seed[0] < PATHOGEN_TRANSMISSION_PROB )
    {
        math_random3( ( n_byte2 * )&immune0->random_seed );
        i = immune0->random_seed[0] % IMMUNE_ANTIGENS;
        if ( ( immune0->antigens[i] > 0 ) &&
                ( PATHOGEN_TRANSMISSION( immune0->shape_antigen[i] ) == transmission_type ) )
        {
            /** does the other being already carry this pathogen ? */
            for ( j = 0; j < IMMUNE_ANTIGENS; j++ )
            {
                if ( immune0->shape_antigen[i] == immune1->shape_antigen[j] )
                {
                    if ( immune1->antigens[j] < 255 )
                    {
                        immune1->antigens[j]++;
                    }
                    break;
                }
            }
            if ( j == IMMUNE_ANTIGENS )
            {
                j = immune0->random_seed[1] % IMMUNE_ANTIGENS;
                if ( immune1->antigens[j] <= MIN_ANTIGENS )
                {
                    /** spread pathogen */
                    immune1->shape_antigen[j] = immune0->shape_antigen[i];
                }
            }
        }
    }
#endif
}

n_int immune_response( simulated_immune_system *immune, n_byte honor_immune, n_int being_energy )
{
#ifdef IMMUNE_ON
    n_int min_antibodies;
    n_int max_bits_matched;
    n_byte2 total_antigens, max_severity;
    n_byte i, j, k, match, best_match, bits_matched, bit;

    /** antibodies die at some fixed rate */
    math_random3( ( n_byte2 * )&immune->random_seed );
    if ( immune->random_seed[0] < ANTIBODY_DEPLETION_PROB )
    {
        i = immune->random_seed[1] % IMMUNE_POPULATION;
        if ( immune->antibodies[i] > 0 )
        {
            immune->antibodies[i]--;
        }
    }

    /** pick an antigen */
    math_random3( ( n_byte2 * )&immune->random_seed );
    i = immune->random_seed[0] % IMMUNE_ANTIGENS;
    if ( immune->antigens[i] != 0 )
    {
        /** mutate with some probability */
        if ( immune->random_seed[1] < PATHOGEN_MUTATION_PROB )
        {
            math_random3( ( n_byte2 * )&immune->random_seed );
            if ( ( immune->shape_antigen[i] & ( 1 << ( immune->random_seed[0] & 7 ) ) ) != 0 )
            {
                immune->shape_antigen[i] ^= ( immune->random_seed[0] & 7 );
            }
            else
            {
                immune->shape_antigen[i] |= ( immune->random_seed[0] & 7 );
            }
        }

        /** try to find a matching antibody */
        max_bits_matched = 0;
        best_match = 0;
        for ( j = 0; j < IMMUNE_POPULATION; j++ )
        {
            match = ( immune->shape_antibody[j] & immune->shape_antigen[i] ) |
                    ( ( ~immune->shape_antibody[j] ) & ( ~immune->shape_antigen[i] ) );
            if ( match != 0 )
            {
                /** how good is the fit ? */
                bits_matched = 0;
                for ( bit = 0; bit < 8; bit++ )
                {
                    if ( ( match & ( 1 << bit ) ) != 0 )
                    {
                        bits_matched++;
                    }
                }
                /** record best fit */
                if ( bits_matched > max_bits_matched )
                {
                    max_bits_matched = bits_matched;
                    best_match = j;
                }
            }
        }

        /** select the antibody with the smallest population */
        min_antibodies = immune->antibodies[0];
        j = 0;
        for ( k = 1; k < IMMUNE_POPULATION; k++ )
        {
            if ( immune->antibodies[k] < min_antibodies )
            {
                min_antibodies = immune->antibodies[k];
                j = k;
            }
        }

        /** match antigen and antibody */
        if ( max_bits_matched > IMMUNE_FIT )
        {
            /** Antibodies multiply
             A better fit results in more antibodies */
            if ( immune->antibodies[best_match] < 255 - max_bits_matched )
            {
                immune->antibodies[best_match] += ( n_byte )max_bits_matched;
                /** apply a minimum threshold so that generated
                 antibodies don't overwrite known good fits */
                if ( immune->antibodies[best_match] < MIN_ANTIBODIES )
                {
                    immune->antibodies[best_match] = MIN_ANTIBODIES;
                }
            }
            /** antigens are depleted according to the immune system strength */
            if ( immune->antigens[i] > honor_immune )
            {
                immune->antigens[i] -= honor_immune;
            }
            else
            {
                immune->antigens[i] = 0;
            }
            /** clone antibody with mutation */
            if ( j != best_match )
            {
                immune->antibodies[j] = 1;
                match = immune->shape_antibody[best_match];
                math_random3( ( n_byte2 * )&immune->random_seed );
                if ( ( match & ( 1 << ( immune->random_seed[0] & 7 ) ) ) != 0 )
                {
                    match ^= ( immune->random_seed[0] & 7 );
                }
                else
                {
                    match |= ( immune->random_seed[0] & 7 );
                }
                immune->shape_antibody[j] = match;
            }
        }
        else
        {
            /** If pathogens are not challenged they multiply */
            if ( immune->antigens[i] < 255 )
            {
                immune->antigens[i]++;
            }

            /** produce differently shaped antibodies */
            math_random3( ( n_byte2 * )&immune->random_seed );
            if ( immune->random_seed[0] < being_energy )
            {
                math_random3( ( n_byte2 * )&immune->random_seed );
                immune->shape_antibody[j] = ( n_byte )( immune->random_seed[0] & 255 );
                immune->antibodies[j] = ( n_byte )( immune->random_seed[1] & 7 );
            }
        }
    }

    /** Energy level is reduced based upon pathogens.
     Note that not all pathogens have the same energy cost. */
    total_antigens = 0;
    max_severity = 0;
    for ( i = 0; i < IMMUNE_ANTIGENS; i++ )
    {
        /* total quantity of pathogen */
        total_antigens += immune->antigens[i];
        /* record the maximum pathogen severity */
        if ( immune->shape_antigen[i] > max_severity )
        {
            max_severity = immune->shape_antigen[i];
        }
    }
    math_random3( ( n_byte2 * )&immune->random_seed );
    if ( ( immune->random_seed[0] < ( total_antigens >> 2 ) ) && ( being_energy >= 1 ) )
    {
        return PATHOGEN_SEVERITY( max_severity );
    }
#endif
    return 0;
}
