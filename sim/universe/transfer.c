/****************************************************************

 transfer.c

 =============================================================

 Copyright 1996-2019 Tom Barbalet. All rights reserved.

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

#ifndef	_WIN32
#include "../entity/entity.h"
#else
#include "..\entity\entity.h"
#endif

#include "universe_internal.h"

/* does not appear to be used here */

void transfer_debug_csv(n_file * fil, n_byte initial)
{
    noble_simulation * local_sim = sim_sim();
    io_write_csv(fil, (n_byte *)(&(local_sim->beings[0])), noble_file_format, FIL_BEI, initial);
}

/* provide an output file buffer to be written */
static void transfer_land(n_file * tranfer_out, noble_simulation * value, noble_file_entry * format)
{
    n_byte2	loc_signature[2] = {(n_byte2)NOBLE_APE_SIGNATURE, (n_byte2)VERSION_NUMBER};

#ifdef USE_FIL_VER
    io_write_buff(tranfer_out, loc_signature, format, FIL_VER, 0L);
#endif
#ifdef USE_FIL_LAN
    io_write_buff(tranfer_out, land_ptr(), format, FIL_LAN, 0L);
#endif
#ifdef USE_FIL_WEA
    io_write_buff(tranfer_out, value->weather, format, FIL_WEA, 0L);
#endif
}

static void transfer_being(n_file * tranfer_out, noble_simulation * value, n_int being, noble_file_entry * format)
{
#ifdef USE_FIL_SOE
    n_int loop = (SOCIAL_SIZE * being);
    n_int loop_end = loop + SOCIAL_SIZE;
#endif
#ifdef USE_FIL_EPI
    n_int loop_episodic = (EPISODIC_SIZE * being);
    n_int loop_episodic_end = loop + EPISODIC_SIZE;
#endif

#ifdef USE_FIL_BEI
    io_write_buff(tranfer_out, &(value->beings[being]), format, FIL_BEI, 0L);
#endif
#ifdef USE_FIL_SOE
    while (loop < loop_end)
    {
        io_write_buff(tranfer_out, being_social(&(value->beings[being])), format, FIL_SOE, &brain_three_byte_command);
        loop++;
    }
#endif
#ifdef USE_FIL_EPI
    while (loop_episodic < loop_episodic_end)
    {
        io_write_buff(tranfer_out, being_episodic(&(value->beings[being])), format, FIL_EPI, 0L);
        loop_episodic++;
    }
#endif
}

static n_object * transfer_land_obj(void)
{
    n_object * noble_land = obj_number(0L, "date", land_date());
    n_byte2  * genetics_values = land_genetics();
    n_array  * land_genetics = array_number(genetics_values[0]);
    array_add(land_genetics, array_number(genetics_values[1]));
    
    obj_array(noble_land, "genetics", land_genetics);
    obj_number(noble_land, "time", land_time());
    return noble_land;
}


static n_object * transfer_being_spacetime_obj(n_spacetime * value)
{
    n_object * noble_spacetime = obj_number(0L, "date", value->date);
    n_array  * noble_location = array_number(value->location[0]);
    array_add(noble_location, array_number(value->location[1]));
    obj_array(noble_spacetime, "location", noble_location);
    obj_number(noble_spacetime, "time", value->time);
    return noble_spacetime;
}

static n_object * transfer_being_constant_obj(noble_being_constant * constant)
{
    n_int genetic_count = 1;
    n_object * noble_being_contant = obj_number(0L, "date_of_birth", constant->date_of_birth);
    n_array * generation = array_number(constant->generation_min);
    n_array * genetics = array_number(constant->genetics[0]);
    while (genetic_count < CHROMOSOMES)
    {
        array_add(genetics, array_number(constant->genetics[genetic_count++]));
    }
    obj_array(noble_being_contant, "genetics", genetics);

    array_add(generation, array_number(constant->generation_max));
    obj_array(noble_being_contant, "generation_range", generation);

    return noble_being_contant;
}


static n_object * transfer_being_delta_obj(noble_being_delta * delta)
{
    n_object * noble_being_delta = obj_number(0L, "direction_facing", delta->direction_facing);
    n_array  * location = array_number(delta->location[0]);
    n_array  * seed = array_number(delta->seed[0]);
    n_array  * goal = array_number(delta->goal[0]);
    n_array  * social_coord = array_number(delta->social_coord_x);

    array_add(location, array_number(delta->location[1]));
    
    array_add(seed, array_number(delta->seed[1]));
    
    array_add(goal, array_number(delta->goal[1]));
    array_add(goal, array_number(delta->goal[2]));
    array_add(goal, array_number(delta->goal[3]));
    
    array_add(social_coord, array_number(delta->social_coord_y));
    array_add(social_coord, array_number(delta->social_coord_nx));
    array_add(social_coord, array_number(delta->social_coord_ny));
    
    obj_array(noble_being_delta, "location", location);

    obj_number(noble_being_delta, "velocity", delta->velocity);
    obj_number(noble_being_delta, "stored_energy", delta->stored_energy);

    obj_array(noble_being_delta, "seed", seed);

    obj_number(noble_being_delta, "macro_state", delta->macro_state);
    obj_number(noble_being_delta, "parasites", delta->parasites);
    obj_number(noble_being_delta, "honor", delta->honor);
    obj_number(noble_being_delta, "crowding", delta->crowding);
    obj_number(noble_being_delta, "height", delta->height);
    obj_number(noble_being_delta, "mass", delta->mass);
    obj_number(noble_being_delta, "posture", delta->posture);

    obj_array(noble_being_delta, "goal", goal);
    obj_array(noble_being_delta, "social_coord", social_coord);

    return noble_being_delta;
}

