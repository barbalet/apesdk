/****************************************************************

 brain.c

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

#include <stdio.h>

/* typical minimum spacing between MVB instructions */
#define BRAINCODE_MIN_MVB_SPACING          2

#define BRAINCODE_CONSTANT0_BIT		  (64)
#define BRAINCODE_CONSTANT1_BIT		  (128)

#define BRAINCODE_DATA_START          BRAINCODE_DAT0
#define BRAINCODE_DATA_NUMBER         (1 + BRAINCODE_DAT1 - BRAINCODE_DATA_START)

#define BRAINCODE_OPERATORS_START     BRAINCODE_ADD
#define BRAINCODE_OPERATORS_NUMBER    (1 + BRAINCODE_LTP - BRAINCODE_OPERATORS_START)

#define BRAINCODE_CONDITIONALS_START  BRAINCODE_JMZ
#define BRAINCODE_CONDITIONALS_NUMBER (1 + BRAINCODE_SLT - BRAINCODE_CONDITIONALS_START)

#define BRAINCODE_SENSORS_START       BRAINCODE_SEN
#define BRAINCODE_SENSORS_NUMBER      (1 + BRAINCODE_SEN3 - BRAINCODE_SENSORS_START)

#define BRAINCODE_ACTUATORS_START     BRAINCODE_ACT
#define BRAINCODE_ACTUATORS_NUMBER    (1 + BRAINCODE_ANE - BRAINCODE_ACTUATORS_START)

#define BRAINCODE_INSTRUCTION(braincode,i) ((braincode[i] & (BRAINCODE_CONSTANT0_BIT-1)) % BRAINCODE_INSTRUCTIONS)
#define BRAINCODE_CONSTANT0(braincode,i)    (braincode[i] & BRAINCODE_CONSTANT0_BIT)
#define BRAINCODE_CONSTANT1(braincode,i)    (braincode[i] & BRAINCODE_CONSTANT1_BIT)
#define BRAINCODE_VALUE(braincode,i,n)      (braincode[(i+1+n) & (BRAINCODE_SIZE - 1)])

/*	Brain definitions */

#define B_SIZE		(32768)
#define B_WR        (B_SIZE - 1)
#define	F_X			(1)
#define	F_Y			(32)
#define	F_Z			(1024)
#define	B_Z			(B_SIZE - F_Z)
#define	B_Y			(B_SIZE - F_Y)
#define	B_X			(B_SIZE - F_X)

/*
 *	The basic brain formula is;
 *	   b(t+1) = a*l + b(t)*m + (b(t)-b(t-1))*n;
 *
 *	The waking mind works differently to the sleeping mind. This is quantified
 *	with two distinct but similar equations. There are two versions for the awake
 *	and asleep states, in this function it is simplified to;
 *	   b(t+1) = a*l_a + b(t)*l_b - b(t-1)*l_c;
 *
 *	   where, l_a = l, l_b = m+n, l_c = n
 */

#define	B_FN(ave, bra, obra) ((ave+bra-obra)>>10)

/** positive and negative, lower and upper halves */
#define B_P_LH (br[loc+F_Z]+br[loc+F_Y]+br[loc+F_X])
#define	B_P_UH (br[loc-F_X]+br[loc-F_Y]+br[loc-F_Z])
#define	B_N_LH (br[(loc+F_X)&B_WR]+br[(loc+F_Y)&B_WR]+br[(loc+F_Z)&B_WR])
#define	B_N_UH (br[(loc+B_Z)&B_WR]+br[(loc+B_Y)&B_WR]+br[(loc+B_X)&B_WR])

/**
 * @brief Updates the brain
 * @param local Pointer to the brain array
 * @param constants Array containing brain constants
 */
void brain_cycle( n_byte *local, n_byte2 *constants )
{
    n_byte br[B_SIZE];
    n_byte  *bract = local, *obr = &local[B_SIZE];
    n_int  l_a = constants[0];
    n_int  l_c = constants[2];
    n_int  l_b = constants[1] + l_c;
    n_int  loc = 0;
    n_int  average;
    n_int  obr_tmp;
    n_int  br_tmp;
    n_int  count = F_Z;

    memory_copy( bract, br, B_SIZE );

    do
    {
        average = ( B_P_LH + B_N_UH );
        br_tmp = br[loc];
        obr_tmp = obr[loc];

        average *= l_a;
        obr_tmp *= l_c;

        br_tmp *= l_b;
        br_tmp -= obr_tmp;
        average += br_tmp;

        br[loc++] = ( n_byte )( average >> 10 );
        count--;

    }
    while ( count );
    count = B_Z - F_Z;
    do
    {
        average =  br[loc - F_Z];
        average += br[loc - F_Y];
        average += br[loc - F_X];

        br_tmp = br[loc];

        average += br[loc + F_X];
        average += br[loc + F_Y];
        average += br[loc + F_Z];

        obr_tmp = obr[loc];
        average *= l_a;
        obr_tmp *= l_c;
        br_tmp *= l_b;
        br_tmp -= obr_tmp;
        average += br_tmp;
        br[loc++] = ( n_byte )( average >> 10 );
        count--;
    }
    while ( count );
    count = F_Z;
    do
    {
        average = B_P_UH;
        br_tmp = br[loc];
        average += B_N_LH;
        obr_tmp = obr[loc];

        average *= l_a;
        obr_tmp *= l_c;
        br_tmp *= l_b;
        br_tmp -= obr_tmp;
        average += br_tmp;

        br[loc++] = ( n_byte )( average >> 10 );
        count--;
    }
    while ( count );

    memory_copy( bract, obr, B_SIZE );
    memory_copy( br, bract, B_SIZE );

}

/**     "XXX_#NNN_#NNN"*/
/**     "0123456789012"*/


enum BC_FORMAT
{
    BC_FORMAT_A = 0,
    BC_FORMAT_C,
    BC_FORMAT_E,
    BC_FORMAT_F,
    BC_FORMAT_G,
    BC_FORMAT_H
};
const n_string braincode_mnemonic[BRAINCODE_INSTRUCTIONS] =
{
    /** data */
    "DAT0",
    "DAT1",

    /** operators */
    "ADD ",
    "SUB ",
    "MUL ",
    "DIV ",
    "MOD ",
    "MVB ",
    "MOV ",
    "JMP ",
    "CTR ",
    "SWP ",
    "INV ",
    "STP ",
    "LTP ",

    /** conditionals */
    "JMZ ",
    "JMN ",
    "DJN ",
    "AND ",
    "OR  ",
    "SEQ ",
    "SNE ",
    "SLT ",

    /** sensors */
    "SEN ",
    "SEN2",
    "SEN3",

    /** actuators */
    "ACT ",
    "ACT2",
    "ACT3",
    "ANE "
};

/**
 * @brief Returns the type of formatting of a three byte instruction
 * @param instruction The instruction type
 * @param command Whether the subsequent two bytes are constants or variables
 * @param value0 The first argument
 * @param value1 The second argument
 * @return The format type used to display a three byte instruction
 */
static enum BC_FORMAT brain_format( n_byte instruction, n_byte command, n_byte value0, n_byte value1 )
{
    n_byte is_constant0 = ( ( command & BRAINCODE_CONSTANT0_BIT ) != 0 );
    n_byte is_constant1 = ( ( command & BRAINCODE_CONSTANT1_BIT ) != 0 );

    switch ( instruction )
    {
    case BRAINCODE_AND:
    case BRAINCODE_OR:
    case BRAINCODE_MOV:
    case BRAINCODE_ADD:
    case BRAINCODE_SUB:
    case BRAINCODE_MUL:
    case BRAINCODE_MOD:
        if ( ( !is_constant0 ) && ( !is_constant1 ) )
        {
            return BC_FORMAT_A;
        }
        return BC_FORMAT_C;
        break;
    case BRAINCODE_JMZ:
    case BRAINCODE_JMN:
    case BRAINCODE_DJN:
        return BC_FORMAT_E;
        break;
    case BRAINCODE_SEQ:
    case BRAINCODE_SNE:
    case BRAINCODE_SLT:
        if ( ( !is_constant0 ) && ( !is_constant1 ) )
        {
            return BC_FORMAT_A;
        }
        return BC_FORMAT_F;
        break;
    case BRAINCODE_DAT0:
    case BRAINCODE_DAT1:
    case BRAINCODE_JMP:
        return BC_FORMAT_C;
        break;
    case BRAINCODE_INV:
        if ( is_constant0 )
        {
            return BC_FORMAT_G;
        }

        return BC_FORMAT_H;

        break;
    case BRAINCODE_STP:
    case BRAINCODE_LTP:
        if ( ( is_constant0 ) && ( !is_constant1 ) )
        {
            return BC_FORMAT_F;
        }
        if ( ( is_constant0 ) && ( is_constant1 ) )
        {
            return BC_FORMAT_C;
        }
        if ( ( !is_constant0 ) && ( is_constant1 ) )
        {
            return BC_FORMAT_E;
        }
        break;
    }
    return BC_FORMAT_A;
}

