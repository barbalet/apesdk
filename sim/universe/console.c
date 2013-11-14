/****************************************************************

 console.c

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

#ifndef	_WIN32
#include "../entity/entity.h"
#else
#include "..\entity\entity.h"
#endif

#include "universe_internal.h"


#include <stdio.h>

/*NOBLEMAKE END=""*/

static n_int simulation_running = 1;
static n_int simulation_executing = 0;

const n_string RUN_STEP_CONST = "RSC";

/** The type of watch */
n_int watch_type = WATCH_NONE;

/** keeps a running count of the total string length */
n_int watch_string_length = 0;

/** enable or disable logging to file */
n_int nolog = 0;

/** number of steps to periodically log simulation indicators */
n_int indicator_index = 1;

/** How many steps at which to periodically save the simulation */
n_uint save_interval_steps = 60 * 24;

n_int          console_file_interaction = 0;
n_string_block console_file_name;

n_int console_executing(void)
{
    return simulation_executing;
}

void console_external_watch(void)
{
    if (io_command_line_execution())
    {
        n_string_block output;
        n_int          position = 0;
        io_string_write(output, "External Action -> Watching ", &position);
        io_string_write(output, being_get_select_name(sim_sim()), &position);
        output[position] = 0;
        io_console_out(output);
    }
}

/**
 *
 * @param ptr pointer to noble_simulation object
 * @param response command parameters
 * @param output_function
 */
n_int console_being(void * ptr, n_string response, n_console_output output_function)
{
    noble_simulation * local_sim = (noble_simulation *) ptr;

    output_function(being_get_select_name(local_sim));

    return 0;
}


/**
 * Show the friends of the given being
 * @param ptr pointer to noble_simulation object
 * @param beingname name of the being
 * @param friend_type 0=friends, 1=enemies, 2=mates
 * @param result returned text
 */
static void show_friends(void * ptr, n_string beingname, n_int friend_type, n_string result)
{
    noble_simulation * local_sim = (noble_simulation *) ptr;
    noble_being * local_being;
    n_int i,found;
    noble_social * local_social_graph;

    /** Get the being object from its name */
    local_being = being_from_name(local_sim, beingname);
    if (local_being == 0) return;

    /** Get the social graph for the being */
    local_social_graph = being_social(local_being);
    if (local_social_graph == 0L) return;

    /** For each entry in the social graph */
    for (i = 1; i < SOCIAL_SIZE_BEINGS; i++)
    {
        /** Skip empty entries */
        if (SOCIAL_GRAPH_ENTRY_EMPTY(local_social_graph, i)) continue;

        found = 0;
        switch(friend_type)
        {
        case 0:   /**< friends */
        {
            if (local_social_graph[i].friend_foe >= social_respect_mean(local_sim,local_being))
            {
                found = 1;
            }
            break;
        }
        case 1:   /**< enemies */
        {
            if (local_social_graph[i].friend_foe < social_respect_mean(local_sim,local_being))
            {
                found = 1;
            }
            break;
        }
        case 2:   /**< attraction */
        {
            if (local_social_graph[i].attraction > 0)
            {
                found = 1;
            }
            break;
        }
        }

        if (found==1)
        {
            n_int relationship_index;
            n_string_block relationship_str1;
            n_string_block relationship_str2;
            n_string_block met_being_name;
            n_string_block result_str;
            
            /** Print the name and familiarity */
            social_graph_link_name(local_sim, local_being, i, BEING_MET, met_being_name);

            /** type of relationship */

            relationship_index = local_social_graph[i].relationship;
            sprintf(relationship_str2," ");

            if (relationship_index > RELATIONSHIP_SELF)
            {
                being_relationship_description(relationship_index,relationship_str1);

                if (IS_FAMILY_MEMBER(local_social_graph,i))
                {
                    sprintf(relationship_str2," (%s)",relationship_str1);
                }
                else
                {
                    n_string_block meeter_being_name;
                    sprintf(meeter_being_name," ");
                    social_graph_link_name(local_sim, local_being, i, BEING_MEETER, meeter_being_name);
                    sprintf(relationship_str2," (%s of %s)",relationship_str1,meeter_being_name);
                }
            }

            if (i != GET_A(local_being,ATTENTION_ACTOR))
            {
                /** Not the current focus of attention */
                sprintf(result_str,"  %05d  %s%s\n",(int)local_social_graph[i].familiarity,met_being_name,relationship_str2);
            }
            else
            {
                /** The current focus of attention */
                sprintf(result_str,"  %05d [%s]%s\n",(int)local_social_graph[i].familiarity,met_being_name,relationship_str2);
            }
            /** Write to the final result string */
            io_string_write(result, result_str, &watch_string_length);
        }
    }
}

/**
 * gets the number of mins/hours/days/months/years
 * @param str text to be processed
 * @param number
 * @param interval
 * @return number of mins/hours/days/months/years
 */
n_int get_time_interval(n_string str, n_int * number, n_int * interval)
{
    n_int i,index=0,ctr=0,result=0,divisor=0;
    char c;
    n_string_block buf;
    n_int retval = -1;
    n_int length = io_length(str,256);

    for (i = 0; i < length; i++)
    {
        if (str[i] != ' ')
        {
            buf[ctr++] = str[i];
        }

        if ((str[i] == ' ') || (i==(length-1)))
        {
            buf[ctr]=0;

            switch(index)
            {
            case 0:
            {
                io_number((n_string)buf, &result, &divisor);
                *number = result;
                retval = 0;
                break;
            }
            case 1:
            {
                if (ctr==1)
                {
                    char lower_c;
                    lower_c = c = buf[0];
                    IO_LOWER_CHAR(lower_c);
                    if (c=='m') *interval = INTERVAL_MINS;
                    if (lower_c=='h') *interval = INTERVAL_HOURS;
                    if (lower_c=='d') *interval = INTERVAL_DAYS;
                    if (c=='M') *interval = INTERVAL_MONTHS;
                    if (lower_c=='y') *interval = INTERVAL_YEARS;
                }
                else
                {
                    IO_LOWER_CHAR(buf[0]);
                    if (io_find((n_string)buf,0,ctr,"min",3)>-1) *interval = INTERVAL_MINS;
                    if ((io_find((n_string)buf,0,ctr,"hour",4)>-1) ||
                            (io_find((n_string)buf,0,ctr,"hr",2)>-1))
                    {
                        *interval = INTERVAL_HOURS;
                    }
                    if (io_find((n_string)buf,0,ctr,"day",3)>-1) *interval = INTERVAL_DAYS;
                    if (io_find((n_string)buf,0,ctr,"mon",3)>-1) *interval = INTERVAL_MONTHS;
                }

                break;
            }
            }

            index++;
            ctr=0;
        }
    }
    return retval;
}

/**
 * Show details of the overall simulation
 * @param ptr pointer to a noble_simulation object
 * @param response parameters of the command
 * @param output_function function to be used to display the result
 * @return 0
 */
n_int console_simulation(void * ptr, n_string response, n_console_output output_function)
{
    noble_simulation * local_sim = (noble_simulation *) ptr;
    n_string_block beingstr, time;
    n_int count=0,juveniles=0;
    n_uint i;
    for (i = 0; i < local_sim->num; i++)
    {
        noble_being * local_being = &local_sim->beings[i];
        if (FIND_SEX(GET_I(local_being)) == SEX_FEMALE)
        {
            count++;
        }
        if ((TIME_IN_DAYS(local_sim->land->date) - being_dob(local_being)) < AGE_OF_MATURITY)
        {
            juveniles++;
        }
    }

    sprintf(beingstr,"Map dimension: %ld\n", land_map_dimension(local_sim->land));
    sprintf(beingstr,"%sLand seed: %d %d\n",beingstr, (int)local_sim->land->genetics[0],(int)local_sim->land->genetics[1]);
    sprintf(beingstr,"%sPopulation: %d   ", beingstr, (int)local_sim->num);
    sprintf(beingstr,"%sAdults: %d   Juveniles: %d\n", beingstr, (int)(local_sim->num - juveniles),(int)juveniles);
    if (local_sim->num > 0)
    {
        sprintf(beingstr,"%sFemales: %d (%.1f%%)   Males: %d (%.1f%%)\n", beingstr,
                (int)count, count*100.0f/local_sim->num,
                (int)(local_sim->num - count),(local_sim->num - count)*100.0f/local_sim->num);
    }
    sprintf(beingstr,"%sTide level: %d\n", beingstr, (int)local_sim->land->tide_level);

    io_time_to_string(time, local_sim->land->time, local_sim->land->date[0], local_sim->land->date[1]);

    sprintf(beingstr,"%s%s", beingstr, time);

    if (simulation_executing)
    {
        sprintf(beingstr,"%s Simulation running", beingstr);
    }
    else
    {
        sprintf(beingstr,"%s Simulation not running", beingstr);
    }

    output_function(beingstr);

    return 0;
}

/**
 * Shows the names of all beings
 * @param ptr
 * @param response parameters of the command
 * @param output_function function used to display the result
 * @return 0
 */
