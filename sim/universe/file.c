/****************************************************************

 file.c

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

/* does not appear to be used here */

void sim_debug_csv(n_file * fil, n_byte initial)
{
    noble_simulation * local_sim = sim_sim();
    io_write_csv(fil, (n_byte *)(&(local_sim->beings[0])), noble_file_format, FIL_BEI, initial);
}

/* provide an output file buffer to be written */


static void fileout_land(n_file * file_out, noble_simulation * value, noble_file_entry * format)
{
    n_byte2	loc_signature[2] = {(n_byte2)NOBLE_APE_SIGNATURE, (n_byte2)VERSION_NUMBER};

#ifdef USE_FIL_VER
    io_write_buff(file_out, loc_signature, format, FIL_VER, 0L);
#endif
#ifdef USE_FIL_LAN
    io_write_buff(file_out, value->land, format, FIL_LAN, 0L);
#endif
#ifdef USE_FIL_WEA
    io_write_buff(file_out, value->weather, format, FIL_WEA, 0L);
#endif
}

static void fileout_being(n_file * file_out, noble_simulation * value, n_int being, noble_file_entry * format)
{
    n_int loop = (SOCIAL_SIZE * being);
    n_int loop_end = loop + SOCIAL_SIZE;
    n_int loop_episodic = (EPISODIC_SIZE * being);
    n_int loop_episodic_end = loop + EPISODIC_SIZE;

#ifdef USE_FIL_BEI
    io_write_buff(file_out, &(value->beings[being]), format, FIL_BEI, 0L);
#endif
#ifdef USE_FIL_SOE
    while (loop < loop_end)
    {
        io_write_buff(file_out, being_social(&(value->beings[being])), format, FIL_SOE, &brain_three_byte_command);
        loop++;
    }
#endif
#ifdef USE_FIL_EPI
    while (loop_episodic < loop_episodic_end)
    {
        io_write_buff(file_out, being_episodic(&(value->beings[being])), format, FIL_EPI, 0L);
        loop_episodic++;
    }
#endif
}


n_file * file_out(void)
{
    noble_simulation *local_sim = sim_sim();
    n_file           *returnFile = io_file_new();
    n_uint	          loop = 0;
    n_string fluff[5] = {SHORT_VERSION_NAME, FULL_DATE, COPYRIGHT_DATE, COPYRIGHT_NAME, COPYRIGHT_FOLLOW };

    if(returnFile == 0L)
    {
        return 0L;
    }
    if(returnFile->data == 0L)
    {
        io_free(returnFile);
        return 0L;
    }

    io_write_buff(returnFile, fluff, 0L, FILE_COPYRIGHT, 0L);

    fileout_land(returnFile, local_sim, (noble_file_entry *)noble_file_format);

    while (loop < local_sim->num)
    {
        fileout_being(returnFile, local_sim, loop, (noble_file_entry *)noble_file_format);
        loop++;
    }

    /* TODO: Brain block */
    return returnFile;
}

n_byte * sim_fileout(n_uint * len)
{
    noble_simulation *local_sim = sim_sim();

    n_uint	loop = 0;
    n_file	file_pass;

    n_string fluff[5] = {SHORT_VERSION_NAME, FULL_DATE, COPYRIGHT_DATE, COPYRIGHT_NAME, COPYRIGHT_FOLLOW };

    file_pass . size = 2048;
    file_pass . location = 0;
    file_pass . data = io_new(2048);
    if(file_pass . data == 0L)
        return 0L;

    io_write_buff(&file_pass, fluff, 0L, FILE_COPYRIGHT, 0L);

    fileout_land(&file_pass, local_sim, (noble_file_entry *)noble_file_format);

    while (loop < local_sim->num)
    {
        fileout_being(&file_pass, local_sim, loop, (noble_file_entry *)noble_file_format);
        loop++;
    }

    /* TODO: Brain block */

    * len = file_pass.location;
    return file_pass.data;
}

