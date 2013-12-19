/****************************************************************

 shared.c

 =============================================================

 Copyright 1996-2013 Tom Barbalet. All rights reserved.

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

/* This is intended to dramatically simplify the interface between
     the various platform versions of Noble Ape */

#include <stdio.h>

#include "gui.h"
#include "shared.h"

static n_int   mouse_x, mouse_y;
static n_byte  mouse_option, mouse_identification;
static n_byte  mouse_down;

static n_byte  mouse_drag;
static n_int   mouse_drag_x , mouse_drag_y;

static n_byte  key_identification;
static n_byte2 key_value;
static n_byte  key_down;

#define	Y_DELTA	36
#define	X_DELTA	20

/*NOBLEMAKE END=""*/

static n_int toggle_pause = 0;

static n_int control_toggle_pause(n_byte actual_toggle)
{
    if (io_command_line_execution())
    {
        console_stop(0L,"",io_console_out);
    }

    if (actual_toggle)
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

static void control_mouse(n_byte wwind, n_int px, n_int py, n_byte option)
{
    noble_being	*local;
    noble_simulation * local_sim = sim_sim();
    if (local_sim == 0L)
        return;

    if(wwind == NUM_VIEW && check_about == 1)
    {
        check_about = 0;
        return;
    }

    if (local_sim->select == 0L)
    {
        return;
    }

    local = local_sim->select;
    if (wwind == NUM_VIEW)
    {
        if (option)
        {
            n_byte2    location[2];

            location[0] = APESPACE_CONFINED(MAPSPACE_TO_APESPACE(px));
            location[1] = APESPACE_CONFINED(MAPSPACE_TO_APESPACE(py));

            being_set_location(local, location);
        }
        else
        {
            noble_being *	desired_ape = local_sim->select;
            n_uint  high_squ = 31;
            n_uint	loop = 0;
            
            while (loop < local_sim->num)
            {
                noble_being	* current_ape = &(local_sim->beings[loop]);
                n_int	screen_x = APESPACE_TO_MAPSPACE(being_location_x(current_ape)) - px;
                n_int	screen_y = APESPACE_TO_MAPSPACE(being_location_y(current_ape)) - py;
                n_uint	current_squ = (screen_x * screen_x) + (screen_y * screen_y);
                if (high_squ > current_squ)
                {
                    high_squ = current_squ;
                    desired_ape = current_ape;
                }
                loop++;
            }
            if (local_sim->select != desired_ape)
            {                
                sim_set_select(desired_ape);
            }
            else
            {
                if (mouse_drag == 0)
                {
                    mouse_drag_x = px;
                    mouse_drag_y = py;
                    mouse_drag = 1;
                }
            }
        }
    }
    if (wwind == NUM_TERRAIN)
    {
        n_int upper_x, upper_y;
        draw_terrain_coord(&upper_x, &upper_y);

        if (option != 0)
        {
            if ((px > (upper_x/4)) && (px < ((upper_x * 3)/4)))
            {
                if (py < (upper_y/4))
                {
                    tilt_z = ((tilt_z + 1) & 255);
                }
                else if (py < ((upper_y * 3)/4))
                {
                    tilt_z = ((tilt_z + 255) & 255);
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
                        being_move(local, -1, 0);
                    }
                    else
                    {
                        being_move(local, -32, 1);
                    }
                }
                else
                {
                    if (sy > 0)
                    {
                        being_move(local, 32, 1);
                    }
                    else
                    {
                        being_move(local, 1, 0);
                    }
                }
            }
        }
    }
}


/* do key down in which window */

static void control_key(n_byte wwind, n_byte2 num)
{
    noble_being * local;
    noble_simulation * local_sim = sim_sim();
    if (local_sim == 0L)
        return;

    if (local_sim->select == 0L)
    {
        return;
    }

    local = local_sim->select;

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
        noble_being * local_select = local_sim->select;
        if (num == 2078)
        {
            if (local_sim->select != &(local_sim->beings[local_sim->num - 1]))
            {
                local_select++;
            }
            else
            {
                local_select = local_sim->beings;
            }
        }
        else
        {
            if (local_sim->select != local_sim->beings)
            {
                local_select--;
            }
            else
            {
                local_select = &(local_sim->beings[local_sim->num - 1]);
            }
        }
        
        sim_set_select(local_select);
    }
}

#ifdef SCRIPT_DEBUG

static n_int script_entry = 1;

static n_int shared_script_debug_ready(void)
{
    if (script_entry == 0)
    {
        return 0;
    }
    if (scdebug_file_ready() != 0L)
    {
        script_entry = 0;
        return 1;
    }
    return 0;
}

void shared_script_debug_handle(n_string cStringFileName)
{
    if (cStringFileName)
    {
        n_file          * outputfile = scdebug_file_ready();
        io_disk_write(outputfile, cStringFileName);
    }
    scdebug_file_cleanup();
    script_entry = 1;
}

#else

void shared_script_debug_handle(n_string cStringFileName)
{
    
}

#endif

