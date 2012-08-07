/****************************************************************

 control.c

 =============================================================

 Copyright 1996-2012 Tom Barbalet. All rights reserved.

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

 This software and Noble Ape are a continuing work of Tom Barbalet,
 begun on 13 June 1996. No apes or cats were harmed in the writing
 of this software.

 ****************************************************************/

/*NOBLEMAKE DEL=""*/

#ifndef	_WIN32

#include "../noble/noble.h"
#include "../universe/universe.h"
#include "../entity/entity.h"

#define	NOBLELIB_INTERFACE

#else

#include "..\noble\noble.h"
#include "..\universe\universe.h"
#include "..\entity\entity.h"

#endif

#include "gui.h"

/*NOBLEMAKE END=""*/
/*NOBLEMAKE VAR=""*/


#define	Y_DELTA	36
#define	X_DELTA	20

/*NOBLEMAKE END=""*/

/* shows which cursor is expected based on the window the mouse is over and the mouse location */

#define CURSOR_NULL 0

#define CURSOR_MOVE_BACKWARD  1
#define CURSOR_MOVE_FORWARD   2
#define CURSOR_MOVE_LEFT      3
#define CURSOR_MOVE_RIGHT     4

#define CURSOR_ROTATE_UP      5
#define CURSOR_ROTATE_DOWN    6

#define CURSOR_SELECT         7


#if (END_BEFORE_BRAIN_PTR != NON_PTR_BEING)

offsets are wrong. This is a basic check! Run CLE and find out more!

#endif

#ifdef __APPLE__

static n_int macosx_console_quit = 0;

n_int control_command_line_execution(void)
{
    return io_command_line_execution();
}

n_int control_macosx_console_quit(void)
{
    return macosx_console_quit;
}


void control_activate_console(void)
{
    if (io_console(sim_sim(), (noble_console_command *) control_commands, io_console_entry_clean, io_console_out) != 0)
    {
        macosx_console_quit = 1;
    }
}

#endif


n_byte control_cursor(n_byte wwind, n_int px, n_int py, n_byte option, n_byte no_bounds)
{
    n_int upper_x, upper_y;
    if(wwind == (NUM_TERRAIN))
    {
        upper_x = TERRAIN_WINDOW_WIDTH;
        upper_y = TERRAIN_WINDOW_HEIGHT;
    }
    else
    {
        upper_x = MAP_DIMENSION;
        upper_y = MAP_DIMENSION;
    }
    if ((py > -1 && py < upper_y && px > -1 && px < upper_x) || no_bounds)
    {
        if (wwind != (NUM_TERRAIN))
        {
            return CURSOR_SELECT;
        }
        if (option != 0)
        {
            if ((px>128) && (px < (128+256)))
            {
                if (py < 128)
                {
                    return CURSOR_ROTATE_DOWN;
                }
                else if (py < 256)
                {
                    return CURSOR_ROTATE_UP;
                }
            }
        }
        else
        {
            n_int sx = px + py - ((upper_x + upper_y)>>1);
            n_int sy = px - py;
            if ((sx * sx) + (sy * sy) > 16384)
            {
                if (sx > 0)
                {
                    if (sy > 0)
                    {
                        return CURSOR_MOVE_RIGHT;
                    }
                    return CURSOR_MOVE_BACKWARD;
                }
                if (sy > 0)
                {
                    return CURSOR_MOVE_FORWARD;
                }
                return CURSOR_MOVE_LEFT;
            }
        }
    }
    return CURSOR_NULL;
}

/* do mouse down interaction over a window, at a point, with keys possibly pressed */

extern n_byte	check_about;
extern n_uint	tilt_z;

