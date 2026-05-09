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

#include "buffer.h"
#include "gui.h"

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
