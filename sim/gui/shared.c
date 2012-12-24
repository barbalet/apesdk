/****************************************************************

 shared.c

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

/* This is intended to dramatically simplify the interface between
     the various platform versions of Noble Ape */

#include <stdio.h>

#ifndef	_WIN32

#include "../noble/noble.h"
#include "../universe/universe.h"

#else

#include "..\noble\noble.h"
#include "..\universe\universe.h"

#endif

#include "gui_internal.h"
#include "gui.h"
#include "shared.h"

static n_int   mouse_x, mouse_y;
static n_byte  mouse_option, mouse_identification;
static n_byte  mouse_down;

static n_byte  key_identification;
static n_byte2 key_value;
static n_byte  key_down;

static void shared_squared_cycle(n_uint ticks, n_int fIdentification)
{    
    sim_thread_console();
    
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
}

void shared_cycle(n_uint ticks, n_int fIdentification)
{    
    ticks = ticks & 67108863; // 71 58 27 88
    ticks *= 60;
    
    if(fIdentification == NUM_TERRAIN)
    {
        control_sim_simulate(ticks);
        draw_cycle(0, 512, 511);
    }
    if(fIdentification == NUM_VIEW)
    {
        draw_cycle(1, 512, 512);
    }
}

void shared_cycle_really_no_draw(n_uint ticks, n_int fIdentification)
{
    shared_squared_cycle(ticks, fIdentification);
    
    ticks = ticks & 67108863; // 71 58 27 88
    ticks *= 60;
    
    if(fIdentification == NUM_TERRAIN)
    {
        control_sim_simulate(ticks);
    }
}

void shared_cycle_no_draw(n_uint ticks, n_int fIdentification)
{
    shared_cycle_really_no_draw(ticks, fIdentification);
    
    if(fIdentification == NUM_VIEW)
    {
        draw_cycle(1, 512, 512);
    }
}

void shared_cycle_really_draw(n_int fIdentification, n_int dim_x, n_int dim_y)
{
    draw_cycle((fIdentification == NUM_VIEW), dim_x, dim_y);
}

n_int shared_init(n_byte view, n_uint random)
{
    n_int   fIdentification = -1;
    n_byte2 fit[256 * 3];

    key_down = 0;
    mouse_down = 0;

    if (view)
    {
        fIdentification = NUM_VIEW;
    }
    else
    {
        fIdentification = NUM_TERRAIN;
        (void)control_init(KIND_START_UP, random);
        if (io_disk_check("NobleApeAutoload.txt") == 1)
        {
            n_file * tester = io_file_new();
            (void)io_disk_read(tester,"NobleApeAutoload.txt");
            if(file_in(tester) == 0)
            {
                control_init(KIND_LOAD_FILE, 0);
                file_bin_read("NobleApeAutoload.txt");
            }
            io_file_free(tester);
        }
        if (io_disk_check("ApeScriptAutoload.txt") == 1)
        {
            n_file * tester = io_file_new();
            (void)io_disk_read(tester,"ApeScriptAutoload.txt");
            (void)file_interpret(tester);
            io_file_free(tester);
        }
    }

    draw_fit(land_points, fit);
    return fIdentification;
}

void shared_close(void)
{
    shared_create_preload(1);
    sim_close();
}

void shared_notPause(void)
{
    (void)control_toggle_pause(1);
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
}

void shared_about(n_string value)
{
    control_about(value);
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

void shared_create_preload(n_int exists)
{
    if (io_disk_check("NobleApeAutoload.txt") == exists)
    {
        shared_saveFileName("NobleApeAutoload.txt");
    }
}

void shared_clearErrors(void)
{
    (void)draw_error(0L);
}

void shared_new(n_uint seed)
{
    static  n_int NewBlock = 0;
    static  n_int generator = 1;
    if (NewBlock) return;
    generator++;
    if (generator == 0xFF)
    {
        generator = 1;
    }
    NewBlock = 1;
    control_init(KIND_NEW_SIMULATION, (n_uint) ( seed * generator ));
    NewBlock = 0;
}

void shared_brainDisplay(n_byte value)
{
    sim_braindisplay(value);
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

#ifdef SCRIPT_DEBUG

static n_int single_entry = 1;

n_int shared_script_debug_ready(void)
{
    n_int result = (scdebug_file_ready() != 0L);

    if (single_entry == 0)
    {
        return 0;
    }

    if (result)
    {
        single_entry = 0;
    }
    return result;
}

void shared_script_debug_handle(n_string cStringFileName)
{
    if (cStringFileName)
    {
        n_file          * outputfile = scdebug_file_ready();
        io_disk_write(outputfile, cStringFileName);
    }
    scdebug_file_cleanup();
    single_entry = 1;
}

#else

n_int shared_script_debug_ready(void)
{
    return 0;
}

void shared_script_debug_handle(n_string cStringFileName)
{

}

#endif
