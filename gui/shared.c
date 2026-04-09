/****************************************************************

 shared.c

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

/* This is intended to dramatically simplify the interface between
     the various platform versions of Simulated Ape */

#include <stdio.h>
#include <math.h>

#include "buffer.h"
#include "gui.h"

#include "../entity/entity_internal.h"

#ifndef    _WIN32

#include "../shared.h"

#else

#include "..\shared.h"

#endif

static n_int   simulation_started = 0;

static n_int   mouse_x, mouse_y;
static n_byte  mouse_option, mouse_identification;
static n_byte  mouse_down;

static n_byte  mouse_drag;
static n_int   mouse_drag_x, mouse_drag_y;

static n_byte  key_identification;
static n_byte2 key_value;
static n_byte  key_down;

static ape_buffer outputBuffer = { 0L, 0L, 0 };

static n_int    ios_control_x = 0;
static n_int    ios_control_y = 0;

static n_int    ios_view_x = 0;
static n_int    ios_view_y = 0;

static n_int    ios_dimension_x = 0;
static n_int    ios_dimension_y = 0;

#define	Y_DELTA	36
#define	X_DELTA	20

static n_int toggle_pause = 0;

static n_int control_toggle_pause( n_byte actual_toggle )
{
    if ( io_command_line_execution() )
    {
        command_stop( 0L, "", io_console_out );
    }

    if ( actual_toggle )
    {
        toggle_pause ^= 1;
    }
    else
    {
        toggle_pause = 1;
    }

    return toggle_pause;
}

/* do mouse down interaction over a window, at a point, with keys possibly pressed */

extern n_byte	check_about;
extern n_uint	tilt_z;

static n_int control_mouse_was_down = 0;

static void control_mouse( n_byte wwind, n_int px, n_int py, n_byte option )
{
    if ( wwind == NUM_CONTROL )
    {
        if ( !mouse_down && control_mouse_was_down )
        {
            sim_control_regular( px, py );
        }
        control_mouse_was_down = mouse_down;
    }

    if ( wwind == NUM_VIEW )
    {
        if ( option )
        {
            sim_view_options( px, py );
        }
        else
        {
            if ( sim_view_regular( px, py ) )
            {
                if ( mouse_drag == 0 )
                {
                    mouse_drag_x = px;
                    mouse_drag_y = py;
                    mouse_drag = 1;
                }
            }
        }
    }
    if ( wwind == NUM_TERRAIN )
    {
        n_int upper_x, upper_y;

        check_about = 0;
        draw_terrain_coord( &upper_x, &upper_y );

        if ( option != 0 )
        {
            if ( ( px > ( upper_x / 4 ) ) && ( px < ( ( upper_x * 3 ) / 4 ) ) )
            {
                if ( py < ( upper_y / 4 ) )
                {
                    tilt_z = ( ( tilt_z + 1 ) & 255 );
                }
                else if ( py < ( ( upper_y * 3 ) / 4 ) )
                {
                    tilt_z = ( ( tilt_z + 255 ) & 255 );
                }
            }
        }
        else
        {
            n_int sx = px - ( ( upper_x ) >> 1 );
            sim_terrain( sx );
        }
    }
}

/* do key down in which window */
static void control_key( n_byte wwind, n_byte2 num )
{
    if ( ( num > 27 ) && ( num < 32 ) ) /* 28, 29, 30, 31 */
    {
        if ( wwind != NUM_VIEW )
        {
            if ( ( num - 28 ) < 2 )
            {
                sim_rotate( 0 - ( 1 - ( ( num - 28 ) << 1 ) ) );
            }
            else
            {
                sim_move( ( 160 - ( ( num - 28 ) << 6 ) ), 1 );
            }
        }
        else
        {
            sim_move( ( 31 - num ), 0 );
        }
    }
    if ( ( num > 2075 ) && ( num < 2078 ) )
    {
        sim_rotate( 0 - ( -1 + ( ( 2077 - num ) << 1 ) ) );
    }
    if ( ( num > 2077 ) && ( num < 2080 ) )
    {
        sim_change_selected( num == 2078 );
    }
}

#ifdef SCRIPT_DEBUG

static n_int script_entry = 1;

static n_int shared_script_debug_ready( void )
{
    if ( script_entry == 0 )
    {
        return 0;
    }
    if ( scdebug_file_ready() != 0L )
    {
        script_entry = 0;
        return 1;
    }
    return 0;
}

void shared_script_debug_handle( n_constant_string cStringFileName )
{
    if ( cStringFileName )
    {
        n_file   *outputfile = scdebug_file_ready();
        io_disk_write( outputfile, cStringFileName );
    }
    scdebug_file_cleanup();
    script_entry = 1;
}

#else

void shared_script_debug_handle( n_constant_string cStringFileName )
{

}

#endif

static void *control_init( KIND_OF_USE kind, n_uint randomise )
{
    void *sim_return = 0L;

    shared_menu( NA_MENU_CLEAR_ERRORS );
    draw_undraw_clear();

    sim_return = sim_init( kind, randomise, OFFSCREENSIZE, VIEWWINDOW( 0 ) );
    sim_set_output( 1 );
    if ( sim_return )
    {
        return draw_offscreen( sim_return );
    }
    return 0;
}

void shared_dimensions( n_int *dimensions )
{
    dimensions[0] = 3;
    dimensions[1] = 512;
    dimensions[2] = 512;
    dimensions[3] = 1;
}