/** "aeio" "fmsv"; */

const n_string braincode_spoken_dictionary[BRAINCODE_INSTRUCTIONS] =
{
    /** data */
    "a",    /**< "DAT0", */
    "o",    /**< "DAT1", */

    /** operators */
    "mam", /**< "ADD ",*/
    "vos", /**< "SUB ",*/
    "sie",  /**< "MUL ",*/
    "fes",  /**< "DIV ",*/
    "feo", /**< "MOD ",*/
    "mas", /**< "MVB ",*/
    "vam", /**< "MOV ",*/
    "amo", /**< "JMP ",*/
    "sam", /**< "CTR ",*/
    "mao", /**< "SWP ",*/
    "ova", /**< "INV ",*/
    "eef",  /**< "STP ",*/
    "fee",  /**< "LTP ",*/

    /** conditionals */
    "om", /**< "JMZ ",*/
    "ov",  /**< "JMN ",*/
    "fi",  /**< "DJN ",*/
    "im", /**< "AND ",*/
    "se", /**< "OR  ",*/
    "es",  /**< "SEQ ",*/
    "os",  /**< "SNE ",*/
    "is",  /**< "SLT ",*/

    /** sensors */
    "favos", /**< "SEN ",*/
    "vamos", /**< "SEN2",*/
    "famov", /**< "SEN3",*/

    /** actuators */
    "iema", /**< "ACT ",*/
    "iova", /**< "ACT2",*/
    "iafi", /**< "ACT3",*/
    "ovma", /**< "ANE "*/
};

static n_byte brain_vc( n_byte value, n_byte vowel )
{
    if ( vowel )
    {
        switch ( value )
        {
        case 3:
            return 'a';
        case 1:
            return 'e';
        case 2:
            return 'i';
        default:
            return 'o';
        }
    }
    switch ( value )
    {
    case 0:
        return 'v';
    case 1:
        return 'f';
    case 2:
        return 's';
    case 3:
        return 't';
    case 4:
        return 'p';
    case 5:
        return 'b';
    case 6:
        return 'j';
    default:
        return 'm';
    }
}

static void brain_longword( n_string output, n_byte value )
{
    output[0] = ( n_char )brain_vc( ( value >> 0 ) & 7, 0 );
    output[1] = ( n_char )brain_vc( ( value >> 3 ) & 3, 1 );
    output[2] = ( n_char )brain_vc( ( value >> 5 ) & 7, 0 );
    output[4] = 0;
}

static void brain_four_character_byte( n_string value, n_byte star, n_byte dashes, n_byte number )
{
    if ( star )
    {
        value[0] = '*';
    }
    else
    {
        value[0] = ' ';
    }
    if ( dashes )
    {
        value[1] = '-';
        value[2] = '-';
        value[3] = '-';
    }
    else
    {
        value[1] = '0' + ( number / 100 ) % 10;
        value[2] = '0' + ( number / 10 ) % 10;
        value[3] = '0' + ( number / 1 ) % 10;

        if ( value[1] == '0' )
        {
            value[1] = ' ';
            if ( value[2] == '0' )
            {
                value[2] = ' ';
            }
        }
    }
}

static void brain_space_nstruction_space( n_string value, n_byte instruction )
{
    n_string instruction_string = braincode_mnemonic[instruction];
    value[0] = ' ';
    value[1] = instruction_string[0];
    value[2] = instruction_string[1];
    value[3] = instruction_string[2];
    value[4] = instruction_string[3];
    value[5] = ' ';
}

/**
 * @brief prints a three byte instruction in the appropriate format
 * @param string The returned string
 * @param response Array containing the three byte instruction
 */
void brain_three_byte_command( n_string string, n_byte *response )
{
    n_byte command      = response[0];
    n_byte value0       = response[1];
    n_byte value1       = response[2];
    n_byte instruction  = ( command & ( BRAINCODE_CONSTANT0_BIT - 1 ) ) % BRAINCODE_INSTRUCTIONS;
    enum BC_FORMAT format       = brain_format( instruction, command, value0, value1 );

    brain_space_nstruction_space( string, instruction );

    switch ( format )
    {
    case BC_FORMAT_A:
        brain_four_character_byte( &string[6],  0, 0, value0 );
        brain_four_character_byte( &string[10], 0, 0, value1 );
        break;
    case BC_FORMAT_C:
        brain_four_character_byte( &string[6],  1, 0, value0 );
        brain_four_character_byte( &string[10], 1, 0, value1 );
        break;
    case BC_FORMAT_E:
        brain_four_character_byte( &string[6],  0, 0, value0 );
        brain_four_character_byte( &string[10], 1, 0, value1 );
        break;
    case BC_FORMAT_F:
        brain_four_character_byte( &string[6],  1, 0, value0 );
        brain_four_character_byte( &string[10], 0, 0, value1 );
        break;
    case BC_FORMAT_G:
        brain_four_character_byte( &string[6],  0, 0, value0 );
        brain_four_character_byte( &string[10], 0, 1, 0 );
        break;
    default:
        brain_four_character_byte( &string[6],  0, 1, 0 );
        brain_four_character_byte( &string[10], 0, 0, value1 );
        break;
    }
    string[14] = 0;
}

/**
 * @brief A simple sort of sentence construction from three byte instructions
 * @param string The returned sentence
 * @param response Array containing three byte instruction
 */
void brain_sentence( n_string string, n_byte *response )
{
    n_byte command      = response[0];
    n_byte value0       = response[1];
    n_byte value1       = response[2];
    n_byte instruction  = ( command & ( BRAINCODE_CONSTANT0_BIT - 1 ) ) % BRAINCODE_INSTRUCTIONS;
    enum BC_FORMAT  format = brain_format( instruction, command, value0, value1 );
    n_string_block      first_word, second_word;
    n_int  position     = 0;

    brain_longword( first_word, value0 );
    brain_longword( second_word, value1 );

    switch ( format )
    {
    case BC_FORMAT_A:
    case BC_FORMAT_C:
    case BC_FORMAT_E:
    case BC_FORMAT_F:
        io_string_write( string, braincode_spoken_dictionary[instruction], &position );
        io_string_write( string, " ", &position );
        io_string_write( string, first_word, &position );
        io_string_write( string, second_word, &position );
        break;
    case BC_FORMAT_G:
        io_string_write( string, braincode_spoken_dictionary[instruction], &position );
        io_string_write( string, " ", &position );
        io_string_write( string, first_word, &position );
        break;
    default:
        io_string_write( string, braincode_spoken_dictionary[instruction], &position );
        io_string_write( string, " ", &position );
        io_string_write( string, second_word, &position );
        break;
    }
}

#ifdef BRAINCODE_ON

/**
 * @brief returns a random braincode instruction of the given type
 * @param instruction_type Number indicating the type of instruction
 * @return braincode instruction
 */
static n_byte get_braincode_instruction_type( n_byte instruction_type )
{
    n_byte2 local_random[2];

    math_random3( local_random );
    switch ( instruction_type )
    {
    case 0: /**< GENE_BRAINCODE_SENSORS(genetics);*/
        return BRAINCODE_DATA_START + ( local_random[0] % ( BRAINCODE_DATA_NUMBER ) );
    case 1: /**< GENE_BRAINCODE_ACTUATORS(genetics); */
        return BRAINCODE_SENSORS_START + ( local_random[0] % ( BRAINCODE_SENSORS_NUMBER ) );
    case 2: /**< GENE_BRAINCODE_CONDITIONALS(genetics); */
        return BRAINCODE_ACTUATORS_START + ( local_random[0] % ( BRAINCODE_ACTUATORS_NUMBER ) );
    case 3: /**< GENE_BRAINCODE_OPERATORS(genetics); */
        return BRAINCODE_OPERATORS_START + ( local_random[0] % ( BRAINCODE_OPERATORS_NUMBER ) );
    case 4: /**< GENE_BRAINCODE_DATA(genetics); */
        return BRAINCODE_CONDITIONALS_START + ( local_random[0] % ( BRAINCODE_CONDITIONALS_NUMBER ) );
    }

    return BRAINCODE_DATA_START;
}