static void * control_init(KIND_OF_USE kind, n_uint randomise)
{
    void * sim_return = 0L;
    
    (void)draw_error(0L);
    draw_undraw_clear();
    
    sim_return = sim_init(kind, randomise, OFFSCREENSIZE, VIEWWINDOW(0));
    
    if (sim_return)
    {
        return draw_offscreen(sim_return);
    }
    return 0;
}

shared_cycle_state shared_cycle(n_uint ticks, n_byte fIdentification, n_int dim_x, n_int dim_y)
{
    shared_cycle_state return_value = SHARED_CYCLE_OK;
    
    ticks = ticks & 67108863; /* 71 58 27 88 */
    ticks *= 60;
    
#ifndef	_WIN32
    sim_thread_console();
#endif
    if((mouse_down == 1) && (mouse_identification == fIdentification))
    {
        control_mouse(mouse_identification, mouse_x, mouse_y, mouse_option);
    }
    if((key_down == 1) && (key_identification == fIdentification))
    {
        if ((key_identification == NUM_VIEW) || (key_identification == NUM_TERRAIN))
        {
            control_key(key_identification, key_value);
        }
    }
    if(fIdentification == NUM_TERRAIN)
    {
        sim_realtime(ticks);
        draw_window(dim_x, dim_y);
                
        if ((io_command_line_execution() != 1) && (!toggle_pause))
        {
            sim_cycle();
        }

        draw_cycle();

                
#ifdef SCRIPT_DEBUG
        if (shared_script_debug_ready())
        {
            return_value = SHARED_CYCLE_DEBUG_OUTPUT;
        }
#endif
        if (sim_thread_console_quit())
        {
            return_value = SHARED_CYCLE_QUIT;
        }
    }
    return return_value;
}

#ifdef NO_LONGER_SUPPORTED
void shared_cycle_draw(n_byte fIdentification, n_int dim_x, n_int dim_y)
{
    draw_cycle(fIdentification, dim_x, dim_y);
}
#endif

n_int shared_init(n_byte view, n_uint random)
{
    n_byte2 fit[256 * 3];

    key_down = 0;
    mouse_down = 0;
    mouse_drag = 0;

    if (view == NUM_TERRAIN)
    {
        if (control_init(KIND_START_UP, random) == 0L)
        {
            return SHOW_ERROR("Initialization failed lack of memory");
        }
    }

    draw_fit(land_points, fit);
    return view;
}

void shared_close(void)
{
    sim_close();
}

n_int shared_notPause(void)
{
    return control_toggle_pause(1);
}

void shared_notWeather(void)
{
    (void)draw_toggle_weather();
}

void shared_notBrain(void)
{
    (void)draw_toggle_brain();
}

void shared_notBrainCode(void)
{
    (void)draw_toggle_braincode();
}

void shared_notTerritory(void)
{
    (void)draw_toggle_territory();
}

void shared_flood(void)
{
    sim_flood();
}

void shared_healthy_carrier(void)
{
    sim_healthy_carrier();
}

void shared_keyReceived(n_byte2 value, n_byte fIdentification)
{
    key_down = 1;
    key_value = value;
    key_identification = fIdentification;
}

void shared_keyUp(void)
{
    key_down = 0;
}

void shared_mouseOption(n_byte option)
{
    mouse_option = option;
}

void shared_mouseReceived(n_int valX, n_int valY, n_byte fIdentification)
{
    mouse_down = 1;
    mouse_identification = fIdentification;
    mouse_x = valX;
    mouse_y = valY;
}

void shared_mouseUp(void)
{
    mouse_down = 0;
    mouse_drag = 0;
}

void shared_about(n_constant_string value)
{
    toggle_pause = 1;
    draw_about(value);
}

n_byte * shared_draw(n_byte fIdentification)
{
    return draw_pointer(fIdentification);
}

void shared_timeForColor(n_byte2 * fit, n_int fIdentification)
{
    noble_simulation * local_sim = sim_sim();
    draw_color_time(fit, local_sim->land->time);
}

void shared_previousApe(void)
{
    (void)control_key(0, 2079);
}

void shared_nextApe(void)
{
    (void)control_key(0, 2078);
}

void shared_clearErrors(void)
{
    (void)draw_error(0L);
}

n_int shared_new(n_uint seed)
{
    static  n_int NewBlock = 0;
    if (NewBlock) return 0;

    NewBlock = 1;
    if (control_init(KIND_NEW_SIMULATION, seed))
    {
        NewBlock = 0;
        return SHOW_ERROR("Failed through lack of memory");
    }

    NewBlock = 0;
    
    return 0;
}

n_byte shared_openFileName(n_string cStringFileName, n_byte isScript)
{
    (void)control_toggle_pause(0);

    if (isScript)
    {
        return (console_script(0L, cStringFileName, 0L) == 0);
    }
    return (console_open(0L, cStringFileName, 0L) == 0);
}

void shared_saveFileName(n_string cStringFileName)
{
    (void)control_toggle_pause(0);
    (void)console_save(0L, cStringFileName, 0L);
}