shared_cycle_state shared_cycle( n_uint ticks, n_int localIdentification )
{
    shared_cycle_state return_value = SHARED_CYCLE_OK;

    if ( simulation_started == 0 )
    {
        return return_value;
    }

    ticks = ticks & 67108863; /* 71 58 27 88 */
    ticks *= 60;
#ifndef	_WIN32
    sim_thread_console();
#endif
    if ( mouse_identification == localIdentification )
    {
        if ( localIdentification == NUM_CONTROL )
        {
            control_mouse( mouse_identification, mouse_x, mouse_y, mouse_option );
        }
        else if ( mouse_down == 1 )
        {
            if ( localIdentification == NUM_VIEW )
            {
#if (MAP_BITS == 8)
                control_mouse( mouse_identification, mouse_x / 2, mouse_y / 2, mouse_option );
#else
                control_mouse( mouse_identification, mouse_x, mouse_y, mouse_option );
#endif
            }
            else
            {
                control_mouse( mouse_identification, mouse_x, mouse_y, mouse_option );
            }
        }
    }

    if ( ( key_down == 1 ) && ( key_identification == localIdentification ) )
    {
        if ( ( key_identification == NUM_VIEW ) || ( key_identification == NUM_TERRAIN ) )
        {
            control_key( key_identification, key_value );
        }
    }
    if ( localIdentification == WINDOW_PROCESSING )
    {
        sim_realtime( ticks );
        if ( ( io_command_line_execution() != 1 ) && ( !toggle_pause ) )
        {
            sim_cycle();
            sim_update_output();
        }
#ifndef    _WIN32
        if ( sim_new_run_condition() )
        {
            return_value = SHARED_CYCLE_NEW_APES;
        }
#endif
#ifdef SCRIPT_DEBUG
        if ( shared_script_debug_ready() )
        {
            return_value = SHARED_CYCLE_DEBUG_OUTPUT;
        }
#endif
#ifndef	_WIN32
        if ( sim_thread_console_quit() )
        {
            return_value = SHARED_CYCLE_QUIT;
        }
#endif
    }    
    return return_value;
}

n_int shared_init( n_int view, n_uint random )
{
    key_down = 0;
    mouse_down = 0;
    mouse_drag = 0;
    if ( view == WINDOW_PROCESSING )
    {
        if ( control_init( KIND_START_UP, random ) == 0L )
        {
            return SHOW_ERROR( "Initialization failed lack of memory" );
        }
        simulation_started = 1;
    }
    return view;
}

void shared_close( void )
{
    simulation_started = 0;
    ape_buffer_free( &outputBuffer );

    sim_close();
}

n_int shared_simulation_started(void)
{
    return simulation_started;
}

void shared_keyReceived( n_int value, n_int fIdentification )
{
    key_down = 1;
    key_value = value;
    key_identification = fIdentification;
}

void shared_keyUp( void )
{
    key_down = 0;
}

void shared_mouseOption( n_byte option )
{
    mouse_option = option;
}

void shared_mouseReceived( n_double valX, n_double valY, n_int fIdentification )
{
    mouse_down = 1;
    mouse_identification = fIdentification;
    if ( fIdentification == NUM_VIEW )
    {
        n_vect2 *being_location = draw_selected_location();
        mouse_x = ( n_int )( valX + 256 + being_location->x ) & 511;
        mouse_y = ( n_int )( valY + 256 + being_location->y ) & 511;
    }
    else
    {
        mouse_x = ( n_int )valX;
        mouse_y = ( n_int )valY;
    }
}

void shared_mouseReceived_ios( n_double valX, n_double valY )
{
    n_int new_rotation_vertical = ( valX < valY );

    if (new_rotation_vertical) printf("port\n"); else printf("land\n");
    printf("mouse %f x %f\n", valX, valY);
    printf("contr %ld x %ld\n", ios_control_x, ios_control_y);
    printf("view  %ld x %ld\n", ios_view_x, ios_view_y);
    printf("dimen %ld x %ld\n", ios_dimension_x, ios_dimension_y);
    printf("------------\n");

    mouse_x = ( n_int )valX;
    mouse_y = ( n_int )valY;
}


void shared_mouseUp( void )
{
    mouse_down = 0;
    mouse_drag = 0;
}

void shared_about( void )
{
    draw_about();
}

static void shared_clearErrors( void )
{
    ( void )draw_error( 0L, 0L, 0 );
}


static n_int shared_new_type( KIND_OF_USE type, n_uint seed )
{
    static  n_int NewBlock = 0;

    if ( NewBlock )
    {
        return 0;
    }
    NewBlock = 1;
    ( void )control_init( type, seed );
    NewBlock = 0;
    return 0;
}

n_int shared_new( n_uint seed )
{
    return shared_new_type( KIND_NEW_SIMULATION, seed );
}

n_int shared_new_agents( n_uint seed )
{
    return shared_new_type( KIND_NEW_APES, seed );
}

n_byte shared_openFileName( n_constant_string cStringFileName, n_int isScript )
{
    ( void )control_toggle_pause( 0 );

    if ( isScript )
    {
        return ( command_script( 0L, (n_string) cStringFileName, 0L ) == 0 );
    }
    return ( command_open( 0L, (n_string) cStringFileName, 0L ) == 0 );
}

void shared_saveFileName( n_constant_string cStringFileName )
{
    ( void )control_toggle_pause( 0 );
    ( void )command_save( 0L, (n_string) cStringFileName, 0L );
}

void shared_delta( n_double delta_x, n_double delta_y, n_int wwind )
{

}

void shared_zoom( n_double num, n_int wwind )
{

}

void shared_rotate( n_double num, n_int wwind )
{
    if ( wwind == NUM_TERRAIN )
    {
        n_int integer_rotation_256 = ( n_int )( ( num * 256 ) / 360 );
        sim_rotate( integer_rotation_256 );

    }
}

n_uint shared_max_fps( void )
{
    return 60;
}

n_int shared_menu( n_int menuVal )
{
    switch ( menuVal )
    {
    case NA_MENU_PAUSE:
        return control_toggle_pause( 1 );
    case NA_MENU_WEATHER:
        return draw_toggle_weather();
    case NA_MENU_BRAIN:
        return draw_toggle_brain();
    case NA_MENU_BRAINCODE:
        return draw_toggle_braincode();
    case NA_MENU_TERRITORY:
        return draw_toggle_territory();
    case NA_MENU_TIDEDAYLIGHT:
        return draw_toggle_tide_daylight();
    case NA_MENU_PREVIOUS_APE:
        ( void )control_key( 0, 2079 );
        return 0;
    case NA_MENU_NEXT_APE:
        ( void )control_key( 0, 2078 );
        return 0;
    case NA_MENU_CLEAR_ERRORS:
        ( void )draw_error( 0L, 0L, 0 );
        return 0;
    case NA_MENU_FLOOD:
        sim_flood();
        return 0;
    case NA_MENU_HEALTHY_CARRIER:
        sim_healthy_carrier();
        return 0;
    case NA_MENU_FOLLOW:
        return draw_toggle_follow();
    case NA_MENU_SOCIAL_WEB:
        return draw_toggle_social_web();
    }
    return -1;
}

