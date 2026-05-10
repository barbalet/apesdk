/****************************************************************

 transfer.c

 =============================================================

 Copyright 1996-2026 Tom Barbalet. All rights reserved.

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

 ****************************************************************/

#include "../entity/entity.h"

#include "universe_internal.h"
#include "universe.h"

/* does not appear to be used here */

void transfer_debug_csv( n_file *fil, n_byte initial )
{
    simulated_group *group = sim_group();

    io_write_csv( fil, ( n_byte * )( &( group->beings[0] ) ), simulated_file_format, FIL_BEI, initial );
}

enum transfer_land_offset
{
    TRANSFER_LAND_DATE = 0,
    TRANSFER_LAND_TIME = 4,
    TRANSFER_LAND_GENETICS = 6
};

enum transfer_being_offset
{
    TRANSFER_BEING_LOCATION = 0,
    TRANSFER_BEING_FACING = 4,
    TRANSFER_BEING_SPEED = 5,
    TRANSFER_BEING_ENERGY = 6,
    TRANSFER_BEING_DOB = 8,
    TRANSFER_BEING_RANDOM = 12,
    TRANSFER_BEING_STATE = 16,
    TRANSFER_BEING_BRAIN_STATE = 18,
    TRANSFER_BEING_HEIGHT = 30,
    TRANSFER_BEING_MASS = 32,
    TRANSFER_BEING_OVERRIDES = 34,
    TRANSFER_BEING_SHOUT = 36,
    TRANSFER_BEING_CROWDING = 42,
    TRANSFER_BEING_POSTURE = 43,
    TRANSFER_BEING_INVENTORY = 44,
    TRANSFER_BEING_PARASITES = 60,
    TRANSFER_BEING_HONOR = 61,
    TRANSFER_BEING_CONCEPTION = 62,
    TRANSFER_BEING_ATTENTION = 66,
    TRANSFER_BEING_GENETICS = 72,
    TRANSFER_BEING_FETAL_GENETICS = 88,
    TRANSFER_BEING_FATHER_NAME = 104,
    TRANSFER_BEING_SOCIAL_COORD = 108,
    TRANSFER_BEING_DRIVES = 116,
    TRANSFER_BEING_GOALS = 120,
    TRANSFER_BEING_PREFERENCES = 128,
    TRANSFER_BEING_GENERATION_MAX = 142,
    TRANSFER_BEING_GENERATION_MIN = 144,
    TRANSFER_BEING_CHILD_GENERATION_MAX = 146,
    TRANSFER_BEING_CHILD_GENERATION_MIN = 148,
    TRANSFER_BEING_TERRITORY = 150,
    TRANSFER_BEING_IMMUNE = 406,
    TRANSFER_BEING_BRAINCODE_REGISTER = 451,
    TRANSFER_BEING_BRAINPROBE = 454,
    TRANSFER_BEING_AWAKE_PLUS_ONE = 550,
    TRANSFER_BEING_NAME = 552
};

enum transfer_social_offset
{
    TRANSFER_SOCIAL_LOCATION = 0,
    TRANSFER_SOCIAL_TIME = 4,
    TRANSFER_SOCIAL_DATE = 6,
    TRANSFER_SOCIAL_FIRST_NAME = 10,
    TRANSFER_SOCIAL_FAMILY_NAME = 14,
    TRANSFER_SOCIAL_ATTRACTION = 18,
    TRANSFER_SOCIAL_FRIEND_FOE = 19,
    TRANSFER_SOCIAL_BELIEF = 20,
    TRANSFER_SOCIAL_FAMILIARITY = 22,
    TRANSFER_SOCIAL_RELATIONSHIP = 24,
    TRANSFER_SOCIAL_ENTITY_TYPE = 25,
    TRANSFER_SOCIAL_BRAINCODE = 26
};

enum transfer_episodic_offset
{
    TRANSFER_EPISODIC_LOCATION = 0,
    TRANSFER_EPISODIC_TIME = 4,
    TRANSFER_EPISODIC_DATE = 6,
    TRANSFER_EPISODIC_FIRST_NAME = 10,
    TRANSFER_EPISODIC_FAMILY_NAME = 14,
    TRANSFER_EPISODIC_EVENT = 18,
    TRANSFER_EPISODIC_FOOD = 19,
    TRANSFER_EPISODIC_AFFECT = 20,
    TRANSFER_EPISODIC_ARG = 22
};

enum transfer_territory_offset
{
    TRANSFER_TERRITORY_INDEX = 0,
    TRANSFER_TERRITORY_NAME = 2,
    TRANSFER_TERRITORY_FAMILIARITY = 4
};

