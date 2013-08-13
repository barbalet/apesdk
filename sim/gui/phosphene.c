/****************************************************************

 Phosphene: Plot data in the style of a Cathode Ray Oscilloscope

 =============================================================

 Copyright 2013 Bob Mottram

 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or
 sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the followingp
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

#include "phosphene.h"

#undef PHOSPHENE_LITE

scope create_scope(unsigned int step_ms)
{
	scope s;

    s.mode = PHOSPHENE_MODE_DEFAULT;
    s.marker_orientation = PHOSPHENE_MARKER_VERTICAL;

	s.no_of_traces = 1;
	s.border_percent = 2;

	s.step_ms = step_ms;

	s.background_colour[0] = 105;
	s.background_colour[1] = 183;
	s.background_colour[2] = 169;

	s.background_border_colour[0] = 38;
	s.background_border_colour[1] = 118;
	s.background_border_colour[2] = 117;

	s.markings_colour[0] = 64;
	s.markings_colour[1] = 119;
	s.markings_colour[2] = 114;

	s.trace_colour[0] = 252;
	s.trace_colour[1] = 252;
	s.trace_colour[2] = 252;

	s.trace_surround_colour[0] = 30;
	s.trace_surround_colour[1] = 164;
	s.trace_surround_colour[2] = 127;

	s.trace1_min = 0;
	s.trace1_max = 0;
	s.trace2_min = 0;
	s.trace2_max = 0;

	s.vertical_percent[0] = 0;
	s.vertical_percent[1] = 0;

	s.marker_position = 0;
    s.marker_thickness = 2;
	s.trace1_scan_ms = 0;
	s.trace2_scan_ms = 0;
	s.time_ms = 2000;

    s.trace1[0] = PHOSPHENE_NO_TRACE;
    s.trace2[0] = PHOSPHENE_NO_TRACE;

	s.offset_ms = 0;
	s.trigger_voltage = 0;

    s.noise = 0.03;

	return s;
}

void scope_clear(scope * s)
{
    s->trace1[0] = PHOSPHENE_NO_TRACE;
    s->trace2[0] = PHOSPHENE_NO_TRACE;
}

void scope_update(scope * s,
                  unsigned int trace_index,
                  double value, double min, double max,
                  unsigned int t_ms)
{
	unsigned int t = t_ms/s->step_ms;

    if (t >= PHOSPHENE_MAX_TIME_STEPS) return;

	if (t_ms < s->time_ms) {
		if (trace_index == 0) {		
			s->trace1[t] = value;
			s->trace1_min = min;
			s->trace1_max = max;
			s->trace1_scan_ms = t_ms;
		}
		else {
            s->no_of_traces = 2;
			s->trace2[t] = value;
			s->trace2_min = min;
			s->trace2_max = max;
			s->trace2_scan_ms = t_ms;
		}
	}
}

/* draws a single point with the beam using a given intensity and radius */
static void scope_point(scope * s,
                        int x, int y,
                        unsigned int radius, double intensity_percent,
                        unsigned char * img,
                        unsigned int width, unsigned int height)
{
    int tx = x - radius;
	int ty = y - radius;
	int bx = x + radius;
	int by = y + radius;
#ifdef PHOSPHENE_LITE
    int radius_squared = radius * radius;
    int dy = ty;
    while (dy <= by)
    {
        if ((dy > -1) && (dy < height))
        {
            int dy_contribution = dy * width * 3;
            int yy = dy - y;
            int dx = tx;
            
            yy *= yy;
            
            while (dx <= bx)
            {
                if ((dx > -1) && (dx < width))
                {
                    int dx_contribution = dx * 3;
                    int xx = dx - x;
                    xx *= xx;
                    if ((xx + yy) < (radius_squared/2))
                    {
                        img[dy_contribution + dx_contribution + 0] = s->trace_colour[0];
                        img[dy_contribution + dx_contribution + 1] = s->trace_colour[1];
                        img[dy_contribution + dx_contribution + 2] = s->trace_colour[2];
                    }
                    else if ((xx + yy) < radius_squared)
                    {
                        img[dy_contribution + dx_contribution + 0] = s->trace_surround_colour[0];
                        img[dy_contribution + dx_contribution + 1] = s->trace_surround_colour[1];
                        img[dy_contribution + dx_contribution + 2] = s->trace_surround_colour[2]; 
                    }
                }
                dx++;
            }
        }
        dy++;
    }
    
#else
	int xx,yy,dx,dy,dist,c,target,n,noise;

	double fraction, diff;
	unsigned int border_y = height*s->border_percent/100;
	unsigned int border_x = width*s->border_percent/100;	

	if (tx < 0) tx = 0;
	if (ty < 0) ty = 0;
    if (bx >= (int)width) bx = (int)width-1;
    if (by >= (int)height) by = (int)height-1;

	intensity_percent /= 100.0;

	for (xx = tx; xx <= bx; xx++) {
        if ((xx < (int)border_x) || (xx > (int)(width-border_x))) continue;

		dx = xx - x;
		for (yy = ty; yy <= by; yy++) {
            if ((yy < (int)border_y) || (yy > (int)(height-border_y))) continue;

			dy = yy - y;
			dist = (int)sqrt(dx*dx + dy*dy);
            if (dist < (int)radius) {
				n = (yy*width + xx)*3;
				if (dist > 1) {
					fraction = (radius - dist) / (double)radius;
				}
				else {
					fraction = 1;
				}
                noise = (rand()%60)-30;
				for (c = 0; c < 3; c++) {
					diff =
						(s->trace_colour[c] - s->trace_surround_colour[c]) *
						intensity_percent;
					target =
						s->trace_surround_colour[c] +
						(unsigned char)(diff * fraction) + noise;
					if (target > 255) target = 255;

					if (target > img[n+c]) {
						img[n+c] += (target - img[n+c])/2;
					}
				}
			}
		}
	}
#endif
}

