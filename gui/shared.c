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

#include "app_shell.h"
#include "buffer.h"
#include "gui.h"
#ifdef BRONZE_SIM
#include <string.h>
#include "../bronze/hybrid/bronze_hybrid.h"
#include "../bronze/tasks/bronze_tasks.h"
#include "../bronze/work/bronze_work.h"
#include "../bronze/actors/bronze_actors.h"
#endif

#if defined(BRONZE_SIM) && defined(BRONZE_DISK_SAVE)
#include "../bronze/save/bronze_save.h"
#endif

#ifndef    _WIN32

#include "../shared.h"

#else

#include "..\shared.h"

#endif

static n_int   simulation_started = 0;
static n_int   shared_session_initializing = 0;
static n_int   shared_session_started_count = 0;
static n_int   shared_default_uses_core = 0;
static n_int   shared_core_memory_ready = 0;

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

typedef struct
{
    n_file *snapshot;
    n_int simulation_started;
    n_int selected_index;
    n_int toggle_pause;
    draw_toggle_state draw_state;
#ifdef BRONZE_SIM
    BronzeActorRegistry* bronze_registry;
    n_file *native_continuation;
#endif
} shared_core_context;

struct shared_session
{
    ape_app_shell shell;
    n_uint initial_random;
    shared_core_context core;
};

static shared_core_context shared_default_core;
static n_byte shared_default_core_ready = 0;
static shared_core_context *shared_active_core = 0L;

static void shared_core_context_release_snapshot( shared_core_context *context )
{
    if ( context == 0L )
    {
        return;
    }

    io_file_free( &context->snapshot );
#ifdef BRONZE_SIM
    io_file_free( &context->native_continuation );
#endif
}

static void shared_core_context_reset_values( shared_core_context *context )
{
    if ( context == 0L )
    {
        return;
    }

    context->simulation_started = 0;
    context->selected_index = -1;
    context->toggle_pause = 0;
    draw_toggle_state_default( &context->draw_state );
}

static void shared_default_core_ensure( void )
{
    if ( shared_default_core_ready )
    {
        return;
    }

    memory_erase( ( n_byte * )&shared_default_core, sizeof( shared_default_core ) );
    shared_core_context_reset_values( &shared_default_core );
    shared_default_core_ready = 1;
}

static n_int shared_current_selected_index( void )
{
    simulated_group *group = sim_group();

    if ( ( group == 0L ) || ( group->select == 0L ) )
    {
        return -1;
    }

    return being_index( group, group->select );
}

static void shared_select_index( n_int selected_index )
{
    simulated_group *group = sim_group();

    if ( ( group == 0L ) || ( selected_index < 0 ) || ( selected_index >= ( n_int )group->num ) )
    {
        return;
    }

    sim_set_select( &( group->beings[selected_index] ) );
}

static void shared_core_context_sync_from_globals( shared_core_context *context )
{
    if ( context == 0L )
    {
        return;
    }

    context->simulation_started = simulation_started;
    context->toggle_pause = toggle_pause;
    context->selected_index = shared_current_selected_index();
    draw_toggle_state_get( &context->draw_state );
}

static void shared_core_context_apply_globals( shared_core_context *context )
{
    if ( context == 0L )
    {
        return;
    }

    simulation_started = context->simulation_started;
    toggle_pause = context->toggle_pause;
    draw_toggle_state_set( &context->draw_state );
}

static n_int shared_core_context_capture( shared_core_context *context )
{
    n_file *snapshot;

    if ( context == 0L )
    {
        return 1;
    }

    shared_core_context_sync_from_globals( context );
    if ( context->simulation_started == 0 )
    {
        return 1;
    }

    if ( sim_new() )
    {
        sim_cycle();
        sim_update_output();
        context->selected_index = shared_current_selected_index();
    }

    snapshot = tranfer_out();
    if ( snapshot != 0L )
    {
#ifdef BRONZE_SIM
        /* Writers use size as capacity; readers treat it as content length.
           Never parse the unwritten allocation tail when restoring a world. */
        snapshot->size = snapshot->location;
        n_file *continuation = bronze_native_continuation_capture();
        if (!continuation) {
            io_file_free(&snapshot);
            return 0;
        }
#endif
        shared_core_context_release_snapshot( context );
        context->snapshot = snapshot;
#ifdef BRONZE_SIM
        context->native_continuation = continuation;
#endif
    }
    return snapshot != 0L;
}

static n_int shared_core_context_restore( shared_core_context *context )
{
    if ( context == 0L )
    {
        return 0;
    }

    if ( context->simulation_started )
    {
        n_file *snapshot = 0L;

        if ( context->snapshot == 0L )
        {
            return 0;
        }

        snapshot = io_file_duplicate( context->snapshot );
        if ( snapshot == 0L )
        {
            return 0;
        }

        if ( sim_state_restore( snapshot, context->selected_index ) != 0 )
        {
            io_file_free( &snapshot );
            return 0;
        }
        io_file_free( &snapshot );
#ifdef BRONZE_SIM
        if (!bronze_native_continuation_restore(context->native_continuation)) return 0;
        if (!bronze_actors_rebind(context->bronze_registry,sim_group())) return 0;
#endif
        shared_core_context_apply_globals( context );
        shared_select_index( context->selected_index );
        return 1;
    }

    shared_core_context_apply_globals( context );
    return 1;
}

static n_int shared_core_activate( shared_core_context *context )
{
    shared_default_core_ensure();

    if ( context == 0L )
    {
        return 0;
    }

    if ( shared_active_core == context )
    {
        return 1;
    }

    if ( shared_active_core != 0L )
    {
#ifdef BRONZE_SIM
        if (!shared_core_context_capture(shared_active_core)) return 0;
#else
        shared_core_context_capture( shared_active_core );
#endif
    }

#ifdef BRONZE_SIM
    if (!context->bronze_registry) context->bronze_registry=bronze_actors_create();
    if (!context->bronze_registry) return 0;
    bronze_actors_activate(context->bronze_registry);
#endif
    if ( context->simulation_started )
    {
        if ( shared_core_context_restore( context ) == 0 )
        {
#ifdef BRONZE_SIM
            bronze_actors_activate(NULL);
            shared_active_core=NULL; /* Force the next owner to restore its snapshot. */
#endif
            return 0;
        }
    }
    else
    {
        shared_core_context_apply_globals( context );
    }

    shared_active_core = context;
    return 1;
}

static void shared_core_context_became_current( shared_core_context *context )
{
    if ( context == 0L )
    {
        return;
    }

    shared_core_context_release_snapshot( context );
    shared_active_core = context;
    shared_core_context_sync_from_globals( context );
}

static n_int shared_core_started_count( void )
{
    n_int count = shared_session_started_count;

    shared_default_core_ensure();
    if ( shared_default_core.simulation_started )
    {
        count++;
    }

    return count;
}

