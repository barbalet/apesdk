/****************************************************************
 
 draw.c
 
 =============================================================
 
 Copyright 1996-2025 Tom Barbalet. All rights reserved.
 
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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "toolkit.h"
#include "script.h"
#include "sim.h"
#include "planet.h"


static n_byte2    color_group[256 * 3];
static n_byte     land_color_initialized = 0;

#define    SUBSTA(c)    ((c<<8)|c)

void land_color_init( void )
{
    static n_byte points[] =
    {
        0, 0, 0, 0,
        106, 43, 70, 120,
        125, 107, 201, 202,
        128, 255, 255, 239,
        150, 88, 169, 79,
        190, 8, 15, 7,
        208, 208, 216, 206,
        255, 255, 255, 255
    };

    /* performs a linear interpolation of n 8-bit points to 256 16-bit blend values */
    n_int    lp = 0, lp2 = 0;
    n_int    dr = 0, dg = 0, db = 0;
    n_int    ar = 0, ag = 0, ab = 0, cntr = 0;
    n_int    fp = 0, fl = 0, del_c = 0;
    while ( lp < 256 )
    {
        if ( lp == points[cntr] )
        {
            ar = SUBSTA( points[( cntr ) | 1] );
            ag = SUBSTA( points[( cntr ) | 2] );
            ab = SUBSTA( points[( cntr ) | 3] );
            fp = lp;
            cntr += 4;

            if ( lp != 255 )
            {
                fl = points[cntr];
                del_c = ( fl - fp );
                dr = SUBSTA( points[( cntr ) | 1] );
                dg = SUBSTA( points[( cntr ) | 2] );
                db = SUBSTA( points[( cntr ) | 3] );
            }
        }

        if ( del_c == 0 )
        {
            return;
        }

        if ( lp != 255 )
        {
            n_int    del_a = ( fl - lp ), del_b = ( lp - fp );
            color_group[lp2++] = ( n_byte2 )( ( ( ar * del_a ) + ( dr * del_b ) ) / del_c );
            color_group[lp2++] = ( n_byte2 )( ( ( ag * del_a ) + ( dg * del_b ) ) / del_c );
            color_group[lp2++] = ( n_byte2 )( ( ( ab * del_a ) + ( db * del_b ) ) / del_c );
        }
        else
        {
            color_group[lp2++] = ( n_byte2 )( ar );
            color_group[lp2++] = ( n_byte2 )( ag );
            color_group[lp2++] = ( n_byte2 )( ab );
        }
        lp ++;
    }

    color_group[( COLOR_WHITE * 3 )    ] = 0xffff;
    color_group[( COLOR_WHITE * 3 ) + 1] = 0xffff;
    color_group[( COLOR_WHITE * 3 ) + 2] = 0xffff;

    color_group[( COLOR_BLUE * 3 )    ] = 0x5500;
    color_group[( COLOR_BLUE * 3 ) + 1] = 0x5500;
    color_group[( COLOR_BLUE * 3 ) + 2] = 0xeeff;

    color_group[( COLOR_RED_DARK * 3 )    ] = ( 0xeeff * 2 ) >> 2; /* return to * 3 following debugging */
    color_group[( COLOR_RED_DARK * 3 ) + 1] = 0x0000;
    color_group[( COLOR_RED_DARK * 3 ) + 2] = 0x0000;

    color_group[( COLOR_RED * 3 )    ] = 0xeeff;
    color_group[( COLOR_RED * 3 ) + 1] = 0x0000;
    color_group[( COLOR_RED * 3 ) + 2] = 0x0000;
}


void  land_color_time( n_byte2 *color_fit, n_int toggle_tidedaylight )
{
    n_int   day_rotation = ( ( land_time() * 255 ) / TIME_DAY_MINUTES );
    n_int   darken =  math_sine( day_rotation + 64 + 128, NEW_SD_MULTIPLE / 400 );
    n_int   loop = 0;
    n_int   sign = 1;
    if ( land_color_initialized == 0 )
    {
        land_color_init();
        land_color_initialized = 1;
    }
    if ( !toggle_tidedaylight )
    {
        if ( darken < 1 )
        {
            sign = -1;
        }
        darken = ( darken * darken ) / 402;
        darken = ( sign * darken ) + 624;
        while ( loop < ( NON_INTERPOLATED * 3 ) )
        {
            n_int cg_val = color_group[loop];
            n_int response = ( cg_val * darken ) >> 10;
            color_fit[loop] = ( n_byte2 )response;
            loop++;
        }
    }
    while ( loop < ( 256 * 3 ) )
    {
        color_fit[loop] = color_group[loop];
        loop++;
    }
}

void  land_color_time_8bit( n_byte *color_fit, n_int toggle_tidedaylight )
{
    n_int   day_rotation = ( ( land_time() * 255 ) / TIME_DAY_MINUTES );
    n_int   darken =  math_sine( day_rotation + 64 + 128, NEW_SD_MULTIPLE / 400 );
    n_int   loop = 0;
    n_int   sign = 1;
    if ( land_color_initialized == 0 )
    {
        land_color_init();
        land_color_initialized = 1;
    }
    if ( !toggle_tidedaylight )
    {
        if ( darken < 1 )
        {
            sign = -1;
        }
        darken = ( darken * darken ) / 402;
        darken = ( sign * darken ) + 624;
        while ( loop < ( NON_INTERPOLATED * 3 ) )
        {
            n_int cg_val = color_group[loop];
            n_int response = ( cg_val * darken ) >> 10;
            color_fit[loop] = ( n_byte )( response >> 8 );
            loop++;
        }
    }
    while ( loop < ( 256 * 3 ) )
    {
        color_fit[loop] = ( n_byte )( color_group[loop] >> 8 );
        loop++;
    }
}


n_int draw_error(n_constant_string error_text, n_constant_string location, n_int line_number)
{
    printf("draw_error %s %s %ld \n", error_text, location, line_number);
    return -1;
}