n_int console_list(void * ptr, n_string response, n_console_output output_function)
{
    noble_simulation * local_sim = (noble_simulation *) ptr;
    noble_being * local_being;
    n_string_block line_text;
    n_int          location = 0;
    n_uint          loop = 0;
    
    /** show names in index order */
    while (loop < local_sim->num)
    {
        n_string_block name;
        n_int          length;
        /** get the being */
        local_being = &local_sim->beings[loop];

        /** get the name of the being */
        being_name_simple(local_being, name);
        
        io_string_write(line_text, name, &location);
        
        length = io_length(name, STRING_BLOCK_SIZE);

        while (length < 24)
        {
            io_string_write(line_text, " ", &location);
            length++;
        }
        
        if ((loop % 3) == 2)
        {
            line_text[location] = 0;
            output_function(line_text);
            location = 0;
        }
        
        loop++;
    }

    if (location != 0)
    {
        line_text[location] = 0;
        output_function(line_text);
    }
    
    return 0;
}
#ifdef BRAINCODE_ON
void console_populate_braincode(noble_simulation * local_sim, line_braincode function)
{
    if (local_sim->select != NO_BEINGS_FOUND)
    {

        noble_being * local_being = &(local_sim->beings[local_sim->select]);
        n_byte *internal_bc = being_braincode_internal(local_being);
        n_byte *external_bc = being_braincode_external(local_being);
        n_int           loop = 0;

        n_string_block  initial_information;
        n_int           position = 0;

        io_string_write(initial_information, "EXT                                                         INT", &position);
        
        initial_information[position] = 0;
    
        (*function)(initial_information, -1);

        while(loop < 22)
        {
            n_string_block command_information;
            
            n_string_block first_internal;
            n_string_block first_external;
            
            
            position = 0;
            
            brain_three_byte_command((n_string)first_internal, &internal_bc[loop*BRAINCODE_BYTES_PER_INSTRUCTION]);
            brain_three_byte_command((n_string)first_external, &external_bc[loop*BRAINCODE_BYTES_PER_INSTRUCTION]);

            if (loop == 21)
            {
                io_string_write(command_information, first_external, &position);
                io_string_write(command_information, "                   ", &position);
                io_string_write(command_information, first_internal, &position);
            }
            else
            {
                n_string_block second_internal;
                n_string_block second_external;

                brain_three_byte_command((n_string)second_internal, &internal_bc[(loop+22)*BRAINCODE_BYTES_PER_INSTRUCTION]);
                brain_three_byte_command((n_string)second_external, &external_bc[(loop+22)*BRAINCODE_BYTES_PER_INSTRUCTION]);
                
                io_string_write(command_information, first_external, &position);
                io_string_write(command_information, "  ", &position);
                io_string_write(command_information, second_external, &position);
                io_string_write(command_information, "   ", &position);
                io_string_write(command_information, first_internal, &position);
                io_string_write(command_information, "  ", &position);
                io_string_write(command_information, second_internal, &position);
            }

            command_information[position] = 0;
            
            (*function)(command_information, loop);
            loop++;
        }
    }

}
#endif

/**
 * Displays a text representation of the braincode programs
 * @param ptr pointer to a noble_simulation object
 * @param local_being being to be analysed
 * @param result resulting text output
 * @param outer if 1 then show the outer braincode, otherwise show the inner
 * @param columns The number of columns to use for displaying the code
 */
/*
static void show_braincode(void * ptr, noble_being * local_being, n_string result, n_int outer, n_int columns)
{
    n_int i, j, col, offset, program_pointer, instructions_per_column;
    n_byte * code;
    n_string_block braincode_str;
    n_byte values[BRAINCODE_BYTES_PER_INSTRUCTION];
    const n_int spacing_between_columns = 5;

    if (columns<1) columns=1;
    instructions_per_column = (BRAINCODE_SIZE/BRAINCODE_BYTES_PER_INSTRUCTION)/columns;

    if (outer==0)
    {
        code = being_braincode_internal(local_being);
    }
    else
    {
        code = being_braincode_external(local_being);
    }

    for (i = 0; i < instructions_per_column; i++)
    {
        for (col = 0; col < columns; col++)
        {
            offset = i + (col*instructions_per_column);
            program_pointer = offset + (i*BRAINCODE_BYTES_PER_INSTRUCTION);

            values[0] = BRAINCODE_INSTRUCTION(code, program_pointer);
            values[1] = BRAINCODE_VALUE(code, program_pointer, 0);
            values[2] = BRAINCODE_VALUE(code, program_pointer, 1);
            brain_three_byte_command(braincode_str, values);
            for (j = 0; j < io_length(braincode_str,STRING_BLOCK_SIZE); j++)
            {
                result[watch_string_length++] = braincode_str[j];
            }
            if (col<columns-1)
            {
                for (j = 0; j < spacing_between_columns; j++)
                {
                    result[watch_string_length++] = ' ';
                }
            }
            else
            {
                result[watch_string_length++] = '\n';
            }
        }
    }
    result[watch_string_length++] = '\n';
}
*/

/**
 * Show the appearance parameters for a being
 * @param ptr pointer to a noble_simulation object
 * @param beingname name of the being
 * @param local_being being to be shown
 * @param result resulting text containing appearance
 */
static void watch_appearance(void *ptr, n_string beingname, noble_being * local_being, n_string result)
{
    n_string_block str;
    n_genetics * genetics = being_genetics(local_being);

    sprintf(str,"Height: %.3f m\n", (int)GET_BEING_HEIGHT(local_being)/1000.0f);
    io_string_write(result, str, &watch_string_length);

    sprintf(str,"Mass: %.2f Kg\n", (float)GET_M(local_being)/100.0f);
    io_string_write(result, str, &watch_string_length);

    sprintf(str,"Body fat: %.2f Kg\n", (float)GET_BODY_FAT(local_being)/100.0f);
    io_string_write(result, str, &watch_string_length);

    sprintf(str,"Hair length: %.1f mm\n", (float)(GENE_HAIR(genetics)*100.0f/160.0f));
    io_string_write(result, str, &watch_string_length);

    sprintf(str,"Pigmentation: %02d\n", (int)(GENE_PIGMENTATION(genetics)));
    io_string_write(result, str, &watch_string_length);

    sprintf(str,"Body frame: %02d\n", (int)(GENE_FRAME(genetics)));
    io_string_write(result, str, &watch_string_length);

    sprintf(str,"Eye separation: %.1f mm\n",
            80.0f + ((float)(GENE_EYE_SEPARATION(genetics))));
    io_string_write(result, str, &watch_string_length);

    sprintf(str,"Eye color: %02d       Eye shape: %02d\n",
            (int)(GENE_EYE_COLOR(genetics)),
            (int)(GENE_EYE_SHAPE(genetics)));
    io_string_write(result, str, &watch_string_length);

    sprintf(str,"Nose shape: %02d      Ear shape: %02d\n",
            (int)(GENE_NOSE_SHAPE(genetics)),
            (int)(GENE_EAR_SHAPE(genetics)));
    io_string_write(result, str, &watch_string_length);

    sprintf(str,"Eyebrow shape: %02d   Mouth shape: %02d\n",
            (int)(GENE_EYEBROW_SHAPE(genetics)),
            (int)(GENE_MOUTH_SHAPE(genetics)));
    io_string_write(result, str, &watch_string_length);
}

/**
 * Display values for the vascular system
 * @param ptr pointer to a noble_simulation object
 * @param beingname name of the being
 * @param local_being being to be watched
 * @param result resulting text
 */
static void watch_vascular(void *ptr, n_string beingname, noble_being * local_being, n_string result)
{
#ifdef METABOLISM_ON
    n_int i,j,radius;
    n_string_block str;
    const n_int col[] = {24,32,41,53};

    io_string_write(result, "\nHeart rate: ", &watch_string_length);
    sprintf(str,"%d bpm  %.1f Hz\n",GET_MT(local_being,METABOLISM_HEART_RATE)*60/1000,GET_MT(local_being,METABOLISM_HEART_RATE)/1000.0f);
    io_string_write(result, str, &watch_string_length);

    io_string_write(result, "\nVessel                  Radius    Flow   Pressure    Temp C\n", &watch_string_length);

    for (i=0; i<60; i++)
        io_string_write(result, "-", &watch_string_length);

    io_string_write(result, "\n", &watch_string_length);
    for (i=0; i<VASCULAR_SIZE; i++)
    {
        metabolism_vascular_description(i,str);
        io_string_write(result, str, &watch_string_length);
        for (j=io_length(str,STRING_BLOCK_SIZE); j<col[0]; j++)
        {
            io_string_write(result, " ", &watch_string_length);
        }

        radius = metabolism_vascular_radius(local_being, i);
        sprintf(str,"%02d.%02d",
                (int)radius/100,
                (int)radius%100);
        io_string_write(result, str, &watch_string_length);
        j+=io_length(str,STRING_BLOCK_SIZE);
        while (j<col[1])
        {
            io_string_write(result, " ", &watch_string_length);
            j++;
        }

        sprintf(str,"%06u",
                (unsigned int)(local_being->vessel[i].flow_rate));
        io_string_write(result, str, &watch_string_length);
        j+=io_length(str,STRING_BLOCK_SIZE);
        while (j<col[2])
        {
            io_string_write(result, " ", &watch_string_length);
            j++;
        }

        sprintf(str,"%10u",
                (unsigned int)(local_being->vessel[i].pressure));
        io_string_write(result, str, &watch_string_length);
        j+=io_length(str,STRING_BLOCK_SIZE);
        while (j<col[3])
        {
            io_string_write(result, " ", &watch_string_length);
            j++;
        }

        sprintf(str,"%02u.%02u",
                (unsigned int)(local_being->vessel[i].temperature/1000),
                (unsigned int)(local_being->vessel[i].temperature%1000)/10);
        io_string_write(result, str, &watch_string_length);

        io_string_write(result, "\n", &watch_string_length);
    }
#endif
}

