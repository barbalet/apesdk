/****************************************************************

 shared.c

 =============================================================

 Copyright 1996-2014 Tom Barbalet. All rights reserved.

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
extern n_byte   terrain_turn;

#ifdef MULTITOUCH_CONTROLS

touch_control_state tc_state = TCS_SHOW_NOTHING;
touch_control_state tc_temp_state = TCS_SHOW_NOTHING;
n_int               tc_countdown = 0;

#endif

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
        
#ifdef MULTITOUCH_CONTROLS
        if ((tc_state&1) == 0)
        {
            tc_state++;
            tc_countdown = 60;
        }
#endif
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
            n_int sx = px - ((upper_x)>>1);
#ifdef MULTITOUCH_CONTROLS
            if (px < TC_FRACTION_X)
            {
                if (tc_state == TCS_LEFT_STATE_CONTROLS)
                {
                    printf("TCS_SHOW_NOTHING\n");
                    tc_temp_state = TCS_SHOW_NOTHING;
                }
                else
                {
                    printf("TCS_RIGHT_STATE\n");
                    tc_temp_state = TCS_RIGHT_STATE;
                }
                tc_countdown = 0;
            }
            
            if (px > (upper_x - TC_FRACTION_X))
            {
                if (tc_state == TCS_RIGHT_STATE_CONTROLS)
                {
                    printf("TCS_SHOW_NOTHING\n");
                    tc_temp_state = TCS_SHOW_NOTHING;
                }
                else
                {
                    printf("TCS_LEFT_STATE\n");
                    tc_temp_state = TCS_LEFT_STATE;
                }
                tc_countdown = 0;
            }
#endif
            if (sx > 0)
            {
                terrain_turn = ( terrain_turn + 1) & 255;
            }

            if (sx <= 0)
            {
                terrain_turn = ( terrain_turn + 255) & 255;
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
        being_change_selected(local_sim, (num == 2078));
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
    
    shared_menu(NA_MENU_CLEAR_ERRORS);
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
    if ((mouse_down == 1) && (mouse_identification == fIdentification))
    {
        
#if (MAP_BITS == 8)
#ifndef NOBLE_IOS
        if (fIdentification == NUM_VIEW)
        {
            control_mouse(mouse_identification, mouse_x/2, mouse_y/2, mouse_option);
        }
        else
#endif
#endif
        {
            control_mouse(mouse_identification, mouse_x, mouse_y, mouse_option);
        }
    }
    
    if ((mouse_down == 0) && (mouse_identification == fIdentification))
    {
        if (tc_temp_state != tc_state)
        {
            tc_state = tc_temp_state;
        }
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
#ifdef MULTITOUCH_CONTROLS
        if (tc_countdown)
        {
            tc_countdown--;
            if (tc_countdown == 0)
            {
                tc_state --;
            }
        }
#endif
        if ((io_command_line_execution() != 1) && (!toggle_pause))
        {
            sim_cycle();
        }
#ifdef SCRIPT_DEBUG
        if (shared_script_debug_ready())
        {
            return_value = SHARED_CYCLE_DEBUG_OUTPUT;
        }
#endif
#ifndef	_WIN32
        if (sim_thread_console_quit())
        {
            return_value = SHARED_CYCLE_QUIT;
        }
#endif
    }
    return return_value;
}

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
    n_string_block full_value;
    n_int          cores = execute_threads_value();
    toggle_pause = 1;
    if (cores > 1)
    {
        sprintf(full_value, "%s %ld Cores", value, cores);
    }
    else
    {
        sprintf(full_value, "%s 1 Core", value);
    }
    draw_about(full_value);
}

void shared_clearErrors(void)
{
    (void)draw_error(0L, 0L, 0);
}

n_int shared_new(n_uint seed)
{
    static  n_int NewBlock = 0;
    if (NewBlock) return 0;

    NewBlock = 1;
    (void)control_init(KIND_NEW_SIMULATION, seed);

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

void shared_rotate(n_double num, n_byte wwind)
{
    if (wwind == NUM_TERRAIN)
    {
        n_int integer_rotation_256 = (n_int)((num * 256) / 360);
        terrain_turn = (n_byte)(((n_int)terrain_turn + 512 + integer_rotation_256) & 255);
    }
}

n_uint shared_max_fps(void)
{
    return 60;
}

n_int shared_menu(n_int menuVal)
{
    switch (menuVal) {
        case NA_MENU_PAUSE:
            return control_toggle_pause(1);
        case NA_MENU_WEATHER:
            return draw_toggle_weather();
        case NA_MENU_BRAIN:
            return draw_toggle_brain();
        case NA_MENU_BRAINCODE:
            return draw_toggle_braincode();
        case NA_MENU_TERRITORY:
            return draw_toggle_territory();
        case NA_MENU_PREVIOUS_APE:
            (void)control_key(0, 2079);
            return 0;
        case NA_MENU_NEXT_APE:
            (void)control_key(0, 2078);
            return 0;
        case NA_MENU_CLEAR_ERRORS:
            (void)draw_error(0L, 0L, 0);
            return 0;
        case NA_MENU_FLOOD:
            sim_flood();
            return 0;
        case NA_MENU_HEALTHY_CARRIER:
            sim_healthy_carrier();
            return 0;
    }
    return -1;
}

void shared_saved_string(shared_saved_string_type ssst, n_string value)
{
    
}

void shared_draw(n_byte * outputBuffer, n_byte fIdentification, n_int dim_x, n_int dim_y)
{
    n_int           ly = 0;
    n_int           loop = 0;
    n_int			loopColors = 0;
    n_byte2         fit[256*3];
    noble_simulation * local_sim = sim_sim();
    n_byte           * index = draw_pointer(fIdentification);
#ifdef NOBLE_IOS
    n_c_uint         * offscreenBuffer = (n_c_uint *) outputBuffer;
    n_c_uint        colorLookUp[256];
#else
    n_byte          colorLookUp[256][3];
#endif
    if (index == 0L) return;
    
    if (fIdentification == NUM_TERRAIN)
    {
        draw_window(dim_x, dim_y);
        draw_cycle();
    }
    
    draw_color_time(fit, local_sim->land->time);

    while(loopColors < 256)
    {
#ifdef NOBLE_IOS
        n_byte colR = fit[loop++] >> 8;
        n_byte colG = fit[loop++] >> 8;
        n_byte colB = fit[loop++] >> 8;
        colorLookUp[ loopColors ] = (colR << 16) | (colG << 8) | (colB << 0);
#else
        colorLookUp[loopColors][0] = fit[loop++] >> 8;
        colorLookUp[loopColors][1] = fit[loop++] >> 8;
        colorLookUp[loopColors][2] = fit[loop++] >> 8;
#endif
        loopColors++;
    }
    
#if (MAP_BITS == 8)
#ifndef NOBLE_IOS
    if (fIdentification == NUM_VIEW)
    {
        loop = 0;
        while(ly < 512)
        {
            n_byte * indexLocalX = &index[(255-(ly>>1))*256];

            if (ly&1)
            {
                io_copy(&outputBuffer[loop - (3*512)], &outputBuffer[loop], (3*512));
                loop += (3*512);
            }
            else
            {
                n_int    lx = 0;
                while(lx < 256)
                {
                    unsigned char value = indexLocalX[lx++] ;
                    outputBuffer[loop++] = colorLookUp[value][0];
                    outputBuffer[loop++] = colorLookUp[value][1];
                    outputBuffer[loop++] = colorLookUp[value][2];
                    outputBuffer[loop++] = colorLookUp[value][0];
                    outputBuffer[loop++] = colorLookUp[value][1];
                    outputBuffer[loop++] = colorLookUp[value][2];

                }
            }
            
            ly++;
        }

        
        return;
    }
#endif
#endif
    
    loop = 0;
    while(ly < dim_y)
    {
        n_int    lx = 0;
        n_byte * indexLocalX = &index[(dim_y-ly-1)*dim_x];
        while(lx < dim_x)
        {
#ifdef NOBLE_IOS
            offscreenBuffer[loop++] = colorLookUp[ indexLocalX[ lx++ ] ];
#else
            unsigned char value = indexLocalX[lx++] ;
            outputBuffer[loop++] = colorLookUp[value][0];
            outputBuffer[loop++] = colorLookUp[value][1];
            outputBuffer[loop++] = colorLookUp[value][2];
#endif
        }
        ly++;
    }
    
}