n_int	file_in(n_file * input_file)
{
    n_int  ret_val;
    n_byte temp_store[LARGEST_INIT_PTR];
    n_uint ape_count = 0;
    n_uint social_count = 0;
    n_uint episodic_count = 0;

    noble_simulation * local_sim = sim_sim();

    input_file->size = input_file->location;
    input_file->location = 0;

    io_whitespace(input_file);

    ret_val = io_read_buff(input_file, temp_store, noble_file_format);

    if(ret_val != FIL_VER) /* signature must be first */
        return SHOW_ERROR("Signature not first in file");

    {
        n_byte2	*signature = (n_byte2 *)temp_store;

        if(signature[0] != NOBLE_APE_SIGNATURE) /* not a Noble Ape file */
            return SHOW_ERROR("Not a Noble Ape File");

        if(signature[1] > VERSION_NUMBER) /* file version greater than this version */
            return SHOW_ERROR("File newer than Simulation");
    }

    do
    {
        n_byte *temp = 0L;
        ret_val = io_read_buff(input_file, temp_store, noble_file_format);
        if (ret_val == -1)
            SHOW_ERROR("Failure in file load");
        if (ret_val < FILE_EOF)
        {
            n_uint	loop_end = 0;
            switch (ret_val)
            {
            case FIL_LAN:
                temp = (n_byte*)(local_sim->land);
                loop_end = NON_PTR_LAND;
                break;
            case FIL_WEA:
                temp = (n_byte*)(local_sim->weather);
                loop_end = sizeof(n_int);
                break;
            case FIL_BEI:
                temp = (n_byte*) &(local_sim->beings[ape_count]);
                loop_end = sizeof(noble_being);
                break;
                case FIL_SOE:
                {
                    social_link * local_social = being_social(&(local_sim->beings[ape_count]));
                    temp = (n_byte*)(&local_social[social_count]);
                    loop_end = sizeof(social_link);
                }
                break;
                case FIL_EPI:
                {
                    episodic_memory * local_episodic = being_episodic(&(local_sim->beings[ape_count]));
                    temp = (n_byte*)(&local_episodic[episodic_count]);
                    loop_end = sizeof(episodic_memory);
                }
                break;
            default:
                return SHOW_ERROR("Unknown kind in file"); /*unkown kind*/
                break;
            }
            if(temp != 0L)
            {
                io_copy(temp_store, temp, loop_end);
            }
            if (ret_val == FIL_BEI)
            {
                ape_count ++;
                if (ape_count == local_sim->max)
                {
                    local_sim->num = ape_count;
                    return SHOW_ERROR("Too many apes for memory");
                }
            }
            if (ret_val == FIL_SOE)
            {
                social_count ++;
                if (social_count == (local_sim->max * SOCIAL_SIZE))
                {
                    local_sim->num = ape_count;
                    return SHOW_ERROR("Too many social graph events for memory");
                }
            }
            if (ret_val == FIL_EPI)
            {
                episodic_count ++;
                if (episodic_count == (local_sim->max * EPISODIC_SIZE))
                {
                    local_sim->num = ape_count;
                    return SHOW_ERROR("Too many episodic events for memory");
                }
            }

        }

    }
    while (ret_val < FILE_EOF);

    if (ret_val == FILE_EOF)
    {
        local_sim->num = ape_count;
        return 0;
    }
    return SHOW_ERROR("Process file failed");
}

/* takes a file buffer and decodes the file into the global being and land variables */
n_int	sim_filein(n_byte * buff, n_uint len)
{
    n_int  ret_val;
    n_byte temp_store[LARGEST_INIT_PTR];
    n_uint ape_count = 0;
    n_uint social_count = 0;
    n_uint episodic_count = 0;
    n_file local;

    noble_simulation * local_sim = sim_sim();

    local . size = len;
    local . location = 0;
    local . data = buff;

    io_whitespace(&local);


    ret_val = io_read_buff(&local, temp_store, noble_file_format);

    if(ret_val != FIL_VER) /* signature must be first */
        return SHOW_ERROR("Signature not first in file");

    {
        n_byte2	*signature = (n_byte2 *)temp_store;

        if(signature[0] != NOBLE_APE_SIGNATURE) /* not a Noble Ape file */
            return SHOW_ERROR("Not a Noble Ape File");

        if(signature[1] > VERSION_NUMBER) /* file version greater than this version */
            return SHOW_ERROR("File newer than Simulation");
    }

    do
    {
        n_byte *temp = 0L;
        ret_val = io_read_buff(&local, temp_store, noble_file_format);
        if (ret_val == -1)
            SHOW_ERROR("Failure in file load");
        if (ret_val < FILE_EOF)
        {
            n_uint	loop_end = 0;
            switch (ret_val)
            {
            case FIL_LAN:
                temp = (n_byte*)(local_sim->land);
                loop_end = NON_PTR_LAND;
                break;
            case FIL_WEA:
                temp = (n_byte*)(local_sim->weather);
                loop_end = sizeof(n_int);
                break;
            case FIL_BEI:
                temp = (n_byte*) &(local_sim->beings[ape_count]);
                loop_end = sizeof(noble_being);
                break;
            case FIL_SOE:
                {
                    social_link * local_social = being_social(&(local_sim->beings[ape_count]));
                    temp = (n_byte*)(&local_social[social_count]);
                    loop_end = sizeof(social_link);
                }
                break;
            case FIL_EPI:
                {
                    episodic_memory * local_episodic = being_episodic(&(local_sim->beings[ape_count]));
                    temp = (n_byte*)(&local_episodic[episodic_count]);
                    loop_end = sizeof(episodic_memory);
                }
                break;
            default:
                return SHOW_ERROR("Unknown kind in file"); /*unkown kind*/
                break;
            }
            if(temp != 0L)
            {
                io_copy(temp_store, temp, loop_end);
            }
            if (ret_val == FIL_BEI)
            {
                ape_count ++;
                if (ape_count == local_sim->max)
                {
                    local_sim->num = ape_count;
                    return SHOW_ERROR("Too many apes for memory");
                }
            }
            if (ret_val == FIL_SOE)
            {
                social_count ++;
                if (social_count == (local_sim->max * SOCIAL_SIZE))
                {
                    local_sim->num = ape_count;
                    return SHOW_ERROR("Too many social graph events for memory");
                }
            }
            if (ret_val == FIL_EPI)
            {
                episodic_count ++;
                if (episodic_count == (local_sim->max * EPISODIC_SIZE))
                {
                    local_sim->num = ape_count;
                    return SHOW_ERROR("Too many episodic events for memory");
                }
            }

        }

    }
    while (ret_val < FILE_EOF);

    if (ret_val == FILE_EOF)
    {
        local_sim->num = ape_count;
        return 0;
    }
    return SHOW_ERROR("Process file failed");
}

/*
 read:
 io_disk_check
 file_chain_new
 add pointers to "to be read"
 file_chain_read_header
 file_chain_read_validate
 file_chain_read
 file_chain_free
 */