/**
 * Show values for the respiration system
 * @param ptr pointer to noble_simulation object
 * @param beingname name of the being
 * @param local_being being to be observed
 * @param result resulting text
 */
static void watch_respiration(void *ptr, n_string beingname, noble_being * local_being, n_string result)
{
#ifdef METABOLISM_ON
    n_string_block str;

    io_string_write(result, "\nBreathing rate: ", &watch_string_length);
    sprintf(str,"%d Vf   %.1f Hz\n",GET_MT(local_being,METABOLISM_BREATHING_RATE)*60/1000,GET_MT(local_being,METABOLISM_BREATHING_RATE)/1000.0f);
    io_string_write(result, str, &watch_string_length);
    io_string_write(result, "Lung Capacity:  ", &watch_string_length);
    sprintf(str,"%d cm^3\n",GET_MT(local_being,METABOLISM_LUNG_CAPACITY));
    io_string_write(result, str, &watch_string_length);
    sprintf(str,"%s %05u   %s %05u\n",
            metabolism_description(METABOLISM_OXYGEN),
            GET_MT(local_being,METABOLISM_OXYGEN),
            metabolism_description(METABOLISM_CO2),
            GET_MT(local_being,METABOLISM_CO2));
    io_string_write(result, str, &watch_string_length);
    sprintf(str,"%s %05u   %s %05u\n",
            metabolism_description(METABOLISM_GLUCOSE),
            GET_MT(local_being,METABOLISM_GLUCOSE),
            metabolism_description(METABOLISM_PYRUVATE),
            GET_MT(local_being,METABOLISM_PYRUVATE));
    io_string_write(result, str, &watch_string_length);
#endif
}

/**
 * Show the metabolism for a given being
 * @param ptr pointer to noble_simulation object
 * @param beingname being name
 * @param local_being being to be viewed
 * @param result resulting text
 */
static void watch_metabolism(void *ptr, n_string beingname, noble_being * local_being, n_string result)
{
#ifdef METABOLISM_ON
    n_string_block str;

    io_string_write(result, "\nStomach:\n", &watch_string_length);
    sprintf(str,"  %s:   %d\t%s: %d\n  %s:  %d\t%s:     %d\n  %s:   %d\t%s:   %d\n  %s:  %d\t%s: %d\n",
            metabolism_description(METABOLISM_WATER),
            GET_MT(local_being,METABOLISM_WATER),
            metabolism_description(METABOLISM_PROTEIN),
            GET_MT(local_being,METABOLISM_PROTEIN),
            metabolism_description(METABOLISM_STARCH),
            GET_MT(local_being,METABOLISM_STARCH),
            metabolism_description(METABOLISM_FAT),
            GET_MT(local_being,METABOLISM_FAT),
            metabolism_description(METABOLISM_SUGAR),
            GET_MT(local_being,METABOLISM_SUGAR),
            metabolism_description(METABOLISM_BILE),
            GET_MT(local_being,METABOLISM_BILE),
            metabolism_description(METABOLISM_LEPTIN),
            GET_MT(local_being,METABOLISM_LEPTIN),
            metabolism_description(METABOLISM_GHRELIN),
            GET_MT(local_being,METABOLISM_GHRELIN));
    io_string_write(result, str, &watch_string_length);

    io_string_write(result, "Liver:\n", &watch_string_length);
    sprintf(str,"  %s: %d\n  %s:  %d\n  %s: %d\n  %s: %d\n  %s: %d\n",
            metabolism_description(METABOLISM_GLUCOGEN),
            GET_MT(local_being,METABOLISM_GLUCOGEN),
            metabolism_description(METABOLISM_GLYCOGEN),
            GET_MT(local_being,METABOLISM_GLYCOGEN),
            metabolism_description(METABOLISM_ADRENALIN),
            GET_MT(local_being,METABOLISM_ADRENALIN),
            metabolism_description(METABOLISM_AMMONIA),
            GET_MT(local_being,METABOLISM_AMMONIA),
            metabolism_description(METABOLISM_LACTATE),
            GET_MT(local_being,METABOLISM_LACTATE));
    io_string_write(result, str, &watch_string_length);

    io_string_write(result, "Kidneys:\n", &watch_string_length);
    sprintf(str,"  %s: %d\n",
            metabolism_description(METABOLISM_UREA),
            GET_MT(local_being,METABOLISM_UREA));
    io_string_write(result, str, &watch_string_length);

    io_string_write(result, "Lungs:\n", &watch_string_length);
    sprintf(str,"  %s: %d\n  %s:    %d\n",
            metabolism_description(METABOLISM_OXYGEN),
            GET_MT(local_being,METABOLISM_OXYGEN),
            metabolism_description(METABOLISM_CO2),
            GET_MT(local_being,METABOLISM_CO2));
    io_string_write(result, str, &watch_string_length);

    io_string_write(result, "Pancreas:\n", &watch_string_length);
    sprintf(str,"  %s: %d\n  %s: %d\n  %s: %d\n",
            metabolism_description(METABOLISM_FATTY_ACIDS),
            GET_MT(local_being,METABOLISM_FATTY_ACIDS),
            metabolism_description(METABOLISM_TRIGLYCERIDE),
            GET_MT(local_being,METABOLISM_TRIGLYCERIDE),
            metabolism_description(METABOLISM_INSULIN),
            GET_MT(local_being,METABOLISM_INSULIN));
    io_string_write(result, str, &watch_string_length);

    sprintf(str,"%s: %d\n",
            metabolism_description(METABOLISM_ADIPOSE),
            GET_MT(local_being,METABOLISM_ADIPOSE));
    io_string_write(result, str, &watch_string_length);

    io_string_write(result, "Tissue:\n", &watch_string_length);
    sprintf(str,"  %s: %d   %s: %d\n  %s: %d   %s: %d\n  %s: %d   %s: %d\n  %s: %d   %s: %d\n  %s: %d\n  %s: %d\n",
            metabolism_description(METABOLISM_PROLACTIN),
            GET_MT(local_being,METABOLISM_PROLACTIN),
            metabolism_description(METABOLISM_MILK),
            GET_MT(local_being,METABOLISM_MILK),
            metabolism_description(METABOLISM_AMINO_ACIDS),
            GET_MT(local_being,METABOLISM_AMINO_ACIDS),
            metabolism_description(METABOLISM_MUSCLE),
            GET_MT(local_being,METABOLISM_MUSCLE),
            metabolism_description(METABOLISM_GLUCOSE),
            GET_MT(local_being,METABOLISM_GLUCOSE),
            metabolism_description(METABOLISM_PYRUVATE),
            GET_MT(local_being,METABOLISM_PYRUVATE),
            metabolism_description(METABOLISM_ADP),
            GET_MT(local_being,METABOLISM_ADP),
            metabolism_description(METABOLISM_ATP),
            GET_MT(local_being,METABOLISM_ATP),
            metabolism_description(METABOLISM_ENERGY),
            GET_MT(local_being,METABOLISM_ENERGY),
            metabolism_description(METABOLISM_HEAT),
            GET_MT(local_being,METABOLISM_HEAT));
    io_string_write(result, str, &watch_string_length);
#endif
}

static n_string static_result;

static void watch_line_braincode(n_string string, n_int line)
{
    io_string_write(static_result, string, &watch_string_length);
    io_string_write(static_result, "\n", &watch_string_length);
}

/**
 * Shows braincode for the given being
 * @param ptr pointer to noble_simulation object
 * @param beingname Name of the being
 * @param local_being being to be viewed
 * @param result returned text
 */
static void watch_braincode(void *ptr, n_string beingname, noble_being * local_being, n_string result)
{
    
#ifdef BRAINCODE_ON
    n_int i;
    io_string_write(result, "\nRegisters:\n", &watch_string_length);
    for (i=0; i<BRAINCODE_PSPACE_REGISTERS; i++)
    {
        result[watch_string_length++]=(char)(65+(local_being->braincode_register[i]%60));
    }
    result[watch_string_length++]='\n';
    result[watch_string_length++]='\n';

    static_result = result;

    console_populate_braincode(ptr,watch_line_braincode);

    static_result = 0L;
    result[watch_string_length++]='\n';
#endif
}

static void watch_speech(void *ptr, n_string beingname, noble_being * local, n_string result)
{
#ifdef BRAINCODE_ON
    n_int loop;
    n_byte * external_bc = being_braincode_external(local);
    for (loop = 0; loop < BRAINCODE_SIZE/BRAINCODE_BYTES_PER_INSTRUCTION; loop++)
    {
        n_string_block sentence;

        brain_sentence((n_string)sentence, &external_bc[loop*3]);

        io_string_write(result, sentence, &watch_string_length);
        if ((loop &3) == 3)
        {
            result[watch_string_length++]='.';
        }
        if (loop < BRAINCODE_SIZE/BRAINCODE_BYTES_PER_INSTRUCTION-1)
        {
            result[watch_string_length++]=' ';
        }
    }
    result[watch_string_length++]='.';
    result[watch_string_length++]='\n';
#endif
}



/**
 * Shows the social graph for the given being
 * @param ptr pointer to noble_simulation object
 * @param beingname Name of the being
 * @param local_being being to be viewed
 * @param result returned text
 */
static void watch_social_graph(void *ptr, n_string beingname, noble_being * local_being, n_string result)
{
    io_string_write(result, "\nFriends:\n", &watch_string_length);
    show_friends(ptr, beingname, 0, result);
    io_string_write(result, "\nEnemies:\n", &watch_string_length);
    show_friends(ptr, beingname, 1, result);
}

