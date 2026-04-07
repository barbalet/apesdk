/****************************************************************

 app_shell.h

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

#ifndef SIMULATEDAPE_APP_SHELL_H
#define SIMULATEDAPE_APP_SHELL_H

#ifndef _WIN32

#include "../shared.h"

#else

#include "..\shared.h"

#endif

#include "buffer.h"

typedef struct
{
    n_byte  identification;
    n_byte2 value;
    n_byte  down;
} ape_app_shell_key;

typedef struct
{
    n_byte down;
    n_byte option;
} ape_app_shell_mouse;

typedef struct
{
    n_int simulation_started;
    ape_buffer output_buffer;
    n_int output_buffer_width;
    n_int output_buffer_height;
    n_int last_key_value;
    ape_app_shell_key key;
    ape_app_shell_mouse mouse;
} ape_app_shell;

typedef n_int (ape_app_shell_init_callback)(void *context, n_int view, n_uint random);
typedef void (ape_app_shell_close_callback)(void *context);
typedef shared_cycle_state (ape_app_shell_cycle_callback)(void *context, n_uint ticks, n_int identification);
typedef void (ape_app_shell_draw_callback)(void *context, n_byte *output_buffer, n_int identification, n_int dim_x, n_int dim_y, n_byte size_changed);

void ape_app_shell_reset(ape_app_shell *shell);
n_int ape_app_shell_init(ape_app_shell *shell, n_int view, n_uint random, ape_app_shell_init_callback *callback, void *context);
void ape_app_shell_shutdown(ape_app_shell *shell, ape_app_shell_close_callback *callback, void *context);

n_int ape_app_shell_simulation_started(const ape_app_shell *shell);
void ape_app_shell_set_simulation_started(ape_app_shell *shell, n_int simulation_started);

n_byte *ape_app_shell_output_buffer(ape_app_shell *shell, n_int width, n_int height);
shared_cycle_state ape_app_shell_cycle(ape_app_shell *shell, n_uint ticks, n_int identification, ape_app_shell_cycle_callback *callback, void *context);
n_byte *ape_app_shell_draw(ape_app_shell *shell, n_int identification, n_int dim_x, n_int dim_y, n_byte size_changed, ape_app_shell_draw_callback *callback, void *context);

void ape_app_shell_key_received(ape_app_shell *shell, n_int value, n_int identification);
void ape_app_shell_key_up(ape_app_shell *shell);
n_byte ape_app_shell_key_active(const ape_app_shell *shell, n_int identification);

void ape_app_shell_mouse_option(ape_app_shell *shell, n_byte option);
n_byte ape_app_shell_mouse_begin(ape_app_shell *shell);
void ape_app_shell_mouse_up(ape_app_shell *shell);

#endif /* SIMULATEDAPE_APP_SHELL_H */
