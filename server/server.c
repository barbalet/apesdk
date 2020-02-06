/****************************************************************

 server.c

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

#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

n_int   hpv_exit, hpv_dt;
n_int   hpv_x, hpv_y, hpv_f, hpv_i;
n_int   hpv_r0, hpv_r1;

void server_parameters(void)
{
    hpv_exit = httpd_qs("exit");
    hpv_dt = httpd_qs("dt");
    hpv_x = httpd_qs("x");
    hpv_y = httpd_qs("y");
    hpv_f = httpd_qs("f");
    hpv_i = httpd_qs("i");
    hpv_r0 = httpd_qs("r0");
    hpv_r1 = httpd_qs("r1");
}


static n_object * server_sim_obj(void)
{
    n_object * ape_sim_identifier = object_number(0L, SERVER_SIM_SIGNATURE, SIMULATED_APE_SIGNATURE);
    object_number(ape_sim_identifier, SERVER_SIM_VERSION, VERSION_NUMBER);
    object_string(ape_sim_identifier, SERVER_SIM_COPYRIGHT, FULL_VERSION_COPYRIGHT);
    object_string(ape_sim_identifier, SERVER_SIM_DATE, FULL_DATE);
    return ape_sim_identifier;
}

static n_object * server_being_obj(simulated_being * being)
{
    n_object * simulated_being = 0L;
    n_string_block simple_name;
    being_name_simple(being, simple_name);
    simulated_being = object_string(0L, SERVER_BEING_NAME, simple_name);
    {
        simulated_being_delta * delta = &being->delta;
        simulated_being_constant *constant =&being->constant;
        n_genetics * genetics = constant->genetics;
        n_array  * location = array_number(delta->location[0]);
        n_array  * genetics_array = array_number(genetics[0]);
        array_add(genetics_array, array_number(genetics[1]));
        array_add(genetics_array, array_number(genetics[2]));
        array_add(genetics_array, array_number(genetics[3]));
        
        array_add(location, array_number(delta->location[1]));
        object_number(simulated_being, SERVER_BEING_FACING, delta->direction_facing);
        object_array(simulated_being,  SERVER_BEING_LOCATION, location);
        object_number(simulated_being, SERVER_BEING_VELOCITY, delta->velocity);
        object_number(simulated_being, SERVER_BEING_ENERGY, delta->stored_energy);
        object_number(simulated_being, SERVER_BEING_STATE, delta->macro_state);
        object_array(simulated_being,  SERVER_BEING_GENETICS, genetics_array);
        object_number(simulated_being, SERVER_BEING_IDENTIFICATION, genetics[0]^genetics[1]^genetics[2]^genetics[3]);
    }
    return simulated_being;
}

static n_object * server_land_obj(void)
{
    n_object * simulated_land = object_number(0L, SERVER_LAND_DATE, land_date());
    n_byte2  * genetics_values = land_genetics();
    n_array  * land_genetics = array_number(genetics_values[0]);
    array_add(land_genetics, array_number(genetics_values[1]));
    
    object_array(simulated_land, SERVER_LAND_GENETICS, land_genetics);
    object_number(simulated_land, SERVER_LAND_TIME, land_time());
    return simulated_land;
}

n_file * server_out_json(void)
{
    n_file *output_file = 0L;
    ape_simulation *local_sim = sim_sim();
    
    n_object * simulation_object = object_object(0L, SERVER_OUT_INFORMATION, server_sim_obj());
    object_object(simulation_object, SERVER_OUT_LAND, server_land_obj());
    
    if (local_sim->num > 0)
    {
        n_uint        count = 1;
        simulated_being *local_beings = local_sim->beings;
        n_object    *being_object = server_being_obj(&(local_beings[0]));
        n_array     *beings = array_object(being_object);
        while (count < local_sim->num)
        {
            being_object = server_being_obj(&(local_beings[count++]));
            array_add(beings, array_object(being_object));
        }
        object_array(simulation_object, SERVER_OUT_BEINGS, beings);
    }
    output_file = obj_json(simulation_object);
    obj_free(&simulation_object);
    return output_file;
}

n_int draw_error(n_constant_string error_text, n_constant_string location, n_int line_number)
{
    printf("ERROR: %s @ %s %ld\n",(const n_string) error_text, location, line_number);
    return -1;
}

static n_int run_once = 0;

void server_sim_start(void)
{
    if (run_once == 0)
    {
        srand((unsigned int) time(NULL) );
        sim_init(KIND_START_UP, rand(), MAP_AREA, 0);
        sim_cycle();
    }
    else
    {
        sim_cycle();
    }
    run_once = 1;
}

int main(int c, char **v)
{
    httpd_for_now("8001", server_sim_start, server_parameters, server_out_json);
    return 0;
}