n_int file_bin_read(n_string name)
{
    noble_simulation * local = sim_sim();
    n_file_chain     * start = 0L;
    /* weather, brain, social, episodic */
    const n_uint       total_ptrs = 2;
    n_string_block     bin_name;

    file_chain_bin_name(name, bin_name);

    if (io_disk_check(name) != 1)
    {
        return -1; /* This is legit for no output error */
    }

    start = file_chain_new(total_ptrs);

    if (start == 0L)
    {
        return SHOW_ERROR("Header not allocated");
    }

    start[1].expected_bytes = sizeof(n_weather);
    start[1].data           = local->weather;

    start[2].expected_bytes = local->num * DOUBLE_BRAIN;
    start[2].data           = being_brain(local->beings);

    if (file_chain_read_header(bin_name, start, total_ptrs) != 0)
    {
        file_chain_free(start);
        return -1; /* Error already out */
    }

    if (file_chain_read_validate(bin_name, start) != 0)
    {
        file_chain_free(start);
        return -1; /* Error already out */
    }
    if (file_chain_read(bin_name, start) != 0)
    {
        file_chain_free(start);
        return -1; /* Error already out */
    }

    file_chain_free(start);
    return 0;
}

/*
 write:
 file_chain_new
 add pointers to "to be written"
 file_chain_write_generate_header
 file_chain_write
 file_chain_free
 */

n_int file_bin_write(n_string name)
{
    noble_simulation * local = sim_sim();
    n_file_chain     * start = 0L;
    /* weather, brain, social, episodic */
    const n_uint       total_ptrs = 2;
    n_string_block     bin_name;

    file_chain_bin_name(name, bin_name);

    start = file_chain_new(total_ptrs);

    if (start == 0L)
    {
        return SHOW_ERROR("Header not allocated");
    }

    start[1].expected_bytes = sizeof(n_weather);
    start[1].data           = local->weather;

    start[2].expected_bytes = local->num * DOUBLE_BRAIN;
    start[2].data           = being_brain(local->beings);

    if (file_chain_write_generate_header(start) != 0)
    {
        file_chain_free(start);
        return -1; /* Error already out */
    }

    if (file_chain_write(bin_name, start) != 0)
    {
        file_chain_free(start);
        return -1; /* Error already out */
    }
    file_chain_free(start);
    return 0;
}