static void scope_marking_point(scope * s,
                                int x, int y,
                                unsigned int radius,
                                unsigned char * img,
                                unsigned int width, unsigned int height)
{
	int xx,yy,dx,dy,dist,c,n,diff,v;
	int tx = x - radius;
	int ty = y - radius;
	int bx = x + radius;
	int by = y + radius;

	if (tx < 0) tx = 0;
	if (ty < 0) ty = 0;
    if (bx >= (int)width) bx = width-1;
    if (by >= (int)height) by = height-1;

	for (xx = tx; xx <= bx; xx++) {
		dx = xx - x;
		for (yy = ty; yy <= by; yy++) {
			dy = yy - y;
			dist = (int)sqrt(dx*dx + dy*dy);
            if (dist < (int)radius) {
				n = (yy*width + xx)*3;
				for (c = 0; c < 3; c++) {
					diff = s->background_colour[c] - s->markings_colour[c];
					v = img[n+c] - diff;
					if (v < 30) v = 30;
					img[n+c] = v;
				}
			}
		}
	}
}

/* draws a dotted screen marking */
static void scope_dotted_line(scope * s,
                              int x0, int y0,
                              int x1, int y1,
                              int dots,
                              unsigned int radius,
                              unsigned char * img,
                              unsigned int width, unsigned int height)
{
	int i, x, y;
	int dx = x1 - x0;
	int dy = y1 - y0;

	for (i = 0; i <= dots; i++) {
		x = x0 + (dx*i/dots);
		y = y0 + (dy*i/dots);
		scope_marking_point(s, x, y,
							radius, img, width, height);
	}
}

/* draws a solid screen marking */
static void scope_marking_line(scope * s,
                               int x0, int y0,
                               int x1, int y1,
                               unsigned char * img,
                               unsigned int width)
{
	int dx = (int)x1 - (int)x0;
	int dy = (int)y1 - (int)y0;
	int length = (int)sqrt(dx*dx + dy*dy);
	int i, x, y, c, diff, v;
	unsigned int n;

	for (i = 0; i <= length; i++) {
		x = (int)x0 + (i*dx/length);
		y = (int)y0 + (i*dy/length);
		n = (y*width + x)*3;
		for (c = 0; c < 3; c++) {
			diff = s->background_colour[c] - s->markings_colour[c];
			v = img[n+c] - diff;
			if (v < 10) v = 10;
			img[n+c] = v;
		}
	}
}

/* draws a solid screen marking */
static void scope_marking(scope * s,
                          int x0, int y0,
                          int x1, int y1,
                          int thickness,
                          unsigned char * img,
                          unsigned int width)
{
	int dx = (int)x1 - (int)x0;
	int dy = (int)y1 - (int)y0;
	int x,y,offset = thickness/2;

	if (dx < 0) dx = -dx;
	if (dy < 0) dy = -dy;

	if (dx > dy) {
		for (y = -thickness; y < 0; y++) {
			scope_marking_line(s, x0,  y0+y+offset, x1, y1+y+offset,
                               img, width);
		}
	}
	else {
		for (x = -thickness; x < 0; x++) {
			scope_marking_line(s, x0+x+offset,  y0, x1+x+offset, y1,
                               img, width);
		}
	}
}