void control_sim_mouse(noble_simulation * local_sim, n_byte wwind, n_int px, n_int py, n_byte option)
{
    noble_being	*local;

    if (local_sim == 0L)
        return;

    if(wwind == NUM_TERRAIN && check_about == 1)
    {
        check_about = 0;
        return;
    }

    if (local_sim->select == NO_BEINGS_FOUND)
    {
        return;
    }

    local = &(local_sim->beings[local_sim->select]);
    if (wwind == NUM_VIEW)
    {
        if (option)
        {
            GET_X(local) = APESPACE_CONFINED(MAPSPACE_TO_APESPACE(px));
            GET_Y(local) = APESPACE_CONFINED(MAPSPACE_TO_APESPACE(py));
        }
        else
        {
            n_uint	desired_ape = local_sim->select;
            n_uint high_squ = 31;
            n_uint	loop = 0;

            while (loop < local_sim->num)
            {
                noble_being	* current_ape = &(local_sim->beings[loop]);
                n_int	screen_x = APESPACE_TO_MAPSPACE(GET_X(current_ape)) - px;
                n_int	screen_y = APESPACE_TO_MAPSPACE(GET_Y(current_ape)) - py;
                n_uint	current_squ = (screen_x * screen_x) + (screen_y * screen_y);
                if (high_squ > current_squ)
                {
                    high_squ = current_squ;
                    desired_ape = loop;
                }
                loop++;
            }
            if (local_sim->select != desired_ape)
            {
                sim_set_select(desired_ape);
            }
        }
    }
    if (wwind == NUM_TERRAIN)
    {
        n_byte ch_cursor = control_cursor(wwind, px, py, option, 1);
        switch (ch_cursor)
        {
        case CURSOR_MOVE_BACKWARD:
            being_move(local, -32, 1);
            break;
        case CURSOR_MOVE_FORWARD:
            being_move(local, 32, 1);
            break;
        case CURSOR_MOVE_LEFT:
            being_move(local, 1, 0);
            break;
        case CURSOR_MOVE_RIGHT:
            being_move(local, -1, 0);
            break;
        case CURSOR_ROTATE_UP:
            tilt_z = ((tilt_z + 255) & 255);
            break;
        case CURSOR_ROTATE_DOWN:
            tilt_z = ((tilt_z + 1) & 255);
            break;
        }
    }
}


/* do key down in which window */

void control_sim_key(noble_simulation * local_sim, n_byte wwind, n_byte2 num)
{
    noble_being * local;
    if (local_sim == 0L)
        return;

    if (local_sim->select == NO_BEINGS_FOUND)
    {
        return;
    }

    local = &(local_sim->beings[local_sim->select]);

    if ((num > 27) && (num < 32))
    {
        if (wwind != NUM_VIEW)
        {
            if ((num - 28) < 2)
                being_move(local, (1 - ((num - 28) << 1)), 0);
            else
                being_move(local, (160-((num - 28) << 6)), 1);
        }
        else
        {
            being_move(local, (31-num), 2);
        }
    }
    if ((num > 2075) && (num < 2078))
    {
        being_move(local, ( -1 + ((2077-num) << 1)), 0);
    }
    if ((num > 2077) && (num < 2080))
    {
        n_uint local_select = local_sim->select;
        n_uint local_number = local_sim->num;

        if (num == 2078)
            local_select = ((local_select + 1) % local_number);
        else
            local_select = ((local_select + (local_number - 1)) % local_number);

        sim_set_select(local_select);
    }
}

void control_sim_simulate(n_byte local_pause, n_uint local_time)
{
    sim_realtime(local_time);
    
    if (io_command_line_execution())
    {
        return;
    }
    
    if (local_pause != 1)
    {
        sim_cycle();
    }
}

void control_sim_draw(noble_simulation * local_sim, n_byte local_weather)
{
    draw_cycle(local_sim, local_weather);
}


void control_simulate(n_byte local_weather, n_byte local_pause, n_uint local_time)
{
    noble_simulation *sim = sim_sim();
    control_sim_simulate(local_pause, local_time);
    control_sim_draw(sim, 0);
    control_sim_draw(sim, 1 + local_weather);
}

void control_mouse(n_byte wwind, n_int px, n_int py, n_byte option)
{
    noble_simulation *sim = sim_sim();
    control_sim_mouse(sim, wwind, px, py, option);
}

void control_key(n_byte wwind, n_byte2 num)
{
    noble_simulation *sim = sim_sim();
    control_sim_key(sim, wwind, num);
}


void * control_init(KIND_OF_USE kind, n_uint randomise)
{
    (void)draw_error(0L);
    draw_undraw_clear();
#ifdef BRAIN_HASH
    brain_hash_count = 0;
    brain_hash_out[0] = 0;
#endif
#ifdef THREADED
    sim_draw_thread_on();
#endif
    return draw_offscreen(sim_init(kind, randomise, OFFSCREENSIZE, VIEWWINDOW(0)));
}

