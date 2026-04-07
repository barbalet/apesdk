/****************************************************************

 app_shell.c

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

#include "app_shell.h"

static void ape_app_shell_input_reset(ape_app_shell *shell)
{
    if (shell == 0L)
    {
        return;
    }

    shell->output_buffer_width = 0;
    shell->output_buffer_height = 0;
    shell->last_key_value = -1;

    shell->key.identification = 0;
    shell->key.value = 0;
    shell->key.down = 0;

    shell->mouse.down = 0;
    shell->mouse.option = 0;
}

void ape_app_shell_reset(ape_app_shell *shell)
{
    if (shell == 0L)
    {
        return;
    }

    shell->simulation_started = 0;
    ape_app_shell_input_reset(shell);
}

n_int ape_app_shell_init(ape_app_shell *shell, n_int view, n_uint random, ape_app_shell_init_callback *callback, void *context)
{
    if (shell == 0L)
    {
        return -1;
    }

    ape_app_shell_reset(shell);

    if ((callback == 0L) || callback(context, view, random))
    {
        shell->simulation_started = 1;
    }

    return 0;
}

void ape_app_shell_shutdown(ape_app_shell *shell, ape_app_shell_close_callback *callback, void *context)
{
    if (shell != 0L)
    {
        shell->simulation_started = 0;
        ape_buffer_free(&shell->output_buffer);
        ape_app_shell_input_reset(shell);
    }

    if (callback != 0L)
    {
        callback(context);
    }
}

n_int ape_app_shell_simulation_started(const ape_app_shell *shell)
{
    if (shell == 0L)
    {
        return 0;
    }

    return shell->simulation_started;
}

void ape_app_shell_set_simulation_started(ape_app_shell *shell, n_int simulation_started)
{
    if (shell == 0L)
    {
        return;
    }

    shell->simulation_started = (simulation_started != 0);
}

n_byte *ape_app_shell_output_buffer(ape_app_shell *shell, n_int width, n_int height)
{
    n_byte *buffer;

    if (shell == 0L)
    {
        return 0L;
    }

    buffer = ape_buffer_require(&shell->output_buffer, width, height);

    if (buffer != 0L)
    {
        shell->output_buffer_width = width;
        shell->output_buffer_height = height;
    }

    return buffer;
}

shared_cycle_state ape_app_shell_cycle(ape_app_shell *shell, n_uint ticks, n_int identification, ape_app_shell_cycle_callback *callback, void *context)
{
    if ((shell == 0L) || (shell->simulation_started == 0) || (callback == 0L))
    {
        return SHARED_CYCLE_OK;
    }

    return callback(context, ticks, identification);
}

n_byte *ape_app_shell_draw(ape_app_shell *shell, n_int identification, n_int dim_x, n_int dim_y, n_byte size_changed, ape_app_shell_draw_callback *callback, void *context)
{
    n_byte *buffer = ape_app_shell_output_buffer(shell, dim_x, dim_y);

    if ((buffer != 0L) && (shell != 0L) && shell->simulation_started && (callback != 0L))
    {
        callback(context, buffer, identification, dim_x, dim_y, size_changed);
    }

    return buffer;
}

void ape_app_shell_key_received(ape_app_shell *shell, n_int value, n_int identification)
{
    if (shell == 0L)
    {
        return;
    }

    shell->key.down = 1;
    shell->key.value = (n_byte2)value;
    shell->key.identification = (n_byte)identification;
}

void ape_app_shell_key_up(ape_app_shell *shell)
{
    if (shell == 0L)
    {
        return;
    }

    shell->key.down = 0;
}

n_byte ape_app_shell_key_active(const ape_app_shell *shell, n_int identification)
{
    if (shell == 0L)
    {
        return 0;
    }

    return (n_byte)(shell->key.down && (shell->key.identification == (n_byte)identification));
}

void ape_app_shell_mouse_option(ape_app_shell *shell, n_byte option)
{
    if (shell == 0L)
    {
        return;
    }

    shell->mouse.option = option;
}

n_byte ape_app_shell_mouse_begin(ape_app_shell *shell)
{
    if (shell == 0L)
    {
        return 0;
    }

    if (shell->mouse.down)
    {
        return 0;
    }

    shell->mouse.down = 1;
    return 1;
}

void ape_app_shell_mouse_up(ape_app_shell *shell)
{
    if (shell == 0L)
    {
        return;
    }

    shell->mouse.down = 0;
}