/* return the number of instruction_types in the braincode */


/**
 * @brief returns a random braincode instruction
 * @param local_being Pointer to the being
 * @return braincode instruction
 */
n_byte get_braincode_instruction( simulated_being *local_being )
{
    n_byte2 prob[5], total, index;
    n_genetics *genetics = being_genetics( local_being );
    n_byte  i;
    const n_byte2 min = 2;

    prob[0] = ( n_byte2 )( min + GENE_BRAINCODE_SENSORS( genetics ) );
    prob[1] = ( n_byte2 )( min + GENE_BRAINCODE_ACTUATORS( genetics ) );
    prob[2] = ( n_byte2 )( min + GENE_BRAINCODE_CONDITIONALS( genetics ) );
    prob[3] = ( n_byte2 )( min + GENE_BRAINCODE_OPERATORS( genetics ) );
    prob[4] = ( n_byte2 )( min + GENE_BRAINCODE_DATA( genetics ) );

    total = prob[0] + prob[1] + prob[2] + prob[3] + prob[4];

    if ( total == 0 )
    {
        index = 0;
    }
    else
    {
        index = being_random( local_being );
    }
    total = 0;
    for ( i = 0; i < 5; i++, total += prob[i] )
    {
        if ( index >= total )
        {
            return get_braincode_instruction_type( i );
        }
    }
    return get_braincode_instruction_type( 4 );
}

static n_int get_actor_index( simulated_isocial *social_graph, n_int value )
{
    n_int i;

    for ( i = 0; i < SOCIAL_SIZE; i++ )
    {
        if ( SOCIAL_GRAPH_ENTRY_EMPTY( social_graph, i ) )
        {
            break;
        }
    }

    if ( i == 0 )
    {
        return 0;
    }

    return value % i;
}

/**
 * @brief returns the social graph index for a being with the same (or similar) name to the given episodic memory event
 * @param social_graph Pointer to the social graph
 * @param episodic_event Pointer to the episodic memory
 * @param episode_index Index of the episode which is the current focus of attention
 * @return Social graph index
 */
static n_int get_actor_index_from_episode(
    simulated_isocial *social_graph,
    simulated_iepisodic *episodic_event,
    n_int episode_index )
{
    n_int i, actor_index = -1;

    for ( i = 1; i < SOCIAL_SIZE_BEINGS; i++ )
    {
        if ( !SOCIAL_GRAPH_ENTRY_EMPTY( social_graph, i ) )
        {
            if ( social_graph[i].family_name == episodic_event[episode_index].family_name )
            {
                actor_index = i;
                if ( social_graph[i].first_name == episodic_event[episode_index].first_name )
                {
                    actor_index = i;
                    break;
                }
            }
        }
    }
    return actor_index;
}

typedef n_int ( n_similar )( simulated_iepisodic *episodic, n_int *carry_through );

/**
 * @brief Returns the index of the most similar episodic memory
 * @param episode_index Current episodic memory index
 * @param episodic Pointer to the episodic memory
 * @param memory_visited Array which keeps track of which episodic memories have already been visited
 * @param carry_through The property to be compared against
 * @param function The similarity function to be used to do the comparrisson
 * @return Episodic memory index of the most similar event
 */
static n_int attention_similar( n_int episode_index,
                                simulated_iepisodic *episodic,
                                n_int *memory_visited,
                                n_int *carry_through,
                                n_similar function )
{
    n_int i;
    n_int visited_max = memory_visited[episode_index] - ( EPISODIC_SIZE >> 1 );
    n_int min = -1;
    n_int next_episode_index = -1;
    if ( visited_max < 0 )
    {
        visited_max = 0;
    }

    for ( i = 0; i < EPISODIC_SIZE; i++ )
    {
        if ( episodic[i].event == 0 )
        {
            continue;
        }

        if ( i != episode_index )
        {
            /** was this episode recently visited? */
            if ( memory_visited[i] <= visited_max )
            {
                /** difference between episodes */
                n_int diff = function( &episodic[i], carry_through );
                if ( diff < 0 )
                {
                    diff = -diff;
                }
                if ( ( min == -1 ) || ( diff < min ) )
                {
                    /** record most similar */
                    min = diff;
                    next_episode_index = i;
                }
            }
        }
    }
    if ( next_episode_index > -1 )
    {
        /** mark this episode as having been visited */
        memory_visited[next_episode_index] = memory_visited[episode_index] + 1;
    }
    return next_episode_index;
}

/**
 * @brief returns the temporal proximity of an episodic event to the given reference time
 * @param episodic Pointer to the episodic memory event
 * @param carry_through Time to be compared against
 * @return Absolute temporal proximity
 */
static n_int similar_time( simulated_iepisodic *episodic, n_int *carry_through )
{
    n_int dt = episodic->space_time.time - carry_through[0];
    if ( dt < 0 )
    {
        dt = - dt;
    }
    return dt;
}

/**
 * @brief Returns the temporal proximity of an episodic memory event to the given time
 * @param episode_index Index of the current focus of attention within the episodic memory
 * @param episodic Pointer to the episodic memory
 * @param memory_visited Time to be compared against
 * @return Absolute temporal proximity
 */
static n_int attention_similar_time( n_int episode_index,
                                     simulated_iepisodic *episodic,
                                     n_int *memory_visited )
{
    n_int time = episodic[episode_index].space_time.time;
    return attention_similar( episode_index, episodic, memory_visited, &time, similar_time );
}

/**
 * @brief Returns the affective similarity of the given affect value to an episodic memory event
 * @param episodic Pointer to the episodic memory event
 * @param carry_through Affect value to be compared against
 * @return Absolute difference in affect value
 */
static n_int similar_affect( simulated_iepisodic *episodic, n_int *carry_through )
{
    n_int da = episodic->affect - carry_through[0];
    if ( da < 0 )
    {
        da = - da;
    }
    return da;
}
/**
 * @brief Returns the affective similarity of the given affect value to an episodic memory event
 * @param episode_index Index of the current focus of attention within the episodic memory
 * @param episodic Pointer to the episodic memory
 * @param memory_visited Affect value to be compared against
 * @return Absolute difference in affect value
 */
static n_int attention_similar_affect( n_int episode_index,
                                       simulated_iepisodic *episodic,
                                       n_int *memory_visited )
{
    n_int affect = episodic[episode_index].affect;
    return attention_similar( episode_index, episodic, memory_visited, &affect, similar_affect );
}

/**
 * @brief Returns the similarity between a given episodic memory event and the given family name
 * @param episodic Pointer to the episodic memory event
 * @param carry_through Family name to be compared against
 * @return Similarity value in the range 0-3
 */
static n_int similar_name( simulated_iepisodic *episodic, n_int *carry_through )
{
    n_int similarity = 3;
    n_byte values[2];

    being_unpack_family( episodic->family_name[BEING_MET], values );

    if ( values[0] == carry_through[0] )
    {
        similarity--;
    }
    if ( values[1] == carry_through[1] )
    {
        similarity--;
    }
    if ( episodic->first_name[BEING_MET] == carry_through[2] )
    {
        similarity--;
    }
    return similarity;
}


/**
 * @brief Returns the similarity between a given episodic memory event and the given being name
 * @param episode_index Index of the current focus of attention within the episodic memory
 * @param episodic Pointer to the episodic memory
 * @param memory_visited Affect value to be compared against
 * @return Similarity value
 */
static n_int attention_similar_name( n_int episode_index,
                                     simulated_iepisodic *episodic,
                                     simulated_being *meeter,
                                     n_int *memory_visited )
{
    n_int name[3];
    n_byte values[2];

    being_unpack_family( episodic->family_name[BEING_MET], values );

    name[0] = values[0];
    name[1] = values[1];
    name[2] = episodic[episode_index].first_name[BEING_MET];

    return attention_similar( episode_index, episodic, memory_visited, name, similar_name );
}