static void transfer_store_byte2( n_byte *data, n_uint offset, n_byte2 value )
{
    memory_copy( ( n_byte * )&value, &data[offset], sizeof( n_byte2 ) );
}

static void transfer_store_byte4( n_byte *data, n_uint offset, n_byte4 value )
{
    memory_copy( ( n_byte * )&value, &data[offset], sizeof( n_byte4 ) );
}

static n_byte2 transfer_read_byte2( n_byte *data, n_uint offset )
{
    n_byte2 value = 0;
    memory_copy( &data[offset], ( n_byte * )&value, sizeof( n_byte2 ) );
    return value;
}

static n_byte4 transfer_read_byte4( n_byte *data, n_uint offset )
{
    n_byte4 value = 0;
    memory_copy( &data[offset], ( n_byte * )&value, sizeof( n_byte4 ) );
    return value;
}

static void transfer_store_byte2_array( n_byte *data, n_uint offset, n_byte2 *values, n_uint count )
{
    n_uint loop = 0;
    while ( loop < count )
    {
        transfer_store_byte2( data, offset + ( loop * sizeof( n_byte2 ) ), values[loop] );
        loop++;
    }
}

static void transfer_read_byte2_array( n_byte *data, n_uint offset, n_byte2 *values, n_uint count )
{
    n_uint loop = 0;
    while ( loop < count )
    {
        values[loop] = transfer_read_byte2( data, offset + ( loop * sizeof( n_byte2 ) ) );
        loop++;
    }
}

static void transfer_pack_land( n_byte *data, n_uint size )
{
    n_byte2 *genetics = land_genetics();
    memory_erase( data, size );
    transfer_store_byte4( data, TRANSFER_LAND_DATE, land_date() );
    transfer_store_byte2( data, TRANSFER_LAND_TIME, ( n_byte2 )land_time() );
    transfer_store_byte2_array( data, TRANSFER_LAND_GENETICS, genetics, 2 );
}

static void transfer_unpack_land( n_byte *data )
{
    n_byte2 genetics[2];
    transfer_read_byte2_array( data, TRANSFER_LAND_GENETICS, genetics, 2 );
    land_load_state( transfer_read_byte4( data, TRANSFER_LAND_DATE ),
                     transfer_read_byte2( data, TRANSFER_LAND_TIME ),
                     genetics );
}

static void transfer_pack_topography( n_byte *data, n_uint size )
{
    memory_erase( data, size );
    memory_copy( land_topography(), data, TOPOGRAPHY_BYTES );
}

static void transfer_unpack_topography( n_byte *data )
{
    memory_copy( data, land_topography(), TOPOGRAPHY_BYTES );
    land_init_high_def( 1 );
    land_tide();
}

static void transfer_pack_weather( n_byte *data, n_uint size )
{
    memory_erase( data, size );
    memory_copy( ( n_byte * )land_weather( 0 ), data, WEATHER_ATMOSPHERE_BYTES );
    memory_copy( land_weather_lightning( 0 ), &data[WEATHER_LIGHTNING_OFFSET], MAP_AREA );
}

static void transfer_unpack_weather( n_byte *data )
{
    memory_copy( data, ( n_byte * )land_weather( 0 ), WEATHER_ATMOSPHERE_BYTES );
    memory_copy( &data[WEATHER_LIGHTNING_OFFSET], land_weather_lightning( 0 ), MAP_AREA );
}

#ifdef TERRITORY_ON
static void transfer_pack_inline_territory( n_byte *data, simulated_iplace *territory )
{
    n_uint loop = 0;
    while ( loop < TERRITORY_AREA )
    {
        n_uint offset = TRANSFER_BEING_TERRITORY + ( loop * 4 );
        transfer_store_byte2( data, offset, ( n_byte2 )territory[loop].name );
        transfer_store_byte2( data, offset + 2, territory[loop].familiarity );
        loop++;
    }
}

static void transfer_unpack_inline_territory( n_byte *data, simulated_iplace *territory )
{
    n_uint loop = 0;
    while ( loop < TERRITORY_AREA )
    {
        n_uint offset = TRANSFER_BEING_TERRITORY + ( loop * 4 );
        territory[loop].name = ( n_byte )transfer_read_byte2( data, offset );
        territory[loop].familiarity = transfer_read_byte2( data, offset + 2 );
        loop++;
    }
}

static void transfer_pack_territory( n_byte *data, n_uint size, n_uint index, simulated_iplace *territory )
{
    memory_erase( data, size );
    transfer_store_byte2( data, TRANSFER_TERRITORY_INDEX, ( n_byte2 )index );
    data[TRANSFER_TERRITORY_NAME] = territory->name;
    transfer_store_byte2( data, TRANSFER_TERRITORY_FAMILIARITY, territory->familiarity );
}

