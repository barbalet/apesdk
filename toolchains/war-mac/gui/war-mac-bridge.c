/****************************************************************

 war-mac-bridge.c

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

#include "war-mac-bridge.h"

#include <math.h>
#include <stdlib.h>

#define WAR_BOARD_WIDTH  (1024)
#define WAR_BOARD_HEIGHT (768)
#define WAR_UNIT_COUNT   (8)
#define WAR_TYPE_COUNT   (4)

typedef struct
{
    n_int type;
    n_int alignment;
    n_int alive;
    n_int total;
    n_double x;
    n_double y;
    n_double target_x;
    n_double target_y;
    n_double facing;
    n_int selected;
    n_int command;
} war_unit;

static war_unit war_units[WAR_UNIT_COUNT];
static n_int war_selected_army = WAR_ARMY_ALL;
static n_int war_paused = 0;
static n_double war_mouse_x = 0.0;
static n_double war_mouse_y = 0.0;
static n_byte *war_buffer = 0L;
static n_int war_buffer_bytes = 0;

static const n_int war_type_stats[WAR_TYPE_COUNT][13] =
{
    { 5, 3, 1, 3, 1, 0, 0, 0, 24, 6, 1, 0, 1 },
    { 6, 4, 2, 2, 2, 0, 0, 0, 30, 8, 2, 0, 1 },
    { 4, 2, 1, 4, 2, 0, 0, 0, 48, 5, 1, 0, 1 },
    { 7, 5, 3, 1, 1, 0, 0, 0, 12, 4, 3, 0, 2 }
};

static n_int war_unit_visible( const war_unit *unit )
{
    return ( war_selected_army == WAR_ARMY_ALL ) || ( unit->alignment == war_selected_army );
}

static void war_reset_units( void )
{
    n_int loop;

    for ( loop = 0; loop < WAR_UNIT_COUNT; loop++ )
    {
        n_int army = loop >= ( WAR_UNIT_COUNT / 2 );
        n_int column = loop % ( WAR_UNIT_COUNT / 2 );
        n_double x = army ? 720.0 + ( column * 54.0 ) : 140.0 + ( column * 58.0 );
        n_double y = 170.0 + ( column * 118.0 );

        war_units[loop].type = loop % WAR_TYPE_COUNT;
        war_units[loop].alignment = army;
        war_units[loop].alive = 18 - ( ( loop % 3 ) * 3 );
        war_units[loop].total = 18;
        war_units[loop].x = x;
        war_units[loop].y = y;
        war_units[loop].target_x = x;
        war_units[loop].target_y = y;
        war_units[loop].facing = army ? 180.0 : 0.0;
        war_units[loop].selected = 0;
        war_units[loop].command = WAR_COMMAND_IDLE;
    }

    war_selected_army = WAR_ARMY_ALL;
    war_paused = 0;
}

static n_byte *war_output_buffer( n_int dim_x, n_int dim_y )
{
    n_int required = dim_x * dim_y * 4;

    if ( required <= 0 )
    {
        return 0L;
    }

    if ( required > war_buffer_bytes )
    {
        n_byte *new_buffer = ( n_byte * )realloc( war_buffer, ( size_t )required );
        if ( new_buffer == 0L )
        {
            return 0L;
        }
        war_buffer = new_buffer;
        war_buffer_bytes = required;
    }

    return war_buffer;
}

static void war_pixel( n_byte *buffer, n_int width, n_int height, n_int x, n_int y, n_byte red, n_byte green, n_byte blue )
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

static void war_rect( n_byte *buffer, n_int width, n_int height, n_int x0, n_int y0, n_int x1, n_int y1, n_byte red, n_byte green, n_byte blue )
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
            war_pixel( buffer, width, height, x, y, red, green, blue );
        }
    }
}

static void war_circle( n_byte *buffer, n_int width, n_int height, n_int cx, n_int cy, n_int radius, n_byte red, n_byte green, n_byte blue )
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
                war_pixel( buffer, width, height, cx + x, cy + y, red, green, blue );
            }
        }
    }
}

static void war_draw_line( n_byte *buffer, n_int width, n_int height, n_int x0, n_int y0, n_int x1, n_int y1, n_byte red, n_byte green, n_byte blue )
{
    n_int dx = labs( x1 - x0 );
    n_int sx = x0 < x1 ? 1 : -1;
    n_int dy = -labs( y1 - y0 );
    n_int sy = y0 < y1 ? 1 : -1;
    n_int error = dx + dy;

    while ( 1 )
    {
        n_int e2;
        war_pixel( buffer, width, height, x0, y0, red, green, blue );
        if ( ( x0 == x1 ) && ( y0 == y1 ) )
        {
            break;
        }
        e2 = 2 * error;
        if ( e2 >= dy )
        {
            error += dy;
            x0 += sx;
        }
        if ( e2 <= dx )
        {
            error += dx;
            y0 += sy;
        }
    }
}

void war_init( n_int view, n_uint random )
{
    ( void )view;
    ( void )random;
    war_reset_units();
}

void war_close( void )
{
    if ( war_buffer )
    {
        free( war_buffer );
        war_buffer = 0L;
        war_buffer_bytes = 0;
    }
}

n_int war_cycle( n_uint ticks, n_int view )
{
    n_int loop;

    ( void )ticks;
    ( void )view;

    if ( war_paused )
    {
        return 0;
    }

    for ( loop = 0; loop < WAR_UNIT_COUNT; loop++ )
    {
        war_unit *unit = &war_units[loop];
        n_double dx = unit->target_x - unit->x;
        n_double dy = unit->target_y - unit->y;
        n_double distance = sqrt( ( dx * dx ) + ( dy * dy ) );

        if ( distance > 1.0 )
        {
            n_double step = 1.5 + ( unit->type * 0.25 );
            unit->x += ( dx / distance ) * step;
            unit->y += ( dy / distance ) * step;
            unit->command = WAR_COMMAND_MOVE;
        }
        else if ( unit->command == WAR_COMMAND_MOVE )
        {
            unit->command = WAR_COMMAND_IDLE;
        }
    }

    return 0;
}

n_byte *war_draw( n_int view, n_int dim_x, n_int dim_y, n_byte size_changed )
{
    n_int x;
    n_int y;
    n_int loop;
    n_byte *buffer = war_output_buffer( dim_x, dim_y );

    ( void )view;
    ( void )size_changed;

    if ( buffer == 0L )
    {
        return 0L;
    }

    war_rect( buffer, dim_x, dim_y, 0, 0, dim_x - 1, dim_y - 1, 36, 45, 41 );

    for ( y = 0; y < dim_y; y += 48 )
    {
        war_draw_line( buffer, dim_x, dim_y, 0, y, dim_x - 1, y, 48, 58, 54 );
    }
    for ( x = 0; x < dim_x; x += 64 )
    {
        war_draw_line( buffer, dim_x, dim_y, x, 0, x, dim_y - 1, 48, 58, 54 );
    }

    war_rect( buffer, dim_x, dim_y, dim_x / 2 - 10, 0, dim_x / 2 + 10, dim_y - 1, 68, 75, 63 );

    for ( loop = 0; loop < WAR_UNIT_COUNT; loop++ )
    {
        const war_unit *unit = &war_units[loop];
        n_int ux = ( n_int )( ( unit->x / WAR_BOARD_WIDTH ) * dim_x );
        n_int uy = ( n_int )( ( unit->y / WAR_BOARD_HEIGHT ) * dim_y );
        n_byte red = unit->alignment ? 79 : 202;
        n_byte green = unit->alignment ? 132 : 77;
        n_byte blue = unit->alignment ? 207 : 67;

        if ( !war_unit_visible( unit ) )
        {
            red /= 3;
            green /= 3;
            blue /= 3;
        }

        if ( unit->command == WAR_COMMAND_MOVE )
        {
            n_int tx = ( n_int )( ( unit->target_x / WAR_BOARD_WIDTH ) * dim_x );
            n_int ty = ( n_int )( ( unit->target_y / WAR_BOARD_HEIGHT ) * dim_y );
            war_draw_line( buffer, dim_x, dim_y, ux, uy, tx, ty, 212, 189, 91 );
        }

        war_circle( buffer, dim_x, dim_y, ux, uy, 17 + ( unit->type * 2 ), red, green, blue );
        war_circle( buffer, dim_x, dim_y, ux, uy, 7, 232, 228, 205 );

        if ( unit->selected )
        {
            war_circle( buffer, dim_x, dim_y, ux, uy, 24, 247, 232, 142 );
            war_circle( buffer, dim_x, dim_y, ux, uy, 19, red, green, blue );
        }
    }

    return buffer;
}

void war_dimensions( n_int *dimensions )
{
    if ( dimensions == 0L )
    {
        return;
    }

    dimensions[0] = 1;
    dimensions[1] = WAR_BOARD_WIDTH;
    dimensions[2] = WAR_BOARD_HEIGHT;
    dimensions[3] = 0;
}

void war_set_selected_army( n_int army )
{
    war_selected_army = army;
}

void war_keyReceived( n_int value, n_int localIdentification )
{
    ( void )localIdentification;

    if ( ( value == 'p' ) || ( value == 'P' ) || ( value == 112 ) )
    {
        war_paused = !war_paused;
    }
    if ( ( value == 'n' ) || ( value == 'N' ) || ( value == 110 ) )
    {
        war_reset_units();
    }
}

void war_mouseReceived( n_double valX, n_double valY, n_int localIdentification )
{
    ( void )localIdentification;
    war_mouse_x = valX;
    war_mouse_y = valY;
}

void war_mouseUp( void )
{
    n_int loop;
    n_int nearest = -1;
    n_double nearest_distance = 100000000.0;

    for ( loop = 0; loop < WAR_UNIT_COUNT; loop++ )
    {
        war_unit *unit = &war_units[loop];
        n_double dx;
        n_double dy;
        n_double distance;

        if ( !war_unit_visible( unit ) )
        {
            continue;
        }

        dx = unit->x - war_mouse_x;
        dy = unit->y - war_mouse_y;
        distance = ( dx * dx ) + ( dy * dy );

        if ( distance < nearest_distance )
        {
            nearest_distance = distance;
            nearest = loop;
        }
    }

    for ( loop = 0; loop < WAR_UNIT_COUNT; loop++ )
    {
        war_units[loop].selected = loop == nearest;
    }
}

n_int war_order_move( n_int destination_x, n_int destination_y )
{
    n_int loop;
    n_int count = 0;

    for ( loop = 0; loop < WAR_UNIT_COUNT; loop++ )
    {
        war_unit *unit = &war_units[loop];
        if ( unit->selected && war_unit_visible( unit ) )
        {
            unit->target_x = destination_x;
            unit->target_y = destination_y;
            unit->command = WAR_COMMAND_MOVE;
            count++;
        }
    }

    return count;
}

n_int war_order_face( n_int destination_x, n_int destination_y )
{
    n_int loop;
    n_int count = 0;

    for ( loop = 0; loop < WAR_UNIT_COUNT; loop++ )
    {
        war_unit *unit = &war_units[loop];
        if ( unit->selected && war_unit_visible( unit ) )
        {
            n_double dx = destination_x - unit->x;
            n_double dy = destination_y - unit->y;
            unit->facing = atan2( dy, dx );
            unit->command = WAR_COMMAND_FACE;
            count++;
        }
    }

    return count;
}

n_int war_unit_count( void )
{
    return WAR_UNIT_COUNT;
}

n_int war_unit_type_count( void )
{
    return WAR_TYPE_COUNT;
}

n_int war_unit_type_stat( n_int type, n_int stat )
{
    if ( ( type < 0 ) || ( type >= WAR_TYPE_COUNT ) || ( stat < 0 ) || ( stat >= 13 ) )
    {
        return 0;
    }
    return war_type_stats[type][stat];
}

n_int war_unit_type_for_unit( n_int unit )
{
    if ( ( unit < 0 ) || ( unit >= WAR_UNIT_COUNT ) )
    {
        return 0;
    }
    return war_units[unit].type;
}

n_int war_unit_alignment( n_int unit )
{
    if ( ( unit < 0 ) || ( unit >= WAR_UNIT_COUNT ) )
    {
        return 0;
    }
    return war_units[unit].alignment;
}

n_int war_unit_living( n_int unit )
{
    if ( ( unit < 0 ) || ( unit >= WAR_UNIT_COUNT ) )
    {
        return 0;
    }
    return war_units[unit].alive;
}

n_int war_unit_total( n_int unit )
{
    if ( ( unit < 0 ) || ( unit >= WAR_UNIT_COUNT ) )
    {
        return 0;
    }
    return war_units[unit].total;
}

n_int war_unit_average_x( n_int unit )
{
    if ( ( unit < 0 ) || ( unit >= WAR_UNIT_COUNT ) )
    {
        return 0;
    }
    return ( n_int )war_units[unit].x;
}

n_int war_unit_average_y( n_int unit )
{
    if ( ( unit < 0 ) || ( unit >= WAR_UNIT_COUNT ) )
    {
        return 0;
    }
    return ( n_int )war_units[unit].y;
}

n_int war_unit_selected( n_int unit )
{
    if ( ( unit < 0 ) || ( unit >= WAR_UNIT_COUNT ) )
    {
        return 0;
    }
    return war_units[unit].selected;
}

n_int war_unit_command_mode( n_int unit )
{
    if ( ( unit < 0 ) || ( unit >= WAR_UNIT_COUNT ) )
    {
        return WAR_COMMAND_IDLE;
    }
    return war_units[unit].command;
}

n_int draw_error( n_constant_string error_text, n_constant_string location, n_int line_number )
{
    ( void )error_text;
    ( void )location;
    ( void )line_number;
    return -1;
}