/**
 * @brief Returns the similarity of a given date to the date of the given episodic memory event
 * @param episodic Pointer to the episodic memory event
 * @param carry_through Date to be compared against
 * @return Absolute difference in date value
 */
static n_int similar_date( simulated_iepisodic *episodic, n_int *carry_through )
{
    n_int dd = episodic->space_time.date - carry_through[0];
    if ( dd < 0 )
    {
        dd = - dd;
    }
    return dd;
}

/**
 * @brief Returns the similarity of a given date to the date of the given episodic memory event
 * @param episode_index Index of the current focus of attention within the episodic memory
 * @param episodic Pointer to the episodic memory
 * @param memory_visited Date to be compared against
 * @return Similarity value
 */
static n_int attention_similar_date( n_int episode_index,
                                     simulated_iepisodic *episodic,
                                     n_int *memory_visited )
{
    n_int time = episodic[episode_index].space_time.date;
    return attention_similar( episode_index, episodic, memory_visited, &time, similar_date );
}

/**
 * @brief Returns the similarity between the given episodic memory event place and a given reference location
 * @param episodic Pointer to the episodic memory event
 * @param carry_through 2D coordinate of the place to be compared against
 * @return Similarity value (2D distance squared)
 */
static n_int similar_place( simulated_iepisodic *episodic, n_int *carry_through )
{
    n_int dx = episodic->space_time.location[0] - carry_through[0];
    n_int dy = episodic->space_time.location[1] - carry_through[1];
    /** TODO should be calculated in the future with wrap around comparison */
    n_int da = ( dx * dx ) + ( dy * dy );
    return da;
}

/**
 * @brief Returns the similarity between the given episodic memory event place and a given reference location
 * @param episode_index Index of the episodic memory which is the current focus of attention
 * @param episodic Pointer to the episodic memory
 * @param memory_visited 2D coordinate of the place to be compared against
 * @return Similarity value (2D distance squared)
 */
static n_int attention_similar_place( n_int episode_index,
                                      simulated_iepisodic *episodic,
                                      n_int *memory_visited )
{
    n_int location[2];
    location[0] = episodic[episode_index].space_time.location[0];
    location[1] = episodic[episode_index].space_time.location[1];
    return attention_similar( episode_index, episodic, memory_visited, location, similar_place );
}


n_byte *math_general_allocation( n_byte *bc0, n_byte *bc1, n_int i )
{
    if ( BRAINCODE_ADDRESS( i ) < BRAINCODE_SIZE )
    {
        /** address within this being */
        return &bc0[BRAINCODE_ADDRESS( i )];
    }
    /** Address within the other being */
    return &bc1[BRAINCODE_ADDRESS( i ) - BRAINCODE_SIZE];
}