n_byte *shared_legacy_draw( n_byte fIdentification, n_int dim_x, n_int dim_y )
{
    if ( fIdentification == NUM_TERRAIN )
    {
        draw_window( dim_x, dim_y );
    }
    draw_cycle( 0, DRAW_WINDOW_VIEW | DRAW_WINDOW_CONTROL | DRAW_WINDOW_TERRAIN );

    return draw_pointer( fIdentification );
}

void shared_color_8_bit_to_48_bit( n_byte2 *fit )
{
    land_color_time( fit, 1 );
}

n_int shared_being_number( void )
{
    simulated_group *group = sim_group();
    if ( group )
    {
        return group->num;
    }
    return 0;
}

void shared_being_name( n_int number, n_string name )
{
    simulated_group *group = sim_group();
    simulated_being *being = 0L;
    if ( group )
    {
        if (number < group->num)
        {
            being = &(group->beings[number]);
        }
    }
    being_name_simple(being, name);
}

void shared_being_select( n_int number)
{
    simulated_group *group = sim_group();
    if ( group )
    {
        if (number < group->num)
        {
            sim_set_select(&group->beings[number]);
        }
    }
}

enum
{
    IMMERSIVEAPE_MATERIAL_WATER = 0,
    IMMERSIVEAPE_MATERIAL_SHORE = 1,
    IMMERSIVEAPE_MATERIAL_GRASS = 2,
    IMMERSIVEAPE_MATERIAL_BUSH = 3,
    IMMERSIVEAPE_MATERIAL_FOREST = 4,
    IMMERSIVEAPE_MATERIAL_ROCK = 5
};

static float shared_immersiveape_daylight( uint32_t time_minutes )
{
    float day_fraction = (float)( time_minutes % TIME_DAY_MINUTES ) / (float)TIME_DAY_MINUTES;
    float value = ( sinf( ( day_fraction * (float)TWO_PI ) - (float)( TWO_PI / 4.0 ) ) * 0.5f ) + 0.5f;

    if ( value < 0.0f )
    {
        return 0.0f;
    }
    if ( value > 1.0f )
    {
        return 1.0f;
    }
    return value;
}

static int32_t shared_immersiveape_ground_height( int32_t ape_x, int32_t ape_y )
{
    return land_location(
        POSITIVE_LAND_COORD( APESPACE_TO_MAPSPACE( ape_x ) ),
        POSITIVE_LAND_COORD( APESPACE_TO_MAPSPACE( ape_y ) ) );
}

static int32_t shared_immersiveape_wrap_delta( int32_t delta )
{
    int32_t resolution = MAP_APE_RESOLUTION_SIZE;
    int32_t half_resolution = resolution >> 1;

    if ( delta > half_resolution )
    {
        delta -= resolution;
    }
    if ( delta < -half_resolution )
    {
        delta += resolution;
    }
    return delta;
}

static uint8_t shared_immersiveape_territory_familiarity(
    simulated_being *being,
    int32_t ape_x,
    int32_t ape_y )
{
#ifdef TERRITORY_ON
    n_uint territory_index;
    n_uint familiarity;
    n_uint territory_loop;
    n_uint max_familiarity = 1;

    if ( being == 0L )
    {
        return 0;
    }

    territory_index =
        APESPACE_TO_TERRITORY( ape_y ) * TERRITORY_DIMENSION +
        APESPACE_TO_TERRITORY( ape_x );
    familiarity = ( n_uint )being->events.territory[territory_index].familiarity;

    for ( territory_loop = 0; territory_loop < TERRITORY_AREA; territory_loop++ )
    {
        if ( being->events.territory[territory_loop].familiarity > max_familiarity )
        {
            max_familiarity = ( n_uint )being->events.territory[territory_loop].familiarity;
        }
    }

    if ( max_familiarity == 0 )
    {
        return 0;
    }

    return ( uint8_t )( familiarity * 255 / max_familiarity );
#else
    ( void )being;
    ( void )ape_x;
    ( void )ape_y;
    return 0;
#endif
}

static uint8_t shared_immersiveape_episodic_temporal_weight(
    simulated_iepisodic *episode )
{
    const int32_t window_minutes = TIME_DAY_MINUTES * 12;
    int32_t total_minutes;
    int32_t day_delta;
    int32_t minute_delta;

    if ( episode == 0L || episode->event == 0 )
    {
        return 0;
    }

    day_delta = ( int32_t )land_date() - ( int32_t )episode->space_time.date;
    minute_delta = ( int32_t )land_time() - ( int32_t )episode->space_time.time;
    total_minutes = ( day_delta * TIME_DAY_MINUTES ) + minute_delta;

    if ( total_minutes < 0 )
    {
        total_minutes = -total_minutes;
    }

    if ( total_minutes >= window_minutes )
    {
        return 0;
    }

    return ( uint8_t )( ( ( window_minutes - total_minutes ) * 255 ) / window_minutes );
}

static n_int shared_immersiveape_episodic_matches_being(
    simulated_iepisodic *episode,
    simulated_being *being )
{
    if ( episode == 0L || being == 0L || episode->event == 0 )
    {
        return 0;
    }

    return being_name_comparison(
               being,
               episode->first_name[BEING_MEETER],
               episode->family_name[BEING_MEETER] ) ||
           being_name_comparison(
               being,
               episode->first_name[BEING_MET],
               episode->family_name[BEING_MET] );
}