/* draws the increment screen markings */
static void scope_increments(scope * s,
                             int x0, int y0,
                             int x1, int y1,
                             int increments,
                             unsigned int radius,
                             unsigned int thickness,
                             unsigned char * img,
                             unsigned int width)
{
	int i, x, y;
	int dx = x1 - x0;
	int dy = y1 - y0;

	for (i = 0; i <= increments; i++) {
		x = x0 + (dx*i/increments);
		y = y0 + (dy*i/increments);

		if (abs(dx) > abs(dy)) {
			scope_marking(s, x, y-radius, x, y+radius-1,
                          thickness, img, width);
		}
		else {
			scope_marking(s, x-radius, y, x+radius-1, y,
                          thickness, img, width);
		}
	}
}

/* draws the screen marking grid */
static void scope_grid(scope * s,
                       unsigned int cells_x, unsigned int cells_y,
                       int radius,
                       int thickness,
                       unsigned char * img,
                       unsigned int width, unsigned int height)
{
	unsigned int x,y,xx,yy,h;
	unsigned int border_y = height*s->border_percent/100;
	unsigned int border_x = width*s->border_percent/100;	

	for (y = 0; y <= cells_y; y++) {
		yy = border_y + ((height-(border_y*2))*y/cells_y);
		scope_marking(s, border_x, yy, width-border_x, yy, thickness,
                      img, width);
	}

	for (x = 0; x <= cells_x; x++) {
		xx = border_x + ((width-(border_x*2))*x/cells_x);
		scope_marking(s, xx, border_y, xx, height-border_y, thickness,
                      img, width);
	}

	h = border_y + ((height-(border_y*2))*20/100);
	scope_dotted_line(s, border_x, h, width-border_x, h,
					  cells_x*5, thickness,
					  img, width, height);

	h = border_y + ((height-(border_y*2))*80/100);
	scope_dotted_line(s, border_x, h, width-border_x, h,
					  cells_x*5, thickness,
					  img, width, height);
	
	h = border_x + ((width-(border_x*2))*50/100);
	scope_increments(s, h, border_y, h, height-border_y,
					 cells_y*5, radius/2, thickness,
                     img, width);

	h = border_y + ((height-(border_y*2))*50/100);
	scope_increments(s, border_x, h, width-border_x, h,
					 cells_x*5, radius/2, thickness,
                     img, width);
}

/* traces a beam line on the screen with the given intensity and radius */
static void scope_trace_line(scope * s,
                             int x0, int y0,
                             int x1, int y1,
                             unsigned int radius, double intensity_percent,
                             unsigned char * img,
                             unsigned int width, unsigned int height)
{
	int dx = (int)x1 - (int)x0;
	int dy = (int)y1 - (int)y0;
	int length = (int)sqrt(dx*dx + dy*dy);
	int i, x, y, prev_x=-9999, prev_y=-9999;

	for (i = 0; i <= length; i++) {
		x = (int)x0 + (i*dx/(length+1));
		y = (int)y0 + (i*dy/(length+1));
		if (!((x==prev_x) && (y==prev_y))) {
			scope_point(s, x, y, radius, intensity_percent,
						img, width, height);		
			prev_x = x;
			prev_y = y;
		}
	}
}

/* returns various vertical parameters */
static void scope_verticals(scope * s, unsigned int trace_index,
                            unsigned int no_of_traces,
                            double * min, double * max,
                            int * screen_by, int * screen_ty,
                            unsigned int height)
{
    unsigned int border_y = height * s->border_percent / 100;
    double vertical_percent;

    if (trace_index == 0) {
        *min = s->trace1_min;
        *max = s->trace1_max;
        vertical_percent = s->vertical_percent[0];
    }
    else {
        *min = s->trace2_min;
        *max = s->trace2_max;
        vertical_percent = s->vertical_percent[1];
    }

    if (no_of_traces == 1) {
        *screen_by = height - border_y -
            (int)((height-(border_y*2))*(20+vertical_percent)/100);
        *screen_ty = height - border_y -
            (int)((height-(border_y*2))*(80+vertical_percent)/100);
        if (*screen_ty < (int)border_y) *screen_ty = border_y;
    }
    else {
        if (trace_index == 0) {
            *screen_by = height - border_y -
                (int)((height-(border_y*2))*(60+vertical_percent)/100);
            *screen_ty = height - border_y -
                (int)((height-(border_y*2))*(85+vertical_percent)/100);
            if (*screen_ty < (int)border_y) *screen_ty = border_y;
        }
        else {
            *screen_by = height - border_y -
                (int)((height-(border_y*2))*(20+vertical_percent)/100);
            *screen_ty = height - border_y -
                (int)((height-(border_y*2))*(45+vertical_percent)/100);
        }
    }
}

