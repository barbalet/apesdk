/****************************************************************

 gui.h

 =============================================================

 Copyright 1996-2020 Tom Barbalet. All rights reserved.

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

 This software is a continuing work of Tom Barbalet, begun on
 13 June 1996. No apes or cats were harmed in the writing of
 this software.

 ****************************************************************/

#ifndef SIMULATEDAPE_GUI_H
#define SIMULATEDAPE_GUI_H

#ifndef	_WIN32


#include "../toolkit/toolkit.h"
#include "../script/script.h"
#include "../sim/sim.h"


#ifdef SIMULATED_APE_CLIENT
#include "client.h"
#else
#include "../universe/universe.h"
#include "../entity/entity.h"
#endif

#else

#ifdef SIMULATED_APE_CLIENT
#include "client.h"
#else
#include "..\universe\universe.h"
#include "..\entity\entity.h"
#endif

#endif

#undef MULTITOUCH_CONTROLS

#define	   TERRAINWINDOW(alpha)   (alpha)
#define    CONTROLWINDOW(alpha)   (alpha + TERRAIN_WINDOW_AREA)
#define    VIEWWINDOW(alpha)      (alpha + TERRAIN_WINDOW_AREA + CONTROL_WINDOW_AREA)

#define IS_WINDOW_KIND(x,y)			(((x)>>(y))&1)

#define	spot_color(alpha,spx,spy,col)	alpha[((spx)|((spy)<<8))]=(col)

/*	Graphics Metrics */

/*	Icon Offset */
#define	ICONOFFSET					27

#ifdef MULTITOUCH_CONTROLS

typedef enum
{
    TCS_SHOW_NOTHING = 0,
    TCS_SHOW_CONTROLS,
    TCS_LEFT_STATE,
    TCS_LEFT_STATE_CONTROLS,
    TCS_RIGHT_STATE,
    TCS_RIGHT_STATE_CONTROLS
} touch_control_state;

#define TC_OFFSET_Y   (40)
#define TC_FRACTION_X (40)

#endif

n_byte * draw_weather_grayscale(void);

void vascular_draw(n_genetics * genetics,
                   n_byte * buffer,
                   n_vect2* img,
                   n_vect2 *tp, n_vect2 * bp,
                   n_byte thickness,
                   n_byte clear,
                   n_int shoulder_angle, n_int elbow_angle, n_int wrist_angle,
                   n_int hip_angle, n_int knee_angle,
                   n_byte show_skeleton_keypoints);

n_vect2 * draw_selected_location(void);

void draw_point(n_int x, n_int y);

n_int draw_toggle_follow(void);

n_int draw_toggle_social_web(void);

n_int draw_toggle_weather(void);

n_int draw_toggle_brain(void);

n_int draw_toggle_braincode(void);

n_int draw_toggle_territory(void);

n_int draw_toggle_tide_daylight(void);
n_int draw_toggle_tide_daylight_value(void);

void  draw_terrain_coord(n_int * co_x, n_int * co_y);

void draw_undraw_clear(void);

n_byte * draw_pointer(n_int which_one);

n_byte * draw_color_fit(void);

void draw_about(void);

void draw_string(n_constant_string str, n_int off_x, n_int off_y, n_join * draw);

void  draw_window(n_int dim_x, n_int dim_y);

void  draw_cycle(n_byte size_changed, n_byte kind);

n_byte * draw_offscreen(n_byte * value);

n_int draw_control_font_scaling(void);

#endif /* SIMULATEDAPE_GUI_H */