n_int sketch_input(void *code, n_byte kind, n_int value)
{
    noble_simulation * local_sim = sim_sim();
    n_int *local_vr = ((n_interpret *)code)->variable_references;
    noble_being	*local_being = 0L;
    n_int temp_select = local_vr[ VARIABLE_SELECT_BEING - VARIABLE_VECT_ANGLE ];

    if( temp_select < 0 )
    {
        return io_apescript_error(AE_SELECTED_ENTITY_OUT_OF_RANGE);
    }
    {
        n_uint local_select = temp_select;
        if( local_select >= local_sim->num)
        {
            return io_apescript_error(AE_SELECTED_ENTITY_OUT_OF_RANGE);
        }
        local_being = &(local_sim->beings[local_select]);
    }

    switch(kind)
    {
    case VARIABLE_BRAIN_VALUE:
    {
        n_int	current_x = local_vr[VARIABLE_BRAIN_X - VARIABLE_VECT_ANGLE];
        n_int	current_y = local_vr[VARIABLE_BRAIN_Y - VARIABLE_VECT_ANGLE];
        n_int	current_z = local_vr[VARIABLE_BRAIN_Z - VARIABLE_VECT_ANGLE];

        if((value < 0) || (value > 255))
        {
            return io_apescript_error(AE_VALUE_OUT_OF_RANGE);
        }

        if( (current_x < 0) || (current_y < 0) || (current_z < 0) ||
                (current_x > 31) || (current_y > 31) || (current_z > 31))
        {
            return io_apescript_error(AE_COORDINATES_OUT_OF_RANGE);
        }
        {
            n_byte *local_brain = being_brain(&(local_sim->beings[((n_interpret *)code)->specific]));
            if (local_brain != 0L)
            {
                TRACK_BRAIN(local_brain, current_x, current_y, current_z) = (n_byte) value;
            }
        }
        /* add brain value */
        return 0;
    }

    case VARIABLE_HONOR:
        local_being->honor = (n_byte) value;
        break;
#ifdef PARASITES_ON
    case VARIABLE_PARASITES:
        local_being->parasites = (n_byte) value;
        break;
#endif
    case VARIABLE_HEIGHT:
        GET_H(local_being) = (n_byte2) value;
        break;
    case VARIABLE_FAMILY_NAME_ONE:
        being_set_family_name(local_being,
                        UNPACK_FAMILY_FIRST_NAME((n_byte2) value),
                        UNPACK_FAMILY_SECOND_NAME(being_family_name(local_being)));
        break;
    case VARIABLE_FAMILY_NAME_TWO:
        being_set_family_name(local_being,
                        UNPACK_FAMILY_FIRST_NAME(being_family_name(local_being)),
                        UNPACK_FAMILY_SECOND_NAME((n_byte2) value));
        break;
    case VARIABLE_GOAL_TYPE:
        local_being->goal[0] = (n_byte) (value % 3);
        break;
    case VARIABLE_GOAL_X:
        local_being->goal[1] = (n_byte2) value;
        break;
    case VARIABLE_GOAL_Y:
        local_being->goal[2] = (n_byte2) value;
        break;
    case VARIABLE_POSTURE:
        GET_PS(local_being) = (n_byte) value;
        break;

    case VARIABLE_DRIVE_HUNGER:
        local_being->drives[DRIVE_HUNGER] = (n_byte) value;
        break;
    case VARIABLE_DRIVE_SOCIAL:
        local_being->drives[DRIVE_SOCIAL] = (n_byte) value;
        break;
    case VARIABLE_DRIVE_FATIGUE:
        local_being->drives[DRIVE_FATIGUE] = (n_byte) value;
        break;
    case VARIABLE_DRIVE_SEX:
        local_being->drives[DRIVE_SEX] = (n_byte) value;
        break;

    case VARIABLE_PREFERENCE_MATE_HEIGHT_MALE:
        local_being->learned_preference[PREFERENCE_MATE_HEIGHT_MALE] = (n_byte) value;
        break;
    case VARIABLE_PREFERENCE_MATE_HEIGHT_FEMALE:
        local_being->learned_preference[PREFERENCE_MATE_HEIGHT_FEMALE] = (n_byte) value;
        break;
    case VARIABLE_PREFERENCE_MATE_PIGMENTATION_MALE:
        local_being->learned_preference[PREFERENCE_MATE_PIGMENTATION_MALE] = (n_byte) value;
        break;
    case VARIABLE_PREFERENCE_MATE_PIGMENTATION_FEMALE:
        local_being->learned_preference[PREFERENCE_MATE_PIGMENTATION_FEMALE] = (n_byte) value;
        break;
    case VARIABLE_PREFERENCE_MATE_HAIR_MALE:
        local_being->learned_preference[PREFERENCE_MATE_HAIR_MALE] = (n_byte) value;
        break;
    case VARIABLE_PREFERENCE_MATE_HAIR_FEMALE:
        local_being->learned_preference[PREFERENCE_MATE_HAIR_FEMALE] = (n_byte) value;
        break;
    case VARIABLE_PREFERENCE_MATE_FRAME_MALE:
        local_being->learned_preference[PREFERENCE_MATE_FRAME_MALE] = (n_byte) value;
        break;
    case VARIABLE_PREFERENCE_MATE_FRAME_FEMALE:
        local_being->learned_preference[PREFERENCE_MATE_FRAME_FEMALE] = (n_byte) value;
        break;
    case VARIABLE_PREFERENCE_GROOM_MALE:
        local_being->learned_preference[PREFERENCE_GROOM_MALE] = (n_byte) value;
        break;
    case VARIABLE_PREFERENCE_GROOM_FEMALE:
        local_being->learned_preference[PREFERENCE_GROOM_FEMALE] = (n_byte) value;
        break;
    case VARIABLE_PREFERENCE_ANECDOTE_EVENT_MUTATION:
        local_being->learned_preference[PREFERENCE_ANECDOTE_EVENT_MUTATION] = (n_byte) value;
        break;
    case VARIABLE_PREFERENCE_ANECDOTE_AFFECT_MUTATION:
        local_being->learned_preference[PREFERENCE_ANECDOTE_AFFECT_MUTATION] = (n_byte) value;
        break;
    case VARIABLE_PREFERENCE_CHAT:
        local_being->learned_preference[PREFERENCE_CHAT] = (n_byte) value;
        break;
    case VARIABLE_ATTENTION_ACTOR_INDEX:
        GET_A(local_being,ATTENTION_ACTOR) = (n_byte) (value % SOCIAL_SIZE);
        break;
    case VARIABLE_ATTENTION_EPISODE_INDEX:
        GET_A(local_being,ATTENTION_EPISODE) = (n_byte) (value % EPISODIC_SIZE);
        break;
    case VARIABLE_ATTENTION_BODY_INDEX:
        GET_A(local_being,ATTENTION_BODY) = (n_byte) (value % INVENTORY_SIZE);
        break;
    }

    if (kind>VARIABLE_BRAIN_VALUE)
    {
        local_vr[kind-VARIABLE_VECT_ANGLE] = value;
        return 0;
    }

    return -1; /* where this fails is more important than this failure */
}