static n_int transfer_unpack_territory( n_byte *data, simulated_being *being )
{
    n_uint index = transfer_read_byte2( data, TRANSFER_TERRITORY_INDEX );
    if ( index >= TERRITORY_AREA )
    {
        return -1;
    }
    being->events.territory[index].name = data[TRANSFER_TERRITORY_NAME];
    being->events.territory[index].familiarity = transfer_read_byte2( data, TRANSFER_TERRITORY_FAMILIARITY );
    return 0;
}
#endif

static void transfer_pack_being( n_byte *data, n_uint size, simulated_being *being )
{
    memory_erase( data, size );
    transfer_store_byte2_array( data, TRANSFER_BEING_LOCATION, being->delta.location, 2 );
    data[TRANSFER_BEING_FACING] = being->delta.direction_facing;
    data[TRANSFER_BEING_SPEED] = being->delta.velocity[0];
    transfer_store_byte2( data, TRANSFER_BEING_ENERGY, being->delta.stored_energy );
    transfer_store_byte4( data, TRANSFER_BEING_DOB, being->constant.date_of_birth );
    transfer_store_byte2_array( data, TRANSFER_BEING_RANDOM, being->delta.random_seed, 2 );
    transfer_store_byte2( data, TRANSFER_BEING_STATE, being->delta.macro_state );
    transfer_store_byte2_array( data, TRANSFER_BEING_BRAIN_STATE, being->braindata.brain_state, 6 );
    transfer_store_byte2( data, TRANSFER_BEING_HEIGHT, being->delta.height );
    transfer_store_byte2( data, TRANSFER_BEING_MASS, being->delta.mass );
    transfer_store_byte2( data, TRANSFER_BEING_OVERRIDES, being->braindata.script_overrides );
    memory_copy( being->changes.shout, &data[TRANSFER_BEING_SHOUT], SHOUT_BYTES );
    data[TRANSFER_BEING_CROWDING] = being->delta.crowding;
    data[TRANSFER_BEING_POSTURE] = being->delta.posture;
    transfer_store_byte2_array( data, TRANSFER_BEING_INVENTORY, being->changes.inventory, INVENTORY_SIZE );
    data[TRANSFER_BEING_PARASITES] = being->delta.parasites;
    data[TRANSFER_BEING_HONOR] = being->delta.honor;
    transfer_store_byte4( data, TRANSFER_BEING_CONCEPTION, being->changes.date_of_conception );
    memory_copy( being->braindata.attention, &data[TRANSFER_BEING_ATTENTION], ATTENTION_SIZE );
    memory_copy( ( n_byte * )being->constant.genetics, &data[TRANSFER_BEING_GENETICS], GENETICS_BYTES );
    memory_copy( ( n_byte * )being->changes.fetal_genetics, &data[TRANSFER_BEING_FETAL_GENETICS], GENETICS_BYTES );
    data[TRANSFER_BEING_FATHER_NAME] = ( n_byte )being->changes.father_name[0];
    data[TRANSFER_BEING_FATHER_NAME + 1] = ( n_byte )being->changes.father_name[1];
    transfer_store_byte2( data, TRANSFER_BEING_SOCIAL_COORD, being->delta.social_coord_x );
    transfer_store_byte2( data, TRANSFER_BEING_SOCIAL_COORD + 2, being->delta.social_coord_y );
    transfer_store_byte2( data, TRANSFER_BEING_SOCIAL_COORD + 4, being->delta.social_coord_nx );
    transfer_store_byte2( data, TRANSFER_BEING_SOCIAL_COORD + 6, being->delta.social_coord_ny );
    memory_copy( being->changes.drives, &data[TRANSFER_BEING_DRIVES], DRIVES );
    transfer_store_byte2_array( data, TRANSFER_BEING_GOALS, being->delta.goal, 4 );
    memory_copy( being->changes.learned_preference, &data[TRANSFER_BEING_PREFERENCES], PREFERENCES );
    transfer_store_byte2( data, TRANSFER_BEING_GENERATION_MAX, being->constant.generation_max );
    transfer_store_byte2( data, TRANSFER_BEING_GENERATION_MIN, being->constant.generation_min );
    transfer_store_byte2( data, TRANSFER_BEING_CHILD_GENERATION_MAX, being->changes.child_generation_max );
    transfer_store_byte2( data, TRANSFER_BEING_CHILD_GENERATION_MIN, being->changes.child_generation_min );
    data[TRANSFER_BEING_AWAKE_PLUS_ONE] = being->delta.awake + 1;
    transfer_store_byte2_array( data, TRANSFER_BEING_NAME, being->constant.name, 2 );
#ifdef TERRITORY_ON
    transfer_pack_inline_territory( data, being->events.territory );
#endif
#ifdef IMMUNE_ON
    memory_copy( ( n_byte * ) & ( being->immune_system ), &data[TRANSFER_BEING_IMMUNE], sizeof( simulated_immune_system ) );
#endif
#ifdef BRAINCODE_ON
    memory_copy( being->braindata.braincode_register, &data[TRANSFER_BEING_BRAINCODE_REGISTER], BRAINCODE_PSPACE_REGISTERS );
    memory_copy( ( n_byte * )being->braindata.brainprobe, &data[TRANSFER_BEING_BRAINPROBE], sizeof( simulated_ibrain_probe ) * BRAINCODE_PROBES );
#endif
}

