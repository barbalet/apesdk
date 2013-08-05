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

#ifndef PHOSPHENE_H
#define PHOSPHENE_H

#define PHOSPHENE_MAX_TIME_STEPS 60000
#define PHOSPHENE_NO_TRACE -999

enum {
    PHOSPHENE_MODE_DEFAULT = 0,
    PHOSPHENE_MODE_XY,
    PHOSPHENE_MODE_POINTS
};

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

struct scope_struct {
    unsigned char mode;
    unsigned int no_of_traces;
    unsigned int border_percent;
    double vertical_percent[2];
    double trace1_min, trace1_max;
    double trace2_min, trace2_max;
    double trace1[PHOSPHENE_MAX_TIME_STEPS];
    double trace2[PHOSPHENE_MAX_TIME_STEPS];
    int offset_ms;
    unsigned int time_ms, step_ms;
    unsigned int trace1_scan_ms, trace2_scan_ms;
    double marker_ms;
    double trigger_voltage;
    unsigned char background_colour[3];
    unsigned char background_border_colour[3];
    unsigned char markings_colour[3];
    unsigned char trace_colour[3];
    unsigned char trace_surround_colour[3];
    double noise;
};
typedef struct scope_struct scope;

scope create_scope(unsigned int step_ms);
void scope_clear(scope * s);
void scope_update(scope * s,
                  unsigned int trace_index,
                  double value, double min, double max,
                  unsigned int t_ms);
void scope_draw(scope * s,
                double intensity_percent,
                int grid_x, int grid_y,
                unsigned char * img,
                unsigned int width,	unsigned int height);

#endif
