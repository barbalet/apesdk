/****************************************************************

 war-mac-bridge.h

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

#ifndef WAR_MAC_BRIDGE_H
#define WAR_MAC_BRIDGE_H

#include "../../../toolkit/toolkit.h"

#define WAR_ARMY_ALL       (-1)
#define WAR_COMMAND_IDLE   (0)
#define WAR_COMMAND_MOVE   (1)
#define WAR_COMMAND_FACE   (2)

void war_init( n_int view, n_uint random );
void war_close( void );
n_int war_cycle( n_uint ticks, n_int view );
n_byte *war_draw( n_int view, n_int dim_x, n_int dim_y, n_byte size_changed );

void war_dimensions( n_int *dimensions );
void war_set_selected_army( n_int army );

void war_keyReceived( n_int value, n_int localIdentification );
void war_mouseReceived( n_double valX, n_double valY, n_int localIdentification );
void war_mouseUp( void );

n_int war_order_move( n_int destination_x, n_int destination_y );
n_int war_order_face( n_int destination_x, n_int destination_y );

n_int war_unit_count( void );
n_int war_unit_type_count( void );
n_int war_unit_type_stat( n_int type, n_int stat );
n_int war_unit_type_for_unit( n_int unit );
n_int war_unit_alignment( n_int unit );
n_int war_unit_living( n_int unit );
n_int war_unit_total( n_int unit );
n_int war_unit_average_x( n_int unit );
n_int war_unit_average_y( n_int unit );
n_int war_unit_selected( n_int unit );
n_int war_unit_command_mode( n_int unit );

#endif /* WAR_MAC_BRIDGE_H */
