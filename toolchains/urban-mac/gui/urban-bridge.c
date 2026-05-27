/****************************************************************

 urban-bridge.c

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

#include "../../../shared.h"

#include <math.h>
#include <stdlib.h>

static n_int urban_started = 0;
static n_double urban_x = 400.0;
static n_double urban_y = 300.0;
static n_double urban_heading = 0.0;
static n_double urban_zoom_level = 1.0;
static n_byte urban_mouse_option = 0;
static n_byte *urban_buffer = 0L;
static n_int urban_buffer_bytes = 0;

static n_byte *urban_output_buffer( n_int dim_x, n_int dim_y )
{
    n_int required = dim_x * dim_y * 4;

    if ( required <= 0 )
    {
        return 0L;
    }

    if ( required > urban_buffer_bytes )
    {
        n_byte *new_buffer = ( n_byte * )realloc( urban_buffer, ( size_t )required );
        if ( new_buffer == 0L )
        {
            return 0L;
        }
        urban_buffer = new_buffer;
        urban_buffer_bytes = required;
    }

    return urban_buffer;
}

static void urban_pixel( n_byte *buffer, n_int width, n_int height, n_int x, n_int y, n_byte red, n_byte green, n_byte blue )
{
    n_int offset;

    if ( ( x < 0 ) || ( y < 0 ) || ( x >= width ) || ( y >= height ) )
    {
        return;
    }

    offset = ( y * width + x ) * 4;
    buffer[offset] = 255;
    buffer[offset + 1] = red;
    buffer[offset + 2] = green;
    buffer[offset + 3] = blue;
}

static void urban_rect( n_byte *buffer, n_int width, n_int height, n_int x0, n_int y0, n_int x1, n_int y1, n_byte red, n_byte green, n_byte blue )
{
    n_int x;
    n_int y;

    if ( x0 > x1 )
    {
        n_int swap = x0;
        x0 = x1;
        x1 = swap;
    }
    if ( y0 > y1 )
    {
        n_int swap = y0;
        y0 = y1;
        y1 = swap;
    }

    for ( y = y0; y <= y1; y++ )
    {
        for ( x = x0; x <= x1; x++ )
        {
            urban_pixel( buffer, width, height, x, y, red, green, blue );
        }
    }
}

static void urban_circle( n_byte *buffer, n_int width, n_int height, n_int cx, n_int cy, n_int radius, n_byte red, n_byte green, n_byte blue )
{
    n_int x;
    n_int y;
    n_int radius_squared = radius * radius;

    for ( y = -radius; y <= radius; y++ )
    {
        for ( x = -radius; x <= radius; x++ )
        {
            if ( ( x * x + y * y ) <= radius_squared )
            {
                urban_pixel( buffer, width, height, cx + x, cy + y, red, green, blue );
            }
        }
    }
}

static void urban_wrap_position( void )
{
    if ( urban_x < 0.0 )
    {
        urban_x = 799.0;
    }
    if ( urban_y < 0.0 )
    {
        urban_y = 599.0;
    }
    if ( urban_x > 799.0 )
    {
        urban_x = 0.0;
    }
    if ( urban_y > 599.0 )
    {
        urban_y = 0.0;
    }
}

void shared_color_8_bit_to_48_bit( n_byte2 *fit )
{
    ( void )fit;
}

void shared_dimensions( n_int *dimensions )
{
    if ( dimensions == 0L )
    {
        return;
    }
    dimensions[0] = 1;
    dimensions[1] = 800;
    dimensions[2] = 600;
    dimensions[3] = 0;
}

n_int shared_init( n_int view, n_uint random )
{
    ( void )view;
    urban_started = 1;
    urban_x = 180.0 + ( n_double )( random % 320 );
    urban_y = 140.0 + ( n_double )( ( random >> 8 ) % 220 );
    urban_heading = 0.0;
    urban_zoom_level = 1.0;
    return 0;
}

void shared_close( void )
{
    urban_started = 0;
    if ( urban_buffer )
    {
        free( urban_buffer );
        urban_buffer = 0L;
        urban_buffer_bytes = 0;
    }
}

n_int shared_simulation_started( void )
{
    return urban_started;
}

n_int shared_menu( n_int menuValue )
{
    ( void )menuValue;
    return 0;
}

n_uint shared_max_fps( void )
{
    return 60;
}

void shared_rotate( n_double num, n_int wwind )
{
    ( void )wwind;
    urban_heading += num;
}

void shared_delta( n_double delta_x, n_double delta_y, n_int wwind )
{
    ( void )wwind;
    urban_x += delta_x * 8.0;
    urban_y += delta_y * 8.0;
    urban_wrap_position();
}

void shared_zoom( n_double num, n_int wwind )
{
    ( void )wwind;
    urban_zoom_level += num;
    if ( urban_zoom_level < 0.35 )
    {
        urban_zoom_level = 0.35;
    }
    if ( urban_zoom_level > 2.5 )
    {
        urban_zoom_level = 2.5;
    }
}

void shared_keyReceived( n_int value, n_int localIdentification )
{
    n_double step = urban_mouse_option ? 18.0 : 9.0;

    ( void )localIdentification;

    if ( ( value == 'w' ) || ( value == 'W' ) || ( value == 30 ) )
    {
        urban_y -= step;
    }
    if ( ( value == 's' ) || ( value == 'S' ) || ( value == 31 ) )
    {
        urban_y += step;
    }
    if ( ( value == 'a' ) || ( value == 'A' ) || ( value == 28 ) )
    {
        urban_x -= step;
    }
    if ( ( value == 'd' ) || ( value == 'D' ) || ( value == 29 ) )
    {
        urban_x += step;
    }
    urban_wrap_position();
}

void shared_keyUp( void )
{
}

void shared_mouseOption( n_byte option )
{
    urban_mouse_option = option;
}

void shared_mouseReceived( n_double valX, n_double valY, n_int localIdentification )
{
    ( void )localIdentification;
    urban_x = valX;
    urban_y = valY;
    urban_wrap_position();
}

void shared_mouseUp( void )
{
}

void shared_mouseReceived_ios( n_double valX, n_double valY )
{
    shared_mouseReceived( valX, valY, 0 );
}

void shared_about( void )
{
}

n_byte *shared_draw( n_int fIdentification, n_int dim_x, n_int dim_y, n_byte size_changed )
{
    n_byte *buffer = urban_output_buffer( dim_x, dim_y );
    n_int block;
    n_int road;
    n_int agent_x;
    n_int agent_y;
    n_int radius;

    ( void )fIdentification;
    ( void )size_changed;

    if ( buffer == 0L )
    {
        return 0L;
    }

    urban_rect( buffer, dim_x, dim_y, 0, 0, dim_x - 1, dim_y - 1, 28, 35, 33 );

    for ( road = 0; road < dim_x; road += 96 )
    {
        urban_rect( buffer, dim_x, dim_y, road, 0, road + 10, dim_y - 1, 58, 61, 57 );
    }
    for ( road = 0; road < dim_y; road += 88 )
    {
        urban_rect( buffer, dim_x, dim_y, 0, road, dim_x - 1, road + 10, 58, 61, 57 );
    }

    for ( block = 0; block < 24; block++ )
    {
        n_int x = 24 + ( ( block * 137 ) % ( dim_x > 80 ? dim_x - 80 : 80 ) );
        n_int y = 26 + ( ( block * 83 ) % ( dim_y > 80 ? dim_y - 80 : 80 ) );
        n_byte tone = ( n_byte )( 76 + ( block % 5 ) * 18 );
        urban_rect( buffer, dim_x, dim_y, x, y, x + 38, y + 30, tone, 92, 86 );
    }

    agent_x = ( n_int )( ( urban_x / 800.0 ) * dim_x );
    agent_y = ( n_int )( ( urban_y / 600.0 ) * dim_y );
    radius = ( n_int )( 14.0 * urban_zoom_level );
    if ( radius < 6 )
    {
        radius = 6;
    }

    urban_circle( buffer, dim_x, dim_y, agent_x, agent_y, radius + 5, 34, 92, 112 );
    urban_circle( buffer, dim_x, dim_y, agent_x, agent_y, radius, 235, 196, 84 );
    urban_circle( buffer, dim_x, dim_y, agent_x + ( n_int )( cos( urban_heading ) * radius ), agent_y + ( n_int )( sin( urban_heading ) * radius ), 4, 42, 36, 31 );

    return buffer;
}

void shared_draw_ios( n_byte4 *outputBuffer, n_int dim_x, n_int dim_y )
{
    n_byte *buffer = shared_draw( 0, dim_x, dim_y, 0 );
    n_int loop;
    n_int count = dim_x * dim_y;

    if ( ( outputBuffer == 0L ) || ( buffer == 0L ) )
    {
        return;
    }

    for ( loop = 0; loop < count; loop++ )
    {
        n_int offset = loop * 4;
        outputBuffer[loop] = ( ( n_byte4 )buffer[offset + 1] << 16 ) |
                             ( ( n_byte4 )buffer[offset + 2] << 8 ) |
                             ( n_byte4 )buffer[offset + 3];
    }
}

shared_cycle_state shared_cycle( n_uint ticks, n_int localIdentification )
{
    ( void )ticks;
    ( void )localIdentification;
    return SHARED_CYCLE_OK;
}

shared_cycle_state shared_cycle_ios( n_uint ticks )
{
    return shared_cycle( ticks, 0 );
}

n_byte *shared_legacy_draw( n_byte fIdentification, n_int dim_x, n_int dim_y )
{
    return shared_draw( fIdentification, dim_x, dim_y, 0 );
}

n_int shared_new( n_uint seed )
{
    return shared_init( 0, seed );
}

n_int shared_new_agents( n_uint seed )
{
    return shared_init( 0, seed );
}

n_byte shared_openFileName( n_constant_string cStringFileName, n_int isScript )
{
    ( void )cStringFileName;
    ( void )isScript;
    return 0;
}

void shared_saveFileName( n_constant_string cStringFileName )
{
    ( void )cStringFileName;
}

void shared_script_debug_handle( n_constant_string cStringFileName )
{
    ( void )cStringFileName;
}

void shared_process( const char *fileUrl )
{
    ( void )fileUrl;
}

n_int shared_initial_tutorial_enabled( void )
{
    return 0;
}

n_int shared_initial_tutorial_count( void )
{
    return 0;
}

n_int shared_initial_tutorial_window( n_int step )
{
    ( void )step;
    return 0;
}

n_int shared_initial_tutorial_anchor_x( n_int step )
{
    ( void )step;
    return 0;
}

n_int shared_initial_tutorial_anchor_y( n_int step )
{
    ( void )step;
    return 0;
}

n_int shared_initial_tutorial_anchor_width( n_int step )
{
    ( void )step;
    return 0;
}

n_int shared_initial_tutorial_anchor_height( n_int step )
{
    ( void )step;
    return 0;
}

n_int shared_initial_tutorial_edge( n_int step )
{
    ( void )step;
    return 0;
}

n_constant_string shared_initial_tutorial_title( n_int step )
{
    ( void )step;
    return "";
}

n_constant_string shared_initial_tutorial_text( n_int step )
{
    ( void )step;
    return "";
}

n_int shared_being_number( void )
{
    return 0;
}

void shared_being_name( n_int number, n_string name )
{
    ( void )number;
    if ( name )
    {
        name[0] = 0;
    }
}

void shared_being_select( n_int number )
{
    ( void )number;
}

n_int draw_error( n_constant_string error_text, n_constant_string location, n_int line_number )
{
    ( void )error_text;
    ( void )location;
    ( void )line_number;
    return -1;
}