/* traces a beam on the screen with the given intensity and radius */
static void scope_trace(scope * s,
                        unsigned int trace_index,
                        unsigned int radius, double intensity_percent,
                        unsigned char * img,
                        unsigned int width, unsigned int height)
{
	int x,y,prev_x=-99999,prev_y=-99999, screen_by, screen_ty,n;
    unsigned int t_ms=0, border_x, t;
    double value, min, max;

	border_x = width * s->border_percent / 100;

    scope_verticals(s, trace_index, s->no_of_traces,
                    &min, &max,
                    &screen_by, &screen_ty,
                    height);

	while (t_ms < s->time_ms) {
		n = (int)t_ms - s->offset_ms;
		if (n < 0) n += (int)s->time_ms;
		t = (((unsigned int)n)%s->time_ms) / s->step_ms;
		if (trace_index == 0) {
            value = s->trace1[t] + (((rand()%10000)-5000)/5000.0*s->noise);
		}
		else {
            value = s->trace2[t] + (((rand()%10000)-5000)/5000.0*s->noise);
		}
        if ((value == PHOSPHENE_NO_TRACE) || (max <= min)) {
            t_ms += s->step_ms;
            continue;
        }

		x = border_x + (t_ms * (width-(border_x*2)) / s->time_ms);
		y = screen_by - (int)((screen_by - screen_ty)*(value-min)/(max - min));

		scope_trace_line(s, prev_x, prev_y, x, y,
						 radius, intensity_percent -
						 (intensity_percent*0.4*t_ms/s->time_ms),
						 img, width, height);

		prev_x = x;
		prev_y = y;
		t_ms += s->step_ms;
	}
}

/* draws a vertical or horizontal alignment marker on the screen */
static void scope_marker(scope * s, unsigned char * img,                         
                         unsigned int width, unsigned int height)
{
    unsigned int border_x, border_y, x, y;
    unsigned int i, trace_index = 0;
    double min=0, max=1;
    int screen_by, screen_ty;

	if (s->marker_position == 0) return;

	border_x = width * s->border_percent / 100;
	border_y = height * s->border_percent / 100;

    if (s->marker_orientation == PHOSPHENE_MARKER_VERTICAL) {
        x = border_x + (s->marker_position * (width-(border_x*2)) / s->time_ms);
        for (i = 0; i < s->marker_thickness; i++) {
            scope_trace_line(s, x+i, border_y, x+i, height - border_y,
                             1, 50, img, width, height);
        }
    }
    else {
        scope_verticals(s, trace_index, s->no_of_traces,
                        &min, &max,
                        &screen_by, &screen_ty,
                        height);

        y = screen_by - (int)((screen_by - screen_ty)*
            (s->marker_position-min)/(max - min));
        for (i = 0; i < s->marker_thickness; i++) {
            scope_trace_line(s, border_x, y+i, width-border_x, y+i,
                             1, 50, img, width, height);
        }
    }

}

static void scope_background(scope * s, unsigned char * img,
                             unsigned int width, unsigned int height)
{
	int x, y, n=0;
#ifndef PHOSPHENE_LITE
    int dx, dy, diff, c;
	int cx = width/2;
	int cy = height/2;
	double fraction_x, fraction_y, fraction;
#endif
    for (y = 0; y < (int)height; y++) {
#ifndef PHOSPHENE_LITE
		dy = y - cy;
		if (dy < 0) dy = -dy;
		fraction_y = cos(dy * 0.5 * 3.1415927 / (double)cy);
#endif
        for (x = 0; x < (int)width; x++, n+=3) {
#ifndef PHOSPHENE_LITE
			dx = x - cx;
			if (dx < 0) dx = -dx;
			fraction_x = cos(dx * 0.5 * 3.1415927 / (double)cx);
			if (fraction_x < fraction_y) {
				fraction = 1.0 - fraction_x;
			}
			else {
				fraction = 1.0 - fraction_y;
			}
			fraction = 1.0 - (fraction*fraction*fraction*fraction);
			for (c = 0; c < 3; c++) {
				diff = s->background_colour[c] - s->background_border_colour[c];
				img[n+c] = s->background_border_colour[c] + (int)(diff*fraction);
			}
#else
            img[n+0] = s->background_colour[0];
            img[n+1] = s->background_colour[1];
            img[n+2] = s->background_colour[2];            
#endif
		}
	}
}

