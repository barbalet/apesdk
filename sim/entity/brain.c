/****************************************************************

 brain.c

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


/*NOBLEMAKE DEL=""*/

#include "entity.h"
#include "entity_internal.h"

#include <stdio.h>

/*NOBLEMAKE END=""*/

/*NOBLEMAKE VAR=""*/


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

/*NOBLEMAKE END=""*/

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
void brain_cycle(n_byte * local, n_byte2 * constants)
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
    
    io_copy(bract, br, B_SIZE);

    do
    {
        average = (B_P_LH + B_N_UH);
        br_tmp = br[loc];
        obr_tmp = obr[loc];

        average *= l_a;
        obr_tmp *= l_c;
        
        br_tmp *= l_b;        
        br_tmp -= obr_tmp;
        average += br_tmp;

        br[loc++] = (n_byte)(average>>10);
        count--;
                
    }while(count);
    count = B_Z - F_Z;
    do
    {
        average =  br[loc-F_Z];
        average += br[loc-F_Y];
        average += br[loc-F_X];
        
        br_tmp = br[loc]; 
        
        average += br[loc+F_X];
        average += br[loc+F_Y];
        average += br[loc+F_Z];
        
        obr_tmp = obr[loc];
        average *= l_a;
        obr_tmp *= l_c;
        br_tmp *= l_b;
        br_tmp -= obr_tmp;
        average += br_tmp;
        br[loc++] = (n_byte)(average>>10);
        count--;
    }while (count);
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

        br[loc++] = (n_byte)(average>>10);
        count--;
    }while (count);

    io_copy(bract, obr, B_SIZE);
    io_copy(br, bract, B_SIZE);

}

void brain_hash(n_byte *local_brain, n_byte * local_brain_hash)   /**< TODO THIS IS WRONG! FIX!*/
{
    const n_byte	*known_values = (n_byte *)"0123456789ABCDEF";
    n_uint	hash_value = 0;
    local_brain_hash[0]  = known_values[ ((hash_value >> 0) & 15 ) ];
    local_brain_hash[1]  = known_values[ ((hash_value >> 4) & 15) ];
    local_brain_hash[2]  = ' ';
    local_brain_hash[3]  = known_values[ ((hash_value >> 8) & 15) ];
    local_brain_hash[4]  = known_values[ ((hash_value >> 12) & 15) ];
    local_brain_hash[5]  = ' ';
    local_brain_hash[6]  = known_values[ ((hash_value >> 16) & 15) ];
    local_brain_hash[7]  = known_values[ ((hash_value >> 20) & 15) ];
    local_brain_hash[8]  = ' ';
    local_brain_hash[9]  = known_values[ ((hash_value >> 24) & 15) ];
    local_brain_hash[10] = known_values[ ((hash_value >> 28) & 15) ];

    local_brain_hash[11] = 0;
}



/**     "XXX_#NNN_#NNN"*/
/**     "0123456789012"*/