n_int sketch_output(void * vcode, n_byte * kind, n_int * number)
{
    noble_simulation * local_sim = sim_sim();
    n_interpret * code = (n_interpret *) vcode;
    n_byte	first_value = kind[0];
    n_byte	second_value = kind[1];
    if(first_value == 'n')
    {
        *number = code->number_buffer[second_value];
        return 0;
    }
    if((first_value == 't') && (VARIABLE_SPECIAL(second_value, code)==0))
    {
        n_int	*local_vr = code->variable_references;

        if( (second_value >= VARIABLE_BIOLOGY_AREA) && (second_value <= VARIABLE_BIOLOGY_EAGLE))
        {
            *number = second_value - VARIABLE_BIOLOGY_AREA;
            return 0;
        }

        if(second_value>VARIABLE_BRAIN_VALUE)
        {
            *number = local_vr[ second_value - VARIABLE_VECT_ANGLE ];
            return 0;
        }

        {
            noble_being * local_current = &(local_sim->beings[code->specific]);
            n_int		  local_number = 0;
            switch(second_value)
            {

            case VARIABLE_VECT_X:
                local_number = VECT_X(local_vr[0]);
                break;

            case VARIABLE_VECT_Y:
                local_number = VECT_Y(local_vr[0]);
                break;
            case VARIABLE_RANDOM:
                local_number = math_random(local_current->seed);
                break;
            case VARIABLE_WATER_LEVEL:
                local_number = WATER_MAP;
                break;
            case VARIABLE_HUNGRY:
                local_number = BEING_HUNGRY;
                break;
            case VARIABLE_LOCATION_Z:
            case VARIABLE_TEST_Z:
            case VARIABLE_IS_VISIBLE:
            case VARIABLE_BIOLOGY_OUTPUT:
            {
                n_int	quick_x;
                n_int	quick_y;

                if(second_value == VARIABLE_LOCATION_Z)
                {
                    quick_x = being_location_x(local_current);
                    quick_y = being_location_y(local_current);
                }
                else
                {
                    quick_x = local_vr[VARIABLE_TEST_X-VARIABLE_VECT_ANGLE];
                    quick_y = local_vr[VARIABLE_TEST_Y-VARIABLE_VECT_ANGLE];
                    if( (quick_x < 0) || (quick_y < 0) ||
                            (quick_x > APESPACE_BOUNDS ) || (quick_y > APESPACE_BOUNDS) )
                    {
                        return io_apescript_error(AE_COORDINATES_OUT_OF_RANGE);
                    }
                }

                if ( second_value == VARIABLE_IS_VISIBLE )
                {
                    /* range already checked */
                    local_number = being_los(local_sim->land, &(local_sim->beings[code->specific]), (n_byte2)quick_x, (n_byte2)quick_y);
                }
                else
                {
                    if(second_value == VARIABLE_BIOLOGY_OUTPUT)
                    {
                        n_int	int_qu_op = local_vr[VARIABLE_BIOLOGY_OPERATOR-VARIABLE_VECT_ANGLE];
                        if((int_qu_op < 0 ) || (int_qu_op > (VARIABLE_BIOLOGY_EAGLE - VARIABLE_BIOLOGY_AREA)))
                        {
                            return io_apescript_error(AE_VALUE_OUT_OF_RANGE);
                        }
                        local_number = land_operator_interpolated(local_sim->land, local_sim->weather,
                                       (n_byte)quick_x, (n_byte)quick_y, (n_byte*)&operators[int_qu_op-VARIABLE_BIOLOGY_AREA]);
                    }
                    else
                    {
                        local_number = QUICK_LAND(local_sim->land, quick_x, quick_y);
                    }
                }
            }
            break;

            case VARIABLE_TIME:
                local_number = local_sim->land->time;
                break;
            case VARIABLE_DATE:
                local_number = (local_sim->land->date[0]) + (local_sim->land->date[1] * TIME_CENTURY_DAYS);
                break;
            case VARIABLE_CURRENT_BEING:
                local_number = code->specific;
                break;
            case VARIABLE_NUMBER_BEINGS:
                local_number = local_sim->num;
                break;

            case VARIABLE_WEATHER:
            {
                n_int	quick_x;
                n_int	quick_y;

                quick_x = local_vr[VARIABLE_TEST_X-VARIABLE_VECT_ANGLE];
                quick_y = local_vr[VARIABLE_TEST_Y-VARIABLE_VECT_ANGLE];
                if( (quick_x < 0) || (quick_y < 0) ||
                        (quick_x > APESPACE_BOUNDS ) || (quick_y > APESPACE_BOUNDS) )
                {
                    return io_apescript_error(AE_COORDINATES_OUT_OF_RANGE);
                }

                local_number = weather_seven_values(local_sim->land, local_sim->weather, quick_x, quick_y);
            }
            break;
            case VARIABLE_BRAIN_VALUE:
            {
                n_int	current_x = local_vr[VARIABLE_BRAIN_X - VARIABLE_VECT_ANGLE];
                n_int	current_y = local_vr[VARIABLE_BRAIN_Y - VARIABLE_VECT_ANGLE];
                n_int	current_z = local_vr[VARIABLE_BRAIN_Z - VARIABLE_VECT_ANGLE];

                if( (current_x < 0) || (current_y < 0) || (current_z < 0) ||
                        (current_x > 31) || (current_y > 31) || (current_z > 31))
                {
                    return io_apescript_error(AE_COORDINATES_OUT_OF_RANGE);
                }
                {
                    n_byte *local_brain = being_brain(&(local_sim->beings[code->specific]));
                    if (local_brain != 0L)
                    {
                        local_number = TRACK_BRAIN(local_brain, current_x, current_y, current_z);
                    }
                }
            }
            break;
            default:
            {
                n_int		 temp_select = local_vr[ VARIABLE_SELECT_BEING - VARIABLE_VECT_ANGLE ];
                noble_being	*local_being = 0L;
                social_link    *local_social_graph = 0L;
                social_link     social_graph;
#ifdef EPISODIC_ON
                episodic_memory    *local_episodic = 0L;
                episodic_memory    episodic;
#endif

                if( temp_select < 0 )
                {
                    return io_apescript_error(AE_SELECTED_ENTITY_OUT_OF_RANGE);
                }
                {
                    n_uint local_select = temp_select;
                    if( local_select >= local_sim->num)
                    {
                        return io_apescript_error(AE_SELECTED_ENTITY_OUT_OF_RANGE);
                    }
                    local_being = &(local_sim->beings[local_select]);
                    if (local_being!=0L)
                    {
                        local_social_graph = being_social(local_being);
                        if (local_social_graph!=0L)
                        {
                            social_graph = local_social_graph[GET_A(local_being,ATTENTION_ACTOR)];
                        }
#ifdef EPISODIC_ON
                        local_episodic = being_episodic(local_being);
                        if (local_episodic != 0L)
                        {
                            episodic = local_episodic[GET_A(local_being,ATTENTION_EPISODE)];
                        }
#endif
                    }
                }
                if ((local_being!=0L) && (local_social_graph!=0L))
                {
                    switch(second_value)
                    {

                    case VARIABLE_HONOR:
                        local_number = local_being->honor;
                        break;
#ifdef PARASITES_ON
                    case VARIABLE_PARASITES:
                        local_number = local_being->parasites;
                        break;
#endif
                    case VARIABLE_HEIGHT:
                        local_number = GET_H(local_being);
                        break;
                    case VARIABLE_FIRST_NAME:
                        local_number = being_gender_name(local_being);
                        break;
                    case VARIABLE_FAMILY_NAME_ONE:
                        local_number = UNPACK_FAMILY_FIRST_NAME(being_family_name(local_being));
                        break;
                    case VARIABLE_FAMILY_NAME_TWO:
                        local_number = UNPACK_FAMILY_SECOND_NAME(being_family_name(local_being));
                        break;
                    case VARIABLE_GOAL_TYPE:
                        local_number = local_being->goal[0];
                        break;
                    case VARIABLE_GOAL_X:
                        local_number = local_being->goal[1];
                        break;
                    case VARIABLE_GOAL_Y:
                        local_number = local_being->goal[2];
                        break;
                    case VARIABLE_POSTURE:
                        local_number = GET_PS(local_being);
                        break;

                    case VARIABLE_DRIVE_HUNGER:
                        local_number = local_being->drives[DRIVE_HUNGER];
                        break;
                    case VARIABLE_DRIVE_SOCIAL:
                        local_number = local_being->drives[DRIVE_SOCIAL];
                        break;
                    case VARIABLE_DRIVE_FATIGUE:
                        local_number = local_being->drives[DRIVE_FATIGUE];
                        break;
                    case VARIABLE_DRIVE_SEX:
                        local_number = local_being->drives[DRIVE_SEX];
                        break;

                    case	VARIABLE_LOCATION_X:
                        local_number = being_location_x(local_being);
                        break;

                    case	VARIABLE_LOCATION_Y:
                        local_number = being_location_y(local_being);
                        break;

                    case	VARIABLE_ID_NUMBER:
                        local_number = GET_I(local_being);
                        break;

                    case	VARIABLE_DATE_OF_BIRTH:
                        local_number = being_dob(local_being);
                        break;
                    case	VARIABLE_STATE:
                        local_number = local_being->state;
                        break;

                    case	VARIABLE_LISTEN:
                        local_number = local_being->speak;
                        break;
                    case	VARIABLE_PREFERENCE_MATE_HEIGHT_MALE:
                        local_number = local_being->learned_preference[PREFERENCE_MATE_HEIGHT_MALE];
                        break;
                    case	VARIABLE_PREFERENCE_MATE_HEIGHT_FEMALE:
                        local_number = local_being->learned_preference[PREFERENCE_MATE_HEIGHT_FEMALE];
                        break;
                    case	VARIABLE_PREFERENCE_MATE_PIGMENTATION_MALE:
                        local_number = local_being->learned_preference[PREFERENCE_MATE_PIGMENTATION_MALE];
                        break;
                    case	VARIABLE_PREFERENCE_MATE_PIGMENTATION_FEMALE:
                        local_number = local_being->learned_preference[PREFERENCE_MATE_PIGMENTATION_FEMALE];
                        break;
                    case	VARIABLE_PREFERENCE_MATE_HAIR_MALE:
                        local_number = local_being->learned_preference[PREFERENCE_MATE_HAIR_MALE];
                        break;
                    case	VARIABLE_PREFERENCE_MATE_HAIR_FEMALE:
                        local_number = local_being->learned_preference[PREFERENCE_MATE_HAIR_FEMALE];
                        break;
                    case	VARIABLE_PREFERENCE_MATE_FRAME_MALE:
                        local_number = local_being->learned_preference[PREFERENCE_MATE_FRAME_MALE];
                        break;
                    case	VARIABLE_PREFERENCE_MATE_FRAME_FEMALE:
                        local_number = local_being->learned_preference[PREFERENCE_MATE_FRAME_FEMALE];
                        break;
                    case	VARIABLE_PREFERENCE_GROOM_MALE:
                        local_number = local_being->learned_preference[PREFERENCE_GROOM_MALE];
                        break;
                    case	VARIABLE_PREFERENCE_GROOM_FEMALE:
                        local_number = local_being->learned_preference[PREFERENCE_GROOM_FEMALE];
                        break;
                    case	VARIABLE_PREFERENCE_ANECDOTE_EVENT_MUTATION:
                        local_number = local_being->learned_preference[PREFERENCE_ANECDOTE_EVENT_MUTATION];
                        break;
                    case	VARIABLE_PREFERENCE_ANECDOTE_AFFECT_MUTATION:
                        local_number = local_being->learned_preference[PREFERENCE_ANECDOTE_AFFECT_MUTATION];
                        break;
                    case	VARIABLE_PREFERENCE_CHAT:
                        local_number = local_being->learned_preference[PREFERENCE_CHAT];
                        break;
                    case	VARIABLE_ATTENTION_ACTOR_INDEX:
                        local_number = GET_A(local_being,ATTENTION_ACTOR);
                        break;
                    case	VARIABLE_ATTENTION_EPISODE_INDEX:
                        local_number = GET_A(local_being,ATTENTION_EPISODE);
                        break;
                    case	VARIABLE_ATTENTION_BODY_INDEX:
                        local_number = GET_A(local_being,ATTENTION_BODY);
                        break;
                    case	VARIABLE_SHOUT_CONTENT:
                        local_number = local_being->shout[SHOUT_CONTENT];
                        break;
                    case	VARIABLE_SHOUT_HEARD:
                        local_number = local_being->shout[SHOUT_HEARD];
                        break;
                    case	VARIABLE_SHOUT_CTR:
                        local_number = local_being->shout[SHOUT_CTR];
                        break;
                    case	VARIABLE_SHOUT_VOLUME:
                        local_number = local_being->shout[SHOUT_VOLUME];
                        break;
                    case	VARIABLE_SHOUT_FAMILY0:
                        local_number = local_being->shout[SHOUT_FAMILY0];
                        break;
                    case	VARIABLE_SHOUT_FAMILY1:
                        local_number = local_being->shout[SHOUT_FAMILY1];
                        break;


                    case VARIABLE_SOCIAL_GRAPH_LOCATION_X:
                        local_number = social_graph.location[0];
                        break;
                    case VARIABLE_SOCIAL_GRAPH_LOCATION_Y:
                        local_number = social_graph.location[1];
                        break;
                    case VARIABLE_SOCIAL_GRAPH_TIME:
                        local_number = social_graph.time;
                        break;
                    case VARIABLE_SOCIAL_GRAPH_DATE:
                        local_number = social_graph.date[0];
                        break;
                    case VARIABLE_SOCIAL_GRAPH_CENTURY:
                        local_number = social_graph.date[1];
                        break;
                    case VARIABLE_SOCIAL_GRAPH_ATTRACTION:
                        local_number = social_graph.attraction;
                        break;
                    case VARIABLE_SOCIAL_GRAPH_FOF:
                        local_number =
                            (n_int)social_graph.friend_foe -
                            (n_int)social_respect_mean(local_sim,local_being);
                        break;
                    case VARIABLE_SOCIAL_GRAPH_FAMILIARITY:
                        local_number = social_graph.familiarity;
                        break;
                    case VARIABLE_MEMORY_FIRST_NAME:
                        local_number = social_graph.first_name[BEING_MET];
                        break;
                    case VARIABLE_MEMORY_FAMILY_NAME_ONE:
                        local_number = UNPACK_FAMILY_FIRST_NAME(social_graph.family_name[BEING_MET]);
                        break;
                    case VARIABLE_MEMORY_FAMILY_NAME_TWO:
                        local_number = UNPACK_FAMILY_SECOND_NAME(social_graph.family_name[BEING_MET]);
                        break;
#ifdef EPISODIC_ON
                    case VARIABLE_MEMORY_LOCATION_X:
                        local_number = episodic.location[0];
                        break;
                    case VARIABLE_MEMORY_LOCATION_Y:
                        local_number = episodic.location[1];
                        break;
                    case VARIABLE_MEMORY_TIME:
                        local_number = episodic.time;
                        break;
                    case VARIABLE_MEMORY_DATE:
                        local_number = episodic.date[0];
                        break;
                    case VARIABLE_MEMORY_CENTURY:
                        local_number = episodic.date[1];
                        break;
                    case VARIABLE_MEMORY_FIRST_NAME0:
                        local_number = episodic.first_name[0];
                        break;
                    case VARIABLE_MEMORY_FAMILY_NAME_ONE0:
                        local_number = UNPACK_FAMILY_FIRST_NAME(episodic.family_name[0]);
                        break;
                    case VARIABLE_MEMORY_FAMILY_NAME_TWO0:
                        local_number = UNPACK_FAMILY_SECOND_NAME(episodic.family_name[0]);
                        break;
                    case VARIABLE_MEMORY_FIRST_NAME1:
                        local_number = episodic.first_name[BEING_MET];
                        break;
                    case VARIABLE_MEMORY_FAMILY_NAME_ONE1:
                        local_number = UNPACK_FAMILY_FIRST_NAME(episodic.family_name[BEING_MET]);
                        break;
                    case VARIABLE_MEMORY_FAMILY_NAME_TWO1:
                        local_number = UNPACK_FAMILY_SECOND_NAME(episodic.family_name[BEING_MET]);
                        break;
                    case VARIABLE_MEMORY_EVENT:
                        local_number = episodic.event;
                        break;
                    case VARIABLE_MEMORY_AFFECT:
                        local_number = episodic.affect - EPISODIC_AFFECT_ZERO;
                        break;
#endif
                    }
                }
            }

            break;
            }
            /* put variable cross here */
            *number = local_number;
            return 0;
        }
    }
    return -1; /* where this fails is more important than this failure */
}