static void transfer_unpack_being( n_byte *data, simulated_being *being )
{
    memory_erase( ( n_byte * )being, sizeof( simulated_being ) );
    transfer_read_byte2_array( data, TRANSFER_BEING_LOCATION, being->delta.location, 2 );
    being->delta.direction_facing = data[TRANSFER_BEING_FACING];
    being->delta.velocity[0] = data[TRANSFER_BEING_SPEED];
    being->delta.stored_energy = transfer_read_byte2( data, TRANSFER_BEING_ENERGY );
    being->constant.date_of_birth = transfer_read_byte4( data, TRANSFER_BEING_DOB );
    transfer_read_byte2_array( data, TRANSFER_BEING_RANDOM, being->delta.random_seed, 2 );
    being->delta.macro_state = transfer_read_byte2( data, TRANSFER_BEING_STATE );
    transfer_read_byte2_array( data, TRANSFER_BEING_BRAIN_STATE, being->braindata.brain_state, 6 );
    being->delta.height = transfer_read_byte2( data, TRANSFER_BEING_HEIGHT );
    being->delta.mass = transfer_read_byte2( data, TRANSFER_BEING_MASS );
    being->braindata.script_overrides = transfer_read_byte2( data, TRANSFER_BEING_OVERRIDES );
    memory_copy( &data[TRANSFER_BEING_SHOUT], being->changes.shout, SHOUT_BYTES );
    being->delta.crowding = data[TRANSFER_BEING_CROWDING];
    being->delta.posture = data[TRANSFER_BEING_POSTURE];
    transfer_read_byte2_array( data, TRANSFER_BEING_INVENTORY, being->changes.inventory, INVENTORY_SIZE );
    being->delta.parasites = data[TRANSFER_BEING_PARASITES];
    being->delta.honor = data[TRANSFER_BEING_HONOR];
    being->changes.date_of_conception = transfer_read_byte4( data, TRANSFER_BEING_CONCEPTION );
    memory_copy( &data[TRANSFER_BEING_ATTENTION], being->braindata.attention, ATTENTION_SIZE );
    memory_copy( &data[TRANSFER_BEING_GENETICS], ( n_byte * )being->constant.genetics, GENETICS_BYTES );
    memory_copy( &data[TRANSFER_BEING_FETAL_GENETICS], ( n_byte * )being->changes.fetal_genetics, GENETICS_BYTES );
    being->changes.father_name[0] = data[TRANSFER_BEING_FATHER_NAME];
    being->changes.father_name[1] = data[TRANSFER_BEING_FATHER_NAME + 1];
    being->delta.social_coord_x = transfer_read_byte2( data, TRANSFER_BEING_SOCIAL_COORD );
    being->delta.social_coord_y = transfer_read_byte2( data, TRANSFER_BEING_SOCIAL_COORD + 2 );
    being->delta.social_coord_nx = transfer_read_byte2( data, TRANSFER_BEING_SOCIAL_COORD + 4 );
    being->delta.social_coord_ny = transfer_read_byte2( data, TRANSFER_BEING_SOCIAL_COORD + 6 );
    memory_copy( &data[TRANSFER_BEING_DRIVES], being->changes.drives, DRIVES );
    transfer_read_byte2_array( data, TRANSFER_BEING_GOALS, being->delta.goal, 4 );
    memory_copy( &data[TRANSFER_BEING_PREFERENCES], being->changes.learned_preference, PREFERENCES );
    being->constant.generation_max = transfer_read_byte2( data, TRANSFER_BEING_GENERATION_MAX );
    being->constant.generation_min = transfer_read_byte2( data, TRANSFER_BEING_GENERATION_MIN );
    being->changes.child_generation_max = transfer_read_byte2( data, TRANSFER_BEING_CHILD_GENERATION_MAX );
    being->changes.child_generation_min = transfer_read_byte2( data, TRANSFER_BEING_CHILD_GENERATION_MIN );
    if ( data[TRANSFER_BEING_AWAKE_PLUS_ONE] != 0 )
    {
        being->delta.awake = data[TRANSFER_BEING_AWAKE_PLUS_ONE] - 1;
    }
    else
    {
        being->delta.awake = ( being->delta.stored_energy > BEING_DEAD ) ? FULLY_AWAKE : FULLY_ASLEEP;
    }
    transfer_read_byte2_array( data, TRANSFER_BEING_NAME, being->constant.name, 2 );
#ifdef TERRITORY_ON
    transfer_unpack_inline_territory( data, being->events.territory );
#endif
#ifdef IMMUNE_ON
    memory_copy( &data[TRANSFER_BEING_IMMUNE], ( n_byte * ) & ( being->immune_system ), sizeof( simulated_immune_system ) );
#endif
#ifdef BRAINCODE_ON
    memory_copy( &data[TRANSFER_BEING_BRAINCODE_REGISTER], being->braindata.braincode_register, BRAINCODE_PSPACE_REGISTERS );
    memory_copy( &data[TRANSFER_BEING_BRAINPROBE], ( n_byte * )being->braindata.brainprobe, sizeof( simulated_ibrain_probe ) * BRAINCODE_PROBES );
#endif
}