void math_general_execution( n_int instruction, n_int is_constant0, n_int is_constant1,
                             n_byte *addr0, n_byte *addr1, n_int value0, n_int *i,
                             n_int is_const0, n_int is_const1,
                             n_byte *pspace,
                             n_byte *bc0, n_byte *bc1,
                             n_int braincode_min_loop )
{
    /** Logical and */
    switch ( instruction )
    {
    case BRAINCODE_AND:
        if ( is_constant0 )
        {
            addr0[0] &= addr1[0];
        }
        else
        {
            if ( ( addr0[0] > 127 ) && ( addr1[0] > 127 ) )
            {
                *i += BRAINCODE_BYTES_PER_INSTRUCTION;
            }
        }
        break;
    /** Logical or */
    case BRAINCODE_OR:
        if ( is_constant0 )
        {
            addr0[0] |= addr1[0];
        }
        else
        {
            if ( ( addr0[0] > 127 ) || ( addr1[0] > 127 ) )
            {
                *i += BRAINCODE_BYTES_PER_INSTRUCTION;
            }
        }
        break;
    /** Move a byte, with no particular alignment */
    case BRAINCODE_MOV:
        if ( ( !is_constant0 ) && ( !is_constant1 ) )
        {
            addr1[0] = addr0[0];
        }
        else
        {
            addr1[0] = ( n_byte )value0;
        }
        break;
    /** Move a block of instructions */
    case BRAINCODE_MVB:
    {
        n_int ptr0, ptr1, n, instructions_to_copy, dat = 0;

        if ( !is_constant0 )
        {
            ptr0 = BRAINCODE_ADDRESS( *i + ( ( n_int )addr0[0] * BRAINCODE_BYTES_PER_INSTRUCTION ) );
        }
        else
        {
            ptr0 = BRAINCODE_ADDRESS( *i + ( ( n_int )value0 * BRAINCODE_BYTES_PER_INSTRUCTION ) );
        }

        ptr1 = BRAINCODE_ADDRESS( *i + ( ( n_int )is_const0 * BRAINCODE_BYTES_PER_INSTRUCTION ) );

        instructions_to_copy = 1 + ( pspace[1] % BRAINCODE_BLOCK_COPY );

        while ( dat < instructions_to_copy )
        {
            if ( ptr0 < BRAINCODE_SIZE )
            {
                addr0 = &bc0[ptr0];
            }
            else
            {
                addr0 = &bc1[ptr0 - BRAINCODE_SIZE];
            }

            if ( ptr1 < BRAINCODE_SIZE )
            {
                addr1 = &bc0[ptr1];
            }
            else
            {
                addr1 = &bc1[ptr1 - BRAINCODE_SIZE];
            }

            for ( n = 0; n < BRAINCODE_BYTES_PER_INSTRUCTION; n++ )
            {
                addr1[n] = addr0[n];
            }
            dat++;
            ptr0 = BRAINCODE_ADDRESS( ptr0 + BRAINCODE_BYTES_PER_INSTRUCTION );
            ptr1 = BRAINCODE_ADDRESS( ptr1 + BRAINCODE_BYTES_PER_INSTRUCTION );
        }
    }
    break;

    /** Add */
    case BRAINCODE_ADD:
        if ( ( !is_constant0 ) && ( !is_constant1 ) )
        {
            addr1[0] += addr0[0];
        }
        else
        {
            addr1[0] += value0;
        }
        break;
    /** Subtract */
    case BRAINCODE_SUB:
        if ( ( !is_constant0 ) && ( !is_constant1 ) )
        {
            addr1[0] -= addr0[0];
        }
        else
        {
            addr1[0] -= value0;
        }
        break;
    /** Multiply */
    case BRAINCODE_MUL:
        if ( ( !is_constant0 ) && ( !is_constant1 ) )
        {
            addr1[0] *= addr0[0];
        }
        else
        {
            addr1[0] *= value0;
        }
        break;
    /** Divide */
    case BRAINCODE_DIV:
        if ( ( !is_constant0 ) && ( !is_constant1 ) )
        {
            addr1[0] >>= ( addr0[0] % 4 );
        }
        else
        {
            addr1[0] >>= ( value0 % 4 );
        }
        break;
    /** Modulus */
    case BRAINCODE_MOD:
        if ( ( !is_constant0 ) && ( !is_constant1 ) )
        {
            if ( addr0[0] != 0 )
            {
                addr1[0] %= addr0[0];
            }
        }
        else
        {
            if ( value0 != 0 )
            {
                addr1[0] %= value0;
            }
        }
        break;
    /** Count up or down */
    case BRAINCODE_CTR:
        if ( addr0[0] > 127 )
        {
            if ( addr1[0] < 255 )
            {
                addr1[0]++;
            }
            else
            {
                addr1[0] = 0;
            }
        }
        else
        {
            if ( addr1[0] > 0 )
            {
                addr1[0]--;
            }
            else
            {
                addr1[0] = 255;
            }
        }
        break;
    /** Goto */
    case BRAINCODE_JMP:
    {
        n_int v0 = is_const0;
        n_int v1 = is_const1;
        n_int i2 = ( *i + ( ( ( v0 * 256 ) + v1 ) * BRAINCODE_BYTES_PER_INSTRUCTION ) ) % BRAINCODE_SIZE;
        if ( i2 <= *i )
        {
            if ( ( *i - i2 ) < braincode_min_loop )
            {
                i2 = *i - braincode_min_loop;
                if ( i2 < 0 )
                {
                    i2 += BRAINCODE_SIZE;
                }
            }
        }
        *i = i2 - BRAINCODE_BYTES_PER_INSTRUCTION;
        break;
    }
    /** Goto if zero */
    case BRAINCODE_JMZ:
    {
        n_int v0 = is_const0;

        if ( v0 == 0 )
        {
            n_int i2 = ( *i + ( ( n_int ) is_const1 * BRAINCODE_BYTES_PER_INSTRUCTION ) ) % BRAINCODE_SIZE;

            if ( i2 <= *i )
            {
                if ( ( *i - i2 ) < braincode_min_loop )
                {
                    i2 = *i - braincode_min_loop;
                    if ( i2 < 0 )
                    {
                        i2 += BRAINCODE_SIZE;
                    }
                }
            }
            *i = i2 - BRAINCODE_BYTES_PER_INSTRUCTION;
        }
        break;
    }
    /** Goto if not zero */
    case BRAINCODE_JMN:
    {
        n_int v0 = is_const0;
        if ( v0 != 0 )
        {
            n_int i2 = ( *i + ( ( n_int ) is_const1 * BRAINCODE_BYTES_PER_INSTRUCTION ) ) % BRAINCODE_SIZE;
            if ( i2 <= *i )
            {
                if ( ( *i - i2 ) < braincode_min_loop )
                {
                    i2 = *i - braincode_min_loop;
                    if ( i2 < 0 )
                    {
                        i2 += BRAINCODE_SIZE;
                    }
                }
            }
            *i = i2 - BRAINCODE_BYTES_PER_INSTRUCTION;
        }
        break;
    }
    /** Goto and decrement if not zero */
    case BRAINCODE_DJN:
        if ( addr0[0] - 1 != 0 )
        {
            n_int i2 = ( *i + ( ( n_int ) is_const1 * BRAINCODE_BYTES_PER_INSTRUCTION ) ) % BRAINCODE_SIZE;
            addr0[0]--;

            if ( i2 <= *i )
            {
                if ( ( *i - i2 ) < braincode_min_loop )
                {
                    i2 = *i - braincode_min_loop;
                    if ( i2 < 0 )
                    {
                        i2 += BRAINCODE_SIZE;
                    }
                }
            }
            *i = i2 - BRAINCODE_BYTES_PER_INSTRUCTION;
        }
        break;
    /** If two values are equal then skip the next n instructions */
    case BRAINCODE_SEQ:
        if ( ( !is_constant0 ) && ( !is_constant1 ) )
        {
            if ( addr1[0] == addr0[0] )
            {
                *i = ( *i + ( BRAINCODE_BYTES_PER_INSTRUCTION * ( 1 + ( n_int )pspace[0] ) ) ) % BRAINCODE_SIZE;
            }
        }
        else
        {
            if ( addr1[0] == value0 )
            {
                *i = ( *i + ( BRAINCODE_BYTES_PER_INSTRUCTION * ( 1 + ( n_int )pspace[0] ) ) ) % BRAINCODE_SIZE;
            }
        }
        break;
    /** If two values are not equal then skip the next n instructions */
    case BRAINCODE_SNE:
        if ( ( !is_constant0 ) && ( !is_constant1 ) )
        {
            if ( addr1[0] != addr0[0] )
            {
                *i = ( *i + ( BRAINCODE_BYTES_PER_INSTRUCTION * ( 1 + ( n_int )pspace[0] ) ) ) % BRAINCODE_SIZE;
            }
        }
        else
        {
            if ( addr1[0] != value0 )
            {
                *i = ( *i + ( BRAINCODE_BYTES_PER_INSTRUCTION * ( 1 + ( n_int )pspace[0] ) ) ) % BRAINCODE_SIZE;
            }
        }
        break;
    /** Skip the next n instructions if less than */
    case BRAINCODE_SLT:
        if ( ( !is_constant0 ) && ( !is_constant1 ) )
        {
            if ( addr1[0] < addr0[0] )
            {
                *i = ( *i + ( BRAINCODE_BYTES_PER_INSTRUCTION * ( 1 + ( n_int )pspace[0] ) ) ) % BRAINCODE_SIZE;
            }
        }
        else
        {
            if ( addr1[0] < value0 )
            {
                *i = ( *i + ( BRAINCODE_BYTES_PER_INSTRUCTION * ( 1 + ( n_int )pspace[0] ) ) ) % BRAINCODE_SIZE;
            }
        }
        break;
    /** No operation (data only) */
    case BRAINCODE_DAT0:
    case BRAINCODE_DAT1:
        break;
    /** swap */
    case BRAINCODE_SWP:
    {
        n_byte tmp = addr0[0];
        addr0[0] = addr1[0];
        addr1[0] = tmp;
        break;
    }
    /** invert */
    case BRAINCODE_INV:
        if ( is_constant0 )
        {
            addr0[0] = 255 - addr0[0];
        }
        else
        {
            addr1[0] = 255 - addr1[0];
        }
        break;
    /** Save to Pspace */
    case BRAINCODE_STP:
    {
        n_byte v0 = ( n_byte )is_const0;
        n_byte v1 = ( n_byte )is_const1;
        pspace[v0 % BRAINCODE_PSPACE_REGISTERS] = v1;
        break;
    }
    /** Load from Pspace */
    case BRAINCODE_LTP:
    {
        n_byte v0 = ( n_byte )is_const0;
        addr1[0] = pspace[v0 % BRAINCODE_PSPACE_REGISTERS];
        break;
    }
    }
}


/**
 * @brief Returns a sensor value
 * @param group Pointer to the simulated_group object
 * @param meeter_being Pointer to the being doing the meeting
 * @param met_being Pointer to the being which was met
 * @param meeter_social_graph Pointer to the meeter being's social graph
 * @param actor_index Social graph index of the being which is the current focus of attention
 * @param switcher The type of sensor
 * @return Sensor value
 */
static n_byte brain_first_sense( simulated_group *group, simulated_being *meeter_being, simulated_being *met_being, simulated_isocial *meeter_social_graph, n_int actor_index, n_byte switcher )
{
    switch ( switcher % 32 )
    {
    case 0:
        return being_honor( meeter_being );
    case 1:
        return being_honor( met_being );
    case 2:
        return ( n_byte )being_parasites( meeter_being );
    case 3:
        return ( n_byte )being_parasites( met_being );
    case 4:
        return being_crowding( meeter_being );
    case 5:
        return being_family_first_name( meeter_being );
    case 6:
        return being_family_second_name( meeter_being );
    case 7:
        return being_family_first_name( met_being );
    case 8:
        return being_family_second_name( met_being );
    case 9:
        return being_facing( meeter_being );
    case 10:
        return being_facing( met_being );
    case 11:
        return being_speed( meeter_being );
    case 12:
        return meeter_social_graph[actor_index].familiarity & 255;
    case 13:
        return meeter_social_graph[actor_index].friend_foe;
    case 14:
        return meeter_social_graph[actor_index].attraction;
    /** Location */

    /*
        case 15:
            return (n_byte)(APESPACE_TO _MAPSPACE(being_location_x(meeter_being)) * 255 / land_map_dimension());
        case 16:
            return (n_byte)(APESPACE_TO _MAPSPACE(being_location_y(meeter_being)) * 255 / land_map_dimension());
     */
    /** Being state (lower)*/
    case 17:
        return ( n_byte )( being_state( meeter_being ) & 255 );
    /** Being state (upper)*/
    case 18:
        return ( n_byte )( ( being_state( meeter_being ) >> 8 ) & 255 );
    /** Drives */
    case 19:
        return ( n_byte )being_drive( meeter_being, DRIVE_HUNGER );
    case 20:
        return ( n_byte )being_drive( meeter_being, DRIVE_SOCIAL );
    case 21:
        return ( n_byte )being_drive( meeter_being, DRIVE_FATIGUE );
    case 22:
        return ( n_byte )being_drive( meeter_being, DRIVE_SEX );

    /** Sexisms */
    case 23:
        if ( FIND_SEX( GET_I( meeter_being ) ) == FIND_SEX( GET_I( met_being ) ) )
        {
            return 0;
        }
        else
        {
            return 255;
        }
        break;
    case 24:
        if ( FIND_SEX( GET_I( met_being ) ) == SEX_FEMALE )
        {
            return 255;
        }
        else
        {
            return 0;
        }
        break;
    case 25:
        if ( FIND_SEX( GET_I( met_being ) ) != SEX_FEMALE )
        {
            return 255;
        }
        else
        {
            return 0;
        }
    /** Overall grooming */
    case 26:
    {
        n_int v = 0;
        n_int n;
        for ( n = 0; n < INVENTORY_SIZE; n++ )
        {
            if ( met_being->changes.inventory[n] & INVENTORY_GROOMED )
            {
                v++;
            }
        }
        return ( n_byte ) ( v << 4 );
    }
    case 27:
    {
        n_int v = 0;
        n_int n;
        for ( n = 0; n < INVENTORY_SIZE; n++ )
        {
            if ( meeter_being->changes.inventory[n] & INVENTORY_GROOMED )
            {
                v++;
            }
        }
        return ( n_byte ) ( v << 4 );
    }

    /** Wounds */
    case 28:
    {
        n_int v = 0;
        n_int n;
        for ( n = 0; n < INVENTORY_SIZE; n++ )
        {
            if ( met_being->changes.inventory[n] & INVENTORY_WOUND )
            {
                v++;
            }
        }
        return ( n_byte ) ( v << 4 );
    }
    case 29:
    {
        n_int v = 0;
        n_int n;
        for ( n = 0; n < INVENTORY_SIZE; n++ )
        {
            if ( meeter_being->changes.inventory[n] & INVENTORY_WOUND )
            {
                v++;
            }
        }
        return ( n_byte ) ( v << 4 );
    }

    /** Posture */
    case 30:
        return being_posture( meeter_being );
    }
    /** case 31: */
    return being_posture( met_being );
}