static n_object * transfer_being_obj(noble_being * being)
{
    n_object * noble_being = 0L;
    n_string_block simple_name;
    being_name_simple(being, simple_name);
    noble_being = obj_string(0L, "name", simple_name);
    obj_object(noble_being, "delta", transfer_being_delta_obj(&(being->delta)));
    obj_object(noble_being, "constant", transfer_being_constant_obj(&(being->constant)));
    return noble_being;
}


static n_object * transfer_sim_obj(void)
{
    n_object * noble_sim_identifier = obj_number(0L, "signature", NOBLE_APE_SIGNATURE);
    obj_number(noble_sim_identifier, "version number", VERSION_NUMBER);
    obj_string(noble_sim_identifier, "copyright", FULL_VERSION_COPYRIGHT);
    obj_string(noble_sim_identifier, "date", FULL_DATE);
    return noble_sim_identifier;
}

n_file * tranfer_out_json(void)
{
    n_file *output_file = 0L;
    noble_simulation *local_sim = sim_sim();
    
    n_object * simulation_object = obj_object(0L, "information", transfer_sim_obj());
    
    obj_object(simulation_object, "land", transfer_land_obj());
    
    if (local_sim->num > 0)
    {
        n_uint        count = 1;
        noble_being *local_beings = local_sim->beings;
        n_object    *being_object = transfer_being_obj(&(local_beings[0]));
        n_array     *beings = array_object(being_object);
        while (count < local_sim->num)
        {
            being_object = transfer_being_obj(&(local_beings[count++]));
            array_add(beings, array_object(being_object));
        }
        obj_array(simulation_object, "beings", beings);
    }
    
    output_file = obj_json(simulation_object);
    
    obj_free((n_array **) &simulation_object);
    
    return output_file;
}

n_file * tranfer_out(void)
{
    noble_simulation *local_sim = sim_sim();
    n_file           *returnFile = io_file_new();
    n_int	          loop = 0;
    n_string fluff[5] = {SHORT_VERSION_NAME, FULL_DATE, COPYRIGHT_DATE, COPYRIGHT_NAME, COPYRIGHT_FOLLOW };

    if(returnFile == 0L)
    {
        return 0L;
    }
    if(returnFile->data == 0L)
    {
        memory_free((void**)&returnFile);
        return 0L;
    }

    io_write_buff(returnFile, fluff, 0L, FILE_COPYRIGHT, 0L);

    transfer_land(returnFile, local_sim, (noble_file_entry *)noble_file_format);

    while (loop < (n_int)local_sim->num)
    {
        transfer_being(returnFile, local_sim, loop, (noble_file_entry *)noble_file_format);
        loop++;
    }

    /* TODO: Brain block */
    return returnFile;
}

n_int	tranfer_in(n_file * input_file)
{
    n_int  ret_val;
    n_byte *temp_store = 0L;
    n_uint ape_count = 0;
    n_uint social_count = 0;
    n_uint episodic_count = 0;

    noble_simulation * local_sim = sim_sim();
    n_uint  size_buffer = io_find_size_data((noble_file_entry *)noble_file_format);

    temp_store = (n_byte *)memory_new(size_buffer);

    if (temp_store == 0L)
    {
        return SHOW_ERROR("No temporary storage memory available");
    }

    io_whitespace(input_file);

    input_file->location = 0;

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
                temp = (n_byte*)land_ptr();
                loop_end = 11; /* Needs to be fixed */
                break;
            case FIL_BEI:
                temp = (n_byte*) &(local_sim->beings[ape_count]);
                loop_end = sizeof(noble_being);
                break;
            case FIL_SOE:
            {
                noble_social * local_social = being_social(&(local_sim->beings[ape_count]));
                temp = (n_byte*)(&local_social[social_count]);
                loop_end = sizeof(noble_social);
            }
            break;
            case FIL_EPI:
            {
                noble_episodic * local_episodic = being_episodic(&(local_sim->beings[ape_count]));
                temp = (n_byte*)(&local_episodic[episodic_count]);
                loop_end = sizeof(noble_episodic);
            }
            break;
            default:
            {
                return SHOW_ERROR("Unknown file kind"); /*unknown kind*/
            }
            break;
            }
            if(temp != 0L)
            {
                memory_copy(temp_store, temp, loop_end);
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

