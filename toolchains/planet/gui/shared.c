/****************************************************************
 
 shared.c
 
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

#include "../../../gui/app_shell.h"
#include "planet.h"

static ape_app_shell app_shell = { 0 };
static n_int initialize_run = 0;

static n_int planet_app_init(void *context, n_int view, n_uint random)
{
    n_byte2 local_random[2];

    (void)context;
    (void)view;

    local_random[0] = (n_byte2)((random >> 16) & 0xffff);
    local_random[1] = (n_byte2)(random & 0xffff);

    land_seed_genetics(local_random);
    land_clear(KIND_NEW_SIMULATION, 0);
    land_init();
    land_color_init();
    weather_init();

    planet_init(local_random);
    initialize_run = 1;
    return 1;
}

static void planet_app_close(void *context)
{
    (void)context;
    initialize_run = 0;
}

static shared_cycle_state planet_app_cycle(void *context, n_uint ticks, n_int localIdentification)
{
    (void)context;
    (void)ticks;
    (void)localIdentification;

    if (initialize_run)
    {
        land_cycle();
        weather_cycle();
        planet_cycle();
    }
    return SHARED_CYCLE_OK;
}

static void planet_app_draw(void *context, n_byte *outputBuffer, n_int fIdentification, n_int dim_x, n_int dim_y, n_byte size_changed);

void shared_dimensions(n_int * dimensions)
{
    dimensions[0] = 1;   /* number windows */
    dimensions[1] = (256 * 3); /* window dimension x */
    dimensions[2] = (256 * 2); /* window dimension y */
    dimensions[3] = 0;   /* has menus */
}

shared_cycle_state shared_cycle(n_uint ticks, n_int localIdentification)
{
    return ape_app_shell_cycle(&app_shell, ticks, localIdentification, planet_app_cycle, 0L);
}

n_int shared_simulation_started(void) {
    return ape_app_shell_simulation_started(&app_shell);
}


n_int shared_init(n_int view, n_uint random)
{
    return ape_app_shell_init(&app_shell, view, random, planet_app_init, 0L);
}

void shared_close(void)
{
    ape_app_shell_shutdown(&app_shell, planet_app_close, 0L);
}

n_int shared_menu(n_int menuValue)
{
    return 0;
}

void shared_delta(n_double delta_x, n_double delta_y, n_int wwind)
{
    (void)delta_x;
    (void)delta_y;
    (void)wwind;
}

void shared_zoom(n_double num, n_int wwind)
{
    (void)num;
    (void)wwind;
}

void shared_rotate(n_double num, n_int wwind)
{
    (void)num;
    (void)wwind;
}

void shared_keyReceived(n_int value, n_int localIdentification)
{
    ape_app_shell_key_received(&app_shell, value, localIdentification);
}

void shared_keyUp(void)
{
    ape_app_shell_key_up(&app_shell);
}

void shared_mouseOption(n_byte option)
{
    ape_app_shell_mouse_option(&app_shell, option);
}

void shared_mouseReceived(n_double valX, n_double valY, n_int localIdentification)
{
    (void)valX;
    (void)valY;
    (void)localIdentification;
    (void)ape_app_shell_mouse_begin(&app_shell);
}

void shared_mouseUp(void)
{
    ape_app_shell_mouse_up(&app_shell);
}

void shared_about(void)
{
    
}

void shared_draw_cell(n_byte * outputBuffer, n_int cell_x, n_int cell_y, n_byte * color)
{
    n_int tile = -1;
    n_byte   weather_grayscale[MAP_AREA];
    
    if (cell_y == 0)
    {
        tile = 0;
    }
    else if (cell_y == 2)
    {
        tile = 5;
    }
    else
    {
        tile = cell_x + 1;
    }
    if (tile != -1)
    {
        n_byte * map = land_location_tile(tile);
        n_c_int *local_pressure = land_weather(tile);

    
        n_int cell_offset_x = (cell_x * 256 * 4);
        n_int cell_offset_y = (cell_y * 256 * 4 * 4 * 256);
        n_int loop_y = 0;
        
        n_int loop = 0;
        while(loop < MAP_AREA)
        {
            n_int value = local_pressure[ loop ]>>7;
            if (value < 0) value = 0;
            if (value > 255) value = 255;
            weather_grayscale[ loop ] = value;
            loop++;
        }
        loop = 0;
        while (loop_y < 256)
        {
            n_int loop_x = 0;
            while (loop_x < 256)
            {
                n_int location = (loop_x * 4) | (loop_y * 4 * 256 * 4) | cell_offset_x | cell_offset_y;
                n_int map_location = loop_x + (loop_y * 256);
                n_int map_value = map[map_location] * 3;
                
                n_byte cloud = weather_grayscale[loop++];
                n_int negCloud = 256 - cloud;
#ifdef METAL_RENDER
                outputBuffer[location | 3] = 0;
                outputBuffer[location | 2] = cloud + ((negCloud*color[map_value++])>>8);
                outputBuffer[location | 1] = cloud + ((negCloud*color[map_value++])>>8);
                outputBuffer[location | 0] = cloud + ((negCloud*color[map_value])>>8);
#else
                outputBuffer[location | 0] = 0;
                outputBuffer[location | 1] = cloud + ((negCloud*color[map_value++])>>8);
                outputBuffer[location | 2] = cloud + ((negCloud*color[map_value++])>>8);
                outputBuffer[location | 3] = cloud + ((negCloud*color[map_value])>>8);
#endif
                loop_x++;
            }
            loop_y++;
        }
    }
}

static void planet_app_draw(void *context, n_byte *outputBuffer, n_int fIdentification, n_int dim_x, n_int dim_y, n_byte size_changed)
{
    n_int cell_y = 0;
    n_byte2 fit[256 * 3];
    n_byte colorLookUp[256 * 3];
    n_int loop = 0;

    (void)context;
    (void)fIdentification;
    (void)size_changed;

    memory_erase(outputBuffer, (n_uint)(dim_x * dim_y * 4));

    land_color_time(fit, 0);

    while (loop < (256 * 3))
    {
        colorLookUp[loop] = fit[loop] >> 8;
        loop++;
    }

    while (cell_y < 3)
    {
        n_int cell_x = 0;
        while (cell_x < 4)
        {
            if ((cell_x == 1) || (cell_y == 1))
            {
                shared_draw_cell(outputBuffer, cell_x, cell_y, colorLookUp);
            }
            cell_x++;
        }
        cell_y++;
    }

    planet_draw(outputBuffer);
}

n_byte * shared_draw( n_int fIdentification, n_int dim_x, n_int dim_y, n_byte size_changed )
{
    return ape_app_shell_draw(&app_shell, fIdentification, dim_x, dim_y, size_changed, planet_app_draw, 0L);
}

n_int shared_new(n_uint seed)
{
    return 0;
}

n_int shared_new_agents(n_uint seed)
{
    return 0;
}


n_byte shared_openFileName(n_constant_string cStringFileName, n_int isScript)
{
    (void)cStringFileName;
    (void)isScript;
    return 0;
}

void shared_saveFileName(n_constant_string cStringFileName)
{
    (void)cStringFileName;
}

void shared_script_debug_handle(n_constant_string cStringFileName)
{
    (void)cStringFileName;
}

n_uint shared_max_fps(void)
{
    return 60;
}


#ifndef    _WIN32

n_int sim_thread_console_quit(void)
{
    return 0;
}

#endif