static n_byte territory_familiarity( simulated_being *local_being,
                                     n_byte2 index )
{
    n_byte result = 0;
#ifdef TERRITORY_ON
    n_uint familiarity = ( n_uint )( local_being->events.territory[index].familiarity );
    n_uint i, max_familiarity = 1;

    /** find the maximum familiarity */
    for ( i = 0; i < TERRITORY_AREA; i++ )
    {
        if ( local_being->events.territory[i].familiarity > max_familiarity )
        {
            max_familiarity = ( n_uint )local_being->events.territory[i].familiarity;
        }
    }

    if ( max_familiarity == 0 )
    {
        return 0;
    }

    result = ( n_byte )( familiarity * 255 / max_familiarity );
#endif
    return result;
}

static void being_second_sense( simulated_group *group, n_byte addr00, n_byte *local_addr10, simulated_being *meeter_being, simulated_being *met_being, n_int actor_index, n_byte is_const1, n_int episode_index, simulated_iepisodic *episodic )
{
    n_int new_episode_index = -1;
    n_int switcher = addr00 % 25;

    n_vect2 location;

    simulated_isocial *meeter_social_graph = being_social( meeter_being );
    n_int territory_index = ( n_int )( being_attention( meeter_being, ATTENTION_TERRITORY ) );
    n_int memory_visited[EPISODIC_SIZE];
    n_int i;
    n_int relationship_index = ( n_int )( being_attention( meeter_being, ATTENTION_RELATIONSHIP ) );


    being_space( meeter_being, &location );
    spacetime_convert_to_map( &location );

    /** clear episodes visited.
     This array helps to avoid repeatedly visiting the same memories */
    for ( i = 0; i < EPISODIC_SIZE; i++ )
    {
        memory_visited[i] = 0;
    }
    switch ( switcher )
    {
    /** Shift attention to a different actor */
    case 0:
        actor_index = get_actor_index( meeter_social_graph, is_const1 % SOCIAL_SIZE );
        /** store the current focus of attention */
        being_set_attention( meeter_being, ATTENTION_ACTOR, actor_index );
        break;
    /** Shift attention to a different episode */
    case 1:
        new_episode_index = is_const1 % EPISODIC_SIZE;
        break;
    /** Shift attention to a different territory */
    case 2:
        territory_index = is_const1;
        being_set_attention( meeter_being, ATTENTION_TERRITORY, territory_index );
        break;
    /** Shift attention to a body region */
    case 3:
        being_set_attention( meeter_being, ATTENTION_BODY, is_const1 % INVENTORY_SIZE );
        break;
    case 4: /** Shift attention to a similar location */
        new_episode_index = attention_similar_place( episode_index, episodic, memory_visited );
        break;
    case 5: /** Shift attention to a similar time */
        new_episode_index = attention_similar_time( episode_index, episodic, memory_visited );
        break;
    case 6: /** Shift attention to a similar date */
        new_episode_index = attention_similar_date( episode_index, episodic, memory_visited );
        break;
    case 7: /** Shift attention to a similar name */
        new_episode_index = attention_similar_name( episode_index, episodic, meeter_being, memory_visited );
        break;
    case 8: /** Shift attention to a similar affect */
        new_episode_index = attention_similar_affect( episode_index, episodic, memory_visited );
        break;
    case 9:
        *local_addr10 = episodic[episode_index].event;
        break;
    case 10:
        *local_addr10 = episodic[episode_index].food;
        break;
    case 11:
        *local_addr10 = episodic[episode_index].affect & 255;
        break;
    case 12:
        *local_addr10 = episodic[episode_index].arg & 255;
        break;
    case 13:
        *local_addr10 = ( n_byte )( episodic[episode_index].space_time.location[0] * 255 / land_map_dimension() );
        break;
    case 14:
        *local_addr10 = ( n_byte )( episodic[episode_index].space_time.location[1] * 255 / land_map_dimension() );
        break;
    case 15:
    {
        /** atmosphere pressure */
        n_int pressure = weather_pressure( POSITIVE_LAND_COORD( location.x ), POSITIVE_LAND_COORD( location.y ) );

        if ( pressure > 100000 )
        {
            pressure = 100000;
        }
        if ( pressure < 0 )
        {
            pressure = 0;
        }
        *local_addr10 = ( n_byte )( pressure >> 9 );
        break;
    }
    case 16:
    {
        /** wind magnitude */
        n_vect2 wind;
        weather_wind_vector( &location, &wind );
        if ( wind.x < 0 )
        {
            wind.x = -wind.x;
        }
        if ( wind.y < 0 )
        {
            wind.y = -wind.y;
        }
        *local_addr10 = ( n_byte )( ( wind.x + wind.y ) >> 7 );
        break;
    }
    case 17:
        *local_addr10 = ( n_byte )( land_time() >> 3 );
        break;
    case 18:
        /** attention to body */
        *local_addr10 = being_attention( meeter_being, ATTENTION_BODY ) * 30;
        break;
    case 19:
#ifdef TERRITORY_ON
        /** territory name */
        *local_addr10 = meeter_being->events.territory[territory_index].name;
#endif
        break;
    case 20:
        /** territory familiarity */
        *local_addr10 = territory_familiarity( meeter_being, ( n_byte2 )territory_index );
        break;
    case 21:
        /** territory familiarity */
        *local_addr10 = territory_familiarity( met_being, ( n_byte2 )territory_index );
        break;
    case 22:
    {
        /** carrying object */
        n_byte2 carrying = being_carried( meeter_being, BODY_RIGHT_HAND );
        n_byte2 obj_type = 0;

        if ( carrying == 0 )
        {
            carrying = being_carried( meeter_being, BODY_LEFT_HAND );
        }
        if ( carrying != 0 )
        {
            /* TODO Is this willed into existence? */
            switch ( addr00 % 12 )
            {
            case 0:
                obj_type = INVENTORY_BRANCH;
                break;
            case 1:
                obj_type = INVENTORY_TWIG;
                break;
            case 2:
                obj_type = INVENTORY_ROCK;
                break;
            case 3:
                obj_type = INVENTORY_SHELL;
                break;
            case 4:
                obj_type = INVENTORY_GRASS;
                break;
            case 5:
                obj_type = INVENTORY_NUT;
                break;
            case 6:
                obj_type = INVENTORY_NUT_CRACKED;
                break;
            case 7:
                obj_type = INVENTORY_SCRAPER;
                break;
            case 8:
                obj_type = INVENTORY_SPEAR;
                break;
            case 9:
                obj_type = INVENTORY_FISH;
                break;
            case 10:
                obj_type = INVENTORY_BIRD_EGGS;
                break;
            case 11:
                obj_type = INVENTORY_LIZARD_EGGS;
                break;
            }
            if ( carrying & obj_type )
            {
                *local_addr10 = 255;
            }
            else
            {
                *local_addr10 = 0;
            }
        }

        break;
    }
    case 23:
    {
        /** shift attention to a given social graph entry based on relationship */
        n_int idx = social_get_relationship( meeter_being, ( n_byte )relationship_index );
        if ( idx > -1 )
        {
            actor_index = idx;
            /** store the current focus of attention */
            being_set_attention( meeter_being, ATTENTION_ACTOR, actor_index );
        }
        break;
    }
    case 24:
    {
        /** shift attention to a different relationship type */
        relationship_index = 1 + ( *local_addr10 % ( OTHER_MOTHER - 1 ) );
        /** store the current relationship attention */
        being_set_attention( meeter_being, ATTENTION_RELATIONSHIP, relationship_index );
        break;
    }
    }

    /** If attention has shifted to a new episode */
    if ( new_episode_index > -1 )
    {
        n_int possible_actor_index;
        episode_index = new_episode_index;
        being_set_attention( meeter_being, ATTENTION_EPISODE, episode_index );
        /** Shift attention to the being in this episode */
        possible_actor_index = get_actor_index_from_episode( meeter_social_graph, episodic, episode_index );
        if ( possible_actor_index > -1 )
        {
            actor_index = possible_actor_index;
            /** store the change in attention */
            being_set_attention( meeter_being, ATTENTION_ACTOR, actor_index );
        }
        /** set territory attention to the location where the episode occurred */
        being_set_attention( meeter_being, ATTENTION_TERRITORY,
                             ( APESPACE_TO_TERRITORY( episodic[episode_index].space_time.location[1] ) * 16 ) +
                             APESPACE_TO_TERRITORY( episodic[episode_index].space_time.location[0] ) );
    }
}

