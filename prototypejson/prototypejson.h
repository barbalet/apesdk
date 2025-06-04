/****************************************************************

 prototypejson.h

 =============================================================

 Copyright 1996-2024 Tom Barbalet. All rights reserved.

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

#ifndef prototypejson_h
#define prototypejson_h

#include <stdio.h>
#include "toolkit.h"


typedef enum
{
    fc_object_name,
    fc_n_spacetime,
    fc_n_byte,
    fc_n_byte2,
    fc_end
} file_contents;

typedef struct
{
    file_contents     contents;
    n_string          value;
    n_int             number;
} simulated_file_definition;

static simulated_file_definition being_delta_json[]=
    {
        {fc_object_name, "simulated_being_delta", 1},
        {fc_n_byte2,     "location", 2},
        {fc_n_byte,      "direction_facing", 1},
        {fc_n_byte,      "velocity", 10},
        {fc_n_byte2,     "stored_energy", 1},
        {fc_n_byte2,     "random_seed", 2},
        {fc_n_byte2,     "macro_state", 1},
        {fc_n_byte,      "parasites", 1},
        {fc_n_byte,      "honor", 1},
        {fc_n_byte,      "crowding", 1},
        {fc_n_byte2,     "height", 1},
        {fc_n_byte2,     "mass", 1},
        {fc_n_byte,      "posture", 1},
        {fc_n_byte2,     "goal", 4},

        {fc_n_byte2,     "social_coord_x", 1},
        {fc_n_byte2,     "social_coord_y", 1},
        {fc_n_byte2,     "social_coord_nx", 1},
        {fc_n_byte2,     "social_coord_ny", 1},

        {fc_n_byte,      "awake", 1},
        {fc_end,         0L, 1}
    };

#endif /* prototypejson_h */


/*
 
 outputs - object definition
 
 typedef struct
 {
     n_byte2     location[2];
     n_byte      direction_facing;
     n_byte      velocity[10];
     n_byte2     stored_energy;
     n_byte2     random_seed[2];
     n_byte2     macro_state;
     n_byte      parasites;
     n_byte      honor;
     n_byte      crowding;
     n_byte2     height;
     n_byte2     mass;
     n_byte      posture;
     n_byte2     goal[4];

     n_byte2     social_coord_x;
     n_byte2     social_coord_y;
     n_byte2     social_coord_nx;
     n_byte2     social_coord_ny;

     n_byte      awake;
     
 } simulated_being_delta;
 
 outputs - object reader
 
 static n_object *transfer_being_delta_obj( simulated_being_delta *delta )
 {
     n_object *simulated_being_delta = object_number( 0L, "direction_facing", delta->direction_facing );
     n_array   *location = array_number( delta->location[0] );
     n_array   *seed = array_number( delta->random_seed[0] );
     n_array   *goal = array_number( delta->goal[0] );
     n_array   *social_coord = array_number( delta->social_coord_x );

     array_add( location, array_number( delta->location[1] ) );

     array_add( seed, array_number( delta->random_seed[1] ) );

     array_add( goal, array_number( delta->goal[1] ) );
     array_add( goal, array_number( delta->goal[2] ) );
     array_add( goal, array_number( delta->goal[3] ) );

     array_add( social_coord, array_number( delta->social_coord_y ) );
     array_add( social_coord, array_number( delta->social_coord_nx ) );
     array_add( social_coord, array_number( delta->social_coord_ny ) );

     object_array( simulated_being_delta, "location", location );

     object_number( simulated_being_delta, "velocity", delta->velocity[0] );
     object_number( simulated_being_delta, "stored_energy", delta->stored_energy );

     object_array( simulated_being_delta, "random_seed", seed );

     object_number( simulated_being_delta, "macro_state", delta->macro_state );
     object_number( simulated_being_delta, "parasites", delta->parasites );
     object_number( simulated_being_delta, "honor", delta->honor );
     object_number( simulated_being_delta, "crowding", delta->crowding );
     object_number( simulated_being_delta, "height", delta->height );
     object_number( simulated_being_delta, "mass", delta->mass );
     object_number( simulated_being_delta, "posture", delta->posture );

     object_array( simulated_being_delta, "goal", goal );
     object_array( simulated_being_delta, "social_coord", social_coord );

     return simulated_being_delta;
 }
 
 */
