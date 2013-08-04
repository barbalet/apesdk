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

scope create_scope(unsigned int step_ms)
{
	scope s;

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

	s.marker_ms = 0;
	s.trace1_scan_ms = 0;
	s.trace2_scan_ms = 0;
	s.time_ms = 2000;

	s.trace1[0] = NO_TRACE;
	s.trace2[0] = NO_TRACE;

	s.offset_ms = 0;
	s.trigger_voltage = 0;

	return s;
}

void scope_clear(scope * s)
{
	s->trace1[0] = NO_TRACE;
	s->trace2[0] = NO_TRACE;
}

void scope_update(scope * s,
                  unsigned int trace_index,
                  double value, double min, double max,
                  unsigned int t_ms)
{
	unsigned int t = t_ms/s->step_ms;

	if (t >= MAX_TIME_STEPS) return;

	if (t_ms < s->time_ms) {
		if (trace_index == 0) {		
			s->trace1[t] = value;
			s->trace1_min = min;
			s->trace1_max = max;
			s->trace1_scan_ms = t_ms;
		}
		else {
			s->trace2[t] = value;
			s->trace2_min = min;
			s->trace2_max = max;
			s->trace2_scan_ms = t_ms;
		}
	}
}

/* draws a single point */
static void scope_point(scope * s,
                        int x, int y,
                        unsigned int radius, double intensity_percent,
                        unsigned char * img,
                        unsigned int width, unsigned int height)
{
	int xx,yy,dx,dy,dist,c,target,n,noise;
	int tx = x - radius;
	int ty = y - radius;
	int bx = x + radius;
	int by = y + radius;
	double fraction, diff;
	unsigned int border_y = height*s->border_percent/100;
	unsigned int border_x = width*s->border_percent/100;	

	if (tx < 0) tx = 0;
	if (ty < 0) ty = 0;
	if (bx >= width) bx = width-1;
	if (by >= height) by = height-1;

	intensity_percent /= 100.0;

	for (xx = tx; xx <= bx; xx++) {
		if ((xx < border_x) || (xx > width-border_x)) continue;

		dx = xx - x;
		for (yy = ty; yy <= by; yy++) {
			if ((yy < border_y) || (yy > height-border_y)) continue;

			dy = yy - y;
			dist = (int)sqrt(dx*dx + dy*dy);
			if (dist < radius) {
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
	if (bx >= width) bx = width-1;
	if (by >= height) by = height-1;

	for (xx = tx; xx <= bx; xx++) {
		dx = xx - x;
		for (yy = ty; yy <= by; yy++) {
			dy = yy - y;
			dist = (int)sqrt(dx*dx + dy*dy);
			if (dist < radius) {
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

static void scope_marking_line(scope * s,
                               int x0, int y0,
                               int x1, int y1,
                               unsigned char * img,
                               unsigned int width, unsigned int height)
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

static void scope_marking(scope * s,
                          int x0, int y0,
                          int x1, int y1,
                          int thickness,
                          unsigned char * img,
                          unsigned int width, unsigned int height)
{
	int dx = (int)x1 - (int)x0;
	int dy = (int)y1 - (int)y0;
	int x,y,offset = thickness/2;

	if (dx < 0) dx = -dx;
	if (dy < 0) dy = -dy;

	if (dx > dy) {
		for (y = -thickness; y < 0; y++) {
			scope_marking_line(s, x0,  y0+y+offset, x1, y1+y+offset,
							   img, width, height);
		}
	}
	else {
		for (x = -thickness; x < 0; x++) {
			scope_marking_line(s, x0+x+offset,  y0, x1+x+offset, y1,
							   img, width, height);
		}
	}
}

static void scope_increments(scope * s,
                             int x0, int y0,
                             int x1, int y1,
                             int increments,
                             unsigned int radius,
                             unsigned int thickness,
                             unsigned char * img,
                             unsigned int width, unsigned int height)
{
	int i, x, y;
	int dx = x1 - x0;
	int dy = y1 - y0;

	for (i = 0; i <= increments; i++) {
		x = x0 + (dx*i/increments);
		y = y0 + (dy*i/increments);

		if (abs(dx) > abs(dy)) {
			scope_marking(s, x, y-radius, x, y+radius-1,
						  thickness, img, width, height);
		}
		else {
			scope_marking(s, x-radius, y, x+radius-1, y,
						  thickness, img, width, height);
		}
	}
}

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
					  img, width, height);
	}

	for (x = 0; x <= cells_x; x++) {
		xx = border_x + ((width-(border_x*2))*x/cells_x);
		scope_marking(s, xx, border_y, xx, height-border_y, thickness,
					  img, width, height);
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
					 img, width, height);

	h = border_y + ((height-(border_y*2))*50/100);
	scope_increments(s, border_x, h, width-border_x, h,
					 cells_x*5, radius/2, thickness,
					 img, width, height);
}

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

static void scope_trace(scope * s,
                        unsigned int trace_index,
                        unsigned int radius, double intensity_percent,
                        unsigned char * img,
                        unsigned int width, unsigned int height)
{
	int x,y,prev_x=-99999,prev_y=-99999, screen_by, screen_ty,n;
	unsigned int t_ms=0, border_x, border_y, t;
	double value, min, max, vertical_percent;

	border_x = width * s->border_percent / 100;
	border_y = height * s->border_percent / 100;

	while (t_ms < s->time_ms) {
		n = (int)t_ms - s->offset_ms;
		if (n < 0) n += (int)s->time_ms;
		t = (((unsigned int)n)%s->time_ms) / s->step_ms;
		if (trace_index == 0) {
			value = s->trace1[t];
			min = s->trace1_min;
			max = s->trace1_max;
			vertical_percent = s->vertical_percent[0];
		}
		else {
			value = s->trace2[t];
			min = s->trace2_min;
			max = s->trace2_max;
			vertical_percent = s->vertical_percent[1];
		}
        if ((value == NO_TRACE) || (max <= min)) {
            t_ms += s->step_ms;
            continue;
        }

		if (s->no_of_traces == 1) {
			screen_by = height - border_y -
				(int)((height-(border_y*2))*(20+vertical_percent)/100);
			screen_ty = height - border_y - 
				(int)((height-(border_y*2))*(80+vertical_percent)/100);
			if (screen_ty < border_y) screen_ty = border_y;
		}
		else {
			if (trace_index == 0) {
				screen_by = height - border_y -
					(int)((height-(border_y*2))*(60+vertical_percent)/100);
				screen_ty = height - border_y - 
					(int)((height-(border_y*2))*(85+vertical_percent)/100);
				if (screen_ty < border_y) screen_ty = border_y;
			}
			else {
				screen_by = height - border_y -
					(int)((height-(border_y*2))*(20+vertical_percent)/100);
				screen_ty = height - border_y -
					(int)((height-(border_y*2))*(45+vertical_percent)/100);
			}
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

static void scope_marker(scope * s, unsigned char * img,
                         unsigned int width, unsigned int height)
{
	unsigned int border_x, border_y, x;

	if (s->marker_ms == 0) return;

	border_x = width * s->border_percent / 100;
	border_y = height * s->border_percent / 100;

	x = border_x + (s->marker_ms * (width-(border_x*2)) / s->time_ms);
	scope_trace_line(s, x, border_y, x, height - border_y,
					 1, 50, img, width, height);

}

static void scope_background(scope * s, unsigned char * img,
                             unsigned int width, unsigned int height)
{
	int x, y, dx, dy, n=0,c,diff;
	int cx = width/2;
	int cy = height/2;
	double fraction_x, fraction_y, fraction;

	for (y = 0; y < height; y++) {
		dy = y - cy;
		if (dy < 0) dy = -dy;
		fraction_y = cos(dy * 0.5 * 3.1415927 / (double)cy);
		for (x = 0; x < width; x++, n+=3) {
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
		}
	}
}


void scope_draw(scope * s,
                double intensity_percent,
                int grid_x, int grid_y,
                unsigned char * img,
                unsigned int width,	unsigned int height)
{
	unsigned int i;
	unsigned int radius = width/(320/5);
	int thickness = width/320;

	if (s->time_ms/s->step_ms >= MAX_TIME_STEPS) {
		s->time_ms = (MAX_TIME_STEPS-1) * s->step_ms;
	}

	if (s->trigger_voltage > 0) {
		for (i = 1; i < s->time_ms/s->step_ms; i++) {
			if ((s->trace1[i-1] < s->trigger_voltage) &&
				(s->trace1[i] > s->trigger_voltage)) {
				s->offset_ms = -(i*s->step_ms) +
					(int)(s->time_ms*0.05);
				s->marker_ms = s->time_ms*0.05;
				break;
			}
		}
	}

	/* background */
	scope_background(s, img, width, height);

	scope_grid(s, grid_x, grid_y, radius, thickness, img, width, height);

	/* show marker */
	scope_marker(s, img, width, height);

	/* draw traces */
	for (i = 0; i < s->no_of_traces; i++) {
		scope_trace(s, i, radius, intensity_percent,
					img, width, height);
	}
}