/**
 * Shows the episodic memory for the given being
 * @param ptr pointer to noble_simulation object
 * @param beingname Name of the being
 * @param local_being being to be viewed
 * @param result returned text
 */
static void watch_episodic(void *ptr, n_string beingname, noble_being * local_being, n_string result)
{
    noble_simulation * local_sim = (noble_simulation *) ptr;

    n_uint i;
    for (i = 0; i < EPISODIC_SIZE; i++)
    {
        n_string_block str = {0};
        n_string_block description = {0};
        n_int          position = 0;
        (void)episode_description(local_sim, local_being, i, str);
        if (io_length(str, STRING_BLOCK_SIZE) > 0)
        {
            if (GET_A(local_being,ATTENTION_EPISODE) != i)
            {
                io_string_write(description, "  ", &position);
                io_string_write(description, str, &position);
                io_string_write(description, "\n", &position);
            }
            else
            {
                io_string_write(description, " [", &position);
                io_string_write(description, str, &position);
                io_string_write(description, "]\n", &position);
            }
            description[position] = 0;
            io_string_write(result, description, &watch_string_length);
        }
    }
}

/**
 * Shows the genome for the given being
 * @param ptr pointer to noble_simulation object
 * @param beingname Name of the being
 * @param local_being being to be viewed
 * @param result returned text
 */
static void watch_genome(void *ptr, n_string beingname, noble_being * local_being, n_string result)
{
    n_int i,j;
    n_byte genome[CHROMOSOMES*8+1];

    for (i = 0; i < 2; i++)
    {
        body_genome((n_byte)i, being_genetics(local_being), genome);
        for (j = 0; j < CHROMOSOMES*8; j++)
        {
            if ((j>0) && (j%8==0))
            {
                io_string_write(result, "\t", &watch_string_length);
            }
            result[watch_string_length++] = genome[j];
        }
        io_string_write((n_string)result, "\n", &watch_string_length);
    }
}

/**
 * Shows brainprobes for the given being
 * @param ptr pointer to noble_simulation object
 * @param beingname Name of the being
 * @param local_being being to be viewed
 * @param result returned text
 */
static void watch_brainprobes(void *ptr, n_string beingname, noble_being * local_being, n_string result)
{
#ifdef BRAINCODE_ON
    n_int i;
    n_string_block str2;
    n_string_block type_str;
    io_string_write(result, "\n  Type    Posn  Freq Offset Addr State\n  ", &watch_string_length);
    for (i = 0; i < 36; i++)
    {
        io_string_write(result, "-", &watch_string_length);
    }
    io_string_write(result, "\n", &watch_string_length);

    sprintf((n_string)type_str,"%s","Input ");

    for (i = 0; i < BRAINCODE_PROBES; i++)
    {
        if (local_being->brainprobe[i].type == INPUT_SENSOR)
        {
            sprintf((n_string)str2,"  %s  %03d   %02d   %03d    %03d  %d\n",
                    type_str,
                    local_being->brainprobe[i].position,
                    local_being->brainprobe[i].frequency,
                    local_being->brainprobe[i].offset,
                    local_being->brainprobe[i].address,
                    local_being->brainprobe[i].state);
            io_string_write(result, (n_string)str2, &watch_string_length);
        }
    }

    sprintf((n_string)type_str,"%s","Output");

    for (i = 0; i < BRAINCODE_PROBES; i++)
    {
        if (local_being->brainprobe[i].type == OUTPUT_ACTUATOR)
        {
            sprintf((n_string)str2,"  %s  %03d   %02d   %03d    %03d  %d\n",
                    type_str,
                    local_being->brainprobe[i].position,
                    local_being->brainprobe[i].frequency,
                    local_being->brainprobe[i].offset,
                    local_being->brainprobe[i].address,
                    local_being->brainprobe[i].state);
            io_string_write(result, (n_string)str2, &watch_string_length);
        }
    }
#endif
}

/**
 * Shows the main parameters for the given being
 * @param ptr pointer to noble_simulation object
 * @param beingname Name of the being
 * @param local_being being to be viewed
 * @param result returned text
 */
static void watch_stats(void *ptr, n_string beingname, noble_being * local_being, n_string result)
{
    noble_simulation  *local_sim = (noble_simulation *)ptr;
    n_string_block     str;
    n_string_block     relationship_str;
    n_string_block     status;
    n_int              heart_rate = 0;
    n_int              breathing_rate = 0;

    if (local_being == 0L)
    {
        (void)SHOW_ERROR("No being for stats");
        return;
    }

#ifdef METABOLISM_ON
    heart_rate = GET_MT(local_being,METABOLISM_HEART_RATE)*60/1000;
    breathing_rate = GET_MT(local_being,METABOLISM_BREATHING_RATE)*60/1000;
#endif

    being_state_description(being_state(local_being), status);
    being_relationship_description(GET_A(local_being,ATTENTION_RELATIONSHIP),relationship_str);

    sprintf(str, "\n=== %s ===\n%s\nGeneration %lu:%lu\nHeart rate %d bpm\tBreathing rate %d Vf\nEnergy %ld\t\tLocation: %ld %ld\nHonor: %d\t\tHeight: %d\nFacing: %d\t\tSex: %c\nAge in days: %ld\nDrives:\n  Hunger: %d\t\tSocial: %d\n  Fatigue: %d\t\tSex: %d\nBody Attention: %s\nRelationship Attention: %s\n",
            beingname, status,
            (n_uint)local_being->generation_min,
            (n_uint)local_being->generation_max,
            (int)heart_rate, (int)breathing_rate,
            being_energy(local_being),
            being_location_x(local_being), being_location_y(local_being),
            being_honor(local_being),
            GET_BEING_HEIGHT(local_being),
            being_facing(local_being),
            ((FIND_SEX(GET_I(local_being)) == SEX_FEMALE) ? 'F' : 'M'),
            TIME_IN_DAYS(local_sim->land->date) - being_dob(local_being),
            (int)being_drive(local_being, DRIVE_HUNGER),
            (int)being_drive(local_being, DRIVE_SOCIAL),
            (int)being_drive(local_being, DRIVE_FATIGUE),
            (int)being_drive(local_being, DRIVE_SEX),
            being_body_inventory_description(GET_A(local_being,ATTENTION_BODY)),
            relationship_str
           );

    io_string_write(result, str, &watch_string_length);
    io_string_write(result, "Friends:\n", &watch_string_length);
    show_friends(ptr, beingname, 0, result);
    io_string_write(result, "Enemies:\n", &watch_string_length);
    show_friends(ptr, beingname, 1, result);
    io_string_write(result, "Episodic:\n", &watch_string_length);
    watch_episodic(ptr, beingname, local_being, result);
}


/**
 * This should duplicate all console for watch functions
 * @param ptr pointer to noble_simulation object
 * @param response command parameters
 * @param output_function function used to show the output
 * @param title title
 * @param watch_function watch function
 * @return 0
 */
static n_int console_duplicate(void * ptr, n_string response, n_console_output output_function, n_string title, console_generic watch_function)
{
    noble_simulation * local_sim = (noble_simulation *) ptr;
    noble_being * local_being = 0L;
    n_string_block beingstr;

    watch_string_length=0;

    if ((response == 0) && local_sim->select != NO_BEINGS_FOUND)
    {
        response = being_get_select_name(local_sim);
        if (title != 0L)
        {
            io_string_write((n_string)beingstr, "\n", &watch_string_length);
            io_string_write((n_string)beingstr, title, &watch_string_length);
            io_string_write((n_string)beingstr, " for ", &watch_string_length);
            io_string_write((n_string)beingstr, response, &watch_string_length);
            io_string_write((n_string)beingstr, "\n", &watch_string_length);
        }
    }

    if (response != 0L)
    {
        local_being = being_from_name(local_sim, response);
        if (local_being == 0L)
        {
            (void)SHOW_ERROR("Being not found");
            return 0;
        }
        being_set_select_name(local_sim, response);

        watch_function(ptr, being_get_select_name(local_sim), local_being, (n_string)beingstr);
        beingstr[watch_string_length] = 0;
        output_function(beingstr);
        return 0;
    }

    (void)SHOW_ERROR("No being was specified");
    return 0;
}

/**
 *
 * @param ptr pointer to noble_simulation object
 * @param response
 * @param output_function
 * @return 0
 */
n_int console_genome(void * ptr, n_string response, n_console_output output_function)
{
    return console_duplicate(ptr, response, output_function, "Genome", watch_genome);
}

/**
 *
 * @param ptr pointer to noble_simulation object
 * @param response
 * @param output_function
 * @return 0
 */
n_int console_stats(void * ptr, n_string response, n_console_output output_function)
{
    return console_duplicate(ptr, response, output_function, 0L, watch_stats);
}

/**
 *
 * @param ptr pointer to noble_simulation object
 * @param response
 * @param output_function
 * @return 0
 */
n_int console_probes(void * ptr, n_string response, n_console_output output_function)
{
    return console_duplicate(ptr, response, output_function, "Brain probes", watch_brainprobes);
}

/**
 * Show the episodic memory
 * @param ptr pointer to noble_simulation object
 * @param response
 * @param output_function
 * @return 0
 */
n_int console_episodic(void * ptr, n_string response, n_console_output output_function)
{
    return console_duplicate(ptr, response, output_function, "Episodic memory", watch_episodic);
}

/**
 * Show the social graph
 * @param ptr pointer to noble_simulation object
 * @param response
 * @param output_function
 * @return 0
 */