static void shared_core_close_if_unused( void )
{
    if ( ( shared_core_memory_ready == 0 ) || ( shared_core_started_count() != 0 ) )
    {
        return;
    }

    sim_close();
    shared_core_memory_ready = 0;
    simulation_started = 0;
    shared_active_core = 0L;
}

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

static n_byte control_mouse_down( ape_app_shell_mouse *session_mouse )
{
    if ( session_mouse != 0L )
    {
        return session_mouse->down;
    }
    return mouse_down;
}

static n_byte control_mouse_drag( ape_app_shell_mouse *session_mouse )
{
    if ( session_mouse != 0L )
    {
        return session_mouse->drag;
    }
    return mouse_drag;
}

static void control_mouse_set_drag( ape_app_shell_mouse *session_mouse, n_int px, n_int py )
{
    if ( session_mouse != 0L )
    {
        session_mouse->drag_x = px;
        session_mouse->drag_y = py;
        session_mouse->drag = 1;
        return;
    }

    mouse_drag_x = px;
    mouse_drag_y = py;
    mouse_drag = 1;
}

static void control_mouse( ape_app_shell_mouse *session_mouse, n_byte wwind, n_int px, n_int py, n_byte option )
{
    if ( wwind == NUM_CONTROL )
    {
        n_int *was_down = ( session_mouse != 0L ) ? &session_mouse->control_was_down : &control_mouse_was_down;

        if ( !control_mouse_down( session_mouse ) && *was_down )
        {
            sim_control_regular( px, py );
        }
        *was_down = control_mouse_down( session_mouse );
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
                if ( control_mouse_drag( session_mouse ) == 0 )
                {
                    control_mouse_set_drag( session_mouse, px, py );
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

    ( void )draw_error( 0L, 0L, 0 );
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

static shared_cycle_state shared_cycle_core( n_uint ticks, n_int localIdentification, ape_app_shell *shell )
{
    shared_cycle_state return_value = SHARED_CYCLE_OK;
    ape_app_shell_mouse *session_mouse = ( shell != 0L ) ? &shell->mouse : 0L;
    ape_app_shell_key *session_key = ( shell != 0L ) ? &shell->key : 0L;
    n_byte current_mouse_identification = ( session_mouse != 0L ) ? session_mouse->identification : mouse_identification;
    n_byte current_mouse_down = ( session_mouse != 0L ) ? session_mouse->down : mouse_down;
    n_byte current_mouse_option = ( session_mouse != 0L ) ? session_mouse->option : mouse_option;
    n_int current_mouse_x = ( session_mouse != 0L ) ? session_mouse->x : mouse_x;
    n_int current_mouse_y = ( session_mouse != 0L ) ? session_mouse->y : mouse_y;
    n_byte current_key_down = ( session_key != 0L ) ? session_key->down : key_down;
    n_byte current_key_identification = ( session_key != 0L ) ? session_key->identification : key_identification;
    n_byte2 current_key_value = ( session_key != 0L ) ? session_key->value : key_value;

    if ( simulation_started == 0 )
    {
        return return_value;
    }

    ticks = ticks & 67108863; /* 71 58 27 88 */
    ticks *= 60;
#if !defined(_WIN32) && !defined(WASM_BUILD)
    sim_thread_console();
#endif
    if ( current_mouse_identification == localIdentification )
    {
        if ( localIdentification == NUM_CONTROL )
        {
            control_mouse( session_mouse, current_mouse_identification, current_mouse_x, current_mouse_y, current_mouse_option );
        }
        else if ( current_mouse_down == 1 )
        {
            if ( localIdentification == NUM_VIEW )
            {
#if (MAP_BITS == 8)
                control_mouse( session_mouse, current_mouse_identification, current_mouse_x / 2, current_mouse_y / 2, current_mouse_option );
#else
                control_mouse( session_mouse, current_mouse_identification, current_mouse_x, current_mouse_y, current_mouse_option );
#endif
            }
            else
            {
                control_mouse( session_mouse, current_mouse_identification, current_mouse_x, current_mouse_y, current_mouse_option );
            }
        }
    }

    if ( ( current_key_down == 1 ) && ( current_key_identification == localIdentification ) )
    {
        if ( ( current_key_identification == NUM_VIEW ) || ( current_key_identification == NUM_TERRAIN ) )
        {
            control_key( current_key_identification, current_key_value );
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
#if !defined(_WIN32) && !defined(WASM_BUILD)
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
#if !defined(_WIN32) && !defined(WASM_BUILD)
        if ( sim_thread_console_quit() )
        {
            return_value = SHARED_CYCLE_QUIT;
        }
#endif
    }    
    return return_value;
}

shared_cycle_state shared_cycle( n_uint ticks, n_int localIdentification )
{
    shared_cycle_state state;

    shared_default_core_ensure();
    if ( shared_core_activate( &shared_default_core ) == 0 )
    {
        return SHARED_CYCLE_OK;
    }

    state = shared_cycle_core( ticks, localIdentification, 0L );
    shared_core_context_sync_from_globals( &shared_default_core );
    return state;
}

static n_int shared_init_core( n_int view, n_uint random )
{
    key_down = 0;
    mouse_down = 0;
    mouse_drag = 0;
    if ( view == WINDOW_PROCESSING )
    {
        KIND_OF_USE init_type = shared_core_memory_ready ? KIND_NEW_SIMULATION : KIND_START_UP;

        if ( control_init( init_type, random ) == 0L )
        {
            return SHOW_ERROR( "Initialization failed lack of memory" );
        }
        shared_core_memory_ready = 1;
        simulation_started = 1;
        if ( shared_session_initializing == 0 )
        {
            shared_default_uses_core = 1;
        }
    }
    return view;
}

n_int shared_init( n_int view, n_uint random )
{
    n_int result;

    shared_default_core_ensure();
    if ( shared_core_activate( &shared_default_core ) == 0 )
    {
        return -1;
    }

    result = shared_init_core( view, random );
    if ( result == view )
    {
        shared_default_core.simulation_started = simulation_started;
        shared_default_core.selected_index = shared_current_selected_index();
        shared_core_context_became_current( &shared_default_core );
    }
    return result;
}

void shared_close( void )
{
    shared_default_core_ensure();

    if ( shared_default_core.simulation_started == 0 )
    {
#ifdef BRONZE_SIM
        bronze_actors_destroy(shared_default_core.bronze_registry);
        shared_default_core.bronze_registry=NULL;
        if(shared_active_core==&shared_default_core) shared_active_core=NULL;
#endif
        shared_core_close_if_unused();
        return;
    }

    if ( shared_active_core != &shared_default_core )
    {
        ( void )shared_core_activate( &shared_default_core );
    }

    simulation_started = 0;
    shared_default_uses_core = 0;
    shared_default_core.simulation_started = 0;
    shared_default_core.selected_index = -1;
    shared_core_context_release_snapshot( &shared_default_core );
#ifdef BRONZE_SIM
    bronze_actors_destroy(shared_default_core.bronze_registry);
    shared_default_core.bronze_registry=NULL;
#endif
    ape_buffer_free( &outputBuffer );

    if ( shared_active_core == &shared_default_core )
    {
        shared_active_core = 0L;
    }

    shared_core_close_if_unused();
}

n_int shared_simulation_started(void)
{
    shared_default_core_ensure();
    if ( shared_active_core == &shared_default_core )
    {
        shared_core_context_sync_from_globals( &shared_default_core );
    }
    return shared_default_core.simulation_started;
}

void shared_keyReceived( n_int value, n_int fIdentification )
{
    shared_default_core_ensure();
    ( void )shared_core_activate( &shared_default_core );
    key_down = 1;
    key_value = value;
    key_identification = fIdentification;
}

void shared_keyUp( void )
{
    shared_default_core_ensure();
    ( void )shared_core_activate( &shared_default_core );
    key_down = 0;
}

void shared_mouseOption( n_byte option )
{
    shared_default_core_ensure();
    ( void )shared_core_activate( &shared_default_core );
    mouse_option = option;
}

static void shared_mouse_coordinates( n_double valX, n_double valY, n_int fIdentification, n_int *x, n_int *y )
{
    if ( ( x == 0L ) || ( y == 0L ) )
    {
        return;
    }

    if ( fIdentification == NUM_VIEW )
    {
        n_vect2 *being_location = draw_selected_location();
        if ( being_location != 0L )
        {
            *x = ( n_int )( valX + 256 + being_location->x ) & 511;
            *y = ( n_int )( valY + 256 + being_location->y ) & 511;
            return;
        }
    }

    *x = ( n_int )valX;
    *y = ( n_int )valY;
}

void shared_mouseReceived( n_double valX, n_double valY, n_int fIdentification )
{
    shared_default_core_ensure();
    ( void )shared_core_activate( &shared_default_core );
    mouse_down = 1;
    mouse_identification = fIdentification;
    shared_mouse_coordinates( valX, valY, fIdentification, &mouse_x, &mouse_y );
}

void shared_mouseReceived_ios( n_double valX, n_double valY )
{
    n_int new_rotation_vertical = ( valX < valY );

    shared_default_core_ensure();
    ( void )shared_core_activate( &shared_default_core );

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
    shared_default_core_ensure();
    ( void )shared_core_activate( &shared_default_core );
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
    n_int result;

    shared_default_core_ensure();
    if ( shared_core_activate( &shared_default_core ) == 0 )
    {
        return -1;
    }

    result = shared_new_type( KIND_NEW_SIMULATION, seed );
    shared_core_context_became_current( &shared_default_core );
    return result;
}

n_int shared_new_agents( n_uint seed )
{
    n_int result;

    shared_default_core_ensure();
    if ( shared_core_activate( &shared_default_core ) == 0 )
    {
        return -1;
    }

    result = shared_new_type( KIND_NEW_APES, seed );
    shared_core_context_became_current( &shared_default_core );
    return result;
}

static n_byte shared_openFileName_core( n_constant_string cStringFileName, n_int isScript )
{
    ( void )control_toggle_pause( 0 );

    if ( isScript )
    {
        return ( command_script( 0L, (n_string) cStringFileName, 0L ) == 0 );
    }
    return ( command_open( 0L, (n_string) cStringFileName, 0L ) == 0 );
}

n_byte shared_openFileName( n_constant_string cStringFileName, n_int isScript )
{
    n_byte result;

    shared_default_core_ensure();
    if ( shared_core_activate( &shared_default_core ) == 0 )
    {
        return 0;
    }

    result = shared_openFileName_core( cStringFileName, isScript );
    shared_core_context_became_current( &shared_default_core );
    return result;
}

static void shared_saveFileName_core( n_constant_string cStringFileName )
{
    ( void )control_toggle_pause( 0 );
    ( void )command_save( 0L, (n_string) cStringFileName, 0L );
}

void shared_saveFileName( n_constant_string cStringFileName )
{
    shared_default_core_ensure();
    if ( shared_core_activate( &shared_default_core ) == 0 )
    {
        return;
    }

    shared_saveFileName_core( cStringFileName );
    shared_core_context_became_current( &shared_default_core );
}

void shared_delta( n_double delta_x, n_double delta_y, n_int wwind )
{

}

void shared_zoom( n_double num, n_int wwind )
{

}

void shared_rotate( n_double num, n_int wwind )
{
    shared_default_core_ensure();
    ( void )shared_core_activate( &shared_default_core );

    if ( wwind == NUM_TERRAIN )
    {
        n_int integer_rotation_256 = ( n_int )( ( num * 256 ) / 360 );
        sim_rotate( integer_rotation_256 );

    }

    shared_core_context_sync_from_globals( &shared_default_core );
}

n_uint shared_max_fps( void )
{
    return 60;
}

static n_int shared_menu_core( n_int menuVal )
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

n_int shared_menu( n_int menuVal )
{
    n_int result;

    shared_default_core_ensure();
    if ( shared_core_activate( &shared_default_core ) == 0 )
    {
        return -1;
    }

    result = shared_menu_core( menuVal );
    shared_core_context_sync_from_globals( &shared_default_core );
    return result;
}

n_byte *shared_legacy_draw( n_byte fIdentification, n_int dim_x, n_int dim_y )
{
    shared_default_core_ensure();
    ( void )shared_core_activate( &shared_default_core );

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
    shared_default_core_ensure();
    ( void )shared_core_activate( &shared_default_core );
    group = sim_group();
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
    shared_default_core_ensure();
    ( void )shared_core_activate( &shared_default_core );
    group = sim_group();
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
    shared_default_core_ensure();
    ( void )shared_core_activate( &shared_default_core );
    group = sim_group();
    if ( group )
    {
        if (number < group->num)
        {
            sim_set_select(&group->beings[number]);
        }
    }
    shared_core_context_sync_from_globals( &shared_default_core );
}

n_int shared_selected_location( n_int *x, n_int *y )
{
    n_vect2 *being_location = draw_selected_location();
    shared_default_core_ensure();
    ( void )shared_core_activate( &shared_default_core );
    being_location = draw_selected_location();
    if ( ( being_location == 0L ) || ( x == 0L ) || ( y == 0L ) )
    {
        return 0;
    }

    *x = being_location->x;
    *y = being_location->y;
    return 1;
}

n_int shared_selected_being_location( n_int *x, n_int *y )
{
    simulated_group *group = sim_group();
    shared_default_core_ensure();
    ( void )shared_core_activate( &shared_default_core );
    group = sim_group();
    if ( ( group == 0L ) || ( group->select == 0L ) || ( x == 0L ) || ( y == 0L ) )
    {
        return 0;
    }

    *x = being_location_x( group->select );
    *y = being_location_y( group->select );
    return 1;
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

#define Y_POINT(ly, dim_y) (ly)

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

static void shared_draw_into_buffer( n_byte *drawBuffer, n_int fIdentification, n_int dim_x, n_int dim_y, n_byte size_changed )
{
    if ( drawBuffer == 0L )
    {
        return;
    }

#ifdef	_WIN32
    {
        n_byte *index = draw_pointer( fIdentification );
        memory_copy( index, drawBuffer, dim_x * dim_y );
    }
#else

    draw_window( dim_x, dim_y );
    draw_cycle( size_changed, ( n_byte )( 1 << fIdentification ) );
    shared_color_update();

#ifdef ALPHA_WEATHER_DRAW
    if ( fIdentification == NUM_VIEW )
    {
        shared_bitcopy_view( drawBuffer, dim_x, dim_y, 0, 0, dim_x );
    }
    else
#endif
    {
        shared_bitcopy( drawBuffer, dim_x, dim_y, 0, 0, dim_x, fIdentification );
    }
#endif
#ifdef BRONZE_SIM
    if (fIdentification == NUM_VIEW)
        bronze_hybrid_draw_hud((n_byte4 *)drawBuffer, dim_x, dim_y);
#endif
}

n_byte * shared_draw( n_int fIdentification, n_int dim_x, n_int dim_y, n_byte size_changed )
{
    n_byte *drawBuffer = shared_output_buffer( dim_x, dim_y );

    shared_default_core_ensure();
    if ( shared_core_activate( &shared_default_core ) == 0 )
    {
        return drawBuffer;
    }

    if ( simulation_started == 0 )
    {
        SHOW_ERROR( "draw - simulation not started" );
        return drawBuffer;
    }

    shared_draw_into_buffer( drawBuffer, fIdentification, dim_x, dim_y, size_changed );

    shared_core_context_sync_from_globals( &shared_default_core );
    return drawBuffer;
}

void shared_copy_rotate_180( n_byte4 *outputBuffer, const n_byte4 *source, n_int dim_x, n_int dim_y )
{
    if ( ( outputBuffer == 0L ) || ( source == 0L ) )
    {
        return;
    }

    for ( n_int ly = 0; ly < dim_y; ly++ )
    {
        n_int row_offset = ly * dim_x;
        n_int source_row_offset = ( dim_y - 1 - ly ) * dim_x;

        for ( n_int lx = 0; lx < dim_x; lx++ )
        {
            outputBuffer[row_offset + lx] = source[source_row_offset + ( dim_x - 1 - lx )];
        }
    }
}

static n_int shared_session_init_callback( void *context, n_int view, n_uint random )
{
    ( void )context;
    return ( shared_init_core( view, random ) == view );
}

static shared_cycle_state shared_session_cycle_callback( void *context, n_uint ticks, n_int identification )
{
    shared_session *session = ( shared_session * )context;
    return shared_cycle_core( ticks, identification, ( session != 0L ) ? &session->shell : 0L );
}

static void shared_session_draw_callback( void *context, n_byte *drawBuffer, n_int identification, n_int dim_x, n_int dim_y, n_byte size_changed )
{
    ( void )context;
    shared_draw_into_buffer( drawBuffer, identification, dim_x, dim_y, size_changed );
}

shared_session *shared_session_create( n_uint random )
{
    shared_session *session = ( shared_session * )memory_new( sizeof( shared_session ) );

    if ( session != 0L )
    {
        memory_erase( ( n_byte * )session, sizeof( shared_session ) );
        session->initial_random = random;
        ape_app_shell_reset( &session->shell );
        shared_core_context_reset_values( &session->core );
    }

    return session;
}

void shared_session_destroy( shared_session *session )
{
    if ( session == 0L )
    {
        return;
    }

    if ( ape_app_shell_simulation_started( &session->shell ) )
    {
        if ( shared_session_started_count > 0 )
        {
            shared_session_started_count--;
        }
    }

    if ( shared_active_core == &session->core )
    {
        shared_active_core = 0L;
    }

    ape_app_shell_shutdown( &session->shell, 0L, 0L );
    session->core.simulation_started = 0;
    shared_core_context_release_snapshot( &session->core );
#ifdef BRONZE_SIM
    bronze_actors_destroy(session->core.bronze_registry);
    session->core.bronze_registry=NULL;
#endif
    shared_core_close_if_unused();

    memory_free( ( void ** )&session );
}

n_int shared_session_init( shared_session *session, n_int view, n_uint random )
{
    n_int was_started;

    if ( session == 0L )
    {
        return -1;
    }

    was_started = ape_app_shell_simulation_started( &session->shell );
    session->initial_random = random;

    if ( shared_core_activate( &session->core ) == 0 )
    {
        return -1;
    }

    shared_session_initializing++;
    ( void )ape_app_shell_init( &session->shell, view, random, shared_session_init_callback, session );
    shared_session_initializing--;

    if ( ape_app_shell_simulation_started( &session->shell ) )
    {
        if ( was_started == 0 )
        {
            shared_session_started_count++;
        }
        session->core.simulation_started = simulation_started;
        session->core.selected_index = shared_current_selected_index();
        shared_core_context_became_current( &session->core );
        return view;
    }

    if ( was_started && ( shared_session_started_count > 0 ) )
    {
        shared_session_started_count--;
    }
    session->core.simulation_started = 0;

    return -1;
}

n_int shared_session_simulation_started( const shared_session *session )
{
    if ( session == 0L )
    {
        return 0;
    }

    return ape_app_shell_simulation_started( &session->shell );
}

shared_cycle_state shared_session_cycle( shared_session *session, n_uint ticks, n_int localIdentification )
{
    shared_cycle_state state;

    if ( session == 0L )
    {
        return SHARED_CYCLE_OK;
    }

    if ( shared_core_activate( &session->core ) == 0 )
    {
        return SHARED_CYCLE_OK;
    }

    state = ape_app_shell_cycle( &session->shell, ticks, localIdentification, shared_session_cycle_callback, session );
    shared_core_context_sync_from_globals( &session->core );
    return state;
}

shared_cycle_state shared_session_cycle_ios( shared_session *session, n_uint ticks )
{
    return shared_session_cycle( session, ticks, WINDOW_PROCESSING );
}

n_byte *shared_session_draw( shared_session *session, n_int fIdentification, n_int dim_x, n_int dim_y, n_byte size_changed )
{
    n_byte *buffer;

    if ( session == 0L )
    {
        return 0L;
    }

    if ( shared_core_activate( &session->core ) == 0 )
    {
        return 0L;
    }

    buffer = ape_app_shell_draw( &session->shell, fIdentification, dim_x, dim_y, size_changed, shared_session_draw_callback, session );
    shared_core_context_sync_from_globals( &session->core );
    return buffer;
}

void shared_session_draw_ios( shared_session *session, n_byte4 *outputBuffer, n_int dim_x, n_int dim_y )
{
    n_byte *buffer;

    if ( outputBuffer == 0L )
    {
        return;
    }

    buffer = shared_session_draw( session, NUM_TERRAIN, dim_x, dim_y, 0 );
    if ( buffer == 0L )
    {
        return;
    }

    shared_copy_rotate_180( outputBuffer, ( const n_byte4 * )buffer, dim_x, dim_y );
}

static n_byte shared_session_is_started( shared_session *session )
{
    return ( n_byte )( ( session != 0L ) && ape_app_shell_simulation_started( &session->shell ) );
}

n_int shared_session_menu( shared_session *session, n_int menuValue )
{
    n_int result;

    if ( shared_session_is_started( session ) == 0 )
    {
        return -1;
    }

    if ( shared_core_activate( &session->core ) == 0 )
    {
        return -1;
    }

    result = shared_menu_core( menuValue );
    shared_core_context_sync_from_globals( &session->core );
    return result;
}

n_int shared_session_new( shared_session *session, n_uint seed )
{
    n_int result;

    if ( shared_session_is_started( session ) == 0 )
    {
        return -1;
    }

    if ( shared_core_activate( &session->core ) == 0 )
    {
        return -1;
    }

    result = shared_new_type( KIND_NEW_SIMULATION, seed );
    shared_core_context_became_current( &session->core );
    return result;
}

n_int shared_session_pause_state( shared_session *session )
{
    if ( session == 0L )
    {
        return 0;
    }

    return session->core.toggle_pause;
}

n_int shared_session_selected_being_index( shared_session *session )
{
    if ( session == 0L )
    {
        return -1;
    }

    return session->core.selected_index;
}

n_int shared_session_selected_being_facing( shared_session *session )
{
    simulated_group *group;
    n_int facing;

    if ( shared_session_is_started( session ) == 0 )
    {
        return -1;
    }

    if ( shared_core_activate( &session->core ) == 0 )
    {
        return -1;
    }

    group = sim_group();
    if ( ( group == 0L ) || ( group->select == 0L ) )
    {
        return -1;
    }

    facing = ( n_int )being_facing( group->select );
    shared_core_context_sync_from_globals( &session->core );
    return facing;
}

n_int shared_session_being_number( shared_session *session )
{
    simulated_group *group;

    if ( shared_session_is_started( session ) == 0 )
    {
        return 0;
    }

    if ( shared_core_activate( &session->core ) == 0 )
    {
        return 0;
    }

    group = sim_group();
    shared_core_context_sync_from_globals( &session->core );
    if ( group == 0L )
    {
        return 0;
    }
    return group->num;
}

n_int shared_session_selected_being_location( shared_session *session, n_int *x, n_int *y )
{
    simulated_group *group;

    if ( ( shared_session_is_started( session ) == 0 ) || ( x == 0L ) || ( y == 0L ) )
    {
        return 0;
    }

    if ( shared_core_activate( &session->core ) == 0 )
    {
        return 0;
    }

    group = sim_group();
    if ( ( group == 0L ) || ( group->select == 0L ) )
    {
        return 0;
    }

    *x = being_location_x( group->select );
    *y = being_location_y( group->select );
    shared_core_context_sync_from_globals( &session->core );
    return 1;
}

void shared_session_rotate( shared_session *session, n_double num, n_int wwind )
{
    if ( shared_session_is_started( session ) == 0 )
    {
        return;
    }

    if ( shared_core_activate( &session->core ) == 0 )
    {
        return;
    }

    shared_rotate( num, wwind );
    shared_core_context_sync_from_globals( &session->core );
}

n_int shared_session_input_is_active( shared_session *session )
{
    if ( session == 0L )
    {
        return 0;
    }

    return ( session->shell.key.down != 0 ) ||
           ( session->shell.mouse.down != 0 ) ||
           ( session->shell.mouse.drag != 0 );
}

void shared_session_mouseOption( shared_session *session, n_byte option )
{
    if ( session == 0L )
    {
        return;
    }

    ape_app_shell_mouse_option( &session->shell, option );
}

void shared_session_mouseReceived( shared_session *session, n_double valX, n_double valY, n_int localIdentification )
{
    n_int session_x = 0;
    n_int session_y = 0;

    if ( session == 0L )
    {
        return;
    }

    if ( shared_core_activate( &session->core ) == 0 )
    {
        return;
    }

    shared_mouse_coordinates( valX, valY, localIdentification, &session_x, &session_y );
    ape_app_shell_mouse_received( &session->shell, session_x, session_y, localIdentification );
    shared_core_context_sync_from_globals( &session->core );
}

void shared_session_mouseUp( shared_session *session )
{
    if ( session == 0L )
    {
        return;
    }

    ape_app_shell_mouse_up( &session->shell );
}

n_byte shared_session_openFileName( shared_session *session, n_constant_string cStringFileName, n_int isScript )
{
    n_byte result;

    if ( shared_session_is_started( session ) == 0 )
    {
        return 0;
    }

    if ( shared_core_activate( &session->core ) == 0 )
    {
        return 0;
    }

    result = shared_openFileName_core( cStringFileName, isScript );
    shared_core_context_became_current( &session->core );
    return result;
}

void shared_session_saveFileName( shared_session *session, n_constant_string cStringFileName )
{
    if ( shared_session_is_started( session ) == 0 )
    {
        return;
    }

    if ( shared_core_activate( &session->core ) == 0 )
    {
        return;
    }

    shared_saveFileName_core( cStringFileName );
    shared_core_context_sync_from_globals( &session->core );
}

#if defined(BRONZE_SIM) && defined(BRONZE_DISK_SAVE)
enum { BRONZE_NATIVE_PREFIX = 8, BRONZE_PACKAGE_LIMIT = 64 * 1024 * 1024 };

static uint32_t bronze_package_read_word(const n_byte *bytes)
{
    uint32_t value = 0;
    for (unsigned i = 0; i < 4; ++i) value |= (uint32_t)bytes[i] << (8 * i);
    return value;
}

static void bronze_package_write_word(n_byte *bytes, uint32_t value)
{
    for (unsigned i = 0; i < 4; ++i) bytes[i] = (n_byte)(value >> (8 * i));
}

n_file *shared_session_bronze_save(shared_session *session, const n_byte *digest)
{
    if (!digest || !shared_session_is_started(session) ||
        !shared_core_activate(&session->core) || sim_new()) return NULL;
    BronzeActorRegistry *registry = session->core.bronze_registry;
    n_file *sidecar = bronze_actors_capture_sidecar(registry, digest);
    if (!sidecar) return NULL;
    n_file *native = tranfer_out();
    n_file *continuation = bronze_native_continuation_capture();
    n_file *package = NULL;
    n_byte *combined = NULL;
    if (native && continuation) {
        n_uint length = BRONZE_NATIVE_PREFIX + native->location + continuation->location;
        n_uint total = BRONZE_SAVE_HEADER + length + sidecar->location;
        if (total <= BRONZE_PACKAGE_LIMIT) {
            combined = memory_new(length);
            package = memory_new(sizeof(*package));
            if (package) {
                package->data = combined ? memory_new(total) : NULL;
                package->size = package->location = total;
                if (package->data) {
                    bronze_package_write_word(combined, (uint32_t)native->location);
                    n_int selected = shared_current_selected_index();
                    bronze_package_write_word(combined + 4, selected < 0 ? UINT32_MAX : (uint32_t)selected);
                    memcpy(combined + BRONZE_NATIVE_PREFIX, native->data, native->location);
                    memcpy(combined + BRONZE_NATIVE_PREFIX + native->location,
                           continuation->data, continuation->location);
                }
                if (!package->data ||
                    !bronze_save_write(package->data, total, (BronzeSaveSection){combined, length},
                                       (BronzeSaveSection){sidecar->data, sidecar->location}))
                    io_file_free(&package);
            }
        }
    }
    memory_free((void **)&combined);
    io_file_free(&native);
    io_file_free(&continuation);
    io_file_free(&sidecar);
    return package;
}

int shared_session_bronze_open(shared_session *session, const n_byte *bytes, n_uint length,
                               const n_byte *digest)
{
    BronzeSaveSection native, sidecar;
    if (!digest || length > BRONZE_PACKAGE_LIMIT || !shared_session_is_started(session) ||
        !bronze_save_read((BronzeSaveSection){bytes, length}, &native, &sidecar) ||
        native.length <= BRONZE_NATIVE_PREFIX || !shared_core_activate(&session->core) || sim_new())
        return 0;
    uint32_t transfer_length = bronze_package_read_word(native.data);
    uint32_t selected = bronze_package_read_word(native.data + 4);
    if (!transfer_length || transfer_length >= native.length - BRONZE_NATIVE_PREFIX) return 0;

    n_file sidecar_file = {sidecar.length, sidecar.length, (n_byte *)sidecar.data};
    BronzeActorRegistry *staged = bronze_actors_stage_sidecar(session->core.bronze_registry,
                                                             &sidecar_file, digest);
    if (!staged) return 0;
    n_uint count = bronze_actors_count(staged);
    n_uint extra = native.length - BRONZE_NATIVE_PREFIX - transfer_length;
    n_file continuation = {extra, extra, (n_byte *)native.data + BRONZE_NATIVE_PREFIX + transfer_length};
    n_file transfer = {transfer_length, transfer_length, memory_new(transfer_length)};
    BronzeNativeStage *native_stage = NULL;
    if (transfer.data && (selected == UINT32_MAX || selected < count)) {
        memcpy(transfer.data, native.data + BRONZE_NATIVE_PREFIX, transfer_length);
        native_stage = bronze_native_stage(&transfer, &continuation, count, bronze_actors_clock(staged)->tick);
    }
    memory_free((void **)&transfer.data);
    if (!native_stage) {
        bronze_actors_destroy(staged);
        return 0;
    }

    BronzeActorRegistry *previous = session->core.bronze_registry;
    bronze_native_install(native_stage, selected == UINT32_MAX ? -1 : (n_int)selected);
    bronze_native_stage_free(&native_stage);
    bronze_actors_activate(staged);
    bronze_actors_rebind(staged, sim_group()); /* Staging established count and slot validity. */
    session->core.bronze_registry = staged;
    bronze_actors_destroy(previous);
    control_toggle_pause(0);
    shared_core_context_became_current(&session->core);
    return 1;
}
#endif

void shared_draw_ios( n_byte4 *outputBuffer, n_int dim_x, n_int dim_y )
{
    n_byte *buffer;

    if ( outputBuffer == 0L )
    {
        return;
    }

    buffer = shared_draw( NUM_TERRAIN, dim_x, dim_y, 0 );
    if ( buffer == 0L )
    {
        return;
    }

    shared_copy_rotate_180( outputBuffer, ( const n_byte4 * )buffer, dim_x, dim_y );
}

shared_cycle_state shared_cycle_ios( n_uint ticks )
{
    return shared_cycle( ticks, WINDOW_PROCESSING );
}

#ifdef BRONZE_SIM
void shared_session_bronze_land_policy(shared_session* session, BronzeLandPolicy policy)
{
    if (!session) return;
    if (!session->core.bronze_registry)
        session->core.bronze_registry = bronze_actors_create();
    BronzeLandscape *landscape = bronze_actors_landscape(session->core.bronze_registry);
    if (landscape) landscape->policy = policy;
}

void shared_session_bronze_professions(shared_session* session, const BronzeProfessionPolicy* policy)
{
    if (!session) return;
    if (!session->core.bronze_registry)
        session->core.bronze_registry = bronze_actors_create();
    bronze_actors_stage_professions(session->core.bronze_registry, policy);
}

void shared_session_bronze_program(shared_session* session, BronzeProgram* program)
{
    if (!session) return;
    if (!session->core.bronze_registry)
        session->core.bronze_registry = bronze_actors_create();
    bronze_actors_stage_program(session->core.bronze_registry, program);
}

static void bronze_summary_stock(char *buffer, size_t size, const char *label,
                                 const BronzeStock *stock, size_t count, const KindTable *kinds)
{
    size_t used = strlen(buffer);
    if (label) snprintf(buffer + used, size - used, "\n%s", label);
    if (!count) {
        used = strlen(buffer);
        snprintf(buffer + used, size - used, "%s", label ? " empty" : "Empty");
    }
    for (size_t i = 0; i < count; ++i) {
        const char *const *names = kinds ? kinds->names.data : NULL;
        const char *name = names && stock[i].kind < kinds->names.len ? names[stock[i].kind] : NULL;
        const char *separator = label || i ? " · " : "";
        used = strlen(buffer);
        if (name)
            snprintf(buffer + used, size - used, "%s%s %.2f", separator, name, stock[i].amount);
        else
            snprintf(buffer + used, size - used, "%skind %u %.2f", separator, stock[i].kind, stock[i].amount);
    }
}

void shared_session_bronze_surface_key(shared_session* session, n_int surface, n_int key)
{
    if ((surface != NUM_VIEW && surface != NUM_TERRAIN) || key < 28 || key > 31 ||
        !shared_session_is_started(session) || !shared_core_activate(&session->core)) return;
    control_key((n_byte)surface, (n_byte2)key);
    shared_core_context_sync_from_globals(&session->core);
}

#include "../bronze/ui/bronze_workspace_render.h"

BronzeWorkspacePoint shared_session_bronze_workspace_center(shared_session* session, n_int selected)
{
    BronzeWorkspacePoint point = {MAP_DIMENSION / 2, MAP_DIMENSION / 2};
    if (!shared_session_is_started(session) || !shared_core_activate(&session->core)) return point;
    const BronzeLandscape *land = bronze_actors_landscape(session->core.bronze_registry);
    if (!selected && land) {
        for (n_uint i = 0; i < land->site_count; ++i) {
            if (land->sites[i].kind == BRONZE_TOWN)
                return (BronzeWorkspacePoint){land->sites[i].x, land->sites[i].y};
        }
    }
    simulated_being *actor = sim_group()->select;
    if (actor) {
        point.x = APESPACE_TO_MAPSPACE(being_location_x(actor));
        point.y = APESPACE_TO_MAPSPACE(being_location_y(actor));
    }
    return point;
}

n_int shared_bronze_workspace_map_dimension(void)
{
    return MAP_DIMENSION;
}

n_uint shared_session_bronze_workspace_day(shared_session* session)
{
    if (!shared_session_is_started(session) || !shared_core_activate(&session->core)) return 0;
    return land_date();
}

n_int shared_session_bronze_workspace_step(shared_session* session)
{
    if (!shared_session_is_started(session) || !shared_core_activate(&session->core) ||
        !toggle_pause || io_command_line_execution() == 1) return 0;
    sim_cycle();
    sim_update_output();
    shared_core_context_sync_from_globals(&session->core);
    return 1;
}

void shared_session_bronze_workspace_draw(shared_session* session, n_byte4* pixels,
                                          n_int width, n_int height, n_int surface,
                                          BronzeWorkspacePoint center, n_double span, n_double bearing, uint64_t selected_site)
{
    if (!pixels || width <= 0 || width > 768 || height <= 0 || height > 768 ||
        (surface != NUM_VIEW && surface != NUM_TERRAIN) ||
        !shared_session_is_started(session) || !shared_core_activate(&session->core)) return;

    BronzeRenderLand land = {0};
    land.tide = (int)land_tide_level();
    land.heights = land_location_tile(0);
    if (!land.heights) return;
    const BronzeLandscape *landscape = bronze_actors_landscape(session->core.bronze_registry);
    if (landscape) {
        for (n_uint i = 0; i < landscape->site_count && i < BRONZE_SITE_LIMIT; ++i) {
            const bronze_site *site = &landscape->sites[i];
            int radius = site->kind == BRONZE_TOWN ? 5 : 3;
            land.sites[land.count++] = (BronzeRenderSite){(int)site->x, (int)site->y, radius, site->kind, site->place_id};
        }
    }
    if (surface == NUM_VIEW) {
        bronze_render_map(pixels, (int)width, (int)height, &land, sim_group(),
                          bronze_render_wrap((int)center.x), bronze_render_wrap((int)center.y),
                          isfinite(span) ? fmin(MAP_DIMENSION, fmax(12, span)) : 64, selected_site);
    } else {
        BronzeWorkspacePoint target = {MAP_DIMENSION / 2, MAP_DIMENSION / 2};
        simulated_being *actor = sim_group()->select;
        if (actor) {
            target.x = APESPACE_TO_MAPSPACE(being_location_x(actor));
            target.y = APESPACE_TO_MAPSPACE(being_location_y(actor));
        }
        for (unsigned i = 0; selected_site && i < land.count; ++i) {
            if (land.sites[i].id == selected_site) {
                target = (BronzeWorkspacePoint){land.sites[i].x, land.sites[i].y};
                break;
            }
        }
        BronzeRenderCamera camera = bronze_render_camera(target.x, target.y,
                                                         isfinite(bearing) ? bearing : 0, &land);
        bronze_render_terrain(pixels, (int)width, (int)height, &land, sim_group(), camera);
    }
}

BronzeWorkspaceSite shared_session_bronze_workspace_select(shared_session* session, BronzeWorkspacePoint center,
                                            n_double dx, n_double dy, n_double radius)
{
    if (!isfinite(dx) || !isfinite(dy) || !isfinite(radius) ||
        fabs(dx) > 768 || fabs(dy) > 768 || radius <= 0 || radius > 32 ||
        !shared_session_is_started(session) || !shared_core_activate(&session->core)) return (BronzeWorkspaceSite){0};
    simulated_group *group = sim_group();
    simulated_being *nearest = NULL;
    double best = radius * radius;
    for (n_uint i = 0; i < group->num; ++i) {
        simulated_being *actor = &group->beings[i];
        int x = (int)APESPACE_TO_MAPSPACE(being_location_x(actor));
        int y = (int)APESPACE_TO_MAPSPACE(being_location_y(actor));
        double ax = bronze_render_delta(x, (int)center.x) - dx;
        double ay = bronze_render_delta(y, (int)center.y) - dy;
        double distance = ax * ax + ay * ay;
        if (distance < best) {
            best = distance;
            nearest = actor;
        }
    }
    BronzeWorkspaceSite picked = {0};
    const BronzeLandscape *land = bronze_actors_landscape(session->core.bronze_registry);
    if (land) {
        for (n_uint i = 0; i < land->site_count; ++i) {
            const bronze_site *site = &land->sites[i];
            double ax = bronze_render_delta((int)site->x, (int)center.x) - dx;
            double ay = bronze_render_delta((int)site->y, (int)center.y) - dy;
            double distance = ax * ax + ay * ay;
            if (distance < best) {
                best = distance;
                picked = (BronzeWorkspaceSite){site->place_id, site->kind};
            }
        }
    }
    if (!picked.id && nearest) {
        sim_set_select(nearest);
        shared_core_context_sync_from_globals(&session->core);
    }
    return picked;
}

static const char *bronze_inspector_task_status(const BronzeActor *actor)
{
    const BronzeTaskState *task = &actor->execution;
    if (task->operation && actor->remaining_ticks && actor->work.reason != BRONZE_WORK_READY)
        return bronze_work_reason_name(actor->work.reason);
    return bronze_task_status_name(task->status);
}

BronzeInspectorActor shared_session_bronze_inspector_actor(shared_session* session)
{
    BronzeInspectorActor result = {0};
    if (!shared_session_is_started(session) || !shared_core_activate(&session->core)) return result;
    BronzeActorRegistry *registry = session->core.bronze_registry;
    const BronzeActor *actor = bronze_actors_for_being(registry, sim_group()->select);
    if (!actor) return result;
    result.actor_id = actor->id;
    result.workplace_id = actor->workplace;
    simulated_being *being = sim_group()->select;
    result.energy = being_energy(being);
    result.energy_full = BEING_FULL;
    result.hunger = being_drives(being)[DRIVE_HUNGER];
    result.fatigue = being_drives(being)[DRIVE_FATIGUE];
    const BronzeProgram *program = bronze_actors_program(registry);
    bronze_summary_stock(result.cargo, sizeof(result.cargo), NULL, actor->resources,
                         actor->resource_count, program ? &program->config->resource_kinds : NULL);
    bronze_summary_stock(result.items, sizeof(result.items), NULL, actor->items,
                         actor->item_count, program ? &program->config->item_kinds : NULL);
    if (actor->economic_status != BRONZE_TASK_IDLE) {
        snprintf(result.exchange, sizeof(result.exchange), "%s · %s",
                 actor->economic_action == BRZ_ACTION_SERVE ? "Paid delivery" : "Trade",
                 bronze_task_status_name(actor->economic_status));
        result.exchange_partner_id = actor->economic_partner;
    }
    const BronzeLandscape *landscape = bronze_actors_landscape(registry);
    uint64_t site_id = actor->workplace ? actor->workplace : actor->home;
    for (unsigned i = 0; i < landscape->place_count; ++i) {
        const BronzePlace *place = &landscape->places[i];
        if (place->id == site_id &&
            (place->kind == BRONZE_PLACE_STORAGE || place->kind == BRONZE_PLACE_SERVICE)) {
            site_id = place->owner;
            break;
        }
    }
    for (n_uint i = 0; i < landscape->site_count; ++i) {
        const bronze_site *site = &landscape->sites[i];
        if (site->place_id != site_id) continue;
        static const char *const names[] = {
            [BRONZE_TOWN] = "Township", [BRONZE_FIELD] = "Field",
            [BRONZE_COPPER_MINE] = "Copper mine", [BRONZE_TIN_MINE] = "Tin mine",
            [BRONZE_WOODLAND] = "Woodland"
        };
        snprintf(result.site, sizeof(result.site), "%s #%llu · (%ld, %ld)",
                 names[site->kind], (unsigned long long)site->place_id, (long)site->x, (long)site->y);
        break;
    }
    const BronzeProfessionPolicy *policy = bronze_actors_professions(registry);
    if (actor->job_status != BRONZE_JOB_ASSIGNED || actor->vocation >= policy->count) return result;
    snprintf(result.vocation, sizeof(result.vocation), "%s", policy->vocations[actor->vocation].name);
    const BronzeTaskState *task = &actor->execution;
    snprintf(result.status, sizeof(result.status), "%s", bronze_inspector_task_status(actor));
    if (task->name) snprintf(result.task, sizeof(result.task), "%s", task->name);
    if (task->action) snprintf(result.action, sizeof(result.action), "%s", task->action);
    result.remaining_minutes = actor->remaining_ticks;
    return result;
}

void shared_session_bronze_actor_summary(shared_session* session, char* buffer, n_uint capacity)
{
    if (!buffer || !capacity || capacity > SIZE_MAX) return;
    size_t size = (size_t)capacity;
    snprintf(buffer, size, "No registered being selected");
    if (!shared_session_is_started(session) || !shared_core_activate(&session->core)) return;
    BronzeActorRegistry* registry = session->core.bronze_registry;
    const BronzeActor* actor = bronze_actors_for_being(registry, sim_group()->select);
    if (!actor) return;
    const BronzeProfessionPolicy* policy = bronze_actors_professions(registry);
    if (actor->job_status == BRONZE_JOB_ASSIGNED && actor->vocation < policy->count) {
        snprintf(buffer, size, "Actor #%llu · %s · skill %.0f/%u · home #%llu · workplace #%llu",
                 (unsigned long long)actor->id, policy->vocations[actor->vocation].name,
                 bronze_actor_skill(actor, actor->vocation), policy->skill_max,
                 (unsigned long long)actor->home, (unsigned long long)actor->workplace);
        const BronzeTaskState* task = &actor->execution;
        const char* status = bronze_inspector_task_status(actor);
        size_t used = strlen(buffer);
        if (task->name)
            snprintf(buffer + used, size - used, " · %s: %s%s%s (line %d, step %u, %llu min left)",
                     task->name, status, task->action ? " / " : "",
                     task->action ? task->action : "", task->line, actor->instruction,
                     (unsigned long long)actor->remaining_ticks);
        else if (task->status != BRONZE_TASK_IDLE)
            snprintf(buffer + used, size - used, " · %s", status);
    } else {
        static const char* reasons[] = {
            "no enabled occupations", "awaiting assignment", "below working age",
            "no reachable home", "no reachable matching workplace", "workplaces full",
            "skill record unavailable"
        };
        snprintf(buffer, size, "Actor #%llu · unassigned (%s) · home #%llu",
                 (unsigned long long)actor->id, reasons[actor->job_status],
                 (unsigned long long)actor->home);
    }
    if (actor->economic_status != BRONZE_TASK_IDLE) {
        const char *action = actor->economic_action == BRZ_ACTION_SERVE ? "Paid delivery" : "Trade";
        const char *outcome = bronze_task_status_name(actor->economic_status);
        size_t used = strlen(buffer);
        if (actor->economic_partner != BRONZE_ACTOR_NONE)
            snprintf(buffer + used, size - used, "\nLatest exchange · %s · %s · partner #%llu",
                     action, outcome, (unsigned long long)actor->economic_partner);
        else
            snprintf(buffer + used, size - used, "\nLatest exchange · %s · %s", action, outcome);
    }
    simulated_being *being = sim_group()->select;
    size_t used = strlen(buffer);
    snprintf(buffer + used, size - used, "\nEnergy %d/%d · Hunger %u · Fatigue %u",
             (int)being_energy(being), BEING_FULL, (unsigned)being_drives(being)[DRIVE_HUNGER],
             (unsigned)being_drives(being)[DRIVE_FATIGUE]);
    const BronzeProgram *program = bronze_actors_program(registry);
    bronze_summary_stock(buffer, size, "Cargo", actor->resources, actor->resource_count,
                         program ? &program->config->resource_kinds : NULL);
    bronze_summary_stock(buffer, size, "Items", actor->items, actor->item_count,
                         program ? &program->config->item_kinds : NULL);

    const BronzeLandscape *landscape = bronze_actors_landscape(registry);
    for (n_uint i = 0; i < landscape->site_count; ++i) {
        const bronze_site *site = &landscape->sites[i];
        if (site->place_id != actor->workplace) continue;
        used = strlen(buffer);
        snprintf(buffer + used, size - used, "\nWorkplace (%ld, %ld) · stock %.2f/%.2f",
                 (long)site->x, (long)site->y, site->stock, site->capacity);
        break;
    }
    for (unsigned i = 0; i < landscape->town_count; ++i) {
        const BronzeTownship *town = &landscape->towns[i];
        if (town->home != actor->home) continue;
        double available = 0, reserved = 0;
        for (unsigned kind = 0; kind < BRONZE_RESOURCE_LIMIT; ++kind) {
            reserved += town->reserved_resources[kind];
            available += town->resources[kind] - town->reserved_resources[kind];
        }
        used = strlen(buffer);
        snprintf(buffer + used, size - used,
                 "\nTown store #%llu · %.2f resource units available · %.2f reserved",
                 (unsigned long long)town->storage, available, reserved);
        break;
    }
}

uint64_t shared_session_bronze_actor_id(shared_session* session)
{
    if(!shared_session_is_started(session) || !shared_core_activate(&session->core)) return BRONZE_ACTOR_NONE;
    BronzeActor* actor=bronze_actors_for_being(session->core.bronze_registry,sim_group()->select);
    return actor?actor->id:BRONZE_ACTOR_NONE;
}
#endif