static void shared_immersiveape_fill_being_episodic_snapshot(
    shared_immersiveape_being_snapshot *destination,
    simulated_being *observer,
    simulated_being *being )
{
    simulated_iepisodic *episodic;
    n_int                best_index = -1;
    n_int                best_score = -1;
    n_int                episode_index;

    if ( destination == 0L )
    {
        return;
    }

    destination->episodic_event = 0;
    destination->episodic_recency = 0;
    destination->episodic_firsthand = 0;
    destination->episodic_intention = 0;
    destination->episodic_affect = 0;

    if ( observer == 0L || being == 0L || observer == being )
    {
        return;
    }

    episodic = being_episodic( observer );
    if ( episodic == 0L )
    {
        return;
    }

    for ( episode_index = 0; episode_index < EPISODIC_SIZE; episode_index++ )
    {
        n_int   affect_delta;
        n_int   score;
        uint8_t temporal_weight;
        n_byte  event = episodic[episode_index].event;
        n_int   firsthand;

        if ( event == 0 )
        {
            continue;
        }

        if ( shared_immersiveape_episodic_matches_being( &episodic[episode_index], being ) == 0 )
        {
            continue;
        }

        affect_delta = ( n_int )episodic[episode_index].affect - EPISODIC_AFFECT_ZERO;
        temporal_weight = shared_immersiveape_episodic_temporal_weight( &episodic[episode_index] );
        firsthand = being_name_comparison(
                        observer,
                        episodic[episode_index].first_name[BEING_MEETER],
                        episodic[episode_index].family_name[BEING_MEETER] );

        score = ABS( affect_delta ) + ( ( n_int )temporal_weight * 3 );
        if ( event >= EVENT_INTENTION )
        {
            score += 96;
        }
        if ( firsthand )
        {
            score += 64;
        }

        if ( score > best_score )
        {
            best_score = score;
            best_index = episode_index;
        }
    }

    if ( best_index < 0 )
    {
        return;
    }

    destination->episodic_event =
        ( uint8_t )( episodic[best_index].event & ( EVENT_INTENTION - 1 ) );
    destination->episodic_recency =
        shared_immersiveape_episodic_temporal_weight( &episodic[best_index] );
    destination->episodic_firsthand = ( uint8_t )( being_name_comparison(
                                          observer,
                                          episodic[best_index].first_name[BEING_MEETER],
                                          episodic[best_index].family_name[BEING_MEETER] ) != 0 );
    destination->episodic_intention =
        ( uint8_t )( episodic[best_index].event >= EVENT_INTENTION );
    destination->episodic_affect =
        ( int16_t )( ( n_int )episodic[best_index].affect - EPISODIC_AFFECT_ZERO );
}

static void shared_immersiveape_fill_being_social_snapshot(
    shared_immersiveape_being_snapshot *destination,
    simulated_being *observer,
    simulated_being *being )
{
    simulated_isocial *social_graph;
    n_int              social_index;

    if ( destination == 0L )
    {
        return;
    }

    destination->social_friend_foe = SOCIAL_RESPECT_NORMAL;
    destination->social_attraction = 0;
    destination->social_familiarity = 0;
    destination->social_relationship = 0;

    if ( observer == 0L || being == 0L )
    {
        return;
    }

    if ( observer == being )
    {
        destination->social_relationship = RELATIONSHIP_SELF;
        return;
    }

    social_graph = being_social( observer );
    if ( social_graph == 0L )
    {
        return;
    }

    for ( social_index = 1; social_index < SOCIAL_SIZE_BEINGS; social_index++ )
    {
        if ( SOCIAL_GRAPH_ENTRY_EMPTY( social_graph, social_index ) )
        {
            continue;
        }

        if ( social_graph[social_index].entity_type != ENTITY_BEING )
        {
            continue;
        }

        if ( being_name_comparison(
                 being,
                 social_graph[social_index].first_name[BEING_MET],
                 social_graph[social_index].family_name[BEING_MET] ) )
        {
            destination->social_friend_foe = social_graph[social_index].friend_foe;
            destination->social_attraction = social_graph[social_index].attraction;
            destination->social_familiarity = social_graph[social_index].familiarity;
            destination->social_relationship = social_graph[social_index].relationship;
            return;
        }
    }
}

static void shared_immersiveape_fill_being_snapshot(
    shared_immersiveape_being_snapshot *destination,
    simulated_being *observer,
    simulated_being *being,
    int32_t index,
    int32_t x_adjust,
    int32_t y_adjust )
{
    n_genetics *genetics = being_genetics( being );
    int32_t     local_x = being_location_x( being ) + x_adjust;
    int32_t     local_y = being_location_y( being ) + y_adjust;
    int32_t     ground = shared_immersiveape_ground_height( local_x, local_y );
    int32_t     water_level = land_tide_level();

    if ( destination == 0L || being == 0L || genetics == 0L )
    {
        return;
    }

    destination->index = index;
    destination->x = (float)local_x;
    destination->y = (float)local_y;
    destination->z = (float)WALK_ON_WATER( ground, water_level );
    destination->facing = (float)( being_facing( being ) * TWO_PI / 256.0 );
    destination->energy = (float)being_energy( being );
    destination->age_days = (float)AGE_IN_DAYS( being );
    destination->height = (float)GET_BEING_HEIGHT( being ) / 1000.0f;
    destination->mass = (float)being_mass( being ) / 1000.0f;
    destination->speaking = (uint8_t)( being_speaking( being ) != 0 );
    destination->female = (uint8_t)( being_female( being ) != 0 );
    destination->pregnant = (uint8_t)( being_pregnant( being ) != 0 );
    destination->pigmentation = (uint8_t)GENE_PIGMENTATION( genetics );
    destination->hair = (uint8_t)GENE_HAIR( genetics );
    destination->frame = (uint8_t)GENE_FRAME( genetics );
    destination->eye_color = (uint8_t)GENE_EYE_COLOR( genetics );
    destination->eye_shape = (uint8_t)GENE_EYE_SHAPE( genetics );
    destination->state = (uint16_t)being_state( being );
    destination->carrying_child = (uint8_t)(
                                      ( ( being->changes.inventory[BODY_FRONT] & INVENTORY_CHILD ) != 0 ) ||
                                      ( ( being->changes.inventory[BODY_BACK] & INVENTORY_CHILD ) != 0 ) );
    destination->goal_type = (uint8_t)being->delta.goal[0];
    destination->honor = (uint8_t)being_honor( being );
    shared_immersiveape_fill_being_social_snapshot( destination, observer, being );
    shared_immersiveape_fill_being_episodic_snapshot( destination, observer, being );
    destination->territory_familiarity = shared_immersiveape_territory_familiarity(
        being,
        being_location_x( being ),
        being_location_y( being ) );
    destination->observer_territory_familiarity = shared_immersiveape_territory_familiarity(
        observer != 0L ? observer : being,
        being_location_x( being ),
        being_location_y( being ) );
    destination->drive_hunger = (uint8_t)being_drive( being, DRIVE_HUNGER );
    destination->drive_social = (uint8_t)being_drive( being, DRIVE_SOCIAL );
    destination->drive_fatigue = (uint8_t)being_drive( being, DRIVE_FATIGUE );
    destination->drive_sex = (uint8_t)being_drive( being, DRIVE_SEX );
}

