/****************************************************************

 client.c

 =============================================================

 Copyright 1996-2020 Tom Barbalet. All rights reserved.

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

#include "client.h"


#include <stdlib.h>
#include <time.h>



n_int   hpv_exit, hpv_dt;
n_int   hpv_x, hpv_y, hpv_f, hpv_i;
n_int   hpv_r0, hpv_r1;

void loop_no_thread(noble_simulation * sim, noble_being * being_not, loop_fn bf_func, void * data)
{
    n_int loop = 0;
    while (loop < sim->count)
    {
        noble_being * output = &(sim->beings[loop]);
        if (output != being_not)
        {
            bf_func(sim, output, data);
        }
        loop++;
    }
}

typedef struct {
    n_int signature, version_number;
    n_string copyright, date;
} client_simulation_identifier;

typedef struct {
    n_vect2 location;
    n_int direction_facing, velocity, stored_energy, identification, state;
    n_int genetics[4];
    n_string name;
} client_being;

typedef struct {
    n_vect2 genetics;
    n_int date, time;
} client_land;

typedef struct {
    client_land land;
    client_simulation_identifier id;
    client_being * beings;
    n_int  being_count;
    n_int  being_max;
    n_uint being_selected_id;
} client_data;

static void client_from_file(client_data * data, n_file * file_json)
{
    io_whitespace_json(file_json);
    {
        n_object * returned_object = object_file_to_tree(file_json);
        if (returned_object)
        {
            n_string str_information = obj_contains(returned_object, "information", OBJECT_OBJECT);
            n_string str_land = obj_contains(returned_object, "land", OBJECT_OBJECT);
            n_string str_beings = obj_contains(returned_object, "beings", OBJECT_ARRAY);
            n_int    value = 0;
            if (str_information)
            {
                n_object * information_obj = obj_get_object(str_information);
                
                n_string potential = obj_contains(information_obj, "copyright", OBJECT_STRING);
                
                if (potential)
                {
                    data->id.copyright = potential;
                }
                potential = obj_contains(information_obj, "date", OBJECT_STRING);
                
                if (potential)
                {
                    data->id.date = potential;
                }
                
                if (obj_contains_number(information_obj, "signature", &value))
                {
                    data->id.signature = value;
                }
                if (obj_contains_number(information_obj, "version_number", &value))
                {
                    data->id.version_number = value;
                }
            }
            if (str_land)
            {
                n_object * land_obj = obj_get_object(str_land);
                
                (void)obj_contains_array_numbers(land_obj, "genetics", data->land.genetics.data, 2);
                
                if (obj_contains_number(land_obj, "date", &value))
                {
                    data->land.date = value;
                }
                if (obj_contains_number(land_obj, "time", &value))
                {
                    data->land.time = value;
                }
            }
            if (str_beings)
            {
                n_array * beings_array = obj_get_array(str_beings);
                n_int     being_count = obj_array_count(beings_array);
                n_array * arr_second_follow = 0L;
                
                if (being_count > data->being_max)
                {
                    if (data->beings)
                    {
                        memory_free((void**)&data->beings);
                    }
                    data->beings = (client_being*) memory_new((n_uint)(sizeof(client_being) * (n_uint)being_count));
                    data->being_max = being_count;
                }
                if (beings_array)
                {
                    being_count = 0;
                    while ((arr_second_follow = obj_array_next(beings_array, arr_second_follow)))
                    {
                        n_object * obj_being = obj_get_object(arr_second_follow->data);
                        client_being * being = &(data->beings[being_count++]);
                        
                        if (obj_contains_number(obj_being, "state", &value))
                        {
                            being->state = value;
                        }
                        
                        if (obj_contains_number(obj_being, "direction_facing", &value))
                        {
                            being->direction_facing = value;
                        }
                        if (obj_contains_number(obj_being, "velocity", &value))
                        {
                            being->velocity = value;
                        }
                        if (obj_contains_number(obj_being, "stored_energy", &value))
                        {
                            being->stored_energy = value;
                        }
                        if (obj_contains_number(obj_being, "identification", &value))
                        {
                            being->identification = value;
                        }
                        (void)obj_contains_array_numbers(obj_being, "location", being->location.data, 2);
                        (void)obj_contains_array_numbers(obj_being, "genetics", being->genetics, 4);
                    }
                }
                
            }
        }
    }
}

