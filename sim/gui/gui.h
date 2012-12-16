/****************************************************************

 gui.h

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

/*NOBLEMAKE VAR=""*/

#ifndef _NOBLEAPE_GUI_H_
#define _NOBLEAPE_GUI_H_

#define	OFFSCREENSIZE				(MAP_AREA + TERRAIN_WINDOW_AREA)

#define	TERRAINWINDOW(alpha)   (alpha)
#define	VIEWWINDOW(alpha)      (alpha + TERRAIN_WINDOW_AREA)

#define	NUM_TERRAIN           (0)
#define	NUM_VIEW			  (1)

#define IS_WINDOW_KIND(x,y)			(((x)>>(y))&1)


#define COLOUR_GREY         (252)
#define COLOUR_YELLOW       (253)
#define COLOUR_RED_DARK     (254)
#define COLOUR_RED          (255)

#define	NON_INTERPOLATED COLOUR_GREY


#define	spot_colour(alpha,spx,spy,col)	alpha[((spx)|((spy)<<8))]=(col)

/*	Graphics Metrics */

#define BRAIN_MAGI					1152


/*	Icon Offset */
#define	ICONOFFSET					27

typedef struct
{
    n_byte	* screen;
    n_byte	* background;
}
n_background8;

typedef struct
{
    n_byte  * screen;
    n_byte	  color;
}
n_color8;


typedef n_int (*control_file_handle)(n_byte * buff, n_uint len);

typedef void  (*control_mp_define)(n_byte * data);


n_int draw_toggle_weather(void);

n_int draw_toggle_brain(void);

n_int draw_toggle_braincode(void);

n_int control_toggle_pause(void);

void control_about(n_string value);

n_byte control_cursor(n_byte  wwind, n_int px, n_int py, n_byte  option, n_byte no_bounds);

void control_simulate(n_uint local_time);
void control_mouse(n_byte wwind, n_int px, n_int py, n_byte option);
void control_key(n_byte wwind, n_byte2 num);

void control_sim_simulate(n_uint local_time);

void  draw_terrain_coord(n_int * co_x, n_int * co_y);

n_byte * control_window(n_byte * initial, n_byte * type, n_int * location, n_byte num);

void * control_init(KIND_OF_USE kind, n_uint randomise);

void draw_fit(n_byte * points, n_byte2 * color_fit);

extern n_byte land_points[];
extern n_byte fur_points[];

void draw_color_time(n_byte2 * color_fit, n_byte2 time);

void draw_undraw_clear(void);

n_byte * draw_pointer(n_byte which_one);

n_byte * draw_color_fit();

void draw_about(n_string platform);

void draw_string(n_string str, n_int off_x, n_int off_y, n_join * draw);

void  draw_cycle(n_byte mod, n_int dim_x, n_int dim_y);

n_byte * draw_offscreen(n_byte * value);

#endif /* _NOBLEAPE_GUI_H_ */

/*NOBLEMAKE END=""*/

/*NOBLEMAKE SET="draw.c"*/
/*NOBLEMAKE SET="control.c"*/