static void shared_immersiveape_insert_nearby(
    shared_immersiveape_being_snapshot *nearby,
    int32_t *distances,
    int32_t *count,
    int32_t max_nearby,
    shared_immersiveape_being_snapshot candidate,
    int32_t distance_squared )
{
    int32_t insert_position;
    int32_t limit;

    if ( nearby == 0L || distances == 0L || count == 0L || max_nearby <= 0 )
    {
        return;
    }

    limit = *count;
    if ( limit > max_nearby )
    {
        limit = max_nearby;
    }

    insert_position = limit;
    while ( insert_position > 0 && distances[insert_position - 1] > distance_squared )
    {
        if ( insert_position < max_nearby )
        {
            nearby[insert_position] = nearby[insert_position - 1];
            distances[insert_position] = distances[insert_position - 1];
        }
        insert_position--;
    }

    if ( insert_position < max_nearby )
    {
        nearby[insert_position] = candidate;
        distances[insert_position] = distance_squared;
        if ( *count < max_nearby )
        {
            ( *count )++;
        }
    }
}

static void shared_immersiveape_insert_food(
    shared_immersiveape_food_snapshot *food,
    float *strengths,
    int32_t *count,
    int32_t max_food,
    shared_immersiveape_food_snapshot candidate )
{
    int32_t insert_position;
    int32_t limit;

    if ( food == 0L || strengths == 0L || count == 0L || max_food <= 0 )
    {
        return;
    }

    limit = *count;
    if ( limit > max_food )
    {
        limit = max_food;
    }

    insert_position = limit;
    while ( insert_position > 0 && strengths[insert_position - 1] < candidate.intensity )
    {
        if ( insert_position < max_food )
        {
            food[insert_position] = food[insert_position - 1];
            strengths[insert_position] = strengths[insert_position - 1];
        }
        insert_position--;
    }

    if ( insert_position < max_food )
    {
        food[insert_position] = candidate;
        strengths[insert_position] = candidate.intensity;
        if ( *count < max_food )
        {
            ( *count )++;
        }
    }
}

static shared_immersiveape_food_snapshot shared_immersiveape_food_candidate(
    int32_t sample_x,
    int32_t sample_y,
    uint32_t seed )
{
    shared_immersiveape_food_snapshot candidate = { 0 };
    int32_t ground = shared_immersiveape_ground_height( sample_x, sample_y );
    int32_t tide = land_tide_level();
    int32_t grass = 0;
    int32_t trees = 0;
    int32_t bush = 0;
    int32_t seaweed = land_operator_interpolated( sample_x, sample_y, ( n_byte * )&operators[VARIABLE_BIOLOGY_SEAWEED - VARIABLE_BIOLOGY_AREA] );
    int32_t rockpool = land_operator_interpolated( sample_x, sample_y, ( n_byte * )&operators[VARIABLE_BIOLOGY_ROCKPOOL - VARIABLE_BIOLOGY_AREA] );
    int32_t beach = land_operator_interpolated( sample_x, sample_y, ( n_byte * )&operators[VARIABLE_BIOLOGY_BEACH - VARIABLE_BIOLOGY_AREA] );
    uint32_t hash_value = math_hash( ( n_byte * )&sample_x, sizeof( sample_x ) );

    hash_value ^= math_hash( ( n_byte * )&sample_y, sizeof( sample_y ) );
    hash_value ^= seed;

    candidate.x = (float)sample_x;
    candidate.y = (float)sample_y;
    candidate.z = (float)WALK_ON_WATER( ground, tide );
    candidate.rain = (uint8_t)( weather_pressure(
                                    POSITIVE_LAND_COORD( APESPACE_TO_MAPSPACE( sample_x ) ),
                                    POSITIVE_LAND_COORD( APESPACE_TO_MAPSPACE( sample_y ) ) ) > WEATHER_RAIN );

    if ( WATER_TEST( ground, tide ) )
    {
        if ( seaweed >= rockpool )
        {
            candidate.food_type = FOOD_SEAWEED;
            candidate.intensity = (float)seaweed;
            candidate.vegetation = IMMERSIVEAPE_MATERIAL_WATER;
        }
        else
        {
            candidate.food_type = FOOD_SHELLFISH;
            candidate.intensity = (float)rockpool;
            candidate.vegetation = IMMERSIVEAPE_MATERIAL_SHORE;
        }

        if ( beach > rockpool && beach > seaweed )
        {
            candidate.food_type = FOOD_SHELLFISH;
            candidate.intensity = (float)beach;
            candidate.vegetation = IMMERSIVEAPE_MATERIAL_SHORE;
        }

        return candidate;
    }

    food_values( sample_x, sample_y, &grass, &trees, &bush );

    candidate.food_type = FOOD_VEGETABLE;
    candidate.intensity = (float)grass;
    candidate.vegetation = IMMERSIVEAPE_MATERIAL_GRASS;

    if ( trees > bush && trees > grass )
    {
        candidate.food_type = ( hash_value & 7 ) == 0 ? FOOD_BIRD_EGGS : FOOD_FRUIT;
        candidate.intensity = (float)trees;
        candidate.vegetation = IMMERSIVEAPE_MATERIAL_FOREST;
    }
    else if ( bush > grass )
    {
        candidate.food_type = ( hash_value & 15 ) == 0 ? FOOD_LIZARD_EGGS : FOOD_FRUIT;
        candidate.intensity = (float)bush;
        candidate.vegetation = IMMERSIVEAPE_MATERIAL_BUSH;
    }

    return candidate;
}