/**
 * @brief Returns a sensor value
 * @param group Pointer to the simulated_group object
 * @param meeter_being Pointer to the being doing the meeting
 * @param met_being Pointer to the being which was met
 * @param internal Non-zero if this is an internal dialogue
 * @param switcher The type of sensor
 * @param additional_write No operation value
 * @return Sensor value
 */
static n_byte brain_third_sense( simulated_group *group, simulated_being *meeter_being, simulated_being *met_being, n_byte internal, n_byte switcher, n_byte *additional_write )
{
    n_byte half_switcher = switcher >> 1;
    simulated_being *important_being = ( ( switcher & 1 ) ? met_being : meeter_being );
    n_genetics *genetics = being_genetics( important_being );
    switch ( half_switcher % 10 )
    {
    /** Facial characteristics.
          Here we shift the 0-15 gene values into a 0-255 range */
    case 0:
        return ( n_byte )( GENE_EYE_SHAPE( genetics ) << 4 );
    case 1:
        return ( n_byte )( GENE_EYE_COLOR( genetics ) << 4 );
    case 2:
        return ( n_byte )( GENE_EYE_SEPARATION( genetics ) << 4 );
    case 3:
        return ( n_byte )( GENE_NOSE_SHAPE( genetics ) << 4 );
    case 4:
        return ( n_byte )( GENE_EAR_SHAPE( genetics ) << 4 );
    case 5:
        return ( n_byte )( GENE_EYEBROW_SHAPE( genetics ) << 4 );
    case 6:
        return ( n_byte )( GENE_MOUTH_SHAPE( genetics ) << 4 );
    case 7:/* healthyness */
    {
        n_byte return_value = 0;
#ifdef IMMUNE_ON
        n_int n;
        simulated_immune_system *immune = &( important_being->immune_system );
        return_value = immune->antigens[0];
        for ( n = 1; n < IMMUNE_ANTIGENS; n++ )
        {
            if ( immune->antigens[n] > return_value )
            {
                return_value = immune->antigens[n];
            }
        }
#endif
        return return_value;
    }
    /** the 8 case is covered in the default: */
    case 9: /** listen for shouts */
        if ( ( internal != 0 ) &&
                ( !( being_state( meeter_being )&BEING_STATE_SHOUTING ) ) &&
                ( !( being_state( meeter_being )&BEING_STATE_SPEAKING ) ) &&
                ( meeter_being->changes.shout[SHOUT_HEARD] > 0 ) )
        {
            return meeter_being->changes.shout[SHOUT_HEARD];
        }
        break;
        break;
    /** listen for name shouts */
    default:
        if ( switcher == 16 ) /** positive affect */
        {
            n_uint positive = being_affect( meeter_being, 1 ) >> 7;
            if ( positive > 255 )
            {
                positive = 255;
            }
            return ( n_byte )positive;

        }
        {
            /* (switcher == 17) negative affect */

            n_uint negative = being_affect( meeter_being, 0 ) >> 1;
            if ( negative > 255 )
            {
                negative = 255;
            }
            return ( n_byte )negative;
        }
    }
    return additional_write[0]; /** no op case. Not sure if the compiler will recognize that though */
}

static void brain_first_action( simulated_group *group, n_byte awake,
                                n_byte *local_addr00, n_byte *local_addr10,
                                simulated_being *meeter_being, simulated_being *met_being, n_int episode_index,
                                simulated_iepisodic *episodic, n_byte pspace0, n_int actor_index,
                                simulated_isocial *meeter_social_graph, n_byte is_const1 )
{
    switch ( *local_addr00 % 6 )
    {
    /** individual or social action */
    case 0:
        if ( ( awake != 0 ) && ( *local_addr00 > 127 ) )
        {
            if ( meeter_being == met_being )
            {
                social_action( meeter_being, 0L, *local_addr10 );
            }
            else
            {
                social_action( meeter_being, met_being, *local_addr10 );
            }

            *local_addr00 = 0;
        }
        break;
    /** Set location goal */
    case 1:
        if ( !( meeter_being->braindata.script_overrides & OVERRIDE_GOAL ) )
        {
            being_set_goal_location( meeter_being,
                                     episodic[episode_index].space_time.location[0],
                                     episodic[episode_index].space_time.location[1] );
        }
        break;
    /** alter friend or foe value */
    case 2:
    {
        n_byte fof0 = pspace0;
        n_byte fof1 = *local_addr10;

        if ( fof0 > ( n_byte )( fof1 + 85 ) )
        {
            if ( meeter_social_graph[actor_index].friend_foe < 170 )
            {
                meeter_social_graph[actor_index].friend_foe++;
            }
        }
        if ( fof1 > ( n_byte )( fof0 + 85 ) )
        {
            if ( meeter_social_graph[actor_index].friend_foe > 85 )
            {
                meeter_social_graph[actor_index].friend_foe--;
            }
        }
        break;
    }
    /** alter attraction */
    case 3:
    {
        n_byte att0 = *local_addr10;
        n_byte att1 = pspace0;

        if ( att0 > ( n_byte )( att1 + 85 ) )
        {
            if ( meeter_social_graph[actor_index].attraction < 255 )
            {
                meeter_social_graph[actor_index].attraction++;
            }
        }
        if ( att1 > ( n_byte )( att0 + 85 ) )
        {
            if ( meeter_social_graph[actor_index].attraction > 16 )
            {
                meeter_social_graph[actor_index].attraction--;
            }
        }
        break;
    }
    /** alter familiarity */
    case 4:
        /** The values 10 and 20 meetings were just found experimentally */
        if ( ( *local_addr10 > 100 ) && ( *local_addr10 < 150 ) )
        {
            if ( meeter_social_graph[actor_index].familiarity < 65535 )
            {
                meeter_social_graph[actor_index].familiarity++;
                *local_addr10 = 0;
            }
        }
        if ( ( *local_addr10 > 150 ) && ( *local_addr10 < 200 ) )
        {
            if ( meeter_social_graph[actor_index].familiarity > 10 )
            {
                meeter_social_graph[actor_index].familiarity--;
                *local_addr10 = 0;
            }
        }
        break;
    /** brainprobe frequency */
    case 5:
    {
        n_int  n = pspace0 % BRAINCODE_PROBES;
        n_byte f = 1 + ( is_const1 % BRAINCODE_MAX_FREQUENCY );

        meeter_being->braindata.brainprobe[n].frequency = f;
        break;
    }
    }
}