static void transfer_pack_social( n_byte *data, n_uint size, simulated_isocial *social )
{
    memory_erase( data, size );
    transfer_store_byte2_array( data, TRANSFER_SOCIAL_LOCATION, social->space_time.location, 2 );
    transfer_store_byte2( data, TRANSFER_SOCIAL_TIME, ( n_byte2 )social->space_time.time );
    transfer_store_byte4( data, TRANSFER_SOCIAL_DATE, social->space_time.date );
    transfer_store_byte2( data, TRANSFER_SOCIAL_FIRST_NAME, social->first_name[BEING_MET] );
    transfer_store_byte2( data, TRANSFER_SOCIAL_FAMILY_NAME, social->family_name[BEING_MET] );
    data[TRANSFER_SOCIAL_ATTRACTION] = social->attraction;
    data[TRANSFER_SOCIAL_FRIEND_FOE] = social->friend_foe;
    transfer_store_byte2( data, TRANSFER_SOCIAL_BELIEF, social->belief );
    transfer_store_byte2( data, TRANSFER_SOCIAL_FAMILIARITY, social->familiarity );
    data[TRANSFER_SOCIAL_RELATIONSHIP] = social->relationship;
    data[TRANSFER_SOCIAL_ENTITY_TYPE] = social->entity_type;
#ifdef BRAINCODE_ON
    memory_copy( social->braincode, &data[TRANSFER_SOCIAL_BRAINCODE], BRAINCODE_SIZE );
#endif
}

static void transfer_unpack_social( n_byte *data, simulated_isocial *social )
{
    memory_erase( ( n_byte * )social, sizeof( simulated_isocial ) );
    transfer_read_byte2_array( data, TRANSFER_SOCIAL_LOCATION, social->space_time.location, 2 );
    social->space_time.time = transfer_read_byte2( data, TRANSFER_SOCIAL_TIME );
    social->space_time.date = transfer_read_byte4( data, TRANSFER_SOCIAL_DATE );
    social->first_name[BEING_MET] = transfer_read_byte2( data, TRANSFER_SOCIAL_FIRST_NAME );
    social->family_name[BEING_MET] = transfer_read_byte2( data, TRANSFER_SOCIAL_FAMILY_NAME );
    social->attraction = data[TRANSFER_SOCIAL_ATTRACTION];
    social->friend_foe = data[TRANSFER_SOCIAL_FRIEND_FOE];
    social->belief = transfer_read_byte2( data, TRANSFER_SOCIAL_BELIEF );
    social->familiarity = transfer_read_byte2( data, TRANSFER_SOCIAL_FAMILIARITY );
    social->relationship = data[TRANSFER_SOCIAL_RELATIONSHIP];
    social->entity_type = data[TRANSFER_SOCIAL_ENTITY_TYPE];
#ifdef BRAINCODE_ON
    memory_copy( &data[TRANSFER_SOCIAL_BRAINCODE], social->braincode, BRAINCODE_SIZE );
#endif
}