int32_t shared_immersiveape_capture_scene(
    shared_immersiveape_scene_snapshot *scene,
    shared_immersiveape_being_snapshot *nearby,
    int32_t max_nearby,
    shared_immersiveape_food_snapshot *food,
    int32_t max_food )
{
    simulated_group *group = sim_group();
    simulated_being *selected;
    shared_immersiveape_being_snapshot nearby_candidate = { 0 };
    int32_t nearby_distances[64] = { 0 };
    float   food_strengths[64] = { 0 };
    int32_t nearby_count = 0;
    int32_t food_count = 0;
    int32_t selected_index = 0;
    int32_t selected_x;
    int32_t selected_y;
    uint32_t world_seed;
    int32_t sample_loop_x;

    if ( scene == 0L || group == 0L || group->num == 0 )
    {
        return 0;
    }

    if ( max_nearby > 64 )
    {
        max_nearby = 64;
    }
    if ( max_food > 64 )
    {
        max_food = 64;
    }

    selected = group->select;
    if ( selected == 0L )
    {
        selected = &group->beings[0];
    }

    selected_index = (int32_t)( selected - group->beings );
    if ( selected_index < 0 || selected_index >= (int32_t)group->num )
    {
        selected_index = 0;
        selected = &group->beings[0];
    }

    selected_x = being_location_x( selected );
    selected_y = being_location_y( selected );

    world_seed = math_hash( ( n_byte * )land_genetics(), sizeof( n_byte2 ) * 2 );
    world_seed ^= math_hash( ( n_byte * )being_genetics( selected ), sizeof( n_genetics ) * CHROMOSOMES );

    scene->has_selection = 1;
    scene->selected_index = selected_index;
    scene->date = land_date();
    scene->time = land_time();
    scene->world_seed = world_seed;
    scene->daylight = shared_immersiveape_daylight( scene->time );
    scene->sun_angle = (float)( ( scene->time % TIME_DAY_MINUTES ) * TWO_PI / TIME_DAY_MINUTES );
    scene->tide = (float)( land_tide_level() - WATER_MAP ) / (float)( TIDE_AMPLITUDE_LUNAR + TIDE_AMPLITUDE_SOLAR );
    scene->water_level = (float)land_tide_level();
    scene->weather = (uint8_t)weather_seven_values( selected_x, selected_y );
    scene->nearby_count = 0;
    scene->food_count = 0;
    scene->reserved0 = 0;

    shared_immersiveape_fill_being_snapshot( &scene->selected, selected, selected, selected_index, 0, 0 );

    for ( sample_loop_x = 0; sample_loop_x < 64; sample_loop_x++ )
    {
        nearby_distances[sample_loop_x] = BIG_INTEGER;
        food_strengths[sample_loop_x] = -1.0f;
    }

    if ( nearby != 0L && max_nearby > 0 )
    {
        n_uint being_loop;

        for ( being_loop = 0; being_loop < group->num; being_loop++ )
        {
            simulated_being *local_being = &group->beings[being_loop];
            int32_t dx;
            int32_t dy;
            int32_t distance_squared;

            if ( local_being == selected )
            {
                continue;
            }

            dx = shared_immersiveape_wrap_delta( being_location_x( local_being ) - selected_x );
            dy = shared_immersiveape_wrap_delta( being_location_y( local_being ) - selected_y );
            distance_squared = ( dx * dx ) + ( dy * dy );

            shared_immersiveape_fill_being_snapshot(
                &nearby_candidate,
                selected,
                local_being,
                (int32_t)being_loop,
                dx,
                dy );

            shared_immersiveape_insert_nearby(
                nearby,
                nearby_distances,
                &nearby_count,
                max_nearby,
                nearby_candidate,
                distance_squared );
        }
    }

    if ( food != 0L && max_food > 0 )
    {
        int32_t sample_x;
        for ( sample_x = -3; sample_x <= 3; sample_x++ )
        {
            int32_t sample_y;
            for ( sample_y = -3; sample_y <= 3; sample_y++ )
            {
                shared_immersiveape_food_snapshot candidate;
                int32_t absolute_x = selected_x + ( sample_x * 384 );
                int32_t absolute_y = selected_y + ( sample_y * 384 );

                if ( sample_x == 0 && sample_y == 0 )
                {
                    continue;
                }

                candidate = shared_immersiveape_food_candidate( absolute_x, absolute_y, world_seed );
                if ( candidate.intensity > 12.0f )
                {
                    shared_immersiveape_insert_food(
                        food,
                        food_strengths,
                        &food_count,
                        max_food,
                        candidate );
                }
            }
        }
    }

    scene->nearby_count = (uint8_t)nearby_count;
    scene->food_count = (uint8_t)food_count;

    return 1;
}

void shared_immersiveape_fill_terrain_patch(
    int32_t center_x,
    int32_t center_y,
    int32_t half_extent,
    int32_t resolution,
    float *heights,
    uint8_t *materials,
    uint8_t *clouds,
    float *water_heights )
{
    int32_t tide = land_tide_level();
    int32_t row;

    if ( heights == 0L || materials == 0L || clouds == 0L || water_heights == 0L || resolution < 2 )
    {
        return;
    }

    for ( row = 0; row < resolution; row++ )
    {
        int32_t column;
        float fy = (float)row / (float)( resolution - 1 );
        int32_t sample_y = center_y + (int32_t)( ( ( fy * 2.0f ) - 1.0f ) * (float)half_extent );

        for ( column = 0; column < resolution; column++ )
        {
            int32_t index = row * resolution + column;
            float fx = (float)column / (float)( resolution - 1 );
            int32_t sample_x = center_x + (int32_t)( ( ( fx * 2.0f ) - 1.0f ) * (float)half_extent );
            int32_t sample_ground = shared_immersiveape_ground_height( sample_x, sample_y );
            int32_t grass = 0;
            int32_t trees = 0;
            int32_t bush = 0;
            int32_t map_x = POSITIVE_LAND_COORD( APESPACE_TO_MAPSPACE( sample_x ) );
            int32_t map_y = POSITIVE_LAND_COORD( APESPACE_TO_MAPSPACE( sample_y ) );
            int32_t cloud_value = weather_pressure( map_x, map_y ) >> 7;

            heights[index] = (float)sample_ground;
            water_heights[index] = (float)tide;

            if ( cloud_value < 0 )
            {
                cloud_value = 0;
            }
            if ( cloud_value > 255 )
            {
                cloud_value = 255;
            }
            clouds[index] = (uint8_t)cloud_value;

            if ( WATER_TEST( sample_ground, tide ) )
            {
                materials[index] = IMMERSIVEAPE_MATERIAL_WATER;
                continue;
            }

            if ( sample_ground < ( tide + 6 ) )
            {
                materials[index] = IMMERSIVEAPE_MATERIAL_SHORE;
                continue;
            }

            food_values( sample_x, sample_y, &grass, &trees, &bush );

            if ( sample_ground > 190 )
            {
                materials[index] = IMMERSIVEAPE_MATERIAL_ROCK;
            }
            else if ( trees > bush && trees > grass && trees > 22 )
            {
                materials[index] = IMMERSIVEAPE_MATERIAL_FOREST;
            }
            else if ( bush > grass && bush > 18 )
            {
                materials[index] = IMMERSIVEAPE_MATERIAL_BUSH;
            }
            else
            {
                materials[index] = IMMERSIVEAPE_MATERIAL_GRASS;
            }
        }
    }
}