#define BC_FORMAT_A      0
#define BC_FORMAT_C      1
#define BC_FORMAT_E      2
#define BC_FORMAT_F      3
#define BC_FORMAT_G      4
#define BC_FORMAT_H      5

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
static n_int brain_format(n_byte instruction, n_byte command, n_byte value0, n_byte value1)
{
    n_byte is_constant0 = ((command & BRAINCODE_CONSTANT0_BIT) != 0);
    n_byte is_constant1 = ((command & BRAINCODE_CONSTANT1_BIT) != 0);
    
    switch(instruction)
    {
        case BRAINCODE_AND:
        case BRAINCODE_OR:
        case BRAINCODE_MOV:
        case BRAINCODE_ADD:
        case BRAINCODE_SUB:
        case BRAINCODE_MUL:
        case BRAINCODE_MOD:
            if ((!is_constant0) && (!is_constant1))
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
            if ((!is_constant0) && (!is_constant1))
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
            if (is_constant0)
            {
                return BC_FORMAT_G;
            }

            return BC_FORMAT_H;

            break;
        case BRAINCODE_STP:
        case BRAINCODE_LTP:
            if ((is_constant0) && (!is_constant1))
            {
                return BC_FORMAT_F;
            }
            if ((is_constant0) && (is_constant1))
            {
                return BC_FORMAT_C;
            }
            if ((!is_constant0) && (is_constant1))
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

static n_byte brain_vc(n_byte value, n_byte vowel)
{
    if (vowel)
    {
        switch (value)
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
    switch (value)
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

static void brain_longword(n_string output, n_byte value)
{
    output[0] = brain_vc((value >> 0) & 7,0);
    output[1] = brain_vc((value >> 3) & 3,1);
    output[2] = brain_vc((value >> 5) & 7,0);
    output[4] = 0;
}

/**
 * @brief prints a three byte instruction in the appropriate format
 * @param string The returned string
 * @param response Array containing the three byte instruction
 */
void brain_three_byte_command(n_string string, n_byte * response)
{
    n_byte command      = response[0];
    n_byte value0       = response[1];
    n_byte value1       = response[2];
    n_byte instruction  = (command & (BRAINCODE_CONSTANT0_BIT-1)) % BRAINCODE_INSTRUCTIONS;
    n_int  format       = brain_format(instruction, command, value0, value1);
    
    switch(format)
    {
        case BC_FORMAT_A:
            sprintf(string,"%s  %03d  %03d", braincode_mnemonic[instruction], value0, value1);
            break;
        case BC_FORMAT_C:
            sprintf(string,"%s *%03d *%03d", braincode_mnemonic[instruction], value0, value1);
            break;
        case BC_FORMAT_E:
            sprintf(string,"%s  %03d *%03d", braincode_mnemonic[instruction], value0, value1);
            break;
        case BC_FORMAT_F:
            sprintf(string,"%s *%03d  %03d", braincode_mnemonic[instruction], value0, value1);
            break;
        case BC_FORMAT_G:
            sprintf(string,"%s  %03d  ---", braincode_mnemonic[instruction], value0);
            break;
        default:
            sprintf(string,"%s  ---  %03d", braincode_mnemonic[instruction], value1);
            break;
    }
}

/**
 * @brief A simple sort of sentence construction from three byte instructions
 * @param string The returned sentence
 * @param response Array containing three byte instruction
 */
void brain_sentence(n_string string, n_byte * response)
{
    n_byte command      = response[0];
    n_byte value0       = response[1];
    n_byte value1       = response[2];
    n_byte instruction  = (command & (BRAINCODE_CONSTANT0_BIT-1)) % BRAINCODE_INSTRUCTIONS;
    n_int  format       = brain_format(instruction, command, value0, value1);
    n_string_block      first_word, second_word;
    
    brain_longword(first_word, value0);
    brain_longword(second_word, value1);
    
    switch(format)
    {
        case BC_FORMAT_A:
            sprintf(string,"%s %s%s", braincode_spoken_dictionary[instruction], first_word, second_word);
            break;
        case BC_FORMAT_C:
            sprintf(string,"%s %s%s", braincode_spoken_dictionary[instruction], first_word, second_word);
            break;
        case BC_FORMAT_E:
            sprintf(string,"%s %s%s", braincode_spoken_dictionary[instruction], first_word, second_word);
            break;
        case BC_FORMAT_F:
            sprintf(string,"%s %s%s", braincode_spoken_dictionary[instruction], first_word, second_word);
            break;
        case BC_FORMAT_G:
            sprintf(string,"%s %s", braincode_spoken_dictionary[instruction], first_word);
            break;
        default:
            sprintf(string,"%s %s", braincode_spoken_dictionary[instruction], second_word);
            break;
    }
}

#ifdef BRAINCODE_ON

/**
 * @brief gathers statistics on braincode instructions
 * @param sim Pointer to the simulation object
 */
void braincode_statistics(noble_simulation * sim)
{
    n_int i,j,k,instruction;
    noble_being * local_being;
    n_uint sensors = 0;
    n_uint actuators = 0;
    n_uint operators = 0;
    n_uint conditionals = 0;
    n_uint data = 0;

    for (i=0; i<(n_int)(sim->num); i++)
    {
        local_being = &(sim->beings[i]);
        for (j=0; j<BRAINCODE_SIZE; j+=BRAINCODE_BYTES_PER_INSTRUCTION)
        {
            for (k=0; k<2; k++)
            {
                if (k==0)
                {
                    instruction = GET_BRAINCODE_INTERNAL(sim,local_being)[j] % BRAINCODE_INSTRUCTIONS;
                }
                else
                {
                    instruction = GET_BRAINCODE_EXTERNAL(sim,local_being)[j] % BRAINCODE_INSTRUCTIONS;
                }
                
                if (instruction < BRAINCODE_OPERATORS_START)
                {
                    data++;
                }
                else if (instruction < BRAINCODE_CONDITIONALS_START)
                {
                    operators++;
                }
                else if (instruction < BRAINCODE_SENSORS_START)
                {
                    conditionals++;
                }
                else if (instruction < BRAINCODE_ACTUATORS_START)
                {
                    sensors++;
                }
                else
                {
                    actuators++;
                }
            }
        }
    }
    if (sim->num>0)
    {
        INDICATOR_SET(sim, IT_AVERAGE_SENSORS, sensors);
        INDICATOR_SET(sim, IT_AVERAGE_ACTUATORS, actuators);
        INDICATOR_SET(sim, IT_AVERAGE_OPERATORS, operators);
        INDICATOR_SET(sim, IT_AVERAGE_CONDITIONALS, conditionals);
        INDICATOR_SET(sim, IT_AVERAGE_DATA, data);
        
        INDICATOR_MULTIPLY(sim, IT_AVERAGE_SENSORS, 10);
        INDICATOR_MULTIPLY(sim, IT_AVERAGE_ACTUATORS, 10);
        INDICATOR_MULTIPLY(sim, IT_AVERAGE_OPERATORS, 10);
        INDICATOR_MULTIPLY(sim, IT_AVERAGE_CONDITIONALS, 10);
        INDICATOR_MULTIPLY(sim, IT_AVERAGE_DATA, 10);
                
        INDICATOR_NORMALIZE(sim, IT_AVERAGE_SENSORS);
        INDICATOR_NORMALIZE(sim, IT_AVERAGE_ACTUATORS);
        INDICATOR_NORMALIZE(sim, IT_AVERAGE_OPERATORS);
        INDICATOR_NORMALIZE(sim, IT_AVERAGE_CONDITIONALS);
        INDICATOR_NORMALIZE(sim, IT_AVERAGE_DATA);
    }
}

/**
 * @brief returns a random braincode instruction of the given type
 * @param instruction_type Number indicating the type of instruction
 * @return braincode instruction
 */
static n_byte get_braincode_instruction_type(n_byte instruction_type)
{
    n_byte2 local_random[2];
    
    math_random3(local_random);
    switch(instruction_type)
    {
    case 0: /**< GENE_BRAINCODE_SENSORS(genetics);*/
        return BRAINCODE_DATA_START+(local_random[0]%(BRAINCODE_DATA_NUMBER));
    case 1: /**< GENE_BRAINCODE_ACTUATORS(genetics); */
        return BRAINCODE_SENSORS_START+(local_random[0]%(BRAINCODE_SENSORS_NUMBER));
    case 2: /**< GENE_BRAINCODE_CONDITIONALS(genetics); */
        return BRAINCODE_ACTUATORS_START+(local_random[0]%(BRAINCODE_ACTUATORS_NUMBER));
    case 3: /**< GENE_BRAINCODE_OPERATORS(genetics); */
        return BRAINCODE_OPERATORS_START+(local_random[0]%(BRAINCODE_OPERATORS_NUMBER));
    case 4: /**< GENE_BRAINCODE_DATA(genetics); */
        return BRAINCODE_CONDITIONALS_START+(local_random[0]%(BRAINCODE_CONDITIONALS_NUMBER));
    }

    return BRAINCODE_DATA_START;
}

/**
 * @brief returns a random braincode instruction
 * @param local_being Pointer to the being
 * @return braincode instruction
 */
n_byte get_braincode_instruction(noble_being * local_being)
{
    n_byte2 prob[5], total, index;
    n_genetics * genetics = being_genetics(local_being);
    n_byte  i;
    const n_byte2 min=2;

    prob[0] = min + GENE_BRAINCODE_SENSORS(genetics);
    prob[1] = min + GENE_BRAINCODE_ACTUATORS(genetics);
    prob[2] = min + GENE_BRAINCODE_CONDITIONALS(genetics);
    prob[3] = min + GENE_BRAINCODE_OPERATORS(genetics);
    prob[4] = min + GENE_BRAINCODE_DATA(genetics);

    total = prob[0] + prob[1] + prob[2] + prob[3] + prob[4];

    math_random3(local_being->seed);
    if (total == 0)
    {
        index = 0;
    }
    else
    {
        index = (local_being->seed[0])%total;
    }
    total = 0;
    for (i=0; i<5; i++,total+=prob[i])
    {
        if (index>=total)
        {
            return get_braincode_instruction_type(i);
        }
    }

    return get_braincode_instruction_type(4);
}

/**
 * @brief
 * @param social_graph
 * @param value
 * @return
 */
static n_int get_actor_index(social_link * social_graph, n_int value)
{
    n_int i;

    for (i=0; i<SOCIAL_SIZE; i++)
    {
        if (SOCIAL_GRAPH_ENTRY_EMPTY(social_graph,i)) break;
    }

    if (i == 0) return 0;

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
    social_link * social_graph,
    episodic_memory * episodic_event,
    n_int episode_index)
{
    n_int i,actor_index=-1;

    for (i=1; i<SOCIAL_SIZE; i++)
    {
        if (!SOCIAL_GRAPH_ENTRY_EMPTY(social_graph,i))
        {
            if (social_graph[i].family_name == episodic_event[episode_index].family_name)
            {
                actor_index = i;
                if (social_graph[i].first_name == episodic_event[episode_index].first_name)
                {
                    actor_index = i;
                    break;
                }
            }
        }
    }
    return actor_index;
}

typedef n_int (n_similar)(episodic_memory * episodic, n_int * carry_through);

/**
 * @brief Returns the index of the most similar episodic memory
 * @param episode_index Current episodic memory index
 * @param episodic Pointer to the episodic memory
 * @param memory_visited Array which keeps track of which episodic memories have already been visited
 * @param carry_through The property to be compared against
 * @param function The similarity function to be used to do the comparrisson
 * @return Episodic memory index of the most similar event
 */
static n_int attention_similar(n_int episode_index,
                               episodic_memory * episodic,
                               n_int * memory_visited,
                               n_int * carry_through,
                               n_similar function)
{
    n_int i;
    n_int visited_max = memory_visited[episode_index] - (EPISODIC_SIZE>>1);
    n_int min=-1;
    n_int next_episode_index = -1;
    if (visited_max<0) visited_max=0;
    
    for (i = 0; i < EPISODIC_SIZE; i++)
    {
        if (episodic[i].event == 0) continue;
        
        if (i != episode_index)
        {
            /** was this episode recently visited? */
            if (memory_visited[i] <= visited_max)
            {
                /** difference between episodes */
                n_int diff = function(&episodic[i], carry_through);
                if (diff < 0) diff = -diff;
                if ((min == -1) || (diff < min))
                {
                    /** record most similar */
                    min = diff;
                    next_episode_index = i;
                }
            }
        }
    }
    if (next_episode_index>-1)
    {
        /** mark this episode as having been visited */
        memory_visited[next_episode_index] = memory_visited[episode_index]+1;
    }
    return next_episode_index;
}

/**
 * @brief returns the temporal proximity of an episodic event to the given reference time
 * @param episodic Pointer to the episodic memory event
 * @param carry_through Time to be compared against
 * @return Absolute temporal proximity
 */
static n_int similar_time(episodic_memory * episodic, n_int * carry_through)
{
    n_int dt = episodic->time - carry_through[0];
    if (dt < 0)
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
static n_int attention_similar_time(n_int episode_index,
                                    episodic_memory * episodic,
                                    n_int * memory_visited)
{
    n_int time = episodic[episode_index].time;
    return attention_similar(episode_index, episodic, memory_visited, &time, similar_time);
}

/**
 * @brief Returns the affective similarity of the given affect value to an episodic memory event
 * @param episodic Pointer to the episodic memory event
 * @param carry_through Affect value to be compared against
 * @return Absolute difference in affect value
 */
static n_int similar_affect(episodic_memory * episodic, n_int * carry_through)
{
    n_int da = episodic->affect - carry_through[0];
    if (da < 0)
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
static n_int attention_similar_affect(n_int episode_index,
                                    episodic_memory * episodic,
                                    n_int * memory_visited)
{
    n_int affect = episodic[episode_index].affect;
    return attention_similar(episode_index, episodic, memory_visited, &affect, similar_affect);
}

/**
 * @brief Returns the similarity between a given episodic memory event and the given family name
 * @param episodic Pointer to the episodic memory event
 * @param carry_through Family name to be compared against
 * @return Similarity value in the range 0-3
 */
static n_int similar_name(episodic_memory * episodic, n_int * carry_through)
{
    n_int similarity = 3;
    
    if (UNPACK_FAMILY_FIRST_NAME(episodic->family_name[BEING_MET]) == carry_through[0]) similarity--;
    if (UNPACK_FAMILY_SECOND_NAME(episodic->family_name[BEING_MET]) == carry_through[1]) similarity--;
    if (episodic->first_name[BEING_MET] == carry_through[2]) similarity--;
    return similarity;
}

/**
 * @brief Returns the similarity between a given episodic memory event and the given being name
 * @param episode_index Index of the current focus of attention within the episodic memory
 * @param episodic Pointer to the episodic memory
 * @param memory_visited Affect value to be compared against
 * @return Similarity value
 */
static n_int attention_similar_name(n_int episode_index,
                                      episodic_memory * episodic,
                                      n_int * memory_visited)
{
    n_int name[3];
    
    name[0] = UNPACK_FAMILY_FIRST_NAME(episodic[episode_index].family_name[BEING_MET]);
    name[1] = UNPACK_FAMILY_SECOND_NAME(episodic[episode_index].family_name[BEING_MET]);
    name[2] = episodic[episode_index].first_name[BEING_MET];
    
    return attention_similar(episode_index, episodic, memory_visited, name, similar_name);
}

/**
 * @brief Returns the similarity of a given date to the date of the given episodic memory event
 * @param episodic Pointer to the episodic memory event
 * @param carry_through Date to be compared against
 * @return Absolute difference in date value
 */
static n_int similar_date(episodic_memory * episodic, n_int * carry_through)
{
    n_int dd = TIME_IN_DAYS(episodic->date) - carry_through[0];
    if (dd < 0)
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
static n_int attention_similar_date(n_int episode_index,
                                    episodic_memory * episodic,
                                    n_int * memory_visited)
{
    n_int time = TIME_IN_DAYS(&episodic[episode_index].date[0]);
    return attention_similar(episode_index, episodic, memory_visited, &time, similar_date);
}

/**
 * @brief Returns the similarity between the given episodic memory event place and a given reference location
 * @param episodic Pointer to the episodic memory event
 * @param carry_through 2D coordinate of the place to be compared against
 * @return Similarity value (2D distance squared)
 */
static n_int similar_place(episodic_memory * episodic, n_int * carry_through)
{
    n_int dx = episodic->location[0] - carry_through[0];
    n_int dy = episodic->location[1] - carry_through[1];
    /** TODO should be calculated in the future with wrap around comparison */
    n_int da = (dx * dx) + (dy * dy);
    return da;
}

/**
 * @brief Returns the similarity between the given episodic memory event place and a given reference location
 * @param episode_index Index of the episodic memory which is the current focus of attention
 * @param episodic Pointer to the episodic memory
 * @param memory_visited 2D coordinate of the place to be compared against
 * @return Similarity value (2D distance squared)
 */
static n_int attention_similar_place(n_int episode_index,
                                    episodic_memory * episodic,
                                    n_int * memory_visited)
{
    n_int location[2];
    location[0] = episodic[episode_index].location[0];
    location[1] = episodic[episode_index].location[1];
    return attention_similar(episode_index, episodic, memory_visited, location, similar_place);
}

/**
 * @brief Returns a sensor value
 * @param sim Pointer to the simulation object
 * @param meeter_being Pointer to the being doing the meeting
 * @param met_being Pointer to the being which was met
 * @param meeter_social_graph Pointer to the meeter being's social graph
 * @param actor_index Social graph index of the being which is the current focus of attention
 * @param switcher The type of sensor
 * @return Sensor value
 */
static n_byte brain_first_sense(noble_simulation * sim, noble_being * meeter_being, noble_being * met_being, social_link * meeter_social_graph, n_int actor_index, n_byte switcher)
{
    switch (switcher % 32)
    {
    case 0:
        return meeter_being->honor;
    case 1:
        return met_being->honor;
    case 2:
        return meeter_being->parasites;
    case 3:
        return met_being->parasites;
    case 4:
        return meeter_being->crowding;
    case 5:
        return GET_FAMILY_FIRST_NAME(sim,meeter_being);
    case 6:
        return GET_FAMILY_SECOND_NAME(sim,meeter_being);
    case 7:
        return GET_FAMILY_FIRST_NAME(sim,met_being);
    case 8:
        return GET_FAMILY_SECOND_NAME(sim,met_being);
    case 9:
        return being_facing(meeter_being);
    case 10:
        return being_facing(met_being);
    case 11:
        return being_speed(meeter_being);
    case 12:
        return meeter_social_graph[actor_index].familiarity&255;
    case 13:
        return meeter_social_graph[actor_index].friend_foe;
    case 14:
        return meeter_social_graph[actor_index].attraction;
        /** Location */
    case 15:
        return (n_byte)(APESPACE_TO_MAPSPACE(being_location_x(meeter_being)) * 255 / MAP_DIMENSION);
    case 16:
        return (n_byte)(APESPACE_TO_MAPSPACE(being_location_y(meeter_being)) * 255 / MAP_DIMENSION);
        /** Being state (lower)*/
    case 17:
        return (n_byte)meeter_being->state&255;
        /** Being state (upper)*/
    case 18:
        return (n_byte)(meeter_being->state>>8)&255;
        /** Drives */
    case 19:
        return (n_byte)being_drive(meeter_being, DRIVE_HUNGER);
    case 20:
        return (n_byte)being_drive(meeter_being, DRIVE_SOCIAL);
    case 21:
        return (n_byte)being_drive(meeter_being, DRIVE_FATIGUE);
    case 22:
        return (n_byte)being_drive(meeter_being, DRIVE_SEX);

        /** Sexisms */
    case 23:
        if (FIND_SEX(GET_I(meeter_being)) == FIND_SEX(GET_I(met_being)))
        {
            return 0;
        }
        else
        {
            return 255;
        }
        break;
    case 24:
        if (FIND_SEX(GET_I(met_being)) == SEX_FEMALE)
        {
            return 255;
        }
        else
        {
            return 0;
        }
        break;
    case 25:
        if (FIND_SEX(GET_I(met_being)) != SEX_FEMALE)
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
        n_int v=0;
        n_int n;
        for (n=0; n<INVENTORY_SIZE; n++)
        {
            if (met_being->inventory[n] & INVENTORY_GROOMED) v++;
        }
        return (n_byte) (v<<4);
    }
    case 27:
    {
        n_int v=0;
        n_int n;
        for (n=0; n<INVENTORY_SIZE; n++)
        {
            if (meeter_being->inventory[n] & INVENTORY_GROOMED) v++;
        }
        return (n_byte) (v<<4);
    }

    /** Wounds */
    case 28:
    {
        n_int v=0;
        n_int n;
        for (n=0; n<INVENTORY_SIZE; n++)
        {
            if (met_being->inventory[n] & INVENTORY_WOUND) v++;
        }
        return (n_byte) (v<<4);
    }
    case 29:
    {
        n_int v=0;
        n_int n;
        for (n=0; n<INVENTORY_SIZE; n++)
        {
            if (meeter_being->inventory[n] & INVENTORY_WOUND) v++;
        }
        return (n_byte) (v<<4);
    }

    /** Posture */
    case 30:
        return GET_PS(meeter_being);
    }
    /** case 31: */
    return GET_PS(met_being);

}

/**
 * @brief Returns a sensor value
 * @param sim Pointer to the simulation object
 * @param meeter_being Pointer to the being doing the meeting
 * @param met_being Pointer to the being which was met
 * @param internal Non-zero if this is an internal dialogue
 * @param switcher The type of sensor
 * @param additional_write No operation value
 * @return Sensor value
 */
static n_byte brain_third_sense(noble_simulation * sim, noble_being * meeter_being, noble_being * met_being, n_byte internal, n_byte switcher, n_byte * additional_write)
{
    n_byte half_switcher = switcher >> 1;
    noble_being * important_being = ((switcher & 1) ? met_being : meeter_being);
    n_genetics * genetics = being_genetics(important_being);
    switch (half_switcher % 10)
    {
        /** Facial characteristics.
              Here we shift the 0-15 gene values into a 0-255 range */
    case 0:
        return GENE_EYE_SHAPE(genetics) << 4;
    case 1:
        return GENE_EYE_COLOR(genetics) << 4;
    case 2:
        return GENE_EYE_SEPARATION(genetics) << 4;
    case 3:
        return GENE_NOSE_SHAPE(genetics) << 4;
    case 4:
        return GENE_EAR_SHAPE(genetics) << 4;
    case 5:
        return GENE_EYEBROW_SHAPE(genetics) << 4;
    case 6:
        return GENE_MOUTH_SHAPE(genetics) << 4;
    case 7:/* healthyness */
    {
        n_byte return_value = 0;
#ifdef IMMUNE_ON
        n_int n;
        noble_immune_system * immune = &(important_being->immune_system);
        return_value = immune->antigens[0];
        for (n=1; n<IMMUNE_ANTIGENS; n++)
        {
            if (immune->antigens[n]>return_value)
            {
                return_value = immune->antigens[n];
            }
        }
#endif
        return return_value;
    }
    /** the 8 case is covered in the default: */
    case 9: /** listen for shouts */
        if ((internal!=0) &&
                (!(meeter_being->state&BEING_STATE_SHOUTING)) &&
                (!(meeter_being->state&BEING_STATE_SPEAKING)) &&
                (meeter_being->shout[SHOUT_HEARD]>0))
        {
            
            INDICATOR_INC(sim, IT_AVERAGE_LISTENS);


            return meeter_being->shout[SHOUT_HEARD];
        }
        break;
        break;
        /** listen for name shouts */
    default:
        if (switcher == 16) /** positive affect */
        {
            n_uint positive = being_affect(sim,meeter_being,1)>>7;
            if (positive>255) positive=255;
            return (n_byte)positive;

        }
        {
            /* (switcher == 17) negative affect */

            n_uint negative=being_affect(sim,meeter_being,0)>>1;
            if (negative>255) negative=255;
            return (n_byte)negative;
        }
    }
    return additional_write[0]; /** no op case. Not sure if the compiler will recognize that though */
}

/**
 * @brief Returns a byte value indicating how familiar the being is with a place
 * @param local_being Pointer to the being
 * @param index an index number corresponding to the place
 * @return A byte value indicating the level of familiarity
 */
static n_byte territory_familiarity(noble_being * local_being,
                                    n_byte2 index)
{
    n_byte result=0;
#ifdef TERRITORY_ON
    n_uint familiarity = (n_uint)(local_being->territory[index].familiarity);
    n_uint i,max_familiarity = 1;

    /** find the maximum familiarity */
    for (i=0; i<TERRITORY_AREA; i++)
    {
        if (local_being->territory[i].familiarity > max_familiarity)
        {
            max_familiarity = (n_uint)local_being->territory[i].familiarity;
        }
    }

    if (max_familiarity == 0)
    {
       return 0; 
    }
    
    result = (n_byte)(familiarity*255/max_familiarity);
#endif
    return result;
}

#define IS_CONST0 (is_constant0 ? value0 : addr0[0])
#define IS_CONST1 (is_constant1 ? value1 : addr1[0])

/**
 * @brief Two beings meet and chat, or a being engages in an internal dialogue
 * @param sim Pointer to the simulation object
 * @param awake Whether the being is awake
 * @param meeter_being Pointer to the being doing the meeting
 * @param met_being Pointer to the being which was met
 * @param bc0 Braincode of the meeter
 * @param bc1 Braincode of the met
 * @param being_index Social graph index of the being which is the current focus of attention
 */
void brain_dialogue(
    noble_simulation * sim,
    n_byte awake,
    noble_being * meeter_being,
    noble_being * met_being,
    n_byte * bc0,
    n_byte * bc1,
    n_int being_index)
{
#ifdef EPISODIC_ON
#ifdef PARASITES_ON
    n_byte internal = (meeter_being == met_being);
    const n_int BRAINCODE_MIN_LOOP = 8*BRAINCODE_BYTES_PER_INSTRUCTION;
    n_int i = 0, itt = 0, i2;
    n_int actor_index, possible_actor_index;
    n_int episode_index = (n_int)(GET_A(meeter_being,ATTENTION_EPISODE));
    n_int territory_index = (n_int)(GET_A(meeter_being,ATTENTION_TERRITORY));
    n_int relationship_index = (n_int)(GET_A(meeter_being,ATTENTION_RELATIONSHIP));
    n_int anecdote_episode_index=-1;
    n_int intention_episode_index=-1;
    n_int memory_visited[EPISODIC_SIZE];

    social_link * meeter_social_graph = GET_SOC(sim,meeter_being);
    episodic_memory * episodic = GET_EPI(sim,meeter_being);
    n_int max_itterations;
    n_byte * pspace = (n_byte*)meeter_being->braincode_register;

    /* what is the current actor index within episodic memory? */
    if (being_index>-1)
    {
        actor_index = being_index;
    }
    else
    {
        being_index = GET_A(meeter_being,ATTENTION_ACTOR);
        actor_index = being_index;
    }

    if (meeter_being == met_being)
    {
        /** fixed number of itterations for internal dialogue */
        max_itterations = BRAINCODE_MAX_ADDRESS/BRAINCODE_BYTES_PER_INSTRUCTION;
    }
    else
    {
        /** variable number of itterations for chat */
        max_itterations = 8 + meeter_being->learned_preference[PREFERENCE_CHAT];
    }

    /** clear episodes visited.
       This array helps to avoid repeatedly visiting the same memories */
    for (i = 0; i < EPISODIC_SIZE; i++)
    {
        memory_visited[i] = 0;
    }

    i = 0;

    while (itt<max_itterations)
    {
        n_byte instruction = BRAINCODE_INSTRUCTION(bc0, i);
        n_byte is_constant0 = BRAINCODE_CONSTANT0(bc0, i);
        n_byte is_constant1 = BRAINCODE_CONSTANT1(bc0, i);
        n_byte value0 = BRAINCODE_VALUE(bc0, i, 0);
        n_byte value1 = BRAINCODE_VALUE(bc0, i, 1);
        n_byte *addr0;
        n_byte *addr1;

        if (BRAINCODE_ADDRESS(i+value0) < BRAINCODE_SIZE)
        {
            /** address within this being */
            addr0 = &bc0[BRAINCODE_ADDRESS(i+value0)];
        }
        else
        {
            /** Address within the other being */
            addr0 = &bc1[BRAINCODE_ADDRESS(i+value0) - BRAINCODE_SIZE];
        }
        if (BRAINCODE_ADDRESS(i+value1) < BRAINCODE_SIZE)
        {
            /** Address within this being */
            addr1 = &bc0[BRAINCODE_ADDRESS(i+value1)];
        }
        else
        {
            /** Address within the other being */
            addr1 = &bc1[BRAINCODE_ADDRESS(i+value1) - BRAINCODE_SIZE];
        }

        switch(instruction)
        {
            /** General sensor */
        case BRAINCODE_SEN:
        {
            addr1[0] = brain_first_sense(sim,meeter_being, met_being, meeter_social_graph, actor_index, addr0[0]);
            break;
        }
        case BRAINCODE_SEN2:
        {
            n_int new_episode_index=-1;
            n_int switcher = addr0[0]%25;
            n_int local_x = APESPACE_TO_MAPSPACE(being_location_x(meeter_being));
            n_int local_y = APESPACE_TO_MAPSPACE(being_location_y(meeter_being));
            switch (switcher)
            {
                /** Shift attention to a different actor */
            case 0:
                actor_index = get_actor_index(meeter_social_graph, IS_CONST1 % SOCIAL_SIZE);
                /** store the current focus of attention */
                GET_A(meeter_being,ATTENTION_ACTOR) = (n_byte)actor_index;
                break;
                /** Shift attention to a different episode */
            case 1:
                new_episode_index = IS_CONST1 % EPISODIC_SIZE;
                break;
                /** Shift attention to a different territory */
            case 2:
                territory_index = IS_CONST1;
                GET_A(meeter_being,ATTENTION_TERRITORY) = territory_index;
                break;
                /** Shift attention to a body region */
            case 3:
                GET_A(meeter_being,ATTENTION_BODY) = IS_CONST1 % INVENTORY_SIZE;
                break;
            case 4: /** Shift attention to a similar location */
                new_episode_index = attention_similar_place(episode_index, episodic, memory_visited);
                break;
            case 5: /** Shift attention to a similar time */
                new_episode_index = attention_similar_time(episode_index, episodic, memory_visited);
                break;
            case 6: /** Shift attention to a similar date */
                new_episode_index = attention_similar_date(episode_index, episodic, memory_visited);
                break;
            case 7: /** Shift attention to a similar name */
                new_episode_index = attention_similar_name(episode_index, episodic, memory_visited);
                break;
            case 8: /** Shift attention to a similar affect */
                new_episode_index = attention_similar_affect(episode_index, episodic, memory_visited);
                break;
            case 9:
                addr1[0] = episodic[episode_index].event;
                break;
            case 10:
                addr1[0] = episodic[episode_index].food;
                break;
            case 11:
                addr1[0] = episodic[episode_index].affect&255;
                break;
            case 12:
                addr1[0] = episodic[episode_index].arg&255;
                break;
            case 13:
                addr1[0] = (n_byte)(episodic[episode_index].location[0] * 255 / MAP_DIMENSION);
                break;
            case 14:
                addr1[0] = (n_byte)(episodic[episode_index].location[1] * 255 / MAP_DIMENSION);
                break;
            case 15:
            {
                /** atmosphere pressure */
                n_int pressure = weather_pressure(sim->weather, POSITIVE_LAND_COORD(local_x) >> 1, POSITIVE_LAND_COORD(local_y) >> 1);

                if (pressure > 100000) pressure = 100000;
                if (pressure < 0) pressure = 0;
                addr1[0] = (n_byte)(pressure>>9);
                break;
            }
            case 16:
            {
                /** wind magnitude */
                n_int w_dx=0, w_dy=0;
                weather_wind_vector(sim->weather, local_x, local_y, &w_dx, &w_dy);
                if (w_dx<0) w_dx=-w_dx;
                if (w_dy<0) w_dy=-w_dy;
                addr1[0] = (n_byte)((w_dx+w_dy)>>7);
                break;
            }
            case 17:
                addr1[0] = (n_byte)(sim->land->time>>3);
                break;
            case 18:
                /** attention to body */
                addr1[0] = GET_A(meeter_being,ATTENTION_BODY)*30;
                break;
            case 19:
#ifdef TERRITORY_ON
                /** territory name */
                addr1[0] = meeter_being->territory[territory_index].name;
#endif
                break;
            case 20:
                /** territory familiarity */
                addr1[0] = territory_familiarity(meeter_being,territory_index);
                break;
            case 21:
                /** territory familiarity */
                addr1[0] = territory_familiarity(met_being,territory_index);
                break;
            case 22:
            {
                /** carrying object */
                n_byte2 carrying = OBJECTS_CARRIED(meeter_being,BODY_RIGHT_HAND);
                n_byte2 obj_type=0;

                if (carrying==0) carrying = OBJECTS_CARRIED(meeter_being,BODY_LEFT_HAND);
                if (carrying!=0)
                {
                    switch(addr0[0]%9)
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
                    }
                    if (carrying & obj_type)
                    {
                        addr1[0] = 255;
                    }
                    else
                    {
                        addr1[0] = 0;
                    }
                }

                break;
            }
            case 23:
            {
                /** shift attention to a given social graph entry based on relationship */
                n_int idx = social_get_relationship(meeter_being,relationship_index,sim);
                if (idx > -1)
                {
                    actor_index = idx;
                    /** store the current focus of attention */
                    GET_A(meeter_being,ATTENTION_ACTOR) = (n_byte)actor_index;
                }
                break;
            }
            case 24:
            {
                /** shift attention to a different relationship type */
                relationship_index = 1+(addr1[0]%(OTHER_MOTHER-1));
                /** store the current relationship attention */
                GET_A(meeter_being,ATTENTION_RELATIONSHIP) = (n_byte)relationship_index;
                break;
            }
            }

            /** If attention has shifted to a new episode */
            if (new_episode_index>-1)
            {
                episode_index = new_episode_index;
                GET_A(meeter_being,ATTENTION_EPISODE) = (n_byte)episode_index;
                /** Shift attention to the being in this episode */
                possible_actor_index = get_actor_index_from_episode(meeter_social_graph,episodic,episode_index);
                if (possible_actor_index>-1)
                {
                    actor_index = possible_actor_index;
                    /** store the change in attention */
                    GET_A(meeter_being,ATTENTION_ACTOR) = (n_byte)actor_index;
                }
                /** set territory attention to the location where the episode occurred */
                GET_A(meeter_being,ATTENTION_TERRITORY) =
                    (APESPACE_TO_TERRITORY(episodic[episode_index].location[1])*16)+
                    APESPACE_TO_TERRITORY(episodic[episode_index].location[0]);
            }
            break;
        }
        case BRAINCODE_SEN3:

            addr1[0] = brain_third_sense(sim,meeter_being, met_being, internal, addr0[0], addr1);
            break;
            /** Action */
        case BRAINCODE_ACT:
        {
            switch(addr0[0]%6)
            {
                /** individual or social action */
            case 0:
                if ((awake != 0) && (addr0[0] > 127))
                {
                    if (internal == 0)
                    {
                        social_action(sim, meeter_being, met_being, addr1[0]);
                    }
                    else
                    {
                        social_action(sim, meeter_being, 0L, addr1[0]);
                    }
                    addr0[0] = 0;
                }
                break;
                /** Set location goal */
            case 1:
                if (!(meeter_being->script_overrides&OVERRIDE_GOAL))
                {
                    meeter_being->goal[0] = GOAL_LOCATION;
                    meeter_being->goal[1] = episodic[episode_index].location[0];
                    meeter_being->goal[2] = episodic[episode_index].location[1];
                    meeter_being->goal[3] = GOAL_TIMEOUT;
                }
                break;
                /** alter friend or foe value */
            case 2:
            {
                n_byte fof0=pspace[0];
                n_byte fof1=addr1[0];

                if (fof0>(n_byte)(fof1+85))
                {
                    if (meeter_social_graph[actor_index].friend_foe < 170)
                    {
                        meeter_social_graph[actor_index].friend_foe++;
                    }
                }
                if (fof1>(n_byte)(fof0+85))
                {
                    if (meeter_social_graph[actor_index].friend_foe > 85)
                    {
                        meeter_social_graph[actor_index].friend_foe--;
                    }
                }
                break;
            }
            /** alter attraction */
            case 3:
            {
                n_byte att0=addr1[0],att1=pspace[0];

                if (att0>(n_byte)(att1+85))
                {
                    if (meeter_social_graph[actor_index].attraction < 255)
                    {
                        meeter_social_graph[actor_index].attraction++;
                    }
                }
                if (att1>(n_byte)(att0+85))
                {
                    if (meeter_social_graph[actor_index].attraction > 16)
                    {
                        meeter_social_graph[actor_index].attraction--;
                    }
                }
                break;
            }
            /** alter familiarity */
            case 4:
                /** The values 10 and 20 meetings were just found experimentally */
                if ((addr1[0]>100) && (addr1[0]<150))
                {
                    if (meeter_social_graph[actor_index].familiarity < 65535)
                    {
                        meeter_social_graph[actor_index].familiarity++;
                        addr1[0] = 0;
                    }
                }
                if ((addr1[0]>150) && (addr1[0]<200))
                {
                    if (meeter_social_graph[actor_index].familiarity > 10)
                    {
                        meeter_social_graph[actor_index].familiarity--;
                        addr1[0] = 0;
                    }
                }
                break;
                /** brainprobe frequency */
            case 5:
            {
                n_int  n = pspace[0] % BRAINCODE_PROBES;
                n_byte f = 1 + (IS_CONST1 % BRAINCODE_MAX_FREQUENCY);

                if (meeter_being->brainprobe[n].frequency != f)
                {
                    INDICATOR_INC(sim, IT_AVERAGE_BRAINPROBE_ACTIVITY);
                }
                meeter_being->brainprobe[n].frequency = f;
                break;
            }
            }
            break;
        }
        case BRAINCODE_ACT2:
        {
            switch(addr0[0]%6)
            {
            case 0: /* brainprobe type */
            {
                n_int  n = pspace[0] % BRAINCODE_PROBES;
                n_byte typ = IS_CONST1 & 1;
                if (meeter_being->brainprobe[n].type != typ)
                {
                    INDICATOR_INC(sim, IT_AVERAGE_BRAINPROBE_ACTIVITY);
                }
                meeter_being->brainprobe[n].type = typ;
                break;
            }
            case 1: /** brainprobe address */
            {
                n_int n = pspace[0] % BRAINCODE_PROBES;
                n_byte adr = IS_CONST1;

                if (meeter_being->brainprobe[n].address != adr)
                {
                    INDICATOR_INC(sim, IT_AVERAGE_BRAINPROBE_ACTIVITY);
                }
                meeter_being->brainprobe[n].address = adr;
                break;
            }
            case 2: /** shout out */
            {
                n_byte msg = addr1[0];
                if (is_constant1)
                {
                    msg = value1;
                }
                if ((internal!=0) && (awake!=0) &&
                        (!(meeter_being->state&BEING_STATE_SHOUTING)) &&
                        (!(meeter_being->state&BEING_STATE_SPEAKING)) &&
                        (meeter_being->shout[SHOUT_CONTENT]==0) &&
                        (meeter_being->shout[SHOUT_HEARD]==0) &&
                        (meeter_being->shout[SHOUT_CTR]==0) &&
                        (msg>0))
                {
                    meeter_being->shout[SHOUT_CTR] = SHOUT_REFRACTORY;
                    meeter_being->state |= BEING_STATE_SHOUTING;
                    /** volume of message */
                    meeter_being->shout[SHOUT_VOLUME] = pspace[0];
                    /** type of message */
                    meeter_being->shout[SHOUT_CONTENT] = msg;
                    INDICATOR_INC(sim, IT_AVERAGE_SHOUTS);

                }
                break;
            }
            case 3: /** intention */
            {
                if (intention_episode_index != episode_index)
                {
                    n_byte v0 = pspace[0];
                    n_byte v1 = IS_CONST1;
                    if (episodic_intention(sim,meeter_being,episode_index,(n_byte2)(v0*10),v1)!=0)
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
                if (meeter_being->brainprobe[n].offset != offset)
                {                    
                    INDICATOR_INC(sim, IT_AVERAGE_BRAINPROBE_ACTIVITY);
                }

                meeter_being->brainprobe[n].offset = offset;
                break;
            }
            case 5: /** posture */
                if (awake != 0)
                {
                    GET_PS(meeter_being)=addr1[0];
                }
                break;
            }
            break;
        }
        case BRAINCODE_ACT3:
            switch(addr0[0]%2)
            {
                /** brainprobe position */
            case 0:
            {
                n_int  n = pspace[0] % BRAINCODE_PROBES;
                n_byte p = IS_CONST1;

                if (meeter_being->brainprobe[n].position != p)
                {
                    INDICATOR_INC(sim, IT_AVERAGE_BRAINPROBE_ACTIVITY);
                }

                meeter_being->brainprobe[n].position = p;
                break;
            }
            /** alter learned preferences */
            case 1:
            {
                n_int n;
                n_byte prf = addr1[0];

                n = pspace[0]%PREFERENCES;

                if ((prf > 55) && (prf<155))
                {
                    if (meeter_being->learned_preference[n] < 255)
                    {
                        meeter_being->learned_preference[n]++;
                        addr1[0] = 0;
                    }
                }
                if (prf >= 155)
                {
                    if (meeter_being->learned_preference[n] > 0)
                    {
                        meeter_being->learned_preference[n]--;
                        addr1[0] = 0;
                    }
                }
                break;
            }
            break;
            }
            /** spread anecdote */
        case BRAINCODE_ANE:
            if (internal == 0)
            {
                /** not internal dialogue */
                /** avoid repeated anecdotes in the same conversation */
                if (anecdote_episode_index != episode_index)
                {
                    if (episodic_anecdote(sim, meeter_being, met_being)!=0)
                    {
                        anecdote_episode_index = episode_index;
                    }
                }
            }
            break;
                
            /** Logical and */
        case BRAINCODE_AND:
            if (is_constant0)
            {
                addr0[0] &= addr1[0];
            }
            else
            {
                if ((addr0[0]>127) && (addr1[0]>127)) i += BRAINCODE_BYTES_PER_INSTRUCTION;
            }
            break;
            /** Logical or */
        case BRAINCODE_OR:
            if (is_constant0)
            {
                addr0[0] |= addr1[0];
            }
            else
            {
                if ((addr0[0]>127) || (addr1[0]>127)) i += BRAINCODE_BYTES_PER_INSTRUCTION;
            }
            break;
            /** Move a byte, with no particular alignment */
        case BRAINCODE_MOV:
            if ((!is_constant0) && (!is_constant1))
            {
                addr1[0] = addr0[0];
            }
            else
            {
                addr1[0] = value0;
            }
            break;
            /** Move a block of instructions */
        case BRAINCODE_MVB:
        {
            n_int ptr0, ptr1, n, instructions_to_copy, dat = 0;

            if (meeter_being != met_being)
            {

                if (!is_constant0)
                {
                    ptr0 = BRAINCODE_ADDRESS(i + ((n_int)addr0[0]*BRAINCODE_BYTES_PER_INSTRUCTION));
                }
                else
                {
                    ptr0 = BRAINCODE_ADDRESS(i + ((n_int)value0*BRAINCODE_BYTES_PER_INSTRUCTION));
                }

                ptr1 = BRAINCODE_ADDRESS(i + ((n_int)IS_CONST1 * BRAINCODE_BYTES_PER_INSTRUCTION));


				instructions_to_copy = 1 + (pspace[1]%BRAINCODE_BLOCK_COPY);
				while (dat < instructions_to_copy)
                {
                    if (ptr0 < BRAINCODE_SIZE)
                    {
                        addr0 = &bc0[ptr0];
                    }
                    else
                    {
                        addr0 = &bc1[ptr0 - BRAINCODE_SIZE];
                    }

                    if (ptr1 < BRAINCODE_SIZE)
                    {
                        addr1 = &bc0[ptr1];
                    }
                    else
                    {
                        addr1 = &bc1[ptr1 - BRAINCODE_SIZE];
                    }

                    for (n = 0; n < BRAINCODE_BYTES_PER_INSTRUCTION; n++)
                    {
                        addr1[n] = addr0[n];
                    }
                    dat++;
                    ptr0 = BRAINCODE_ADDRESS(ptr0 + BRAINCODE_BYTES_PER_INSTRUCTION);
                    ptr1 = BRAINCODE_ADDRESS(ptr1 + BRAINCODE_BYTES_PER_INSTRUCTION);
                }
            }
            break;
        }
        /** Add */
        case BRAINCODE_ADD:
            if ((!is_constant0) && (!is_constant1))
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
            if ((!is_constant0) && (!is_constant1))
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
            if ((!is_constant0) && (!is_constant1))
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
            if ((!is_constant0) && (!is_constant1))
            {
                addr1[0] >>= (addr0[0]%4);
            }
            else
            {
                addr1[0] >>= (value0%4);
            }
            break;
            /** Modulus */
        case BRAINCODE_MOD:
            if ((!is_constant0) && (!is_constant1))
            {
                if (addr0[0] != 0)
                {
                    addr1[0] %= addr0[0];
                }
            }
            else
            {
                if (value0 != 0)
                {
                    addr1[0] %= value0;
                }
            }
            break;
            /** Count up or down */
        case BRAINCODE_CTR:
            if (addr0[0] > 127)
            {
                if (addr1[0] < 255)
                {
                    addr1[0]++;
                }
                else
                {
                    addr1[0]=0;
                }
            }
            else
            {
                if (addr1[0] > 0)
                {
                    addr1[0]--;
                }
                else
                {
                    addr1[0]=255;
                }
            }
            break;
            /** Goto */
        case BRAINCODE_JMP:
        {
            n_int v0 = IS_CONST0;
            n_int v1 = IS_CONST1;

            i2 = (i + (((v0*256) + v1)*BRAINCODE_BYTES_PER_INSTRUCTION)) % BRAINCODE_SIZE;
            if (i2 <= i)
            {
                if (i-i2 < BRAINCODE_MIN_LOOP)
                {
                    i2 = i - BRAINCODE_MIN_LOOP;
                    if (i2 < 0) i2 += BRAINCODE_SIZE;
                }
            }
            i = i2-BRAINCODE_BYTES_PER_INSTRUCTION;
            break;
        }
        /** Goto if zero */
        case BRAINCODE_JMZ:
        {
            n_int v0 = IS_CONST0;

            if (v0 == 0)
            {

                i2 = (i + ((n_int) IS_CONST1 *BRAINCODE_BYTES_PER_INSTRUCTION)) % BRAINCODE_SIZE;

                if (i2 <= i)
                {
                    if (i-i2 < BRAINCODE_MIN_LOOP)
                    {
                        i2 = i - BRAINCODE_MIN_LOOP;
                        if (i2 < 0) i2 += BRAINCODE_SIZE;
                    }
                }
                i = i2-BRAINCODE_BYTES_PER_INSTRUCTION;
            }
            break;
        }
        /** Goto if not zero */
        case BRAINCODE_JMN:
        {
            n_int v0 = IS_CONST0;
            if (v0 != 0)
            {
                i2 = (i + ((n_int) IS_CONST1 *BRAINCODE_BYTES_PER_INSTRUCTION)) % BRAINCODE_SIZE;
                if (i2 <= i)
                {
                    if (i-i2 < BRAINCODE_MIN_LOOP)
                    {
                        i2 = i - BRAINCODE_MIN_LOOP;
                        if (i2 < 0) i2 += BRAINCODE_SIZE;
                    }
                }
                i = i2-BRAINCODE_BYTES_PER_INSTRUCTION;
            }
            break;
        }
        /** Goto and decrement if not zero */
        case BRAINCODE_DJN:
            if (addr0[0]-1 != 0)
            {
                addr0[0]--;
                i2 = (i + ((n_int) IS_CONST1 *BRAINCODE_BYTES_PER_INSTRUCTION)) % BRAINCODE_SIZE;
                if (i2 <= i)
                {
                    if (i-i2 < BRAINCODE_MIN_LOOP)
                    {
                        i2 = i - BRAINCODE_MIN_LOOP;
                        if (i2 < 0) i2 += BRAINCODE_SIZE;
                    }
                }
                i = i2-BRAINCODE_BYTES_PER_INSTRUCTION;
            }
            break;
            /** If two values are equal then skip the next n instructions */
        case BRAINCODE_SEQ:
            if ((!is_constant0) && (!is_constant1))
            {
                if (addr1[0] == addr0[0])
                {
                    i = (i + (BRAINCODE_BYTES_PER_INSTRUCTION * (1 + (n_int)pspace[0]))) % BRAINCODE_SIZE;
                }
            }
            else
            {
                if (addr1[0] == value0)
                {
                    i = (i + (BRAINCODE_BYTES_PER_INSTRUCTION * (1 + (n_int)pspace[0]))) % BRAINCODE_SIZE;
                }
            }
            break;
            /** If two values are not equal then skip the next n instructions */
        case BRAINCODE_SNE:
            if ((!is_constant0) && (!is_constant1))
            {
                if (addr1[0] != addr0[0])
                {
                    i = (i + (BRAINCODE_BYTES_PER_INSTRUCTION * (1 + (n_int)pspace[0]))) % BRAINCODE_SIZE;
                }
            }
            else
            {
                if (addr1[0] != value0)
                {
                    i = (i + (BRAINCODE_BYTES_PER_INSTRUCTION * (1 + (n_int)pspace[0]))) % BRAINCODE_SIZE;
                }
            }
            break;
            /** Skip the next n instructions if less than */
        case BRAINCODE_SLT:
            if ((!is_constant0) && (!is_constant1))
            {
                if (addr1[0] < addr0[0])
                {
                    i = (i + (BRAINCODE_BYTES_PER_INSTRUCTION * (1 + (n_int)pspace[0]))) % BRAINCODE_SIZE;
                }
            }
            else
            {
                if (addr1[0] < value0)
                {
                    i = (i + (BRAINCODE_BYTES_PER_INSTRUCTION * (1 + (n_int)pspace[0]))) % BRAINCODE_SIZE;
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
            if (is_constant0)
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
            n_byte v0 = IS_CONST0;
            n_byte v1 = IS_CONST1;
            pspace[v0 % BRAINCODE_PSPACE_REGISTERS] = v1;
            break;
        }
        /** Load from Pspace */
        case BRAINCODE_LTP:
        {
            n_byte v0 = IS_CONST0;
            addr1[0] = pspace[v0 % BRAINCODE_PSPACE_REGISTERS];
            break;
        }

        }
        i += BRAINCODE_BYTES_PER_INSTRUCTION;
        itt++;

        if (i >= BRAINCODE_SIZE)
        {
            i -= BRAINCODE_SIZE;
        }
    }
#endif
#endif
}

#endif