#define IS_CONST0 (is_constant0 ? value0 : addr0[0])
#define IS_CONST1 (is_constant1 ? value1 : addr1[0])

/**
 * @brief Two beings meet and chat, or a being engages in an internal dialogue
 * @param group Pointer to the simulated_group object
 * @param awake Whether the being is awake
 * @param meeter_being Pointer to the being doing the meeting
 * @param met_being Pointer to the being which was met
 * @param bc0 Braincode of the meeter
 * @param bc1 Braincode of the met
 * @param being_index Social graph index of the being which is the current focus of attention
 */
void brain_dialogue(
    simulated_group *group,
    n_byte awake,
    simulated_being *meeter_being,
    simulated_being *met_being,
    n_byte *bc0,
    n_byte *bc1,
    n_int being_index )
{
#ifdef EPISODIC_ON
    n_byte internal = ( meeter_being == met_being );
    const n_int braincode_min_loop = 8 * BRAINCODE_BYTES_PER_INSTRUCTION;
    n_int i = 0, itt = 0;
    n_int actor_index;
    n_int episode_index = ( n_int )( being_attention( meeter_being, ATTENTION_EPISODE ) );
    n_int anecdote_episode_index = -1;
    n_int intention_episode_index = -1;

    simulated_isocial *meeter_social_graph = being_social( meeter_being );
    simulated_iepisodic *episodic = being_episodic( meeter_being );
    n_int max_itterations;
    n_byte *pspace = ( n_byte * )meeter_being->braindata.braincode_register;

    /* what is the current actor index within episodic memory? */
    if ( being_index > -1 )
    {
        actor_index = being_index;
    }
    else
    {
        being_index = being_attention( meeter_being, ATTENTION_ACTOR );
        actor_index = being_index;
    }

    if ( meeter_being == met_being )
    {
        /** fixed number of itterations for internal dialogue */
        max_itterations = BRAINCODE_MAX_ADDRESS / BRAINCODE_BYTES_PER_INSTRUCTION;
    }
    else
    {
        /** variable number of itterations for chat */
        max_itterations = 8 + meeter_being->changes.learned_preference[PREFERENCE_CHAT];
    }

    i = 0;

    while ( itt < max_itterations )
    {
        n_byte instruction = BRAINCODE_INSTRUCTION( bc0, i );
        n_byte is_constant0 = BRAINCODE_CONSTANT0( bc0, i );
        n_byte is_constant1 = BRAINCODE_CONSTANT1( bc0, i );
        n_byte value0 = BRAINCODE_VALUE( bc0, i, 0 );
        n_byte value1 = BRAINCODE_VALUE( bc0, i, 1 );
        n_byte *addr0 = math_general_allocation( bc0, bc1, i + value0 );
        n_byte *addr1 = math_general_allocation( bc0, bc1, i + value1 );

        switch ( instruction )
        {
        /** General sensor */
        case BRAINCODE_SEN:
        {
            addr1[0] = brain_first_sense( group, meeter_being, met_being, meeter_social_graph, actor_index, addr0[0] );
            break;
        }
        case BRAINCODE_SEN2:
        {
            being_second_sense( group, addr0[0], &addr1[0], meeter_being, met_being, actor_index, IS_CONST1, episode_index, episodic );
            break;
        }
        case BRAINCODE_SEN3:

            addr1[0] = brain_third_sense( group, meeter_being, met_being, internal, addr0[0], addr1 );
            break;
        /** Action */
        case BRAINCODE_ACT:
        {

            brain_first_action( group, awake, &addr0[0], &addr1[0], meeter_being, met_being, episode_index,
                                episodic, pspace[0], actor_index, meeter_social_graph, IS_CONST1 );
            break;
        }
        case BRAINCODE_ACT2:
        {
            switch ( addr0[0] % 6 )
            {
            case 0: /* brainprobe type */
            {
                n_int  n = pspace[0] % BRAINCODE_PROBES;
                n_byte typ = IS_CONST1 & 1;

                meeter_being->braindata.brainprobe[n].type = typ;
                break;
            }
            case 1: /** brainprobe address */
            {
                n_int n = pspace[0] % BRAINCODE_PROBES;
                n_byte adr = IS_CONST1;

                meeter_being->braindata.brainprobe[n].address = adr;
                break;
            }
            case 2: /** shout out */
            {
                n_byte msg = addr1[0];
                if ( is_constant1 )
                {
                    msg = value1;
                }
                if ( ( internal != 0 ) && ( awake != 0 ) &&
                        ( !( being_state( meeter_being )&BEING_STATE_SHOUTING ) ) &&
                        ( !( being_state( meeter_being )&BEING_STATE_SPEAKING ) ) &&
                        ( meeter_being->changes.shout[SHOUT_CONTENT] == 0 ) &&
                        ( meeter_being->changes.shout[SHOUT_HEARD] == 0 ) &&
                        ( meeter_being->changes.shout[SHOUT_CTR] == 0 ) &&
                        ( msg > 0 ) )
                {
                    meeter_being->changes.shout[SHOUT_CTR] = SHOUT_REFRACTORY;
                    being_add_state( meeter_being, BEING_STATE_SHOUTING );
                    /** volume of message */
                    meeter_being->changes.shout[SHOUT_VOLUME] = pspace[0];
                    /** type of message */
                    meeter_being->changes.shout[SHOUT_CONTENT] = msg;
                }
                break;
            }
            case 3: /** intention */
            {
                if ( intention_episode_index != episode_index )
                {
                    n_byte v0 = pspace[0];
                    n_byte v1 = IS_CONST1;
                    if ( episodic_intention( meeter_being, episode_index, ( n_byte2 )( v0 * 10 ), v1 ) != 0 )
                    {
                        intention_episode_index = episode_index;
                    }
                }
                break;
            }
            case 4: /** brainprobe offset */
            {
                n_int n = pspace[0] % BRAINCODE_PROBES;
                n_byte offset = IS_CONST1;

                meeter_being->braindata.brainprobe[n].offset = offset;
                break;
            }
            case 5: /** posture */
                if ( awake != 0 )
                {
                    being_set_posture( meeter_being, addr1[0] );
                }
                break;
            }
            break;
        }
        case BRAINCODE_ACT3:
            switch ( addr0[0] % 2 )
            {
            /** brainprobe position */
            case 0:
            {
                n_int  n = pspace[0] % BRAINCODE_PROBES;
                n_byte p = IS_CONST1;
                meeter_being->braindata.brainprobe[n].position = p;
                break;
            }
            /** alter learned preferences */
            case 1:
            {
                n_int n;
                n_byte prf = addr1[0];

                n = pspace[0] % PREFERENCES;

                if ( ( prf > 55 ) && ( prf < 155 ) )
                {
                    if ( meeter_being->changes.learned_preference[n] < 255 )
                    {
                        meeter_being->changes.learned_preference[n]++;
                        addr1[0] = 0;
                    }
                }
                if ( prf >= 155 )
                {
                    if ( meeter_being->changes.learned_preference[n] > 0 )
                    {
                        meeter_being->changes.learned_preference[n]--;
                        addr1[0] = 0;
                    }
                }
                break;
            }
            break;
            }
        /** spread anecdote */
        case BRAINCODE_ANE:
            if ( internal == 0 )
            {
                /** not internal dialogue */
                /** avoid repeated anecdotes in the same conversation */
                if ( anecdote_episode_index != episode_index )
                {
                    if ( episodic_anecdote( meeter_being, met_being ) != 0 )
                    {
                        anecdote_episode_index = episode_index;
                    }
                }
            }
            break;

        default:
            math_general_execution( instruction, is_constant0, is_constant1,
                                    addr0, addr1, value0, &i,
                                    IS_CONST0, IS_CONST1,
                                    pspace,
                                    bc0, bc1,
                                    braincode_min_loop );
            break;


        }
        i += BRAINCODE_BYTES_PER_INSTRUCTION;
        itt++;

        if ( i >= BRAINCODE_SIZE )
        {
            i -= BRAINCODE_SIZE;
        }
    }
#endif
}

#endif