/* plots traces 0 and 1 together */
static void scope_xy(scope * s,
                     unsigned int radius, double intensity_percent,
                     unsigned char draw_lines,
                     unsigned char * img,
                     unsigned int width, unsigned int height)
{
    int x, y, prev_x = -9999, prev_y = -9999;
    int screen_bx, screen_tx, screen_by, screen_ty, n;
    unsigned int t_ms=0, t;
    double value_x, value_y, min_x, max_x, min_y, max_y;

    /* limits of trace 0 */
    scope_verticals(s, 0, 1,
                    &min_x, &max_x,
                    &screen_bx, &screen_tx,
                    width);

    /* limits of trace 1 */
    scope_verticals(s, 1, 1,
                    &min_y, &max_y,
                    &screen_by, &screen_ty,
                    height);

    if ((max_x <= min_x) || (max_y <= min_y)) return;

    while (t_ms < s->time_ms) {
        n = (int)t_ms - s->offset_ms;
        if (n < 0) n += (int)s->time_ms;
        t = (((unsigned int)n)%s->time_ms) / s->step_ms;
        value_x = s->trace1[t] + (((rand()%10000)-5000)/5000.0*s->noise);
        value_y = s->trace2[t] + (((rand()%10000)-5000)/5000.0*s->noise);
        if ((value_x == PHOSPHENE_NO_TRACE) ||
            (value_y == PHOSPHENE_NO_TRACE)) {
            t_ms += s->step_ms;
            continue;
        }

        x = screen_bx - (int)((screen_bx - screen_tx)*(value_x-min_x)/(max_x - min_x));
        y = screen_by - (int)((screen_by - screen_ty)*(value_y-min_y)/(max_y - min_y));

        if (draw_lines == 0) {
            scope_point(s, x, y,
                        radius, intensity_percent,
                        img, width, height);
        }
        else {
            scope_trace_line(s, prev_x, prev_y, x, y,
                             radius, intensity_percent,
                             img, width, height);
        }

        prev_x = x;
        prev_y = y;
        t_ms += s->step_ms;
    }
}

/* draws the background */
static void scope_draw_background(scope * s,
                                  int grid_x, int grid_y,
                                  unsigned int radius,
                                  unsigned char * img,
                                  unsigned int width, unsigned int height)
{
    int thickness = width/320;

    if (thickness < 1) thickness = 1;

    /* background */
    scope_background(s, img, width, height);

    scope_grid(s, grid_x, grid_y, radius, thickness, img, width, height);
}


/* the main drawing function */
void scope_draw(scope * s,
                unsigned char draw_type,
                double intensity_percent,
                int grid_x, int grid_y,
                unsigned char * img,
                unsigned int width,	unsigned int height)
{
	unsigned int i;
	unsigned int radius = width/(320/5);

    if (radius < 1) radius = 1;

    /* draw the background */
    if ((draw_type == PHOSPHENE_DRAW_ALL) ||
        (draw_type == PHOSPHENE_DRAW_BACKGROUND)) {
        scope_draw_background(s, grid_x, grid_y, radius,
                              img, width, height);
    }

    if ((draw_type == PHOSPHENE_DRAW_ALL) ||
        (draw_type == PHOSPHENE_DRAW_FOREGROUND)) {
        /* check that the time base doesn't exceed the maximum */
        if (s->time_ms/s->step_ms >= PHOSPHENE_MAX_TIME_STEPS) {
            s->time_ms = (PHOSPHENE_MAX_TIME_STEPS-1) * s->step_ms;
        }

        /* if a trigger voltage is set then check for it
           and adjust the marker position accordingly */
        if (s->trigger_voltage > 0) {
            for (i = 1; i < s->time_ms/s->step_ms; i++) {
                if ((s->trace1[i-1] < s->trigger_voltage) &&
                    (s->trace1[i] > s->trigger_voltage)) {
                    s->offset_ms = -(i*s->step_ms) +
                        (int)(s->time_ms*0.05);
                    s->marker_position = s->time_ms*0.05;
                    break;
                }
            }
        }

        /* show marker */
        scope_marker(s, img, width, height);

        /* draw traces */
        switch(s->mode) {
        case PHOSPHENE_MODE_DEFAULT: {
            for (i = 0; i < s->no_of_traces; i++) {
                scope_trace(s, i, radius, intensity_percent,
                            img, width, height);
            }
            break;
        }
        case PHOSPHENE_MODE_XY: {
            scope_xy(s, radius, intensity_percent, 1,
                     img, width, height);
            break;
        }
        case PHOSPHENE_MODE_POINTS: {
            scope_xy(s, radius, intensity_percent, 0,
                     img, width, height);
            break;
        }
        }
    }
}