static void transfer_pack_episodic( n_byte *data, n_uint size, simulated_iepisodic *episodic )
{
    memory_erase( data, size );
    transfer_store_byte2_array( data, TRANSFER_EPISODIC_LOCATION, episodic->space_time.location, 2 );
    transfer_store_byte2( data, TRANSFER_EPISODIC_TIME, ( n_byte2 )episodic->space_time.time );
    transfer_store_byte4( data, TRANSFER_EPISODIC_DATE, episodic->space_time.date );
    transfer_store_byte2_array( data, TRANSFER_EPISODIC_FIRST_NAME, episodic->first_name, 2 );
    transfer_store_byte2_array( data, TRANSFER_EPISODIC_FAMILY_NAME, episodic->family_name, 2 );
    data[TRANSFER_EPISODIC_EVENT] = episodic->event;
    data[TRANSFER_EPISODIC_FOOD] = episodic->food;
    transfer_store_byte2( data, TRANSFER_EPISODIC_AFFECT, episodic->affect );
    transfer_store_byte2( data, TRANSFER_EPISODIC_ARG, episodic->arg );
}

static void transfer_unpack_episodic( n_byte *data, simulated_iepisodic *episodic )
{
    memory_erase( ( n_byte * )episodic, sizeof( simulated_iepisodic ) );
    transfer_read_byte2_array( data, TRANSFER_EPISODIC_LOCATION, episodic->space_time.location, 2 );
    episodic->space_time.time = transfer_read_byte2( data, TRANSFER_EPISODIC_TIME );
    episodic->space_time.date = transfer_read_byte4( data, TRANSFER_EPISODIC_DATE );
    transfer_read_byte2_array( data, TRANSFER_EPISODIC_FIRST_NAME, episodic->first_name, 2 );
    transfer_read_byte2_array( data, TRANSFER_EPISODIC_FAMILY_NAME, episodic->family_name, 2 );
    episodic->event = data[TRANSFER_EPISODIC_EVENT];
    episodic->food = data[TRANSFER_EPISODIC_FOOD];
    episodic->affect = transfer_read_byte2( data, TRANSFER_EPISODIC_AFFECT );
    episodic->arg = transfer_read_byte2( data, TRANSFER_EPISODIC_ARG );
}

/* provide an output file buffer to be written */
static void transfer_land( n_file *tranfer_out, simulated_group *group, simulated_file_entry *format, n_byte *store, n_uint store_size )
{
    n_byte2    loc_signature[2] = {( n_byte2 )SIMULATED_APE_SIGNATURE, ( n_byte2 )VERSION_NUMBER};
    ( void )group;

#ifdef USE_FIL_VER
    io_write_buff( tranfer_out, loc_signature, format, FIL_VER, 0L );
#endif
#ifdef USE_FIL_LAN
    transfer_pack_land( store, store_size );
    io_write_buff( tranfer_out, store, format, FIL_LAN, 0L );
#endif
#ifdef USE_FIL_TOP
    transfer_pack_topography( store, store_size );
    io_write_buff( tranfer_out, store, format, FIL_TOP, 0L );
#endif
#ifdef USE_FIL_WEA
    transfer_pack_weather( store, store_size );
    io_write_buff( tranfer_out, store, format, FIL_WEA, 0L );
#endif
}

static void transfer_being( n_file *tranfer_out, simulated_group *group, n_int being, simulated_file_entry *format, n_byte *store, n_uint store_size )
{
    simulated_being *local_being = &( group->beings[being] );
#ifdef USE_FIL_BEI
    transfer_pack_being( store, store_size, local_being );
    io_write_buff( tranfer_out, store, format, FIL_BEI, 0L );
#endif
#ifdef USE_FIL_SOE
    {
        n_int loop = 0;
        simulated_isocial *social = being_social( local_being );
        while ( loop < SOCIAL_SIZE )
        {
            transfer_pack_social( store, store_size, &social[loop] );
            io_write_buff( tranfer_out, store, format, FIL_SOE, &brain_three_byte_command );
            loop++;
        }
    }
#endif
#ifdef USE_FIL_EPI
    {
        n_int loop = 0;
        simulated_iepisodic *episodic = being_episodic( local_being );
        while ( loop < EPISODIC_SIZE )
        {
            transfer_pack_episodic( store, store_size, &episodic[loop] );
            io_write_buff( tranfer_out, store, format, FIL_EPI, 0L );
            loop++;
        }
    }
#endif
#ifdef USE_FIL_TER
#ifdef TERRITORY_ON
    {
        n_uint loop = 0;
        while ( loop < TERRITORY_AREA )
        {
            if ( ( local_being->events.territory[loop].name != 0 ) ||
                    ( local_being->events.territory[loop].familiarity != 0 ) )
            {
                transfer_pack_territory( store, store_size, loop, &local_being->events.territory[loop] );
                io_write_buff( tranfer_out, store, format, FIL_TER, 0L );
            }
            loop++;
        }
    }
#endif
#endif
}