static n_byte4         colorLookUp[256][256];
static n_uint          old_hash = 0;

static void shared_color_update( void )
{
    n_int           loop = 0;
    n_int           loopColors = 0;
    n_byte          fit[256 * 3];
    n_int           cloud = 1;
    n_uint          new_hash;

    land_color_time_8bit( fit, draw_toggle_tide_daylight_value() );

    new_hash = math_hash( fit, 256 * 3 );

    if ( new_hash != old_hash )
    {
        old_hash = new_hash;
        while ( loopColors < 256 )
        {
            n_byte *juxtapose = ( n_byte * )&colorLookUp[0][loopColors];
            n_byte red = fit[loop++];
            n_byte green = fit[loop++];
            n_byte blue = fit[loop++];
            n_byte alpha = 0;


            juxtapose[0] = alpha;
            juxtapose[1] = red;
            juxtapose[2] = green;
            juxtapose[3] = blue;
            
            loopColors++;
        }
        while ( cloud < 256 )
        {
            n_int negCloud = 256 - cloud;
            loopColors = 0;
            while ( loopColors < 256 )
            {
                n_byte *juxtapose  = ( n_byte * )&colorLookUp[cloud][loopColors];
                n_byte *juxtapose0 = ( n_byte * )&colorLookUp[0][loopColors];
                juxtapose[0] = ( n_byte )( cloud + ( ( negCloud * juxtapose0[0] ) >> 8 ) );
                juxtapose[1] = ( n_byte )( cloud + ( ( negCloud * juxtapose0[1] ) >> 8 ) );
                juxtapose[2] = ( n_byte )( cloud + ( ( negCloud * juxtapose0[2] ) >> 8 ) );
                juxtapose[3] = ( n_byte )( cloud + ( ( negCloud * juxtapose0[3] ) >> 8 ) );
                loopColors++;
            }
            cloud++;
        }
    }
}

static n_byte * shared_output_buffer(n_int width, n_int height)
{
    return ape_buffer_require( &outputBuffer, width, height );
}

#ifdef TARGET_OS_IOS

#define Y_POINT(ly, dim_y) (dim_y - ly - 1)

#else

#define Y_POINT(ly, dim_y) (ly)

#endif

static void shared_bitcopy_view(n_byte *outputBuffer, n_int dim_x, n_int dim_y,
                                 n_int offset_x, n_int offset_y, n_int multi_x)
{
    n_byte *index = draw_pointer(NUM_VIEW);
    n_byte *local_weather = draw_weather_grayscale();
    n_vect2 *being_location = draw_selected_location();
    n_byte4 *outputBuffer4 = (n_byte4 *)outputBuffer;

    if (index == 0L || local_weather == 0L || being_location == 0L)
    {
        return;
    }

    n_int base_x = (256 + being_location->x + dim_x) % dim_x;
    n_int base_y = (256 + being_location->y + dim_y) % dim_y;

    for (n_int ly = 0; ly < dim_y; ly++)
    {
        n_int point_y = (Y_POINT(ly, dim_y) + base_y) % dim_y;
        n_int row_offset = point_y * dim_x;
        n_byte *index_row = &index[row_offset];
        n_byte *weather_row = &local_weather[row_offset];
        n_int out_offset = (ly + offset_y) * multi_x + offset_x;

        for (n_int lx = 0; lx < dim_x; lx++)
        {
            n_int point_x = (lx + base_x) % dim_x;
            n_byte value = index_row[point_x];
            n_byte cloud = weather_row[point_x];
            n_byte4 *color_row = (n_byte4 *)colorLookUp[cloud];
            outputBuffer4[out_offset + lx] = color_row[value];
        }
    }
}

static void shared_bitcopy_view_ios(n_byte *outputBuffer, n_int dimension,
                                    n_int offset_x, n_int offset_y, n_int multi_x)
{
    n_byte *index = draw_pointer(NUM_VIEW);
    n_byte *local_weather = draw_weather_grayscale();
    n_vect2 *being_location = draw_selected_location();
    n_byte4 *outputBuffer4 = (n_byte4 *)outputBuffer;

    n_int map_dim_x = MAP_DIMENSION;
    n_int map_dim_y = MAP_DIMENSION;

    if (index == 0L || local_weather == 0L || being_location == 0L)
    {
        return;
    }

    if (dimension > MAP_DIMENSION)
    {
        dimension = MAP_DIMENSION;
    }

    n_int point_start = (MAP_DIMENSION - dimension) / 2;
    if (point_start < 0) point_start = 0;

    n_int base_x = (256 + being_location->x + map_dim_x) % map_dim_x;
    n_int base_y = (256 + being_location->y + map_dim_y) % map_dim_y;

    for (n_int screen_y = 0; screen_y < dimension; screen_y++)
    {
        n_int ly = point_start + screen_y;
        n_int point_y = (Y_POINT(ly, map_dim_y) + base_y) % map_dim_y;
        n_int row_offset = point_y * map_dim_x;
        n_byte *index_row = &index[row_offset];
        n_byte *weather_row = &local_weather[row_offset];
        n_int out_offset = (ly + offset_y) * multi_x + offset_x;

        for (n_int screen_x = 0; screen_x < dimension; screen_x++)
        {
            n_int lx = point_start + screen_x;
            n_int point_x = (lx + base_x) % map_dim_x;
            n_byte value = index_row[point_x];
            n_byte cloud = weather_row[point_x];
            n_byte4 *color_row = (n_byte4 *)colorLookUp[cloud];
            outputBuffer4[out_offset + screen_x] = color_row[value];
        }
    }
}