n_int console_social_graph(void * ptr, n_string response, n_console_output output_function)
{
    return console_duplicate(ptr, response, output_function, "Social graph", watch_social_graph);
}

/**
 * Show the braincode
 * @param ptr pointer to noble_simulation object
 * @param response
 * @param output_function
 * @return 0
 */
n_int console_braincode(void * ptr, n_string response, n_console_output output_function)
{
    return console_duplicate(ptr, response, output_function, "Braincode", watch_braincode);
}

n_int console_speech(void * ptr, n_string response, n_console_output output_function)
{
    return console_duplicate(ptr, response, output_function, "Speech", watch_speech);
}

/**
 * Show the vascular system
 * @param ptr pointer to noble_simulation object
 * @param response
 * @param output_function
 * @return 0
 */
n_int console_vascular(void * ptr, n_string response, n_console_output output_function)
{
#ifdef METABOLISM_ON
    return console_duplicate(ptr, response, output_function, "Vascular system", watch_vascular);
#else
    return 0;
#endif
}

/**
 * Show appearance values
 * @param ptr pointer to noble_simulation object
 * @param response
 * @param output_function
 * @return 0
 */
n_int console_appearance(void * ptr, n_string response, n_console_output output_function)
{
    return console_duplicate(ptr, response, output_function, "Appearance", watch_appearance);
}

/**
 * Show respiration values
 * @param ptr pointer to noble_simulation object
 * @param response
 * @param output_function
 * @return 0
 */
n_int console_respiration(void * ptr, n_string response, n_console_output output_function)
{
#ifdef METABOLISM_ON
    return console_duplicate(ptr, response, output_function, "Respiration system", watch_respiration);
#else
    return 0;
#endif
}

/**
 * Show metabolism values
 * @param ptr pointer to noble_simulation object
 * @param response
 * @param output_function
 * @return 0
 */
n_int console_metabolism(void * ptr, n_string response, n_console_output output_function)
{
#ifdef METABOLISM_ON
    return console_duplicate(ptr, response, output_function, "Metabolism", watch_metabolism);
#else
    return 0;
#endif
}

/**
 * Update a histogram of being states
 * @param local_sim pointer to the simulation object
 * @param histogram histogram array to be updated
 * @param normalize whether to normalize the histogram
 */
static void histogram_being_state(noble_simulation * local_sim, n_uint * histogram, n_byte normalize)
{
    n_uint i, n=2, tot=0;

    for (i = 0; i < BEING_STATES; i++) histogram[i] = 0;

    for (i = 0; i < local_sim->num; i++)
    {
        noble_being * local_being = &local_sim->beings[i];
        if (being_state(local_being) == BEING_STATE_ASLEEP)
        {
            histogram[0]++;
        }
        else
        {
            while (n < BEING_STATES)
            {
                if (being_state(local_being) & (1<<(n-1)))
                {
                    histogram[n]++;
                }
                n++;
            }
        }
    }

    if (normalize)
    {
        for (i = 0; i < BEING_STATES; i++) tot += histogram[i];
        if (tot > 0)
        {
            for (i = 0; i < BEING_STATES; i++) histogram[i] = histogram[i]*1000/tot;
        }
    }
}


extern n_int being_remove_internal;
extern n_int being_remove_external;
/**
 * Watch a particular being
 * @param ptr pointer to noble_simulation object
 * @param output_function output function to be used
 */
static void watch_being(void * ptr, n_console_output output_function)
{
    noble_simulation * local_sim = (noble_simulation *) ptr;

    noble_being * local_being;
    n_string_block beingstr;
    n_uint i;
    n_int  j;
    n_byte2 state;


    if (being_remove_internal)
        do {}
        while(being_remove_internal);

    being_remove_external = 1;

    if (watch_type == WATCH_STATES)
    {
        n_uint histogram[16];
        n_string_block str;

        watch_string_length=0;

        sprintf((n_string)str,"\nTime:        %02d:%02d\n\n",
                (int)(local_sim->land->time/60),
                (int)(local_sim->land->time%60));
        io_string_write(beingstr,str,&watch_string_length);
        histogram_being_state(local_sim, (n_uint*)histogram, 1);
        for (i = 0; i < BEING_STATES; i++)
        {
            if (i == 1) continue; /**< skip the awake state */

            if (i==0)
            {
                state = 0;
            }
            else
            {
                state = (n_byte2)(1 << (i-1));
            }

            being_state_description(state, (n_string)str);
            io_string_write(beingstr,(n_string)str,&watch_string_length);
            io_string_write(beingstr,":",&watch_string_length);
            for (j = 0; j < 12 - io_length((n_string)str,STRING_BLOCK_SIZE); j++)
            {
                io_string_write(beingstr," ",&watch_string_length);
            }

            if (histogram[i] > 0)
            {
                sprintf((n_string)str,"%.1f\n",histogram[i]/10.0f);
                io_string_write(beingstr,str,&watch_string_length);
            }
            else
            {
                io_string_write(beingstr,"----\n",&watch_string_length);
            }
        }
        beingstr[watch_string_length] = 0;

        output_function(beingstr);
        return;
    }

    if (local_sim->select != NO_BEINGS_FOUND)
    {
        local_being = &(local_sim->beings[local_sim->select]);

        watch_string_length = 0;

        switch(watch_type)
        {
        case WATCH_ALL:
        {
            watch_stats(ptr, being_get_select_name(local_sim), local_being, beingstr);
            break;
        }
        case WATCH_SOCIAL_GRAPH:
        {
            watch_social_graph(ptr, being_get_select_name(local_sim), local_being, beingstr);
            break;
        }
        case WATCH_EPISODIC:
        {
            watch_episodic(ptr, being_get_select_name(local_sim), local_being, beingstr);
            break;
        }
        case WATCH_BRAINCODE:
        {
            watch_braincode(ptr, being_get_select_name(local_sim), local_being, beingstr);
            break;
        }
        case WATCH_BRAINPROBES:
        {
            watch_brainprobes(ptr, being_get_select_name(local_sim), local_being, beingstr);
            break;
        }
        case WATCH_VASCULAR:
        {
            watch_vascular(ptr, being_get_select_name(local_sim), local_being, beingstr);
            break;
        }
        case WATCH_RESPIRATION:
        {
            watch_respiration(ptr, being_get_select_name(local_sim), local_being, beingstr);
            break;
        }
        case WATCH_METABOLISM:
        {
            watch_metabolism(ptr, being_get_select_name(local_sim), local_being, beingstr);
            break;
        }
        case WATCH_APPEARANCE:
        {
            watch_appearance(ptr, being_get_select_name(local_sim), local_being, beingstr);
            break;
        }
        case WATCH_SPEECH:
        {
            watch_speech(ptr, being_get_select_name(local_sim), local_being, beingstr);
            break;
        }
        }

        beingstr[watch_string_length] = 0;

        if (watch_type!=WATCH_NONE)
        {
            output_function(beingstr);
        }
    }
    being_remove_external = 0;
}

static n_int console_on_off(n_string response)
{
    n_uint length;

    if (response == 0) return -1;

    length = io_length(response,STRING_BLOCK_SIZE);
    if ((io_find(response,0,length,"off",3)>-1) ||
            (io_find(response,0,length,"0",1)>-1) ||
            (io_find(response,0,length,"false",5)>-1) ||
            (io_find(response,0,length,"no",2)>-1))
    {
        return 0;
    }

    if ((io_find(response,0,length,"on",2)>-1) ||
            (io_find(response,0,length,"1",1)>-1) ||
            (io_find(response,0,length,"true",4)>-1) ||
            (io_find(response,0,length,"yes",3)>-1))
    {
        return 1;
    }

    return -1;
}

n_int console_event(void * ptr, n_string response, n_console_output output_function)
{
    n_int return_response = console_on_off(response);

    if (return_response == -1)
    {
        if (io_find(response, 0, io_length(response,STRING_BLOCK_SIZE),"social",6)>-1)
        {
            episodic_logging(output_function, 1);
            output_function("Event output for social turned on");
        }
        return 0;
    }

    if (return_response == 0)
    {
        episodic_logging(0L, 0);
        output_function("Event output turned off");
    }
    else
    {
        episodic_logging(output_function, 0);
        output_function("Event output turned on");
    }

    return 0;
}


/**
 * Enable or disable logging
 * @param ptr pointer to noble_simulation object
 * @param response command parameters - off/on/0/1/yes/no
 * @param output_function function to be used to display output
 * @return 0
 */
n_int console_logging(void * ptr, n_string response, n_console_output output_function)
{
    n_int return_response = console_on_off(response);

    if (return_response == -1)
    {
        return 0;
    }
    if (return_response == 0)
    {
        nolog = 1;
        indicator_index = 0;
        watch_type = WATCH_NONE;
        output_function("Logging turned off");
    }
    else
    {
        nolog = 0;
        indicator_index = 1;
        output_function("Logging turned on");
    }
    return 0;
}


/**
 * Compare two braincode arrays
 * @param braincode0 Braincode array for the first being
 * @param braincode1 Braincode byte array for the second being
 * @param block_size The number of instructions to compare within a block
 * @returns Location of the first match within the first braincode array
 */