static n_object *transfer_land_obj( void )
{
    n_object *simulated_iland = object_number( 0L, "date", land_date() );
    n_byte2   *genetics_values = land_genetics();
    n_array   *land_genetics = array_number( genetics_values[0] );
    array_add( land_genetics, array_number( genetics_values[1] ) );

    object_array( simulated_iland, "genetics", land_genetics );
    object_number( simulated_iland, "time", land_time() );
    return simulated_iland;
}

#if 0
static n_object *transfer_being_spacetime_obj( n_spacetime *value )
{
    n_object *simulated_ispacetime = object_number( 0L, "date", value->date );
    n_array   *simulated_ilocation = array_number( value->location[0] );
    array_add( simulated_ilocation, array_number( value->location[1] ) );
    object_array( simulated_ispacetime, "location", simulated_ilocation );
    object_number( simulated_ispacetime, "time", value->time );
    return simulated_ispacetime;
}
#endif

static n_object *transfer_being_constant_obj( simulated_being_constant *constant )
{
    n_int genetic_count = 1;
    n_object *simulated_being_contant = object_number( 0L, "date_of_birth", constant->date_of_birth );
    n_array *generation = array_number( constant->generation_min );
    n_array *genetics = array_number( constant->genetics[0] );
    while ( genetic_count < CHROMOSOMES )
    {
        array_add( genetics, array_number( constant->genetics[genetic_count++] ) );
    }
    object_array( simulated_being_contant, "genetics", genetics );

    array_add( generation, array_number( constant->generation_max ) );
    object_array( simulated_being_contant, "generation_range", generation );

    return simulated_being_contant;
}


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

static n_object *transfer_being_obj( simulated_being *being )
{
    n_object *simulated_being = 0L;
    n_string_block simple_name;
    being_name_simple( being, simple_name );
    simulated_being = object_string( 0L, "name", simple_name );
    object_object( simulated_being, "delta", transfer_being_delta_obj( &( being->delta ) ) );
    object_object( simulated_being, "constant", transfer_being_constant_obj( &( being->constant ) ) );
    return simulated_being;
}


static n_object *transfer_sim_obj( void )
{
    n_object *simulated_isim_identifier = object_number( 0L, "signature", SIMULATED_APE_SIGNATURE );
    object_number( simulated_isim_identifier, "version number", VERSION_NUMBER );
    object_string( simulated_isim_identifier, "copyright", FULL_VERSION_COPYRIGHT );
    object_string( simulated_isim_identifier, "date", FULL_DATE );
    return simulated_isim_identifier;
}

n_file *tranfer_out_json( void )
{
    n_file *output_file = 0L;
    simulated_group *group = sim_group();

    n_object *simulation_object = object_object( 0L, "information", transfer_sim_obj() );

    object_object( simulation_object, "land", transfer_land_obj() );

    if ( group->num > 0 )
    {
        n_uint        count = 1;
        simulated_being *local_beings = group->beings;
        n_object    *being_object = transfer_being_obj( &( local_beings[0] ) );
        n_array     *beings = array_object( being_object );
        while ( count < group->num )
        {
            being_object = transfer_being_obj( &( local_beings[count++] ) );
            array_add( beings, array_object( being_object ) );
        }
        object_array( simulation_object, "beings", beings );
    }

    output_file = unknown_json( simulation_object, OBJECT_OBJECT );

    obj_free( &simulation_object );

    return output_file;
}

n_file *tranfer_out( void )
{
    simulated_group *group = sim_group();

    n_file           *returnFile = io_file_new();
    n_int              loop = 0;
    n_string fluff[5] = {SHORT_VERSION_NAME, FULL_DATE, COPYRIGHT_DATE, COPYRIGHT_NAME, COPYRIGHT_FOLLOW };
    n_uint             size_buffer = io_find_size_data( ( simulated_file_entry * )simulated_file_format );
    n_byte            *temp_store = 0L;

    if ( returnFile == 0L )
    {
        return 0L;
    }
    if ( returnFile->data == 0L )
    {
        memory_free( ( void ** )&returnFile );
        return 0L;
    }

    temp_store = ( n_byte * )memory_new( size_buffer );
    if ( temp_store == 0L )
    {
        io_file_free( &returnFile );
        return 0L;
    }

    io_write_buff( returnFile, fluff, 0L, FILE_COPYRIGHT, 0L );

    transfer_land( returnFile, group, ( simulated_file_entry * )simulated_file_format, temp_store, size_buffer );

    while ( loop < ( n_int )( group->num ) )
    {
        transfer_being( returnFile, group, loop, ( simulated_file_entry * )simulated_file_format, temp_store, size_buffer );
        loop++;
    }

    /* TODO: Brain block */
    memory_free( ( void ** )&temp_store );
    return returnFile;
}