static void shared_bitcopy(n_byte *outputBuffer, n_int dim_x, n_int dim_y,
                           n_int offset_x, n_int offset_y, n_int multi_x, n_int fIdentification)
{
    n_byte *index = draw_pointer(fIdentification);
    n_byte4 *outputBuffer4 = (n_byte4 *)outputBuffer;

    if (index == 0L)
    {
        return;
    }

    n_byte4 *color_row = (n_byte4 *)colorLookUp[0];

    for (n_int ly = 0; ly < dim_y; ly++)
    {
        n_byte *index_row = &index[Y_POINT(ly, dim_y) * dim_x];
        n_int out_offset = (ly + offset_y) * multi_x + offset_x;

        for (n_int lx = 0; lx < dim_x; lx++)
        {
            n_byte value = index_row[lx];
            outputBuffer4[out_offset + lx] = color_row[value];
        }
    }
}


n_byte * shared_draw( n_int fIdentification, n_int dim_x, n_int dim_y, n_byte size_changed )
{
    n_byte * outputBuffer = shared_output_buffer(dim_x, dim_y);
    
    if ( simulation_started == 0 )
    {
        SHOW_ERROR( "draw - simulation not started" );
        return outputBuffer;
    }
    {
#ifdef	_WIN32
        {
            n_byte *index = draw_pointer( fIdentification );
            memory_copy( index, outputBuffer, dim_x * dim_y );
        }
#else
        
        draw_window( dim_x, dim_y );
        draw_cycle( size_changed, ( n_byte )( 1 << fIdentification ) );
        shared_color_update();

#ifdef ALPHA_WEATHER_DRAW
        if ( fIdentification == NUM_VIEW )
        { // fix here for MacOS 26
            dim_y -= 4;
            
            shared_bitcopy_view( outputBuffer, dim_x, dim_y, 0, 0, dim_x );
        }
        else
#endif
        {
            shared_bitcopy( outputBuffer, dim_x, dim_y, 0, 0, dim_x, fIdentification );
        }
#endif
    }

    return outputBuffer;
}

#ifdef TARGET_OS_IOS

#define IOS_PROCESSOR_SAVER

#ifdef IOS_PROCESSOR_SAVER
static n_int ios_time_cycle = 0;
#endif

shared_cycle_state shared_cycle_ios( n_uint ticks )
{
    shared_cycle_state return_value = SHARED_CYCLE_OK;
    if ( simulation_started == 0 )
    {
        return return_value;
    }

    ticks = ticks & 67108863; /* 71 58 27 88 */
    ticks *= 60;
#ifndef    _WIN32
    sim_thread_console();
#endif

    sim_realtime( ticks );

#ifdef IOS_PROCESSOR_SAVER
    ios_time_cycle++;
    if ( ( ios_time_cycle & 3 ) == 1 )
#endif
    {
        if ( ( io_command_line_execution() != 1 ) && ( !toggle_pause ) )
        {
            sim_cycle();
            sim_update_output();
        }
    }
    if ( sim_new_run_condition() )
    {
        return_value = SHARED_CYCLE_NEW_APES;
    }
#ifdef SCRIPT_DEBUG
    if ( shared_script_debug_ready() )
    {
        return_value = SHARED_CYCLE_DEBUG_OUTPUT;
    }
#endif
#ifndef    _WIN32
    if ( sim_thread_console_quit() )
    {
        return_value = SHARED_CYCLE_QUIT;
    }
#endif
    return return_value;
}

static n_int rotation_vertical = -1;

void shared_draw_ios( n_byte4 *outputBuffer, n_int dim_x, n_int dim_y )
{
    n_int new_rotation_vertical = ( dim_x < dim_y );

    ios_dimension_x = dim_x;
    ios_dimension_y = dim_y;
    
    if ( simulation_started == 0 )
    {
        SHOW_ERROR( "draw - simulation not started" );
        return;
    }

#ifdef IOS_PROCESSOR_SAVER
    if ( ( ( ios_time_cycle & 3 ) != 3 ) && ( rotation_vertical == new_rotation_vertical ) )
    {
        return;
    }
#endif

    if ( rotation_vertical != new_rotation_vertical )
    {
        memory_erase( ( n_byte * )outputBuffer, dim_x * dim_y * 4 );
    }
    rotation_vertical = new_rotation_vertical;
    if ( dim_x < dim_y )
    {
        /* portrait */
        draw_window( dim_x, dim_y - dim_x );
        draw_cycle( 0, DRAW_WINDOW_CONTROL | DRAW_WINDOW_VIEW );
        shared_color_update();

        ios_control_x = dim_x;
        ios_control_y = dim_y - dim_x  - 120;

        ios_view_x = 0;
        ios_view_y = dim_y - dim_x  - 120;
        
        shared_bitcopy( ( n_byte * )outputBuffer, dim_x, dim_y - dim_x - 120, 0, 0, dim_x, NUM_CONTROL );
        shared_bitcopy_view_ios( ( n_byte * )outputBuffer, dim_x, 0, dim_y - dim_x - 120, dim_x );
    }
    else
    {
        /* landscape */
        n_int     point_start = ( MAP_DIMENSION - dim_y ) / 2;
        if ( point_start < 0 )
        {
            point_start = 0;
        }
        draw_window( dim_x - dim_y, dim_y );
        draw_cycle( 0, DRAW_WINDOW_CONTROL | DRAW_WINDOW_VIEW );
        shared_color_update();

        ios_control_x = dim_x - dim_y;
        ios_control_y = dim_y;

        ios_view_x = 0;
        ios_view_y = 0 - point_start;
        
        shared_bitcopy( ( n_byte * )outputBuffer, dim_x - dim_y , dim_y, dim_y, 0, dim_x, NUM_CONTROL );
        shared_bitcopy_view_ios( ( n_byte * )outputBuffer, dim_y, 0, 0 - point_start, dim_x );
    }
}

#endif