static n_int console_compare_brain(n_byte * braincode0, n_byte * braincode1, n_int block_size)
{
    n_int block_size_bytes = block_size*BRAINCODE_BYTES_PER_INSTRUCTION;
    n_int loop = 0;
    while (loop < (BRAINCODE_SIZE - block_size_bytes))
    {
        n_int loop2 = 0;
        while (loop2 < (BRAINCODE_SIZE - block_size_bytes))
        {
            n_int block_step = 0;
            while (block_step < block_size)
            {
                if (braincode0[loop + block_step*BRAINCODE_BYTES_PER_INSTRUCTION] ==
                        braincode1[loop2 + block_step*BRAINCODE_BYTES_PER_INSTRUCTION])
                {
                    block_step++;
                    if (block_step == block_size)
                    {
                        return loop;
                    }
                }
                else
                {
                    break;
                }
            }
            loop2 += BRAINCODE_BYTES_PER_INSTRUCTION;
        }
        loop += BRAINCODE_BYTES_PER_INSTRUCTION;
    }

    return -1;
}

/**
 * Shows repeated sections of braincode
 * @param ptr
 * @param response
 * @param output_function
 * @returns 0
 */
n_int console_idea(void * ptr, n_string response, n_console_output output_function)
{
#ifdef BRAINCODE_ON
#ifndef CONSOLE_IDEA_MIN_BLOCK_SIZE
#define CONSOLE_IDEA_MIN_BLOCK_SIZE 3
#define CONSOLE_IDEA_MAX_BLOCK_SIZE 8
#endif
    const n_int min_block_size = 3;
    const n_int max_block_size = 8;
    n_uint i, total_matches=0, total_tests=0;
    n_uint histogram[CONSOLE_IDEA_MIN_BLOCK_SIZE - CONSOLE_IDEA_MIN_BLOCK_SIZE + 1];
    noble_simulation * local_sim = (noble_simulation *) ptr;

    /* clear the histogram */
    for (i = 0; i <= (n_uint)(max_block_size - min_block_size); i++)
    {
        histogram[i]=0;
    }

    if (local_sim->select != NO_BEINGS_FOUND)
    {
        n_uint loop = 0;
        while (loop < local_sim->num)
        {
            noble_being * local_being = &(local_sim->beings[loop]);
            n_byte * bc_external = being_braincode_external(local_being);
            if (bc_external)
            {

                n_uint loop2 = loop+1;
                while (loop2 < local_sim->num)
                {
                    noble_being * local_being2 = &(local_sim->beings[loop2]);
                    n_byte * bc_external2 = being_braincode_external(local_being2);

                    if (bc_external2)
                    {
                        n_int   location = 0;
                        n_int   block_size = min_block_size;

                        while (block_size <= max_block_size)
                        {
                            location = console_compare_brain(bc_external,
                                                             bc_external2,
                                                             block_size);

                            if (location != -1)
                            {
                                histogram[block_size-min_block_size]++;
                                total_matches++;
                                /* n_string_block output;
                                sprintf(output, "%ld %ld, %ld",loop, loop2, block_size);
                                output_function(output); */
                            }
                            total_tests++;
                            block_size++;
                        }
                    }
                    loop2++;
                }

            }
            loop++;
        }
    }

    if (total_tests > 0)
    {
        n_string_block output;
        
        sprintf(output, "Matches %03u.%04u percent\n",
                (n_c_int)(total_matches*100/total_tests),
                (n_c_int)(total_matches*1000000/total_tests)%10000);
        output_function(output);

    
        output_function("Block Percent   Instances");
        output_function("-------------------------");

        for (i = 0; i <= (n_uint)(max_block_size - min_block_size); i++)
        {
            sprintf(output, "%02u    %03u.%04u  %04u",
                    (n_c_int)(i+min_block_size),
                    (n_c_int)(histogram[i]*100/total_tests),
                    (n_c_int)((histogram[i]*1000000/total_tests)%10000),
                    (n_c_int)histogram[i]);
            output_function(output);
        }
    }

#endif
    return 0;
}

/**
 * Watch a particular being
 * @param ptr pointer to noble_simulation object
 * @param response command parameters
 * @param output_function function to be used to display output
 * @return 0
 */
n_int console_watch(void * ptr, n_string response, n_console_output output_function)
{
    noble_simulation * local_sim = (noble_simulation *) ptr;
    n_int          length;
    n_string_block output;
    n_int          position = 0;

    if (response == 0L)
    {
        return 0;
    }
    else
    {
        length = io_length(response,STRING_BLOCK_SIZE);
    }

    if ((length<5) && (io_find(response,0,length,"off",3)>-1))
    {
        output_function("Stopped watching");
        watch_type=WATCH_NONE;
        return 0;
    }

    if ((length<10) && (io_find(response,0,length,"state",5)>-1))
    {
        watch_type = WATCH_STATES;
        output_function("Watching being states");
        return 0;
    }

    if (being_from_name(local_sim, response) != 0)
    {
        being_set_select_name(local_sim, response);
        io_string_write(output, "Watching ", &position);
        io_string_write(output, being_get_select_name(local_sim), &position);
        output[position] = 0;
        output_function(output);
        position = 0;
        watch_type = WATCH_ALL;
    }
    else
    {
        if (local_sim->select != NO_BEINGS_FOUND)
        {
            if (io_find(response,0,length,"braincode",9)>-1)
            {
                watch_type = WATCH_BRAINCODE;
                io_string_write(output, "Watching braincode for ", &position);
                io_string_write(output, being_get_select_name(local_sim), &position);
                output[position] = 0;
                output_function(output);
                return 0;
            }
            if ((io_find(response,0,length,"brainprobe",10)>-1) ||
                    (io_find(response,0,length,"brain probe",11)>-1) ||
                    (io_find(response,0,length,"probes",6)>-1))
            {
                watch_type = WATCH_BRAINPROBES;
                io_string_write(output, "Watching brain probes for ", &position);
                io_string_write(output, being_get_select_name(local_sim), &position);
                output[position] = 0;
                output_function(output);
                return 0;
            }
            if ((io_find(response,0,length,"graph",5)>-1) ||
                    (io_find(response,0,length,"friend",6)>-1))
            {
                watch_type = WATCH_SOCIAL_GRAPH;
                
                io_string_write(output, "Watching social graph for ", &position);
                io_string_write(output, being_get_select_name(local_sim), &position);
                output[position] = 0;
                output_function(output);
                return 0;
            }
            if ((io_find(response,0,length,"episodic",8)>-1) ||
                    (io_find(response,0,length,"episodic memory",15)>-1) ||
                    (io_find(response,0,length,"memory",6)>-1))
            {
                watch_type = WATCH_EPISODIC;
                
                io_string_write(output, "Watching episodic memory for ", &position);
                io_string_write(output, being_get_select_name(local_sim), &position);
                output[position] = 0;
                output_function(output);
                return 0;
            }
            if (io_find(response,0,length,"speech",6)>-1)
            {
                watch_type = WATCH_SPEECH;
                
                io_string_write(output, "Watching speech for ", &position);
                io_string_write(output, being_get_select_name(local_sim), &position);
                output[position] = 0;
                output_function(output);
                return 0;
            }
            if ((length<5) && (io_find(response,0,length,"all",3)>-1))
            {
                watch_type = WATCH_ALL;

                io_string_write(output, "Watching ", &position);
                io_string_write(output, being_get_select_name(local_sim), &position);
                output[position] = 0;
                output_function(output);
                return 0;
            }
            if ((io_find(response,0,length,"cardi",5)>-1) ||
                    (io_find(response,0,length,"vasc",4)>-1))
            {
                watch_type = WATCH_VASCULAR;
                
                io_string_write(output, "Watching vascular system for ", &position);
                io_string_write(output, being_get_select_name(local_sim), &position);
                output[position] = 0;
                output_function(output);
                return 0;
            }
            if ((io_find(response,0,length,"breath",6)>-1) ||
                    (io_find(response,0,length,"respir",6)>-1))
            {
                watch_type = WATCH_RESPIRATION;
                
                io_string_write(output, "Watching respiration system for ", &position);
                io_string_write(output, being_get_select_name(local_sim), &position);
                output[position] = 0;
                output_function(output);
                return 0;
            }
            if ((io_find(response,0,length,"metab",5)>-1) ||
                    (io_find(response,0,length,"chem",4)>-1))
            {
                watch_type = WATCH_METABOLISM;
                
                io_string_write(output, "Watching metabolism for ", &position);
                io_string_write(output, being_get_select_name(local_sim), &position);
                output[position] = 0;
                output_function(output);
                return 0;
            }
            if (io_find(response,0,length,"appear",6)>-1)
            {
                watch_type = WATCH_APPEARANCE;
                io_string_write(output, "Watching appearance for ", &position);
                io_string_write(output, being_get_select_name(local_sim), &position);
                output[position] = 0;
                output_function(output);
                return 0;
            }
        }

        output_function("Being not found\n");
    }
    return 0;
}

/**
 * Set the time interval for simulation
 * @param ptr pointer to noble_simulation object
 * @param response command parameters
 * @param output_function function used to display the output
 * @return 0
 */
n_int console_interval(void * ptr, n_string response, n_console_output output_function)
{
    n_string_block  output;
    n_int number=1,interval=INTERVAL_DAYS,interval_set=0;

    if (response != 0)
    {
        if (io_length(response, STRING_BLOCK_SIZE) > 0)
        {
            if (get_time_interval(response, &number, &interval) > -1)
            {
                if (number > 0)
                {
                    save_interval_steps = number * interval_steps[interval];
                    sprintf(output, "Logging interval set to %u %s",(unsigned int)number, interval_description[interval]);
                    output_function(output);
                    interval_set=1;
                }
            }
        }
    }

    if (interval_set == 0)
    {
        if (save_interval_steps < 60)
        {
            sprintf(output,"Current time interval is %d mins", (int)save_interval_steps);
            output_function(output);
        }
        else
        {
            if (save_interval_steps < 60*24)
            {
                sprintf(output,"Current time interval is %d hours", (int)save_interval_steps/60);
                output_function(output);
            }
            else
            {
                sprintf(output,"Current time interval is %d days", (int)save_interval_steps/(60*24));
                output_function(output);
            }
        }
    }
    return 0;
}