n_int    tranfer_in( n_file *input_file )
{
    n_int  ret_val;
    n_byte *temp_store = 0L;
    n_uint ape_count = 0;
    n_uint social_count = 0;
    n_uint episodic_count = 0;
    n_int current_being = -1;

    simulated_group *group = sim_group();
    n_uint  size_buffer = io_find_size_data( ( simulated_file_entry * )simulated_file_format );

    if ( input_file == 0L )
    {
        return SHOW_ERROR( "No file provided" );
    }

    temp_store = ( n_byte * )memory_new( size_buffer );

    if ( temp_store == 0L )
    {
        return SHOW_ERROR( "No temporary storage memory available" );
    }

    io_whitespace( input_file );

    input_file->location = 0;

    ret_val = io_read_buff( input_file, temp_store, simulated_file_format );

    if ( ret_val != FIL_VER ) /* signature must be first */
    {
        memory_free( ( void ** )&temp_store );
        return SHOW_ERROR( "Signature not first in file" );
    }

    {
        n_byte2    *signature = ( n_byte2 * )temp_store;

        if ( signature[0] != SIMULATED_APE_SIGNATURE ) /* not a Simulated Ape file */
        {
            memory_free( ( void ** )&temp_store );
            return SHOW_ERROR( "Not a Simulated Ape File" );
        }

        if ( signature[1] > VERSION_NUMBER ) /* file version greater than this version */
        {
            memory_free( ( void ** )&temp_store );
            return SHOW_ERROR( "File newer than Simulation" );
        }
    }

    do
    {
        ret_val = io_read_buff( input_file, temp_store, simulated_file_format );
        if ( ret_val == -1 )
        {
            memory_free( ( void ** )&temp_store );
            return SHOW_ERROR( "Failure in file load" );
        }
        if ( ret_val < FILE_EOF )
        {
            switch ( ret_val )
            {
            case FIL_LAN:
                transfer_unpack_land( temp_store );
                break;
            case FIL_TOP:
                transfer_unpack_topography( temp_store );
                break;
            case FIL_WEA:
                transfer_unpack_weather( temp_store );
                break;
            case FIL_BEI:
                if ( ape_count >= group->max )
                {
                    group->num = ape_count;
                    memory_free( ( void ** )&temp_store );
                    return SHOW_ERROR( "Too many apes for memory" );
                }
                transfer_unpack_being( temp_store, &( group->beings[ape_count] ) );
                current_being = ( n_int )ape_count;
                ape_count ++;
                social_count = 0;
                episodic_count = 0;
                break;
            case FIL_SOE:
                if ( current_being < 0 )
                {
                    memory_free( ( void ** )&temp_store );
                    return SHOW_ERROR( "Social graph before being" );
                }
                if ( social_count >= SOCIAL_SIZE )
                {
                    group->num = ape_count;
                    memory_free( ( void ** )&temp_store );
                    return SHOW_ERROR( "Too many social graph events for memory" );
                }
                transfer_unpack_social( temp_store, &being_social( &( group->beings[current_being] ) )[social_count] );
                social_count ++;
                break;
            case FIL_EPI:
                if ( current_being < 0 )
                {
                    memory_free( ( void ** )&temp_store );
                    return SHOW_ERROR( "Episodic event before being" );
                }
                if ( episodic_count >= EPISODIC_SIZE )
                {
                    group->num = ape_count;
                    memory_free( ( void ** )&temp_store );
                    return SHOW_ERROR( "Too many episodic events for memory" );
                }
                transfer_unpack_episodic( temp_store, &being_episodic( &( group->beings[current_being] ) )[episodic_count] );
                episodic_count ++;
                break;
            case FIL_TER:
#ifdef TERRITORY_ON
                if ( current_being < 0 )
                {
                    memory_free( ( void ** )&temp_store );
                    return SHOW_ERROR( "Territory before being" );
                }
                if ( transfer_unpack_territory( temp_store, &( group->beings[current_being] ) ) != 0 )
                {
                    group->num = ape_count;
                    memory_free( ( void ** )&temp_store );
                    return SHOW_ERROR( "Too many territory events for memory" );
                }
#endif
                break;
            default:
            {
                memory_free( ( void ** )&temp_store );
                return SHOW_ERROR( "Unknown file kind" ); /*unknown kind*/
            }
            break;
            }

        }

    }
    while ( ret_val < FILE_EOF );

    if ( ret_val == FILE_EOF )
    {
        group->num = ape_count;
        group->select = ( ape_count > 0 ) ? group->beings : 0L;
        memory_free( ( void ** )&temp_store );
        return 0;
    }
    memory_free( ( void ** )&temp_store );
    return SHOW_ERROR( "Process file failed" );
}