void sim_start_conditions(void * code, void * structure, n_int identifier)
{
    n_interpret * interp = (n_interpret *)code;
    n_int       * variables = interp->variable_references;
    noble_being * local_being = &(((noble_being*)structure)[identifier]);

    interp->specific = identifier;

    variables[VARIABLE_FACING - VARIABLE_VECT_ANGLE] = being_facing(local_being);
    variables[VARIABLE_SPEED - VARIABLE_VECT_ANGLE] =  being_speed(local_being);
    variables[VARIABLE_ENERGY - VARIABLE_VECT_ANGLE] = being_energy(local_being);
    variables[VARIABLE_SELECT_BEING - VARIABLE_VECT_ANGLE] = identifier;
    variables[VARIABLE_SPEAK - VARIABLE_VECT_ANGLE] = local_being->speak;
    variables[VARIABLE_HEIGHT - VARIABLE_VECT_ANGLE] = GET_H(local_being);
    variables[VARIABLE_GOAL_TYPE - VARIABLE_VECT_ANGLE] = local_being->goal[0];
    variables[VARIABLE_GOAL_X - VARIABLE_VECT_ANGLE] = local_being->goal[1];
    variables[VARIABLE_GOAL_Y - VARIABLE_VECT_ANGLE] = local_being->goal[2];
    variables[VARIABLE_DRIVE_HUNGER - VARIABLE_VECT_ANGLE] = local_being->drives[DRIVE_HUNGER];
    variables[VARIABLE_DRIVE_SOCIAL - VARIABLE_VECT_ANGLE] = local_being->drives[DRIVE_SOCIAL];
    variables[VARIABLE_DRIVE_FATIGUE - VARIABLE_VECT_ANGLE] = local_being->drives[DRIVE_FATIGUE];
    variables[VARIABLE_DRIVE_SEX - VARIABLE_VECT_ANGLE] = local_being->drives[DRIVE_SEX];
    variables[VARIABLE_FAMILY_NAME_ONE - VARIABLE_VECT_ANGLE] = being_family_first_name(local_being);
    variables[VARIABLE_FAMILY_NAME_TWO - VARIABLE_VECT_ANGLE] = being_family_second_name(local_being);
#ifdef PARASITES_ON
    variables[VARIABLE_HONOR - VARIABLE_VECT_ANGLE] = local_being->honor;
    variables[VARIABLE_PARASITES - VARIABLE_VECT_ANGLE] = local_being->parasites;
#endif

}