n_int console_stop(void * ptr, n_string response, n_console_output output_function)
{
    simulation_running = 0;
    if (output_function)
    {
        output_function("Simulation stopped");
    }
    return 0;
}

/**
 *
 * @param ptr
 * @param response
 * @param output_function
 */
n_int console_file(void * ptr, n_string response, n_console_output output_function)
{
    io_search_file_format(noble_file_format, response);
    return 0;
}

/**
 * Run the simulation for a single time interval
 * @param ptr pointer to noble_simulation object
 * @param response command parameters
 * @param output_function function used to display the output
 * @return 0
 */
n_int console_step(void * ptr, n_string response, n_console_output output_function)
{
    noble_simulation * local_sim = (noble_simulation *) ptr;
    n_uint loop = 0;

    if (response != RUN_STEP_CONST)
    {
        if (simulation_executing == 1)
        {
            output_function("Simulation already running");
            return 0;
        }

        if (console_file_interaction)
        {
            output_function("File interaction in use: step");
            return 0;
        }

        simulation_executing = 1;
    }
    simulation_running = 1;

    while ((loop < save_interval_steps) && simulation_running)
    {
        sim_cycle();
        watch_being(local_sim, output_function);
        if (local_sim->num == 0)
        {
            simulation_running = 0;
        }
        loop++;
    }

    if (response != RUN_STEP_CONST)
    {
        simulation_executing = 0;
    }

    return 0;
}

/**
 * Run the simulation
 * @param ptr pointer to noble_simulation object
 * @param response command parameters
 * @param output_function function used to display the output
 * @return 0
 */
n_int console_run(void * ptr, n_string response, n_console_output output_function)
{
    n_uint run = 0;
    n_int  number = 0, interval = INTERVAL_DAYS;
    n_int  forever = 0;

    if (simulation_executing == 1)
    {
        output_function("Simulation already running");
        return 0;
    }

    if (console_file_interaction)
    {
        output_function("File interaction in use: run");
        return 0;
    }

    simulation_executing = 1;

    simulation_running = 1;

    if (response != 0L)
    {
        n_int length = io_length(response,STRING_BLOCK_SIZE);
        if (length > 0)
        {
            if ((io_find(response,0,length,"forever",7)>-1))
            {
                forever = 1;
                number = 1;
            }
            else if (get_time_interval(response, &number, &interval) <= -1)
            {
                number = -1;
            }

            if (number > 0)
            {
                n_uint i = 0;
                n_string_block  output;
                n_uint end_point = (number * interval_steps[interval]);
                n_uint temp_save_interval_steps = save_interval_steps;
                save_interval_steps = 1;

                if (forever)
                {
                    sprintf(output, "Running forever (type \"stop\" to end)");
                }
                else
                {
                    sprintf(output, "Running for %d %s", (int)number, interval_description[interval]);
                }

                output_function(output);

                while ((i < end_point) && simulation_running)
                {
                    console_step(ptr, RUN_STEP_CONST, output_function);
                    if (!forever) i++;
                }
                save_interval_steps = temp_save_interval_steps;
                run = 1;
            }

        }
    }

    simulation_executing = 0;

    if (run == 0)
    {
        (void)SHOW_ERROR("Time not specified, examples: run 2 days, run 6 hours");
    }

    return 0;
}

/**
 * Reset the simulation
 * @param ptr pointer to noble_simulation object
 * @param response command parameters
 * @param output_function function used to display the output
 * @return 0
 */
n_int console_reset(void * ptr, n_string response, n_console_output output_function)
{
    noble_simulation * local_sim = (noble_simulation *) ptr;
    n_byte2 seed[2];

    seed[0] = local_sim->land->genetics[0];
    seed[1] = local_sim->land->genetics[1];

    math_random3(seed);

    if (sim_init(KIND_NEW_SIMULATION, (seed[0]<<16)|seed[1], MAP_AREA, 0))
    {
        output_function("Simulation reset");
        return 0;
    }
    output_function("Simulation has not enough memory");
    return 1;
}

/**
 * Returns a mode value used by epic and top commands
 * @param response command parameter: female/male/juvenile
 * @return 0
 */
static n_byte get_response_mode(n_string response)
{
    n_int length;
    if (response == 0L) return 0;
    length = io_length(response,STRING_BLOCK_SIZE);
    if (response != 0)
    {
        /** females */
        if (io_find(response,0,length,"fem",3)>-1)
        {
            return 1;
        }
        /** males */
        if (io_find(response,0,length,"male",4)>-1)
        {
            return 2;
        }
        /** juveniles */
        if ((io_find(response,0,length,"juv",3)>-1) ||
                (io_find(response,0,length,"chil",4)>-1))
        {
            return 3;
        }
    }
    return 0;
}

n_int console_speak(void * ptr, n_string response, n_console_output output_function)
{
    n_string_block paragraph = {0};
    noble_simulation * local_sim = (noble_simulation*) ptr;
    noble_being * local = &(local_sim->beings[local_sim->select]);
    watch_speech(ptr, 0L, local, paragraph);
    watch_string_length = 0;
    speak_out(response, paragraph);
    return 0;
}

n_int console_alphabet(void * ptr, n_string response, n_console_output output_function)
{
    speak_out(response, " aeio a e i o vfstpbjm abefijmopstv a b e f i j m o p s t v.  .  \n");
    return 0;
}


n_int console_save(void * ptr, n_string response, n_console_output output_function)
{
    n_file * file_opened;
    n_string_block output_string;

    if (response == 0L) return 0;

    if (console_file_interaction)
    {
        if (output_function)
        {
            output_function("File interaction in use: save");
        }
        return 0;
    }

    console_stop(ptr,"",output_function);

    file_opened = file_out();
    if (file_opened == 0L)
    {
        return SHOW_ERROR("Failed to generate output contents");
    }

    console_file_interaction = 1;

    io_disk_write(file_opened, response);
    io_file_free(&file_opened);

    if (output_function)
    {
        n_int position = 0;
        io_string_write(console_file_name, response, &position);
        
        console_file_name[position] = 0;
        
        position = 0;
        
        io_string_write(output_string, "Simulation file ", &position);
        io_string_write(output_string, response, &position);
        io_string_write(output_string, " saved\n", &position);
        
        console_file_name[position] = 0;
        
        output_function(output_string);
    }

    console_file_interaction = 0;

    return 0;
}

/* load simulation-data/script */
static n_int console_base_open(void * ptr, n_string response, n_console_output output_function, n_byte script)
{
    if (response == 0L) return 0;

    if (console_file_interaction)
    {
        if (output_function)
        {
            output_function("File interaction in use: open");
        }
        return 0;
    }

    console_stop(ptr,"",output_function);
    console_file_interaction = 1;

    if (io_disk_check(response)!=0)
    {
        n_file * file_opened = io_file_new();
        n_string_block output_string;

        if(io_disk_read(file_opened, response) != FILE_OKAY)
        {
            io_file_free(&file_opened);
            console_file_interaction = 0;
            return SHOW_ERROR("Failed to open file");
        }

        if (script)
        {
            if (file_interpret(file_opened) != 0)
            {
                io_file_free(&file_opened);
                console_file_interaction = 0;
                return SHOW_ERROR("Failed to interpret file");
            }
        }
        else
        {
            if (file_in(file_opened) != 0)
            {
                io_file_free(&file_opened);
                console_file_interaction = 0;
                return SHOW_ERROR("Failed to read in file");
            }
            if (sim_init(KIND_LOAD_FILE, 0, MAP_AREA, 0) == 0L)
            {
                return SHOW_ERROR("Not enough memory to load file");
            }
            io_file_free(&file_opened);
        }
        console_file_interaction = 0;
        if (output_function)
        {
            n_int position = 0;
            io_string_write(console_file_name, response, &position);
            
            console_file_name[position] = 0;
            
            position = 0;
            
            io_string_write(output_string, "Simulation file ", &position);
            io_string_write(output_string, response, &position);
            io_string_write(output_string, " open\n", &position);
            
            console_file_name[position] = 0;
            
            output_function(output_string);
        }
    }
    return 0;
}


/* load simulation data */
n_int console_open(void * ptr, n_string response, n_console_output output_function)
{
    return console_base_open(ptr, response, output_function, 0);
}

/* load apescript file */
n_int console_script(void * ptr, n_string response, n_console_output output_function)
{
    return console_base_open(ptr, response, output_function, 1);
}

/**
 * Displays beings in descending order of honor value
 * @param ptr pointer to noble_simulation object
 * @param response command parameters
 * @param output_function function used to display the output
 * @return 0
 */