void sim_end_conditions(void * code, void * structure, n_int identifier)
{
    n_interpret * interp = (n_interpret *)code;
    n_int       * variables = interp->variable_references;
    noble_being * local_being = &(((noble_being*)structure)[identifier]);

    n_int	local_facing = variables[VARIABLE_FACING - VARIABLE_VECT_ANGLE];
    n_int	local_speed  = variables[VARIABLE_SPEED  - VARIABLE_VECT_ANGLE];
    n_int	local_energy = variables[VARIABLE_ENERGY - VARIABLE_VECT_ANGLE];
    n_int	local_height = variables[VARIABLE_HEIGHT - VARIABLE_VECT_ANGLE];
    n_int	local_goal_type = variables[VARIABLE_GOAL_TYPE - VARIABLE_VECT_ANGLE];
    n_int	local_goal_x = variables[VARIABLE_GOAL_X - VARIABLE_VECT_ANGLE];
    n_int	local_goal_y = variables[VARIABLE_GOAL_Y - VARIABLE_VECT_ANGLE];
    n_int	local_drive_hunger = variables[VARIABLE_DRIVE_HUNGER - VARIABLE_VECT_ANGLE];
    n_int	local_drive_social = variables[VARIABLE_DRIVE_SOCIAL - VARIABLE_VECT_ANGLE];
    n_int	local_drive_fatigue = variables[VARIABLE_DRIVE_FATIGUE - VARIABLE_VECT_ANGLE];
    n_int	local_drive_sex = variables[VARIABLE_DRIVE_SEX - VARIABLE_VECT_ANGLE];
    n_int	local_family_name1 = variables[VARIABLE_FAMILY_NAME_ONE - VARIABLE_VECT_ANGLE];
    n_int	local_family_name2 = variables[VARIABLE_FAMILY_NAME_TWO - VARIABLE_VECT_ANGLE];
#ifdef PARASITES_ON
    n_int	local_honor = variables[VARIABLE_HONOR - VARIABLE_VECT_ANGLE];
    n_int	local_parasites = variables[VARIABLE_PARASITES - VARIABLE_VECT_ANGLE];
#endif
    n_int	local_speak  = variables[VARIABLE_SPEAK  - VARIABLE_VECT_ANGLE];

    interp->specific = identifier;

    if(local_facing< 0)
    {
        local_facing = 255 - ( (0 - local_facing) & 255 );
    }
    else
    {
        local_facing = local_facing & 255;
    }

    if (local_speed > 39) local_speed = 39;
    if (local_speed < 0)  local_speed = 0;

    if (local_energy < BEING_DEAD) local_energy = BEING_DEAD;
    if (local_energy > BEING_FULL) local_energy = BEING_FULL;

    if (local_speak < 0)      local_speak = 0;
    if (local_speak > 0xffff) local_speak = 0xffff;

    being_wander(local_being, local_facing - being_facing(local_being));

    being_set_speed(local_being, (n_byte) local_speed);
    being_set_energy(local_being, local_energy);

    local_being->speak  = (n_byte2)local_speak;
    GET_H(local_being)  = (n_byte2)local_height;
    if (local_goal_type!=GOAL_NONE)
    {
        local_being->goal[0]  = (n_byte2)local_goal_type;
        local_being->goal[1]  = (n_byte2)local_goal_x;
        local_being->goal[2]  = (n_byte2)local_goal_y;
        local_being->goal[3]  = GOAL_TIMEOUT;

        local_being->script_overrides |= OVERRIDE_GOAL;
    }
    if (local_drive_hunger>-1)
    {
        local_being->drives[DRIVE_HUNGER]  = (n_byte)local_drive_hunger;
    }
    if (local_drive_social>-1)
    {
        local_being->drives[DRIVE_SOCIAL]  = (n_byte)local_drive_social;
    }
    if (local_drive_fatigue>-1)
    {
        local_being->drives[DRIVE_FATIGUE]  = (n_byte)local_drive_fatigue;
    }
    if (local_drive_sex>-1)
    {
        local_being->drives[DRIVE_SEX]  = (n_byte)local_drive_sex;
    }
    being_set_family_name(local_being,(n_byte)local_family_name1,(n_byte)local_family_name2);
#ifdef PARASITES_ON
    local_being->honor = (n_byte)local_honor;
    local_being->parasites = (n_byte)local_parasites;
#endif
}