n_int console_top(void * ptr, n_string response, n_console_output output_function)
{
#ifdef PARASITES_ON
    noble_simulation * local_sim = (noble_simulation *) ptr;
    n_uint i,j;
    n_int  k;
    n_uint max=10;
    n_byte * eliminated;
    n_uint current_date,local_dob,age_in_years,age_in_months,age_in_days;
    n_string_block str;
    noble_being * b;
    n_byte mode = get_response_mode(response);

    output_function("Honor Name                     Sex\tAge");
    output_function("-----------------------------------------------------------------");

    eliminated = (n_byte *)io_new(local_sim->num*sizeof(n_byte));
    for (i = 0; i < local_sim->num; i++) eliminated[i] = 0;

    if (local_sim->num < max) max = local_sim->num;
    for (i = 0; i < max; i++)
    {
        n_int winner = -1;
        n_int max_honor = 0;
        n_byte passed;
        n_string_block output_value;


        for (j = 0; j < local_sim->num; j++)
        {
            if (eliminated[j] == 0)
            {
                n_int honor;
                b = &local_sim->beings[j];

                honor = being_honor(b);
                
                if (honor >= max_honor)
                {

                    passed=0;
                    switch(mode)
                    {
                    case 0:
                    {
                        passed=1;
                        break;
                    }
                    case 1:
                    {
                        if (FIND_SEX(GET_I(b)) == SEX_FEMALE) passed=1;
                        break;
                    }
                    case 2:
                    {
                        if (FIND_SEX(GET_I(b)) != SEX_FEMALE) passed=1;
                        break;
                    }
                    case 3:
                    {
                        if (AGE_IN_DAYS(local_sim,b)<AGE_OF_MATURITY) passed=1;
                        break;
                    }
                    }

                    if (passed!=0)
                    {
                        winner = j;
                        max_honor = honor;
                    }

                }

            }
        }

        if (winner==-1) break;

        eliminated[winner] = 1;
        b = &local_sim->beings[winner];

        sprintf(output_value, "%03d   ", (int)(being_honor(b)));

        being_name_simple(b, str);
        
        sprintf(output_value, "%s%s", output_value,str);

        for (k=0; k<25-io_length(str,STRING_BLOCK_SIZE); k++) sprintf(output_value, "%s ", output_value);

        if (FIND_SEX(GET_I(b)) == SEX_FEMALE)
        {
            sprintf(output_value,"%sFemale\t",output_value);
        }
        else
        {
            sprintf(output_value,"%sMale\t",output_value);
        }

        current_date = TIME_IN_DAYS(local_sim->land->date);
        local_dob = being_dob(b);
        age_in_years = AGE_IN_YEARS(local_sim,b);
        age_in_months = ((current_date - local_dob) - (age_in_years * TIME_YEAR_DAYS)) / (TIME_YEAR_DAYS/12);
        age_in_days = (current_date - local_dob) - ((TIME_YEAR_DAYS/12) * age_in_months) - (age_in_years * TIME_YEAR_DAYS);

        if (age_in_years>0)
        {
            sprintf(output_value, "%s%02d yrs ", output_value, (int)age_in_years);
        }
        if (age_in_months>0)
        {
            sprintf(output_value,"%s%02d mnths ", output_value,(int)age_in_months);
        }
        sprintf(output_value,"%s%02d days",output_value, (int)age_in_days);

        output_function(output_value);

    }

    io_free((void**)&eliminated);
#endif
    return 0;
}

n_int console_debug(void * ptr, n_string response, n_console_output output_function)
{
    file_audit();
    
    return 0;
}

/**
 * Lists the most talked about beings, based upon episodic memories
 * @param ptr pointer to noble_simulation object
 * @param response command parameters
 * @param output_function function used to display the output
 * @return 0
 */
n_int console_epic(void * ptr, n_string response, n_console_output output_function)
{
    noble_simulation * local_sim = (noble_simulation *) ptr;
    n_uint i, j, k, e;
    noble_being * local_being;
    noble_episodic * local_episodic;
    const n_uint max = 1024;
    n_byte2 * first_name = (n_byte2*)io_new(max*sizeof(n_byte2));
    n_byte2 * family_name = (n_byte2*)io_new(max*sizeof(n_byte2));
    n_byte2 * hits = (n_byte2*)io_new(max*sizeof(n_byte2));
    n_byte2 temp;
    n_string_block name;
    n_byte passed,mode = get_response_mode(response);

    /** clear the list of beings and their hit scores */
    for (i = 0; i < max; i++)
    {
        first_name[i] = 0;
        family_name[i] = 0;
        hits[i] = 0;
    }

    for (i = 0; i < local_sim->num; i++)
    {
        /** get the being */
        local_being = &local_sim->beings[i];

        /** get the episodic memories for the being */
        local_episodic = being_episodic(local_being);

        /** skip is no memories were retrieved */
        if (local_episodic == 0L) continue;

        /** for all memories */
        for (e = 0; e < EPISODIC_SIZE; e++)
        {
            /** non-empty slot */
            if (local_episodic[e].event > 0)
            {
                /** j = 0 is the being having the memory
                     j = 1 is the being who is the subject of the memory */
                for (j = BEING_MEETER; j <= BEING_MET; j++)
                {
                    /** name should be non-zero */
                    if (local_episodic[e].first_name[j] +
                            local_episodic[e].family_name[j] > 0)
                    {

                        passed=0;
                        switch(mode)
                        {
                        case 0:
                        {
                            passed=1;
                            break;
                        }
                        case 1:
                        {
                            if ((local_episodic[e].first_name[j]>>8) == SEX_FEMALE) passed=1;
                            break;
                        }
                        case 2:
                        {
                            if ((local_episodic[e].first_name[j]>>8) != SEX_FEMALE) passed=1;
                            break;
                        }
                        case 3:
                        {
                            noble_being * b=0L;
                            n_string_block name;
                            
                            being_name_byte2(local_episodic[e].first_name[j], local_episodic[e].family_name[j], name);
                            
                            b = being_from_name(local_sim, name);
                            if (b!=0L)
                            {
                                if (AGE_IN_DAYS(local_sim,b)<AGE_OF_MATURITY) passed=1;
                            }
                            break;
                        }
                        }

                        if (passed != 0)
                        {
                            /** Avoid memories about yourself, since we're interested
                               in gossip about other beings */
                            if ((local_episodic[e].first_name[j] != being_gender_name(local_being)) ||
                                    (local_episodic[e].family_name[j] != being_family_name(local_being)))
                            {
                                if (((j == BEING_MET) &&
                                        (local_episodic[e].event != EVENT_SEEK_MATE) &&
                                        (local_episodic[e].event != EVENT_EAT)) ||
                                        (j == BEING_MEETER))
                                {
                                    /** add this being to the list, or increment its hit score */
                                    for (k = 0; k < max; k++)
                                    {
                                        if (hits[k] == 0) /**< last entry in the list */
                                        {
                                            first_name[k] = local_episodic[e].first_name[j];
                                            family_name[k] = local_episodic[e].family_name[j];
                                            break;
                                        }
                                        if (first_name[k] == local_episodic[e].first_name[j])
                                        {
                                            if (family_name[k] == local_episodic[e].family_name[j])
                                            {
                                                /** being found in the list */
                                                break;
                                            }
                                        }
                                    }
                                    /** increment the hit score for the being */
                                    if (k < max) hits[k]++;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    /** top 10 most epic beings */
    for (i = 0; i < 10; i++)
    {
        /** search the rest of the list */
        for (j = i+1; j < max; j++)
        {
            if (hits[j] == 0) break; /**< end of the list */

            /** if this being has more hits then swap list entries, so that
             the most popular beings are at the top of the list */
            if (hits[j] > hits[i])
            {
                /** swap */
                temp = first_name[j];
                first_name[j] = first_name[i];
                first_name[i] = temp;

                temp = family_name[j];
                family_name[j] = family_name[i];
                family_name[i] = temp;

                temp = hits[j];
                hits[j] = hits[i];
                hits[i] = temp;
            }
        }
        if (hits[i] > 0)
        {
            n_string_block output_value;
            /** get the name of the being */
            
            being_name_byte2(first_name[i], family_name[i], name);
            
            sprintf(output_value, "%06d %s", (int)hits[i], name);
            output_function(output_value);
        }
    }

    /** free list memory */
    io_free((void**)&first_name);
    io_free((void**)&family_name);
    io_free((void**)&hits);

    return 0;
}

n_int console_quit(void * ptr, n_string response, n_console_output output_function)
{
    (void)console_stop(ptr, response, output_function);
    return io_quit(ptr, response, output_function);
}

n_file                  * file_death_record = 0L;
static n_int              death_record_single_entry = 1;

n_file * death_record_file_ready(void)
{
    return io_file_ready(death_record_single_entry, file_death_record);
}

void death_record_file_cleanup(void)
{
    io_file_cleanup(&death_record_single_entry, &file_death_record);
}

/*
void death_record_writeoff(void)
{
    io_file_writeoff(&death_record_single_entry, file_death_record);
}
*/
 
void console_capture_death(noble_being * deceased, void * sim)
{
    n_string_block output_string = {0};
    n_string_block being_name = {0};
    
    being_name_simple(deceased, being_name);
    
    watch_stats(sim, being_name, deceased, output_string);
    
    io_file_writeon(&death_record_single_entry, &file_death_record, 0);
    io_file_string(death_record_single_entry, file_death_record, output_string);
}

n_int console_death(void * ptr, n_string response, n_console_output output_function)
{
    
    if (response == 0L) return 0;
    
    console_stop(ptr,"",output_function);

    if (file_death_record == 0L)
    {
        (void)SHOW_ERROR("No output contents");
        return 0;
    }

    if (io_disk_write(file_death_record, response) != FILE_ERROR)
    {
        if (output_function)
        {
            n_string_block output_string;
            n_int          location = 0;
            
            io_string_write(output_string, "Death record file ", &location);
            io_string_write(output_string, response, &location);
            io_string_write(output_string, " saved\n", &location);
            output_string[location] = 0;
            
            output_function(output_string);
        }
    }
    return 0;
}
