/****************************************************************

 graph.c

 =============================================================

 Copyright 1996-2013 Tom Barbalet. All rights reserved.

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

#ifndef	_WIN32

#include "../noble/noble.h"
#include "../universe/universe.h"
#include "../entity/entity.h"

#else

#include "..\noble\noble.h"
#include "..\universe.h\universe.h"
#include "..\entity\entity.h"


#endif

#include <math.h>

#include "phosphene.h"

#define MAX_SKELETON_DIAGRAM_POINTS 4000

static n_int  graph_state = GC_VASCULAR;
static n_byte graph_clear = 1;

enum
{
    SKELETON_NECK = 0,
    SKELETON_LUMBAR,
    SKELETON_PELVIS,
    SKELETON_LEFT_HIP,
    SKELETON_RIGHT_HIP,
    SKELETON_LEFT_KNEE,
    SKELETON_RIGHT_KNEE,
    SKELETON_LEFT_ANKLE,
    SKELETON_RIGHT_ANKLE,
    SKELETON_LEFT_SHOULDER,
    SKELETON_RIGHT_SHOULDER,
    SKELETON_LEFT_SHOULDER_SOCKET,
    SKELETON_RIGHT_SHOULDER_SOCKET,
    SKELETON_LEFT_ELBOW,
    SKELETON_RIGHT_ELBOW,
    SKELETON_LEFT_WRIST,
    SKELETON_RIGHT_WRIST,
    SKELETON_LEFT_COLLAR,
    SKELETON_RIGHT_COLLAR,
    SKELETON_POINTS
};

enum BODY_BONES
{
    BONE_ARM_UPPER = 0,
    BONE_ARM_LOWER1,
    BONE_ARM_LOWER2,
    BONE_CLAVICAL,
    BONE_LEG_UPPER,
    BONE_LEG_LOWER1,
    BONE_LEG_LOWER2,
    BONE_PELVIS,
    BONE_HAND,
    BONE_FINGER,
    BONE_VERTIBRA,
    BONE_SCAPULA,
    BONE_RIBS,
    BONES
};

#define SKELETON_LUMBAR_VERTIBRA  4
#define SKELETON_LUMBAR_VERTIBRA2 8
#define SKELETON_VERTIBRA_RIBS    10

static void graph_erase(n_byte * buffer, n_int img_width, n_int img_height)
{
    n_int i;
    for (i = 0; i < img_width*img_height*3; i++)
    {
        buffer[i] = 255;
    }
}

/* draws a line */
static void graph_line(n_byte * buffer,
                       n_int img_width, n_int img_height,
                       n_int prev_x, n_int prev_y, n_int x, n_int y,
                       n_byte r, n_byte g, n_byte b,
                       n_byte thickness)
{
    n_int i,max;
    
    n_int dx = x-prev_x;
    n_int dy = y-prev_y;
    n_int abs_dx = dx;
    n_int abs_dy = dy;
    
    
    if (dx < 0) abs_dx = -dx;
    if (dy < 0) abs_dy = -dy;
    
    max = abs_dx;
    if (abs_dy > max) max = abs_dy;
    
    for (i=0; i<max; i++)
    {
        n_int xx = prev_x + (i*(x-prev_x)/max);
        if ((xx > -1) && (xx < img_width))
        {
            n_int yy = prev_y + (i*(y-prev_y)/max);
            if ((yy > -1) && (yy < img_height))
            {
                n_int n = (yy*img_width + xx)*3;
                buffer[n] = r;
                buffer[n+1] = g;
                buffer[n+2] = b;
            }
        }
    }
}


/**
 * @brief Draws a curve using three points
 * @param buffer Image buffer (three bytes per pixel)
 * @param img_width Width of the image
 * @param img_height Height of the image
 * @param x0 x coordinate of the start point
 * @param y0 y coordinate of the start point
 * @param x1 x coordinate of the middle point
 * @param y1 y coordinate of the middle point
 * @param x0 x coordinate of the end point
 * @param y0 y coordinate of the end point
 * @param r red
 * @param g green
 * @param b blue
 * @param radius_percent Radius of the curve as a percentage
 * @param start_thickness Thickness of the curve at the start point
 * @param end_thickness Thickness of the curve at the end point
 */
static void graph_curve(n_byte * buffer,
                        n_int img_width, n_int img_height,
                        n_int x0, n_int y0,
                        n_int x1, n_int y1,
                        n_int x2, n_int y2,
                        n_byte r, n_byte g, n_byte b,
                        n_byte radius_percent,
                        n_uint start_thickness,
                        n_uint end_thickness)
{
    n_int pts[8], x, y, prev_x=0, prev_y=0;
    n_uint i;
    const n_uint divisions = 20;
    double c[5],d[5],f;
    
    /** turn three points into four using the curve radius */
    pts[0] = x0;
    pts[1] = y1;
    
    pts[2] = x1 + ((x0 - x1)*radius_percent/100);
    pts[3] = y1 + ((y0 - y1)*radius_percent/100);
    
    pts[4] = x1 + ((x2 - x1)*radius_percent/100);
    pts[5] = y1 + ((y2 - y1)*radius_percent/100);
    
    pts[6] = x2;
    pts[7] = y2;
    
    c[0] = (-pts[0*2] + 3 * pts[1*2] - 3 * pts[2*2] + pts[3*2]) / 6.0;
    c[1] = (3 * pts[0*2] - 6 * pts[1*2] + 3 * pts[2*2]) / 6.0;
    c[2] = (-3 * pts[0*2] + 3 * pts[2*2]) / 6.0;
    c[3] = (pts[0*2] + 4 * pts[1*2] + pts[2*2]) / 6.0;
    
    d[0] = (-pts[(0*2)+1] + 3 * pts[(1*2)+1] - 3 * pts[(2*2)+1] + pts[(3*2)+1]) / 6.0;
    d[1] = (3 * pts[(0*2)+1] - 6 * pts[(1*2)+1] + 3 * pts[(2*2)+1]) / 6.0;
    d[2] = (-3 * pts[(0*2)+1] + 3 * pts[(2*2)+1]) / 6.0;
    d[3] = (pts[(0*2)+1] + 4 * pts[(1*2)+1] + pts[(2*2)+1]) / 6.0;
    
    for (i = 0; i < divisions; i++)
    {
        f = (double)i / (double)divisions;
        x = (n_int)((c[2] + f * (c[1] + f * c[0])) * f + c[3]);
        y = (n_int)((d[2] + f * (d[1] + f * d[0])) * f + d[3]);
        
        if (i > 0)
        {
            graph_line(buffer, img_width, img_height,
                       prev_x, prev_y, x, y,
                       r, g, b,
                       start_thickness +
                       ((end_thickness - start_thickness) * i / divisions));
        }
        prev_x = x;
        prev_y = y;
    }
    
}

#define  MAX_POLYGON_CORNERS 1000

/**
 * @brief Draw a filled polygon
 * @param points Array containing 2D points
 * @param no_of_points The number of 2D points
 * @param r Red
 * @param g Green
 * @param b Blue
 * @param transparency Degree of transparency
 * @param buffer Image buffer (3 bytes per pixel)
 * @param img_width Image width
 * @param img_height Image height
 */
static void graph_fill_polygon(n_vect2 * points, n_int no_of_points,
                               n_byte r, n_byte g, n_byte b, n_byte transparency,
                               n_byte * buffer, n_int img_width, n_int img_height)
{
    n_int nodes, nodeX[MAX_POLYGON_CORNERS], i, j, swap, n, x, y;
    n_int min_x = 99999, min_y = 99999;
    n_int max_x = -99999, max_y = -99999;
    
    for (i = 0; i < no_of_points; i++)
    {
        x = points[i].x;
        y = points[i].y;
        if ((x==9999) || (y==9999)) continue;
        if (x < min_x) min_x = x;
        if (y < min_y) min_y = y;
        if (x > max_x) max_x = x;
        if (y > max_y) max_y = y;
    }
    
    if (min_x < 0) min_x = 0;
    if (min_y < 0) min_y = 0;
    if (max_x >= img_width) max_x = img_width-1;
    if (max_y >= img_height) max_y = img_height-1;
    
    for (y = min_y; y <= max_y; y++)
    {
        /**  Build a list of nodes */
        nodes = 0;
        j = no_of_points-1;
        for (i = 0; i < no_of_points; i++)
        {
            if (((points[i].y < y) && (points[j].y >= y)) ||
                ((points[j].y < y) && (points[i].y >= y)))
            {
                nodeX[nodes++] =
                points[i].x + (y - points[i].y) *
                (points[j].x - points[i].x) /
                (points[j].y - points[i].y);
            }
            j = i;
            if (nodes == MAX_POLYGON_CORNERS) break;
        }
        
        /**  Sort the nodes, via a simple “Bubble” sort */
        i = 0;
        while (i < nodes-1)
        {
            if (nodeX[i] > nodeX[i+1])
            {
                swap = nodeX[i];
                nodeX[i] = nodeX[i+1];
                nodeX[i+1] = swap;
                if (i) i--;
            }
            else
            {
                i++;
            }
        }
        
        /**  Fill the pixels between node pairs */
        for (i = 0; i < nodes; i += 2)
        {
            if (nodeX[i] >= max_x) break;
            if (nodeX[i+1] > min_x)
            {
                /** range check */
                if (nodeX[i] <= min_x) nodeX[i] = min_x+1;
                if (nodeX[i+1] >= max_x) nodeX[i+1] = max_x-1;
                
                for (x = nodeX[i]; x < nodeX[i+1]; x++)
                {
                    n = ((y*img_width)+x)*3;
                    if (transparency == 0)
                    {
                        buffer[n] = b;
                        buffer[n+1] = g;
                        buffer[n+2] = r;
                    }
                    else
                    {
                        buffer[n]   = ((b*(255-transparency)) + (buffer[n]*transparency))/256;
                        buffer[n+1] = ((g*(255-transparency)) + (buffer[n+1]*transparency))/256;
                        buffer[n+2] = ((r*(255-transparency)) + (buffer[n+2]*transparency))/256;
                    }
                }
            }
        }
    }
}

const n_int bone_points[] =
{
    54, /* arm upper */
    44, /* arm lower1 */
    41, /* arm lower2 */
    33, /* clavical */
    58, /* leg upper */
    30, /* leg lower1 */
    37, /* leg lower2 */
    88, /* pelvis */
    24, /* hand */
    33, /* finger */
    36, /* vertibra */
    48, /* scapula */
    82  /* ribs */
};

const n_vect2 bone_arm_upper[] =
{
    {42,0},    {53,475}, /* axis */
    {49,0},    {16,8},    {9,21},    {2,37},    {5,50},    {10,64},  {14,77},  {14,95},  {12,143},  {12,193},
    {10,241},  {12,259},  {18,286},  {21,314},  {21,343},  {20,372}, {17,408}, {13,424}, {5,438},   {1,444},
    {3,458},   {14,470},  {18,472},  {28,473},  {42,477},  {52,476}, {63,469}, {75,477}, {92,477},  {90,458},
    {99,455},  {107,450}, {111,440}, {107,429}, {100,425}, {88,418}, {77,407}, {69,393}, {65,382},  {62,347},
    {60,313},  {56,270},  {51,218},  {52,174},  {52,127},  {54,88},  {57,71},  {64,68},  {69,54},   {75,47},
    {81,36},   {84,27},   {77,10},   {67,1}
};

const n_vect2 bone_arm_lower1[] =
{
    {34,4},   {59,346}, /* axis */
    {48,0},   {35,0},   {21,1},   {14,9},   {15,15},  {22,23},  {24,31},  {24,42},  {20,67},  {15,102},
    {9,132},  {4,157},  {2,177},  {3,197},  {7,225},  {11,258}, {16,290}, {18,308}, {19,327}, {20,337},
    {19,347}, {23,351}, {36,349}, {50,347}, {60,342}, {70,340}, {72,336}, {72,331}, {67,327}, {51,314},
    {46,305}, {40,285}, {33,259}, {31,233}, {28,208}, {28,183}, {30,166}, {37,131}, {48,77},  {52,60},
    {45,45},  {43,31},  {45,13},  {44,4}
};

const n_vect2 bone_arm_lower2[] =
{
    {34,4},   {59,346}, /* axis */
    {65,0},   {53,0},   {48,11},  {45,25},   {40,40},   {35,102},  {35,127},  {35,151},  {39,178},  {43,209},
    {48,237}, {54,266}, {58,299}, {60,317},  {55,329},  {53,339},  {54,343},  {69,344},  {80,346},  {88,349},
    {91,343}, {89,335}, {84,324}, {78,314},  {75,293},  {74,271},  {70,239},  {64,207},  {60,174},  {57,132},
    {58,112}, {62,77},  {65,58},  {72,48},   {74,35},   {85,27},   {95,23},   {98,16},   {96,8},    {86,5},
    {77,2}
};

const n_vect2 bone_clavical[] =
{
    {23,201}, {31,13}, /* axis */
    {26,4},   {20,12},   {19,22},   {24,30},   {30,39},   {29,53},   {24,77},   {15,100},   {10,114},   {6,130},
    {4,146},  {5,159},   {9,176},   {14,189},  {15,200},  {18,206},  {23,207},  {28,206},   {31,199},   {31,192},
    {27,182}, {25,173},  {22,156},  {22,143},  {24,126},  {28,110},  {39,88},   {45,68},    {49,52},    {50,40},
    {48,27},  {43,12},   {38,7}
};

const n_vect2 bone_leg_upper[] =
{
    {119,33},  {80,416}, /* axis */
    {47,3},    {36,7},   {30,15},   {27,22},   {23,43},   {19,73},   {18,90},   {20,112},   {22,140},   {23,178},
    {24,223},  {29,286}, {32,330},  {30,360},  {26,378},  {24,386},  {28,398},  {36,410},   {49,419},   {65,425},
    {78,422},  {84,417}, {91,410},  {102,405}, {114,408}, {126,408}, {133,401}, {134,389},  {135,373},  {125,362},
    {114,354}, {99,347}, {89,330},  {78,305},  {71,274},  {64,221},  {64,174},  {67,126},   {73,93},    {76,77},
    {87,67},   {99,63},  {111,59},  {116,61},  {121,63},  {126,58},  {127,51},  {122,38},   {117,22},   {109,14},
    {99,13},   {91,19},  {86,24},   {78,26},   {70,25},   {64,17},   {66,7},    {60,3}
};

const n_vect2 bone_leg_lower1[] =
{
    {94,5},   {46,287}, /* axis */
    {23,26},  {14,33},  {12,45},  {14,55},  {16,68},  {17,101},  {18,132},  {18,166},  {17,195},  {18,223},
    {20,241}, {17,251}, {11,260}, {5,269},  {5,275},  {14,280},  {25,284},  {33,283},  {37,275},  {36,261},
    {32,240}, {28,210}, {27,171}, {30,154}, {31,134}, {30,94},   {31,69},   {36,48},   {36,36},   {32,27}
};

const n_vect2 bone_leg_lower2[] =
{
    {94,5},   {46,287}, /* axis */
    {66,1},   {44,1},    {28,1},    {24,8},   {25,24},  {34,37},   {41,49},   {49,66},   {54,85},   {58,106},
    {59,130}, {59,165},  {57,196},  {51,227}, {45,245}, {40,254},  {36,266},  {39,280},  {47,287},  {59,287},
    {77,281}, {87,277},  {90,272},  {90,265}, {84,250}, {83,230},  {83,188},  {83,149},  {85,109},  {88,74},
    {92,47},  {100,38}, {110,29},  {116,18},  {114,6},  {98,5},    {80,1}
};

const n_vect2 bone_pelvis[] =
{
    {214,27},  {203,382}, /* axis */
    {213,24},  {183,21},    {166,22},    {156,18},    {139,12},    {118,6},    {100,6},   {79,8},    {68,11},    {60,19},
    {46,34},   {35,49},     {25,65},     {17,81},     {9,96},      {6,108},    {6,122},   {15,136},  {27,150},   {44,165},
    {59,181},  {71,197},    {75,221},    {78,240},    {77,255},    {84,254},   {97,259},  {104,271}, {107,284},  {106,295},
    {105,303}, {112,315},   {116,330},   {118,347},   {112,358},   {105,365},  {101,367}, {102,376}, {111,384},  {127,393},
    {141,393}, {158,390},   {177,387},   {189,381},   {201,369},   {213,359},  {215,336}, {216,349}, {218,358},  {225,370},
    {240,380}, {254,385},   {272,387},   {294,387},   {307,387},   {316,375},  {317,367}, {310,357}, {300,337},  {300,320},
    {308,308}, {316,304},   {321,294},   {320,283},   {322,275},   {321,266},  {330,259}, {342,254}, {349,245},  {348,217},
    {354,192}, {364,176},   {381,162},   {402,144},   {423,131},   {432,121},  {433,104}, {427,84},  {411,60},   {392,37},
    {377,22},  {360,11},    {336,7},     {311,11},    {277,18},    {258,25},   {244,26},  {222,24},
    {101,272}, /* left leg */
    {321,265}  /* right leg */
};

const n_vect2 bone_hand[] =
{
    {20,6},   {14,100}, /* axis */
    {19,4},   {12,6},   {9,12},   {9,23},    {10,40},   {9,59},    {8,75},    {6,86},    {3,91},    {8,98},
    {9,102},  {15,102}, {20,101}, {24,96},   {27,90},   {27,84},   {24,76},   {23,64},   {24,40},   {24,24},
    {26,18},  {29,10},  {28,4},   {23,3}
};

const n_vect2 bone_finger[] =
{
    {14,1},   {15,114}, /* axis */
    {13,1},   {6,3},   {4,6},   {3,10},    {6,14},    {6,26},    {6,40},    {8,52},    {9,56},    {8,60},
    {6,66},   {6,72},  {8,82},  {7,92},    {9,98},    {10,104},  {11,112},  {14,114},  {16,113},  {18,107},
    {19,100}, {19,92}, {20,86}, {20,78},   {20,68},   {23,60},   {25,52},   {26,44},   {24,34},   {23,18},
    {23,7},   {21,3},  {16,2}
};

const n_vect2 bone_vertibra[] =
{
    {53,1},  {53,47}, /* axis */
    {53,2},  {41,2},   {30,4},   {22,7},   {22,16},   {22,24},   {20,32},   {14,32},   {4,34},   {1,36},
    {1,42},  {5,46},   {14,48},  {18,48},  {18,54},   {24,53},   {33,50},   {43,48},   {53,49},  {62,50},
    {72,53}, {81,56},  {87,56},  {88,50},  {95,50},   {98,48},   {100,42},  {98,38},   {91,37},  {84,35},
    {84,33}, {84,24},  {86,14},  {84,10},  {74,6},    {62,0}
};

const n_vect2 bone_scapula[] =
{
    {35, 0},   {134,214}, /* axis */
    {37,3},    {22,7},    {11,15},    {6,21},    {4,28},    {4,40},    {9,58},    {19,80},    {38,113},    {60,142},
    {81,178},  {101,204}, {114,219},  {128,226}, {138,230}, {153,221}, {165,211}, {171,194},  {173,170},   {175,143},
    {177,116}, {176,97},  {168,81},   {177,69},  {187,60},  {188,41},  {184,23},  {175,13},   {162,11},    {151,12},
    {146,22},  {136,34},  {128,46},   {118,59},  {113,56},  {109,43},  {100,34},  {93,34},    {84,36},     {83,46},
    {75,51},   {67,52},   {58,42},    {58,34},   {48,27},   {51,19},   {52,12},   {45,6}
};

const n_vect2 bone_ribs[] =
{
    {227,11},  {212,350}, /* axis */
    {226,10},  {199,7},   {177,15},   {152,25},   {139,37},  {122,52},   {112,62},   {104,75},   {91,93},   {85,108},
    {76,127},  {69,149},  {62,169},   {61,183},   {53,208},  {53,218},   {44,230},   {39,244},   {40,255},  {36,262},
    {30,278},  {28,295},  {28,299},   {22,317},   {22,336},  {23,348},   {16,365},   {15,389},   {14,411},  {10,437},
    {13,449},  {18,446},  {18,434},   {33,425},   {56,407},  {83,392},   {116,382},  {141,373},  {159,362}, {173,351},
    {184,351}, {213,348}, {235,355},  {242,356},  {244,352}, {264,363},  {283,377},  {304,386},  {333,396}, {355,409},
    {375,423}, {395,443}, {409,458},  {416,450},  {419,440}, {415,429},  {417,404},  {411,383},  {408,366}, {411,348},
    {411,327}, {409,315}, {407,296},  {402,278},  {395,267}, {393,243},  {385,232},  {384,213},  {379,200}, {371,194},
    {367,172}, {359,155}, {357,134},  {344,114},  {334,90},  {322,80},   {318,67},   {301,47},   {289,30},  {275,22},
    {247,15},  {227,12},
    {35,3},    {416,3},   /* shoulder sockets */
    {35,-20},  {416,-20}, /* shoulders */
};

/**
 * @brief Returns an array of 2D points used for drawing diagrams
 * @param source_points Array of 2D points which is the template
 * @param no_of_source_points Number of 2D points in the template
 * @param extra_points The number of points to be returned via the extra parameters
 * @param x The starting x coordinate
 * @param y The starting y coordinate
 * @param mirror Flip in the vertical axis
 * @param scale_width Horizontal scaling factor x1000
 * @param scale_length Length (vertical) scaling factor x1000
 * @param angle Rotation angle of the result
 * @param axis_x Returned x coordinate such that (x,y)-(axis_x,axis_y) defines the axis of the object
 * @param axis_y Returned y coordinate such that (x,y)-(axis_x,axis_y) defines the axis of the object
 * @param extra_x1
 * @param extra_y1
 * @param extra_x2
 * @param extra_y2
 * @param extra_x3
 * @param extra_y3
 * @param extra_x4
 * @param extra_y4
 * @param points Returned 2D points
 * @param no_of_points Number of returned 2D points
 * @param max_points The maximum number of points which may be returned
 */
static void outline_points(const n_vect2 * source_points,
                           n_int no_of_source_points, n_int extra_points,
                           n_int x, n_int y,
                           n_byte mirror,
                           n_int scale_width, n_int scale_length,
                           n_int angle,
                           n_vect2 *axis,
                           n_vect2 *extra_1,
                           n_vect2 *extra_2,
                           n_vect2 *extra_3,
                           n_vect2 *extra_4,
                           n_vect2 * points, n_int * no_of_points,
                           n_int max_points)
{
    n_vect2  ds, scale, location, vector;
    n_int    i, axis_length,point_length;
    float axis_angle, point_angle;
    float ang = angle*TWO_PI/7200;
    
    vect2_populate(&location, x, y);
    vect2_populate(&scale, scale_width, scale_length);
    vect2_subtract(&ds, (n_vect2 *)&source_points[1], (n_vect2 *)source_points);
    vect2_multiplier(&ds, &ds, &scale, 1, 1000);
    
    /** length of the object */
    axis_length = (n_int)math_root(vect2_dot(&ds, &ds, 1, 1));
    if (axis_length < 1) axis_length=1;
    
    /** invert around the vertical axis if needed */
    if (mirror != 0)
    {
        ds.x = -ds.x;
    }
    
    /** find the orientation angle of the axis */
    axis_angle = (float)acos(ds.x/(float)axis_length);
    
    if (ds.y < 0)
    {
        axis_angle = TWO_PI-axis_angle;
    }
    
    vect2_populate(&vector, (n_int)(axis_length*sin(ang+(TWO_PI/4)-axis_angle)),
                   (n_int)(axis_length*cos(ang+(TWO_PI/4)-axis_angle)));
    
    /** calculate the position of the end point of the axis */
    
    vect2_add(axis, &location, &vector);
    
    /** draw lines between each point */
    for (i = 2; i < no_of_source_points + 2 + extra_points; i++)
    {
        n_vect2 point;
        vect2_subtract(&ds, (n_vect2 *)&source_points[i], (n_vect2 *)source_points);
        vect2_multiplier(&ds, &ds, &scale, 1, 1000);
        point_length = (n_int)math_root(vect2_dot(&ds, &ds, 1, 1));
        if (point_length < 1)
        {
            point_length = 1;
        }
        
        /** invert the line around the vertical axis if necessary */
        if (mirror != 0)
        {
            ds.x = -ds.x;
        }
        
        /** angle of the line */
        point_angle = (float)acos(ds.x/(float)point_length);
        if (ds.y < 0)
        {
            point_angle = (TWO_PI)-point_angle;
        }
        
        /** position of the end of the line */
        vect2_populate(&vector, (n_int)(point_length*sin(ang+point_angle-axis_angle)),
                       (n_int)(point_length*cos(ang+point_angle-axis_angle)));
        
        vect2_add(&point, &location, &vector);
        
        /** store the calculated point positions in an array */
        if (*no_of_points < max_points)
        {
            if (i < no_of_source_points + 2)
            {
                vect2_copy(&points[*no_of_points], &point);
                *no_of_points = *no_of_points + 1;
            }
        }
        else
        {
            (void)SHOW_ERROR("Maximum number of skeleton points reached");
        }
        
        /** This is a crude way of keeping track of the last few points
         so that they can be returned by the function */
        vect2_copy(extra_1, extra_2);
        vect2_copy(extra_2, extra_3);
        vect2_copy(extra_3, extra_4);
        vect2_copy(extra_4, &point);
    }
    
    /*printf("test points %d\n", *no_of_points);*/
    points[*no_of_points].x = 9999;
    points[*no_of_points].y = 9999;
    *no_of_points = *no_of_points + 1;
}

/**
 * @brief Returns vertical and horizontal scaling factors based upon the
 * genetics for a particular body segment
 * @param being Pointer to the being object
 * @param keypoint The index within the BONES enumeration
 * @param scale_width The returned width scale for the bone x1000
 * @param scale_length The returned length scale for the bone x1000
 */
void body_skeleton_gene(noble_being * being, n_byte keypoint, n_int * scale_width, n_int * scale_length)
{
    /** the maximum variation in body segment proportions,
     typically in the range 0-500 */
    const n_int MAX_SEGMENT_VARIANCE = 200;
    
    /** these are 4 bit gene values in the range 0-15 */
    n_byte gene_val1 = GENE_HOX(being_genetics(being),keypoint);
    n_byte gene_val2 = GENE_HOX(being_genetics(being),20-keypoint);
    
    /** convert the gene values into scaling values */
    *scale_width = 1000 - MAX_SEGMENT_VARIANCE + (((n_int)gene_val1)*MAX_SEGMENT_VARIANCE*2/15);
    *scale_length = 1000 - MAX_SEGMENT_VARIANCE + (((n_int)gene_val2)*MAX_SEGMENT_VARIANCE*2/15);
}

/**
 * @brief returns a set of points corresponding to key locations on the skeleton
 * @param being Pointer to the being object
 * @param keypoints Array which returns the x,y coordinates of each key point on the skeleton
 * @param points The returned 2D points of the skeleton diagram
 * @param shoulder_angle Angle of the shoulders in degrees
 * @param elbow_angle Angle of the elbows in degrees
 * @param wrist_angle Angle of the wrists in degrees
 * @param hip_angle Angle of the hips in degrees
 * @param knee_angle Angle of the knees in degrees
 * @return Number of 2D points within the skeleton diagram
 */
n_int graph_skeleton_points(noble_being * being, n_vect2 * keypoints, n_vect2 *points, n_int max_points,
                           n_int shoulder_angle, n_int elbow_angle, n_int wrist_angle,
                           n_int hip_angle, n_int knee_angle)
{
    n_int scale_width=1000, scale_length=1000, angle=0;
    n_vect2 extra[4];
    n_vect2 vertibra = {0,0};
    n_vect2 knuckle = {0,0};
    n_int i, vertical, no_of_points = 0;
    
    NA_ASSERT(being, "being NULL");
    NA_ASSERT(keypoints, "keypoints NULL");
    NA_ASSERT(points, "points NULL");
    
    /** position of the bottom of the neck */
    keypoints[SKELETON_NECK].x = 0;
    keypoints[SKELETON_NECK].y = 0;
    
    /** position of the bottom of the ribs */
    body_skeleton_gene(being, BONE_RIBS, &scale_width, &scale_length);
    outline_points(bone_ribs, bone_points[BONE_RIBS],4,
                   keypoints[SKELETON_NECK].x, keypoints[SKELETON_NECK].y, 0,
                   scale_width, scale_length,
                   angle,
                   &keypoints[SKELETON_LUMBAR],
                   &keypoints[SKELETON_LEFT_SHOULDER_SOCKET],
                   &keypoints[SKELETON_RIGHT_SHOULDER_SOCKET],
                   &keypoints[SKELETON_LEFT_SHOULDER],
                   &keypoints[SKELETON_RIGHT_SHOULDER],
                   points, &i, max_points);
    
    /** left scapula */
    body_skeleton_gene(being, BONE_SCAPULA, &scale_width, &scale_length);
    outline_points(bone_scapula, bone_points[BONE_SCAPULA],0,
                   keypoints[SKELETON_LEFT_SHOULDER].x,
                   keypoints[SKELETON_LEFT_SHOULDER].y,
                   0, scale_width, scale_length,
                   angle-600,
                   &extra[0],
                   &extra[0],
                   &extra[1],
                   &extra[2],
                   &extra[3],
                   points, &no_of_points, max_points);
    
    /** right scapula */
    outline_points(bone_scapula, bone_points[BONE_SCAPULA],0,
                   keypoints[SKELETON_RIGHT_SHOULDER].x,
                   keypoints[SKELETON_RIGHT_SHOULDER].y,
                   1, scale_width, scale_length,
                   angle+500,
                   &extra[0],
                   &extra[0],
                   &extra[1],
                   &extra[2],
                   &extra[3],
                   points, &no_of_points, max_points);
    
    /** ribs */
    body_skeleton_gene(being, BONE_RIBS, &scale_width, &scale_length);
    outline_points(bone_ribs, bone_points[BONE_RIBS],4,
                   keypoints[SKELETON_NECK].x, keypoints[SKELETON_NECK].y, 0,
                   scale_width, scale_length,
                   angle,
                   &keypoints[SKELETON_LUMBAR],
                   &keypoints[SKELETON_LEFT_SHOULDER_SOCKET],
                   &keypoints[SKELETON_RIGHT_SHOULDER_SOCKET],
                   &keypoints[SKELETON_LEFT_SHOULDER],
                   &keypoints[SKELETON_RIGHT_SHOULDER],
                   points, &no_of_points, max_points);
    
    /** position of the top of the pelvis */
    keypoints[SKELETON_PELVIS].x = keypoints[SKELETON_LUMBAR].x;
    keypoints[SKELETON_PELVIS].y =
    keypoints[SKELETON_LUMBAR].y +
    ((keypoints[SKELETON_LUMBAR].y-keypoints[SKELETON_NECK].y)*40/100);
    
    /** position of hips */
    body_skeleton_gene(being, BONE_PELVIS, &scale_width, &scale_length);
    outline_points(bone_pelvis, bone_points[BONE_PELVIS],2,
                   keypoints[SKELETON_PELVIS].x, keypoints[SKELETON_PELVIS].y,
                   0, scale_width, scale_length,
                   angle,
                   &extra[0],
                   &extra[0],
                   &extra[1],
                   &keypoints[SKELETON_LEFT_HIP],
                   &keypoints[SKELETON_RIGHT_HIP],
                   points, &no_of_points, max_points);
    
    /** left upper leg */
    body_skeleton_gene(being, BONE_LEG_UPPER, &scale_width, &scale_length);
    outline_points(bone_leg_upper, bone_points[BONE_LEG_UPPER],0,
                   keypoints[SKELETON_LEFT_HIP].x, keypoints[SKELETON_LEFT_HIP].y,
                   0, scale_width, scale_length,
                   angle+(hip_angle*10),
                   &keypoints[SKELETON_LEFT_KNEE],
                   &extra[0],
                   &extra[1],
                   &extra[2],
                   &extra[3],
                   points, &no_of_points, max_points);
    
    /** left lower leg 1 */
    body_skeleton_gene(being, BONE_LEG_LOWER1, &scale_width, &scale_length);
    outline_points(bone_leg_lower1, bone_points[BONE_LEG_LOWER1],0,
                   keypoints[SKELETON_LEFT_KNEE].x, keypoints[SKELETON_LEFT_KNEE].y,
                   0, scale_width, scale_length,
                   angle+(knee_angle*10),
                   &keypoints[SKELETON_LEFT_ANKLE],
                   &extra[0],
                   &extra[1],
                   &extra[2],
                   &extra[3],
                   points, &no_of_points, max_points);
    
    /** left lower leg 2 */
    body_skeleton_gene(being, BONE_LEG_LOWER1, &scale_width, &scale_length);
    outline_points(bone_leg_lower2, bone_points[BONE_LEG_LOWER2],0,
                   keypoints[SKELETON_LEFT_KNEE].x, keypoints[SKELETON_LEFT_KNEE].y,
                   0, scale_width, scale_length,
                   angle+(knee_angle*10),
                   &keypoints[SKELETON_LEFT_ANKLE],
                   &extra[0],
                   &extra[1],
                   &extra[2],
                   &extra[3],
                   points, &no_of_points, max_points);
    
    /** right upper leg */
    body_skeleton_gene(being, BONE_LEG_UPPER, &scale_width, &scale_length);
    outline_points(bone_leg_upper, bone_points[BONE_LEG_UPPER],0,
                   keypoints[SKELETON_RIGHT_HIP].x, keypoints[SKELETON_RIGHT_HIP].y,
                   1, scale_width, scale_length,
                   angle-(hip_angle*10),
                   &keypoints[SKELETON_RIGHT_KNEE],
                   &extra[0],
                   &extra[1],
                   &extra[2],
                   &extra[3],
                   points, &no_of_points, max_points);
    
    /** right lower leg 1 */
    body_skeleton_gene(being, BONE_LEG_LOWER1, &scale_width, &scale_length);
    outline_points(bone_leg_lower1, bone_points[BONE_LEG_LOWER1],0,
                   keypoints[SKELETON_RIGHT_KNEE].x, keypoints[SKELETON_RIGHT_KNEE].y,
                   1, scale_width, scale_length,
                   angle-(knee_angle*10),
                   &keypoints[SKELETON_RIGHT_ANKLE],
                   &extra[0],
                   &extra[1],
                   &extra[2],
                   &extra[3],
                   points, &no_of_points, max_points);
    
    /** right lower leg 2 */
    body_skeleton_gene(being, BONE_LEG_LOWER1, &scale_width, &scale_length);
    outline_points(bone_leg_lower2, bone_points[BONE_LEG_LOWER2],0,
                   keypoints[SKELETON_RIGHT_KNEE].x, keypoints[SKELETON_RIGHT_KNEE].y,
                   1, scale_width, scale_length,
                   angle-(knee_angle*10),
                   &keypoints[SKELETON_RIGHT_ANKLE],
                   &extra[0],
                   &extra[1],
                   &extra[2],
                   &extra[3],
                   points, &no_of_points, max_points);
    
    /** left upper arm */
    body_skeleton_gene(being, BONE_ARM_UPPER, &scale_width, &scale_length);
    outline_points(bone_arm_upper, bone_points[BONE_ARM_UPPER],0,
                   keypoints[SKELETON_LEFT_SHOULDER_SOCKET].x,
                   keypoints[SKELETON_LEFT_SHOULDER_SOCKET].y,
                   0, scale_width, scale_length,
                   angle+(shoulder_angle*10),
                   &keypoints[SKELETON_LEFT_ELBOW],
                   &extra[0],
                   &extra[1],
                   &extra[2],
                   &extra[3],
                   points, &no_of_points, max_points);
    
    /** left lower arm 1 */
    body_skeleton_gene(being, BONE_ARM_LOWER1, &scale_width, &scale_length);
    outline_points(bone_arm_lower1, bone_points[BONE_ARM_LOWER1],0,
                   keypoints[SKELETON_LEFT_ELBOW].x,
                   keypoints[SKELETON_LEFT_ELBOW].y,
                   0, scale_width, scale_length,
                   angle+(elbow_angle*10),
                   &keypoints[SKELETON_LEFT_WRIST],
                   &extra[0],
                   &extra[1],
                   &extra[2],
                   &extra[3],
                   points, &no_of_points, max_points);
    
    /** left lower arm 2 */
    body_skeleton_gene(being, BONE_ARM_LOWER1, &scale_width, &scale_length);
    outline_points(bone_arm_lower2, bone_points[BONE_ARM_LOWER2],0,
                   keypoints[SKELETON_LEFT_ELBOW].x,
                   keypoints[SKELETON_LEFT_ELBOW].y,
                   0, scale_width, scale_length,
                   angle+(elbow_angle*10),
                   &keypoints[SKELETON_LEFT_WRIST],
                   &extra[0],
                   &extra[1],
                   &extra[2],
                   &extra[3],
                   points, &no_of_points, max_points);
    
    /** right upper arm */
    body_skeleton_gene(being, BONE_ARM_UPPER, &scale_width, &scale_length);
    outline_points(bone_arm_upper, bone_points[BONE_ARM_UPPER],0,
                   keypoints[SKELETON_RIGHT_SHOULDER_SOCKET].x,
                   keypoints[SKELETON_RIGHT_SHOULDER_SOCKET].y,
                   1, scale_width, scale_length,
                   angle-(shoulder_angle*10),
                   &keypoints[SKELETON_RIGHT_ELBOW],
                   &extra[0],
                   &extra[1],
                   &extra[2],
                   &extra[3],
                   points, &no_of_points, max_points);
    
    /** right lower arm 1 */
    body_skeleton_gene(being, BONE_ARM_LOWER1, &scale_width, &scale_length);
    outline_points(bone_arm_lower1, bone_points[BONE_ARM_LOWER1],0,
                   keypoints[SKELETON_RIGHT_ELBOW].x,
                   keypoints[SKELETON_RIGHT_ELBOW].y,
                   1, scale_width, scale_length,
                   angle-(elbow_angle*10),
                   &keypoints[SKELETON_RIGHT_WRIST],
                   &extra[0],
                   &extra[1],
                   &extra[2],
                   &extra[3],
                   points, &no_of_points, max_points);
    
    /** left lower arm 2 */
    body_skeleton_gene(being, BONE_ARM_LOWER1, &scale_width, &scale_length);
    outline_points(bone_arm_lower2, bone_points[BONE_ARM_LOWER2],0,
                   keypoints[SKELETON_RIGHT_ELBOW].x,
                   keypoints[SKELETON_RIGHT_ELBOW].y,
                   1, scale_width, scale_length,
                   angle-(elbow_angle*10),
                   &keypoints[SKELETON_RIGHT_WRIST],
                   &extra[0],
                   &extra[1],
                   &extra[2],
                   &extra[3],
                   points, &no_of_points, max_points);
    
    /** left clavical */
    body_skeleton_gene(being, BONE_CLAVICAL, &scale_width, &scale_length);
    outline_points(bone_clavical, bone_points[BONE_CLAVICAL],0,
                   keypoints[SKELETON_LEFT_SHOULDER].x,
                   keypoints[SKELETON_LEFT_SHOULDER].y,
                   0, scale_width, scale_length,
                   angle-1800,
                   &keypoints[SKELETON_LEFT_COLLAR],
                   &extra[0],
                   &extra[1],
                   &extra[2],
                   &extra[3],
                   points, &no_of_points, max_points);
    
    /** right clavical */
    outline_points(bone_clavical, bone_points[BONE_CLAVICAL],0,
                   keypoints[SKELETON_RIGHT_SHOULDER].x,
                   keypoints[SKELETON_RIGHT_SHOULDER].y,
                   1, scale_width, scale_length,
                   angle+1700,
                   &keypoints[SKELETON_RIGHT_COLLAR],
                   &extra[0],
                   &extra[1],
                   &extra[2],
                   &extra[3],
                   points, &no_of_points, max_points);
    
    vertical = keypoints[SKELETON_NECK].y;
    for (i = 0; i < SKELETON_VERTIBRA_RIBS; i++)
    {
        body_skeleton_gene(being, BONE_RIBS, &scale_width, &scale_length);
        outline_points(bone_vertibra, bone_points[BONE_VERTIBRA],0,
                       keypoints[SKELETON_NECK].x+((keypoints[SKELETON_LUMBAR].x-keypoints[SKELETON_NECK].x)*i/SKELETON_VERTIBRA_RIBS),
                       vertical,
                       0,
                       (scale_width*5/10)+((scale_width*5/10)*i/SKELETON_VERTIBRA_RIBS),
                       (scale_length*5/10)+((scale_length*5/10)*i/SKELETON_VERTIBRA_RIBS),
                       angle,
                       &vertibra,
                       &extra[0],
                       &extra[1],
                       &extra[2],
                       &extra[3],
                       points, &no_of_points, max_points);
        vertical = vertibra.y;
    }
    
    vertical = keypoints[SKELETON_LUMBAR].y;
    for (i = 0; i < SKELETON_LUMBAR_VERTIBRA; i++)
    {
        outline_points(bone_vertibra, bone_points[BONE_VERTIBRA],0,
                       keypoints[SKELETON_LUMBAR].x,
                       vertical,
                       0, scale_width, scale_length,
                       angle,
                       &vertibra,
                       &extra[0],
                       &extra[1],
                       &extra[2],
                       &extra[3],
                       points, &no_of_points, max_points);
        vertical = vertibra.y;
    }
    for (i = 0; i < SKELETON_LUMBAR_VERTIBRA2; i++)
    {
        outline_points(bone_vertibra, bone_points[BONE_VERTIBRA],0,
                       keypoints[SKELETON_LUMBAR].x,
                       vertical,
                       0, scale_width*(SKELETON_LUMBAR_VERTIBRA2-i)/SKELETON_LUMBAR_VERTIBRA2,
                       ((scale_length*2/3)*(SKELETON_LUMBAR_VERTIBRA2-i)/SKELETON_LUMBAR_VERTIBRA2),
                       angle,
                       &vertibra,
                       &extra[0],
                       &extra[1],
                       &extra[2],
                       &extra[3],
                       points, &no_of_points, max_points);
        vertical = vertibra.y;
    }
    
    for (i = 0; i < 4; i++)
    {
        /** left hand */
        body_skeleton_gene(being, BONE_HAND, &scale_width, &scale_length);
        outline_points(bone_hand, bone_points[BONE_HAND],0,
                       keypoints[SKELETON_LEFT_WRIST].x-(i*15),
                       keypoints[SKELETON_LEFT_WRIST].y,
                       0, scale_width, scale_length,
                       angle+200-(i*400/4)+(wrist_angle*10),
                       &knuckle,
                       &extra[0],
                       &extra[1],
                       &extra[2],
                       &extra[3],
                       points, &no_of_points, max_points);
        
        /** left finger */
        body_skeleton_gene(being, BONE_FINGER, &scale_width, &scale_length);
        outline_points(bone_finger, bone_points[BONE_FINGER],0,
                       knuckle.x, knuckle.y,
                       0, scale_width, scale_length,
                       angle+400-(i*800/4)+(wrist_angle*10),
                       &extra[0],
                       &extra[0],
                       &extra[1],
                       &extra[2],
                       &extra[3],
                       points, &no_of_points, max_points);
        
        /** right hand */
        body_skeleton_gene(being, BONE_HAND, &scale_width, &scale_length);
        outline_points(bone_hand, bone_points[BONE_HAND],0,
                       keypoints[SKELETON_RIGHT_WRIST].x+((3-i)*15),
                       keypoints[SKELETON_RIGHT_WRIST].y,
                       1, scale_width, scale_length,
                       angle+200-(i*400/4)-(wrist_angle*10),
                       &knuckle,
                       &extra[0],
                       &extra[1],
                       &extra[2],
                       &extra[3],
                       points, &no_of_points, max_points);
        
        /** right finger */
        body_skeleton_gene(being, BONE_FINGER, &scale_width, &scale_length);
        outline_points(bone_finger, bone_points[BONE_FINGER],0,
                       knuckle.x, knuckle.y,
                       1, scale_width, scale_length,
                       angle+400-(i*800/4)-(wrist_angle*10),
                       &extra[0],
                       &extra[0],
                       &extra[1],
                       &extra[2],
                       &extra[3],
                       points, &no_of_points, max_points);
    }
    
    /** left thumb */
    body_skeleton_gene(being, BONE_FINGER, &scale_width, &scale_length);
    outline_points(bone_finger, bone_points[BONE_FINGER],0,
                   keypoints[SKELETON_LEFT_WRIST].x-50,
                   keypoints[SKELETON_LEFT_WRIST].y,
                   0, scale_width, scale_length,
                   angle-800+(wrist_angle*10),
                   &extra[0],
                   &extra[0],
                   &extra[1],
                   &extra[2],
                   &extra[3],
                   points, &no_of_points, max_points);
    
    /** right thumb */
    outline_points(bone_finger, bone_points[BONE_FINGER],0,
                   keypoints[SKELETON_RIGHT_WRIST].x+50,
                   keypoints[SKELETON_RIGHT_WRIST].y,
                   1, scale_width, scale_length,
                   angle+800-(wrist_angle*10),
                   &extra[0],
                   &extra[0],
                   &extra[1],
                   &extra[2],
                   &extra[3],
                   points, &no_of_points, max_points);
    
    return no_of_points;
}


/**
 * @brief Draws a the skeleton
 * @param being Pointer to the being
 * @param buffer Image buffer (3 bytes per pixel)
 * @param img_width Image width
 * @param img_height Image height
 * @param tx Top left coordinate of the bounding box
 * @param ty Top coordinate of the bounding box
 * @param bx Bottom right coordinate of the bounding box
 * @param by Bottom coordinate of the bounding box
 * @param thickness Thickness of the outline in pixels
 * @param returned 2D keypoints on the skeleton
 * @param shoulder_angle Angle of the shoulders in degrees
 * @param elbow_angle Angle of the elbows in degrees
 * @param wrist_angle Angle of the wrists in degrees
 * @param hip_angle Angle of the hips in degrees
 * @param knee_angle Angle of the knees in degrees
 * @param show_keypoints If non-zero show key points on the skeleton
 */
static void graph_skeleton(noble_being * being,
                           n_byte * buffer,
                           n_int img_width, n_int img_height,
                           n_int tx, n_int ty, n_int bx, n_int by,
                           n_byte thickness,
                           n_vect2 * keypoints,
                           n_int shoulder_angle, n_int elbow_angle, n_int wrist_angle,
                           n_int hip_angle, n_int knee_angle,
                           n_byte show_keypoints)
{
    n_vect2 skeleton_points[MAX_SKELETON_DIAGRAM_POINTS];
    n_int min_x = 99999, min_y = 99999;
    n_int max_x = -99999, max_y = -99999;
    n_int x,y,prev_x=0,prev_y=0,i,no_of_points,first_point=0,ctr=0;
    n_byte r=150, g=150, b=150, bone_shade=220;
    
    /** get points on the skeleton */
    no_of_points = graph_skeleton_points(being, keypoints, skeleton_points,
                                        MAX_SKELETON_DIAGRAM_POINTS,
                                        shoulder_angle, elbow_angle, wrist_angle,
                                        hip_angle, knee_angle);
    
    /** get the bounding box for the points */
    for (i = 0; i < no_of_points; i++)
    {
        x = skeleton_points[i].x;
        y = skeleton_points[i].y;
        if ((x==9999) || (y==9999)) continue;
        if (x < min_x) min_x = x;
        if (y < min_y) min_y = y;
        if (x > max_x) max_x = x;
        if (y > max_y) max_y = y;
    }
    
    if ((max_x - min_x) && (max_y - min_y))
    {
        /** rescale the skeleton keypoints into the bounding box */
        for (i = 0; i < SKELETON_POINTS; i++)
        {
            if (keypoints[i].x != 9999)
            {
                keypoints[i].x = tx + ((keypoints[i].x - min_x)  *(bx - tx)/(max_x - min_x));
                keypoints[i].y = ty + ((keypoints[i].y - min_y)*(by - ty)/(max_y - min_y));
            }
        }
    }
    
    if ((max_x - min_x) && (max_y - min_y))
    {
        /** rescale the drawing points into the bounding box */
        for (i = 0; i < no_of_points; i++)
        {
            if (skeleton_points[i].x != 9999)
            {
                skeleton_points[i].x = tx + ((skeleton_points[i].x - min_x)*(bx - tx)/(max_x - min_x));
                skeleton_points[i].y = ty + ((skeleton_points[i].y - min_y)*(by - ty)/(max_y - min_y));
            }
        }
    }
    /** do the drawing */
    for (i = 0; i < no_of_points; i++)
    {
        x = skeleton_points[i].x;
        y = skeleton_points[i].y;
        if (i > 0)
        {
            if ((x!=9999) && (prev_x!=9999))
            {
                if (first_point == -1)
                {
                    first_point = i;
                }
                graph_line(buffer, img_width, img_height,
                           prev_x, prev_y, x, y,
                           r, g, b, thickness);
            }
            else
            {
                if ((first_point > -1) && (i - first_point > 2))
                {
                    graph_fill_polygon(&skeleton_points[first_point], i - first_point,
                                       bone_shade, bone_shade, bone_shade, 127,
                                       buffer, img_width, img_height);
                    ctr++;
                }
                first_point = -1;
            }
        }
        prev_x = x;
        prev_y = y;
    }
    /** optionally show the keypoints on the skeleton */
    if (show_keypoints != 0)
    {
        for (i = 0; i < SKELETON_POINTS; i++)
        {
            if (keypoints[i].x != 9999)
            {
                graph_line(buffer, img_width, img_height,
                           keypoints[i].x-10, keypoints[i].y, keypoints[i].x+10, keypoints[i].y,
                           0, 255, 0, 1);
                graph_line(buffer, img_width, img_height,
                           keypoints[i].x, keypoints[i].y-10, keypoints[i].x, keypoints[i].y+10,
                           0, 255, 0, 1);
            }
        }
    }
}

/**
 * @brief body_point_relative_to_skeleton
 * @param keypoints Key points on the skeleton
 * @param start_keypoint_index Index of the start skeleton keypoint
 * @param end_keypoint_index Index of the end skeleton keypoint
 * @param distance_along_axis_percent Distance of the point along the axis between the start and end skeleton keypoints
 * @param distance_from_axis_percent Distance perpendicular to the axis as a percentage of the axis length
 * @param x Returned x coordinate
 * @param y Returned y coordinate
 */
static void point_relative_to_skeleton(n_int * keypoints,
                                       n_int start_keypoint_index,
                                       n_int end_keypoint_index,
                                       n_int distance_along_axis_percent,
                                       n_int distance_from_axis_percent,
                                       n_int * x, n_int * y)
{
    /** length of the axis */
    n_int dx = keypoints[end_keypoint_index*2] - keypoints[start_keypoint_index*2];
    n_int dy = keypoints[end_keypoint_index*2+1] - keypoints[start_keypoint_index*2+1];
    
    /** point position on the axis */
    n_int axis_x = keypoints[start_keypoint_index*2] + (dx * distance_along_axis_percent / 100);
    n_int axis_y = keypoints[start_keypoint_index*2+1] + (dy * distance_along_axis_percent / 100);
    
    /** point position */
    *x = axis_x + (dy * distance_from_axis_percent / 100);
    *y = axis_y + (dx * distance_from_axis_percent / 100);
}

const n_int no_of_vascular_diagram_points = 40;
const n_int vascular_diagram_fields = 10;
const n_int vascular_diagram_points[] =
{
    -1, SKELETON_NECK, SKELETON_LUMBAR, 0, 0, SKELETON_NECK, SKELETON_LUMBAR, 90, 0, 2, /**< 0 */
    
    /** left arm */
    0, SKELETON_NECK, SKELETON_LUMBAR, 0, 0, SKELETON_NECK, SKELETON_LEFT_SHOULDER, 100, 0, 12, /**< 1 */
    1, SKELETON_LEFT_SHOULDER, SKELETON_LEFT_ELBOW, 0, 0, SKELETON_LEFT_SHOULDER, SKELETON_LEFT_ELBOW, 100, 0, 12, /**< 2 */
    2, SKELETON_LEFT_ELBOW, SKELETON_LEFT_WRIST, 0, 0, SKELETON_LEFT_ELBOW, SKELETON_LEFT_WRIST, 100, 3, 19, /**< 3 */
    2, SKELETON_LEFT_ELBOW, SKELETON_LEFT_WRIST, 0, 0, SKELETON_LEFT_ELBOW, SKELETON_LEFT_WRIST, 100, -3, 20, /**< 4 */
    
    /** right arm */
    0, SKELETON_NECK, SKELETON_LUMBAR, 0, 0, SKELETON_NECK, SKELETON_RIGHT_SHOULDER, 10, 0, 8, /**< 5 */
    5, SKELETON_NECK, SKELETON_RIGHT_SHOULDER, 10, 0, SKELETON_NECK, SKELETON_RIGHT_SHOULDER, 100, 0, 9, /**< 6 */
    6, SKELETON_RIGHT_SHOULDER, SKELETON_RIGHT_ELBOW, 0, 0, SKELETON_RIGHT_SHOULDER, SKELETON_RIGHT_ELBOW, 100, 0, 9, /**< 7 */
    7, SKELETON_RIGHT_ELBOW, SKELETON_RIGHT_WRIST, 0, 0, SKELETON_RIGHT_ELBOW, SKELETON_RIGHT_WRIST, 100, -3, 14, /**< 8 */
    7, SKELETON_RIGHT_ELBOW, SKELETON_RIGHT_WRIST, 0, 0, SKELETON_RIGHT_ELBOW, SKELETON_RIGHT_WRIST, 100, 3, 13, /**< 9 */
    
    /** abdomen */
    0, SKELETON_NECK, SKELETON_LUMBAR, 90, 0, SKELETON_NECK, SKELETON_LUMBAR, 95, 20, 21, /**< 10 */
    10, SKELETON_NECK, SKELETON_LUMBAR, 90, 0, SKELETON_LUMBAR, SKELETON_PELVIS, 50, 0, 3, /**< 11 */
    11, SKELETON_LUMBAR, SKELETON_PELVIS, 50, 0, SKELETON_LUMBAR, SKELETON_PELVIS, 60, 40, 22, /**< 12 */
    11, SKELETON_LUMBAR, SKELETON_PELVIS, 50, 0, SKELETON_LUMBAR, SKELETON_PELVIS, 60, -40, 22, /**< 13 */
    11, SKELETON_LUMBAR, SKELETON_PELVIS, 50, 0, SKELETON_LUMBAR, SKELETON_PELVIS, 80, 40, 23, /**< 14 */
    11, SKELETON_LUMBAR, SKELETON_PELVIS, 50, 0, SKELETON_LUMBAR, SKELETON_PELVIS, 90, 0, 4, /**< 15 */
    15, SKELETON_LUMBAR, SKELETON_PELVIS, 90, 0, SKELETON_LUMBAR, SKELETON_PELVIS, 100, 30, 24, /**< 16 */
    15, SKELETON_LUMBAR, SKELETON_PELVIS, 90, 0, SKELETON_LUMBAR, SKELETON_PELVIS, 150, 0, 4, /**< 17 */
    
    /** left leg */
    17, SKELETON_LUMBAR, SKELETON_PELVIS, 150, 0, SKELETON_PELVIS, SKELETON_LEFT_HIP, 100, 0, 5, /**< 18 */
    18, SKELETON_LEFT_HIP, SKELETON_LEFT_KNEE, 0, 0, SKELETON_LEFT_HIP, SKELETON_LEFT_KNEE, 20, 12, 5, /**< 19 */
    19, SKELETON_LEFT_HIP, SKELETON_LEFT_KNEE, 20, 12, SKELETON_LEFT_HIP, SKELETON_LEFT_KNEE, 90, 12, 25, /**< 20 */
    19, SKELETON_LEFT_HIP, SKELETON_LEFT_KNEE, 20, 12, SKELETON_LEFT_HIP, SKELETON_LEFT_KNEE, 100, 5, 6, /**< 21 */
    21, SKELETON_LEFT_HIP, SKELETON_LEFT_KNEE, 100, 5, SKELETON_LEFT_KNEE, SKELETON_LEFT_ANKLE, 40, 5, 7, /**< 22 */
    21, SKELETON_LEFT_HIP, SKELETON_LEFT_KNEE, 100, 5, SKELETON_LEFT_KNEE, SKELETON_LEFT_ANKLE, 60, 15, 26, /**< 23 */
    22, SKELETON_LEFT_KNEE, SKELETON_LEFT_ANKLE, 40, 5, SKELETON_LEFT_KNEE, SKELETON_LEFT_ANKLE, 95, 0, 27, /**< 24 */
    22, SKELETON_LEFT_KNEE, SKELETON_LEFT_ANKLE, 40, 5, SKELETON_LEFT_KNEE, SKELETON_LEFT_ANKLE, 95, 10, 28, /**< 25 */
    
    /** right leg */
    17, SKELETON_LUMBAR, SKELETON_PELVIS, 150, 0, SKELETON_PELVIS, SKELETON_RIGHT_HIP, 100, 0, 5, /**< 26 */
    26, SKELETON_RIGHT_HIP, SKELETON_RIGHT_KNEE, 0, 0, SKELETON_RIGHT_HIP, SKELETON_RIGHT_KNEE, 20, -12, 5, /**< 27 */
    27, SKELETON_RIGHT_HIP, SKELETON_RIGHT_KNEE, 20, -12, SKELETON_RIGHT_HIP, SKELETON_RIGHT_KNEE, 90, -12, 25, /**< 28 */
    27, SKELETON_RIGHT_HIP, SKELETON_RIGHT_KNEE, 20, -12, SKELETON_RIGHT_HIP, SKELETON_RIGHT_KNEE, 100, -5, 6, /**< 29 */
    29, SKELETON_RIGHT_HIP, SKELETON_RIGHT_KNEE, 100, -5, SKELETON_RIGHT_KNEE, SKELETON_RIGHT_ANKLE, 40, -5, 7, /**< 30 */
    29, SKELETON_RIGHT_HIP, SKELETON_RIGHT_KNEE, 100, -5, SKELETON_RIGHT_KNEE, SKELETON_RIGHT_ANKLE, 60, -15, 26, /**< 31 */
    30, SKELETON_RIGHT_KNEE, SKELETON_RIGHT_ANKLE, 40, -5, SKELETON_RIGHT_KNEE, SKELETON_RIGHT_ANKLE, 95, 0, 27, /**< 32 */
    30, SKELETON_RIGHT_KNEE, SKELETON_RIGHT_ANKLE, 40, -5, SKELETON_RIGHT_KNEE, SKELETON_RIGHT_ANKLE, 95, -10, 28, /**< 33 */
    
    /** right neck */
    5, SKELETON_NECK, SKELETON_RIGHT_SHOULDER, 10, 0, SKELETON_NECK, SKELETON_RIGHT_SHOULDER, -15, 80, 10, /**< 34 */
    34, SKELETON_NECK, SKELETON_RIGHT_SHOULDER, -15, 80, SKELETON_NECK, SKELETON_RIGHT_SHOULDER, -25, 150, 15, /**< 35 */
    34, SKELETON_NECK, SKELETON_RIGHT_SHOULDER, -15, 80, SKELETON_NECK, SKELETON_RIGHT_SHOULDER, -45, 150, 16, /**< 36 */
    
    /** left neck */
    -1, SKELETON_NECK, SKELETON_LEFT_SHOULDER, 20, 0, SKELETON_NECK, SKELETON_LEFT_SHOULDER, 15, -80, 11, /**< 37 */
    37, SKELETON_NECK, SKELETON_LEFT_SHOULDER, 16, -80, SKELETON_NECK, SKELETON_LEFT_SHOULDER, 10, -130, 17, /**< 38 */
    37, SKELETON_NECK, SKELETON_LEFT_SHOULDER, 16, -80, SKELETON_NECK, SKELETON_LEFT_SHOULDER, 30, -130, 18, /**< 39 */
};


/**
 * @brief Draws a representation of the vascular system
 * @param being Pointer to the being
 * @param buffer Image buffer (3 bytes per pixel)
 * @param img_width Image width
 * @param img_height Image height
 * @param tx Top left coordinate of the bounding box
 * @param ty Top coordinate of the bounding box
 * @param bx Bottom right coordinate of the bounding box
 * @param by Bottom coordinate of the bounding box
 * @param thickness Thickness of the outline in pixels
 * @param clear Non zero if the image should be cleared before drawing
 * @param shoulder_angle Angle of the shoulders in degrees
 * @param elbow_angle Angle of the elbows in degrees
 * @param wrist_angle Angle of the wrists in degrees
 * @param hip_angle Angle of the hips in degrees
 * @param knee_angle Angle of the knees in degrees
 * @param show_skeleton_keypoints If non-zero show key points on the skeleton
 */
void graph_vascular(noble_being * being,
                    n_byte * buffer,
                    n_int img_width, n_int img_height,
                    n_int tx, n_int ty, n_int bx, n_int by,
                    n_byte thickness,
                    n_byte clear,
                    n_int shoulder_angle, n_int elbow_angle, n_int wrist_angle,
                    n_int hip_angle, n_int knee_angle,
                    n_byte show_skeleton_keypoints)
{
    n_vect2 keypoints[SKELETON_POINTS];
#ifdef VASCULAR_DRAWING_READY
    n_int i,x[3],y[3],vascular_model_index,previous_index;
    n_uint start_thickness=4, end_thickness=4;
#endif
    /** clear the image if necessary */
    if (clear != 0)
    {
        graph_erase(buffer, img_height, img_width);
    }
    
    /** draw the skeleton */
    graph_skeleton(being, buffer, img_width, img_height,
                   tx, ty, bx, by, thickness, keypoints,
                   shoulder_angle, elbow_angle, wrist_angle,
                   hip_angle, knee_angle, show_skeleton_keypoints);
    
#ifdef VASCULAR_DRAWING_READY
    for (i = 0; i < no_of_vascular_diagram_points; i++)
    {
        /** index of the previous segment */
        previous_index = vascular_diagram_points[i*vascular_diagram_fields];
        x[0] = -1;
        if (previous_index > -1)
        {
            /** end point of the vessel */
            point_relative_to_skeleton(keypoints,
                                       vascular_diagram_points[previous_index*vascular_diagram_fields+5],
                                       vascular_diagram_points[previous_index*vascular_diagram_fields+6],
                                       vascular_diagram_points[previous_index*vascular_diagram_fields+7],
                                       vascular_diagram_points[previous_index*vascular_diagram_fields+8],
                                       &x[0], &y[0]);
        }
        
        /** beginning point of the vessel */
        point_relative_to_skeleton(keypoints,
                                   vascular_diagram_points[i*vascular_diagram_fields+1],
                                   vascular_diagram_points[i*vascular_diagram_fields+2],
                                   vascular_diagram_points[i*vascular_diagram_fields+3],
                                   vascular_diagram_points[i*vascular_diagram_fields+4],
                                   &x[1], &y[1]);
        
        /** end point of the vessel */
        point_relative_to_skeleton(keypoints,
                                   vascular_diagram_points[i*vascular_diagram_fields+5],
                                   vascular_diagram_points[i*vascular_diagram_fields+6],
                                   vascular_diagram_points[i*vascular_diagram_fields+7],
                                   vascular_diagram_points[i*vascular_diagram_fields+8],
                                   &x[2], &y[2]);
        
        /** compartment index within the vascular simulation */
        vascular_model_index = vascular_diagram_points[i*vascular_diagram_fields+9];
        
        if (x[0]!=-1)
        {
            /* draw a curve */
            graph_curve(buffer, img_width, img_height,
                        x[0],y[0], x[1],y[1], x[2],y[2],
                        255,0,0,
                        10,start_thickness,end_thickness);
        }
        else
        {
            /* draw a line */
            graph_line(buffer, img_width, img_height,
                       x[1], y[1], x[2], y[2],
                       255,0,0, end_thickness);
        }
    }
#endif
}

/**
 * @brief Shows distribution of honor.  Note that beings are sorted in order of honor
 * @param sim Pointer to the simulation object
 * @param update type Whether to draw the background or foreground
 * @param buffer Image buffer (3 bytes per pixel)
 * @param img_width Image width
 * @param img_height Image height
 */
void graph_honor_distribution(noble_simulation * sim, n_byte update_type, n_byte * buffer, n_int img_width, n_int img_height)
{
    n_uint i,j,temp;
    n_int * honor_value;
    scope s;
    unsigned int channel = 0;
    unsigned int intensity_percent = 100;
    unsigned int grid_horizontal = 10;
    unsigned int grid_vertical = 8;
    
    s = create_scope((unsigned int)1);
    s.time_ms = (unsigned int)(sim->num);
    s.noise = 0.5;
    
    if (update_type == PHOSPHENE_DRAW_BACKGROUND) {
        scope_draw(&s, update_type, intensity_percent,
                   grid_horizontal, grid_vertical,
                   (unsigned char*)buffer, (unsigned int)img_width, (unsigned int)img_height);
        return;
    }
    
    honor_value = (n_int*)io_new(sim->num*sizeof(n_int));
    
    /** get the honor values */
    for (i = 0; i < sim->num; i++)
    {
        noble_being * local_being = &(sim->beings[i]);
        honor_value[i] = being_honor(local_being);
    }
    
    /** sort the honor values */
    for (i = 0; i < sim->num; i++)
    {
        for (j = i+1; j < sim->num; j++)
        {
            if (honor_value[i]<honor_value[j]) {
                temp = honor_value[i];
                honor_value[i] = honor_value[j];
                honor_value[j] = temp;
            }
        }
    }
    
    
    for (i = 0; i < sim->num; i++)
    {
        scope_update(&s, channel, (double)honor_value[i], 0.0, 256.0, (unsigned int)i);
    }
    
    io_free(honor_value);
    
    scope_draw(&s, update_type, intensity_percent,
               grid_horizontal, grid_vertical,
               (unsigned char*)buffer, (unsigned int)img_width, (unsigned int)img_height);
}

static n_int graph_being_score(noble_simulation * sim, noble_being * local_being, n_byte score_type)
{
    n_int nucleotide,i,score = 0;
    n_byte * bases = (n_byte*)being_genetics(local_being);
    
    switch (score_type)
    {
        case 0:
            for (i = 0; i < BRAINCODE_SIZE; i++)
            {
                score += being_braincode_external(local_being)[i] + being_braincode_internal(local_being)[i];
            }
            break;
        case 1:
            for (i = 0; i < CHROMOSOMES; i++)
            {
                for (nucleotide = 0; nucleotide < 8; nucleotide++)
                {
                    score += (bases[i]>>(nucleotide*2)) & 3;
                }
            }
            break;
    }
    return score;
}

/*
 Updates an index array which is used to sort beings in order of honor value
 */
static void graph_being_index(noble_simulation * sim, n_int *index, n_byte score_type)
{
#ifdef BRAINCODE_ON
#ifdef PARASITES_ON
    n_int score;
    n_uint i;
    n_byte * used = (unsigned char*)io_new(sim->num);
    for (i = 0; i < sim->num; i++)
    {
        used[i] = 0;
        index[i] = i;
    }
    /* sort by honor value */
    for (i = 0; i < sim->num; i++)
    {
        if (used[i]==0)
        {
            n_uint j;
            n_int max = -1;
            n_int idx = -1;
            for (j = 0; j < sim->num; j++)
            {
                if (used[j]==0)
                {
                    noble_being * local_being = &(sim->beings[j]);
                    score = graph_being_score(sim, local_being,score_type);
                    if (score>max)
                    {
                        max = score;
                        idx = j;
                    }
                }
            }
            if (idx>-1)
            {
                index[i] = idx;
                used[idx] = 1;
            }
        }
    }
    io_free((void*)used);
#endif
#endif
}

/*
 Displays the braincode programs (one per row) for each being in the population.
 Colors represent different instruction types, and individuals are sorted by honor.
 */
void graph_ideosphere(noble_simulation * sim, n_byte * buffer, n_int img_width, n_int img_height)
{
#ifdef BRAINCODE_ON
#ifdef PARASITES_ON
    n_int i,x,y,n,half_width,max_height=img_height;
    n_int *index = (n_int*)io_new((sim->num)*sizeof(n_int));
    noble_being * local_being;
    n_byte * code;
    
    graph_being_index(sim, index,0);
    
    half_width = img_width/2;
    n = 0;
    if (sim->num>0)
    {
        for (y = 0; y < max_height; y++)
        {
            i = index[y*(sim->num-1)/max_height];
            local_being = &(sim->beings[i]);
            for (x = 0; x < img_width; x++, n+=3)
            {
                if (x<half_width)
                {
                    i = (x * ((BRAINCODE_SIZE/BRAINCODE_BYTES_PER_INSTRUCTION)-1) / half_width)*BRAINCODE_BYTES_PER_INSTRUCTION;
                    code = being_braincode_internal(local_being);
                }
                else
                {
                    i = ((x-half_width) * ((BRAINCODE_SIZE/BRAINCODE_BYTES_PER_INSTRUCTION)-1) / half_width)*BRAINCODE_BYTES_PER_INSTRUCTION;
                    code = being_braincode_external(local_being);
                }
                buffer[n] = code[i];
                buffer[n+1] = code[i+1];
                buffer[n+2] = code[i+2];
            }
        }
    }
    
    io_free((void*)index);
#endif
#endif
}

/*
 Shows the genome for each individual in the population, with one genome per row.
 Individuals are sorted in order of honor.
 */
void graph_genepool(noble_simulation * sim, n_byte * buffer, n_int img_width, n_int img_height)
{
#ifdef PARASITES_ON
    n_int i,x,y,n,ch,idx,nucleotide,max_height=img_height;
    n_genetics * bases;
    n_int * index;
    noble_being * local_being;
    
    const n_byte col[] =
    {
        200,0,0,	0,200,0,	0,0,200,	200,200,0
    };
    
    index = (n_int*)io_new(sim->num*sizeof(n_int));
    graph_being_index(sim,index,1);
    
    n = 0;
    if (sim->num>0)
    {
        for (y = 0; y < max_height; y++)
        {
            i = index[y*(sim->num-1)/max_height];
            local_being = &(sim->beings[i]);
            bases = being_genetics(local_being);
            for (x = 0; x < img_width; x++, n+=3)
            {
                nucleotide = x * (CHROMOSOMES*16) / img_width;
                ch = nucleotide>>4;
                idx = ((bases[ch]>>((nucleotide-(16*ch))*2))&3)*3;
                buffer[n] = col[idx++];
                buffer[n+1] = col[idx++];
                buffer[n+2] = col[idx];
            }
        }
    }
    
    io_free((void*)index);
#endif
}

/*
 A matrix showing the relationships between beings.
 Green squares represent friendly relationships, red represent unfriendly relationships
 and black represents "don't care"
 Individuals are plotted on each axis in the same (honor sorted) order, such that a being's
 relationship with itself is along the diagonal axis
 */
void graph_relationship_matrix(noble_simulation * sim, n_byte * buffer, n_int img_width, n_int img_height)
{
#ifdef PARASITES_ON
    n_int j,x,y;
    n_uint i, k;
    n_int *index = (n_int*)io_new(sim->num*sizeof(n_int));
    graph_being_index(sim,index,1);
    
    graph_erase(buffer, img_height, img_width);
    
    for (i = 0; i < sim->num; i++)
    {
        noble_being * local_being = &(sim->beings[index[i]]);
        social_link * graph = being_social(local_being);
        n_uint respect_threshold = social_respect_mean(sim, local_being);
        n_int tx = i*(img_width-1)/sim->num;
        n_int bx = (i+1)*(img_width-1)/sim->num;
        for (j = 0; j < SOCIAL_SIZE_BEINGS; j++)
        {
            if (!SOCIAL_GRAPH_ENTRY_EMPTY(graph, j))
            {
                if (j == 0)
                {
                    k = i;
                }
                else
                {
                    for (k = 0; k < sim->num; k++)
                    {
                        noble_being * local_being2 = &(sim->beings[index[k]]);
                        if ((being_family_name(local_being2) == graph[j].family_name[BEING_MET]) &&
                            (being_first_name(local_being2) == UNPACK_FAMILY_FIRST_NAME(graph[j].first_name[BEING_MET])))
                        {
                            break;
                        }
                    }
                }
                
                if (k < sim->num)
                {
                    n_int ty = k*(img_height-1)/sim->num;
                    n_int by = (k+1)*(img_height-1)/sim->num;
                    
                    for (y=ty; y<by; y++)
                    {
                        for (x=tx; x<bx; x++)
                        {
                            n_int v, n = ((y*img_width)+x)*3;
                            if (graph[j].friend_foe>=respect_threshold)
                            {
                                v = (graph[j].friend_foe - respect_threshold)*8;
                                if (v>255) v=255;
                                buffer[n] = 0;
                                buffer[n+1] = (n_byte)v;
                                buffer[n+2] = 0;
                            }
                            else
                            {
                                v = (respect_threshold - graph[j].friend_foe)*8;
                                if (v>255) v=255;
                                buffer[n] = (n_byte)v;
                                buffer[n+1] = 0;
                                buffer[n+2] = 0;
                            }
                        }
                    }
                }
            }
        }
    }
    
    io_free((void*)index);
#endif
}

/*
 Draws the number of antigens and antibodies in the population
 There are 256 possible antigens/antibodies which are along the horizontal axis.
 Antigens are shown in red and antibodies in green.
 */
void graph_pathogens(noble_simulation * sim, n_byte update_type, n_byte * buffer, n_int img_width, n_int img_height)
{
    n_c_uint i;
    n_c_uint * antibodies;
    n_c_uint * antigens;
#ifdef IMMUNE_ON
    noble_being * local_being;
    n_int j,p;
    n_uint max_val=1;
    noble_immune_system * immune;
    scope s;
    unsigned int intensity_percent = 100;
    unsigned int grid_horizontal = 10;
    unsigned int grid_vertical = 8;
#endif
    
    s = create_scope((unsigned int)1);
    s.time_ms = (unsigned int)256;
    s.no_of_traces = 2;
    s.noise = 200;
    
    if (update_type == PHOSPHENE_DRAW_BACKGROUND) {
        scope_draw(&s, update_type, intensity_percent,
                   grid_horizontal, grid_vertical,
                   (unsigned char*)buffer, (unsigned int)img_width, (unsigned int)img_height);
        return;
    }
    
    antibodies = (n_c_uint*)io_new(256*sizeof(n_c_uint));
    antigens = (n_c_uint*)io_new(256*sizeof(n_c_uint));
    
    for (i=0; i<256; i++)
    {
        antibodies[i]=0;
        antigens[i]=0;
    }
    
#ifdef IMMUNE_ON
    
    if (sim->num>0)
    {
        /* update histograms */
        for (p=0; p<256; p++)
        {
            for (i=0; i<sim->num; i++)
            {
                local_being = &(sim->beings[i]);
                immune = &(local_being->immune_system);
                for (j=0; j<IMMUNE_POPULATION; j++)
                {
                    antibodies[immune->shape_antibody[j]]++;
                }
                for (j=0; j<IMMUNE_ANTIGENS; j++)
                {
                    antigens[immune->shape_antigen[j]]++;
                }
            }
        }
        
        /* find the maximum value */
        for (p=0; p<256; p++)
        {
            if (antibodies[p]>max_val)
            {
                max_val=antibodies[p];
            }
            if (antigens[p]>max_val)
            {
                max_val=antigens[p];
            }
        }
        
        for (p=0; p<256; p++)
        {
            scope_update(&s, 0, (double)antibodies[p], 0.0, (double)max_val, (unsigned int)p);
            scope_update(&s, 1, (double)antigens[p], 0.0, (double)max_val, (unsigned int)p);
        }
    }
    
    scope_draw(&s, update_type, intensity_percent,
               grid_horizontal, grid_vertical,
               (unsigned char*)buffer, (unsigned int)img_width, (unsigned int)img_height);
#endif
    
    io_free((void*)antibodies);
    io_free((void*)antigens);
}

/*
 Shows a histogram of ages
 */
void graph_age_demographic(noble_simulation * sim, n_byte * buffer, n_int img_width, n_int img_height)
{
    const n_int max_age = AGE_OF_MATURITY*4;
    const n_int divisor = 4;
    n_int groups = max_age/divisor;
    n_int * age_group;
    n_int i,idx,max=1;
    n_int prev_x = 0, prev_y=img_height-1;
    n_int x,y;
    n_uint current_date;
    n_uint j;
    
    age_group = (n_int*)io_new(groups*sizeof(n_int));
    for (i=0; i<groups; i++)
    {
        age_group[i]=0;
    }
    
    graph_erase(buffer, img_height, img_width);
    
    current_date = TIME_IN_DAYS(sim->land->date);
    
    for (j = 0; j < sim->num; j++)
    {
        noble_being * local_being = &(sim->beings[j]);
        n_uint local_dob = being_dob(local_being);
        n_int age_days = current_date - local_dob;
        if (age_days >= max_age) age_days = max_age-1;
        idx = age_days/divisor;
        age_group[idx]++;
        if (age_group[idx] > max) max = age_group[idx];
    }
    max = max * 120/100;
    for (i = 0; i < groups; i++)
    {
        x = i*img_width/groups;
        y = img_height-1-(age_group[i]*img_height/max);
        graph_line(buffer,img_width,img_height,prev_x,prev_y,x,y,0,0,0,1);
        prev_x = x;
        prev_y = y;
    }
    io_free((void*)age_group);
}

/*
 Show a histogram of being heights
 */
void graph_heights(noble_simulation * sim, n_byte * buffer, n_int img_width, n_int img_height)
{
    const n_int divisor = BEING_MAX_HEIGHT/16;
    n_int groups = BEING_MAX_HEIGHT/divisor;
    n_int * height_group;
    n_int i,idx,max=1;
    n_uint j;
    n_int prev_x = 0, prev_y=img_height-1;
    n_int x,y;
    
    height_group = (n_int*)io_new(groups*sizeof(n_int));
    for (i=0; i<groups; i++)
    {
        height_group[i]=0;
    }
    graph_erase(buffer, img_height, img_width);
    
    for (j = 0; j < sim->num; j++)
    {
        noble_being * local_being = &(sim->beings[j]);
        idx = GET_H(local_being)/divisor;
        height_group[idx]++;
        if (height_group[idx] > max) max = height_group[idx];
    }
    max = max * 120/100;
    for (i = 0; i < groups; i++)
    {
        x = i*img_width/groups;
        y = img_height-1-(height_group[i]*img_height/max);
        graph_line(buffer,img_width,img_height,prev_x,prev_y,x,y,0,0,0,1);
        prev_x = x;
        prev_y = y;
    }
    
    io_free((void*)height_group);
}

/* return the braincode standard deviation */

static n_uint braincode_standard_deviation(noble_simulation * sim, noble_being * local_being)
{
    n_uint sd = 0;
#ifdef BRAINCODE_ON
    n_int i,av=0,diff;
    
    for (i=0; i<BRAINCODE_SIZE; i++)
    {
        av += being_braincode_internal(local_being)[i];
        av += being_braincode_external(local_being)[i];
    }
    av /= (BRAINCODE_SIZE*2);
    
    for (i=0; i<BRAINCODE_SIZE; i++)
    {
        diff = (n_int)(being_braincode_internal(local_being)[i]) - av;
        if (diff<0) diff=-diff;
        sd += (n_uint)(diff);
        diff = (n_int)(being_braincode_external(local_being)[i]) - av;
        if (diff<0) diff=-diff;
        sd += (n_uint)(diff);
    }
#endif
    return sd;
}


/* return coordinates of the braincode system for phase space plot */
static void graph_braincode_coords(noble_simulation * sim, noble_being * local_being, n_uint * x, n_uint * y)
{
    n_int i;
    *x=0;
    *y=0;
    for (i=0; i<BRAINCODE_SIZE/2; i++)
    {
        *x = *x + being_braincode_internal(local_being)[i] +
        being_braincode_external(local_being)[i];
    }
    while (i < BRAINCODE_SIZE) {
        *y = *y + being_braincode_internal(local_being)[i] +
        being_braincode_external(local_being)[i];
        i++;
    }
}

/* return coordinates of the genome for phase space plot */
static void graph_genespace_coords(noble_being * local_being, n_uint * x, n_uint * y)
{
    n_int ch,b;
    n_genetics * genetics = being_genetics(local_being);
    *x=0;
    *y=0;
    for (ch=0; ch<CHROMOSOMES; ch++)
    {
        for (b=0; b<8; b++)
        {
            *x = *x + ((genetics[ch]>>(b*2))&3);
            *y = *y + ((genetics[ch]>>(16+(b*2)))&3);
        }
    }
}


static void graph_phasespace_dots(noble_simulation * sim, n_byte update_type, n_byte * buffer, n_int img_width, n_int img_height, n_byte graph_type)
{
#ifdef PARASITES_ON
    n_uint i,x=0,y=0;
    n_int min_x,max_x,min_y,max_y;
    n_int dx,dy;
    n_int av_x=0, av_y=0, av_dx=0, av_dy=0;
    const n_int min_variance = 8;
    scope s;
    unsigned int intensity_percent = 100;
    unsigned int grid_horizontal = 10;
    unsigned int grid_vertical = 8;
    
    if (sim->num == 0) {
        /* clear the image */
        graph_erase(buffer, img_height, img_width);
        return;
    }
    
    s = create_scope((unsigned int)1);
    s.time_ms = (unsigned int)(sim->num);
    s.noise = 0.1;
    s.mode = PHOSPHENE_MODE_POINTS;
    
    if (update_type == PHOSPHENE_DRAW_BACKGROUND) {
        scope_draw(&s, update_type, intensity_percent,
                   grid_horizontal, grid_vertical,
                   (unsigned char*)buffer, (unsigned int)img_width, (unsigned int)img_height);
        return;
    }
    
    
    for (i=0; i<sim->num; i++)
    {
        switch(graph_type)
        {
            case 0:
                graph_braincode_coords(sim, &(sim->beings[i]), &x, &y);
                break;
            case 1:
                graph_genespace_coords(&(sim->beings[i]), &x, &y);
                break;
        }
        av_x += (n_int)x;
        av_y += (n_int)y;
    }
    av_x /= (n_int)sim->num;
    av_y /= (n_int)sim->num;
    
    for (i=0; i<sim->num; i++)
    {
        switch(graph_type)
        {
            case 0:
                graph_braincode_coords(sim, &(sim->beings[i]), &x, &y);
                break;
            case 1:
                graph_genespace_coords(&(sim->beings[i]), &x, &y);
                break;
        }
        
        dx = (n_int)x - av_x;
        if (dx < 0) dx = -dx;
        
        dy = (n_int)y - av_y;
        if (dy < 0) dy = -dy;
        
        av_dx += dx;
        av_dy += dy;
    }
    av_dx /= (n_int)sim->num;
    av_dy /= (n_int)sim->num;
    
    if (av_dx < min_variance) av_dx = min_variance;
    if (av_dy < min_variance) av_dy = min_variance;
    
    min_x = av_x - av_dx;
    max_x = av_x + av_dx;
    min_y = av_y - av_dy;
    max_y = av_y + av_dy;
    
    for (i=0; i<sim->num; i++)
    {
        switch(graph_type)
        {
            case 0:
                graph_braincode_coords(sim, &(sim->beings[i]), &x, &y);
                break;
            case 1:
                graph_genespace_coords(&(sim->beings[i]), &x, &y);
                break;
        }
        
        scope_update(&s, 0, x, min_x, max_x, (unsigned int)i);
        scope_update(&s, 1, y, min_y, max_y, (unsigned int)i);
    }
    
    scope_draw(&s, update_type, intensity_percent,
               grid_horizontal, grid_vertical,
               (unsigned char*)buffer, (unsigned int)img_width, (unsigned int)img_height);
#endif
}

static void graph_phasespace_density(noble_simulation * sim, n_byte * buffer, n_int img_width, n_int img_height, n_byte graph_type)
{
#ifdef BRAINCODE_ON
    const n_uint grid = 32;
    n_uint x=0,y=0;
    n_uint n0,n1,tx,ty,bx,by,xx,yy;
    n_uint density[32*32],max=1;
    n_byte r,b;
    n_int i;
    n_uint j;
    for (i = 0; i < img_width*img_height*3; i+=3)
    {
        buffer[i]=0;
        buffer[i+1]=0;
        buffer[i+2]=255;
    }
    
    io_erase((n_byte*)density, sizeof(n_uint)*32*32);
    
    for (j=0; j < sim->num; j++)
    {
        switch(graph_type)
        {
            case 0:
                graph_braincode_coords(sim, &(sim->beings[j]), &x, &y);
                x = x * (grid-1) / (256*BRAINCODE_SIZE);
                y = (grid-1) - (y * (grid-1) / (255*BRAINCODE_SIZE));
                break;
            case 1:
                graph_genespace_coords(&(sim->beings[j]), &x, &y);
                x = x * (grid-1) / (4*8*CHROMOSOMES);
                y = (grid-1) - (y * (grid-1) / (4*8*CHROMOSOMES));
                break;
        }
        density[y*grid+x]++;
    }
    
    for (j = 0; j < grid*grid; j++)
    {
        if (density[j] > max) max = density[j];
    }
    
    n0 = 0;
    for (y = 0; y < grid; y++)
    {
        for (x = 0; x < grid; x++, n0++)
        {
            if (density[n0] > 0)
            {
                r = (n_byte)(density[n0]*255/max);
                b = 255-r;
                
                tx = x * img_width / grid;
                bx = (x+1) * img_width / grid;
                ty = y * img_height / grid;
                by = (y+1) * img_height / grid;
                
                for (yy = ty; yy < by; yy++)
                {
                    n1 = ((yy*img_width)+tx)*3;
                    for (xx = tx; xx < bx; xx++, n1+=3)
                    {
                        buffer[n1] = r;
                        buffer[n1+1] = 0;
                        buffer[n1+2] = b;
                    }
                }
            }
        }
    }
#endif
}

void graph_socialsim(noble_simulation * sim, n_byte update_type, n_byte * buffer, n_int img_width, n_int img_height)
{
    n_uint i;
    n_int min_x=65536, max_x=-1, min_y=65536, max_y=-1;
    
    noble_being * local_being;
    scope s;
    unsigned int intensity_percent = 100;
    unsigned int grid_horizontal = 10;
    unsigned int grid_vertical = 10;
    
    if (sim->num == 0) {
        /* clear the image */
        graph_erase(buffer, img_height, img_width);
        return;
    }
    
    s = create_scope((unsigned int)1);
    s.time_ms = (unsigned int)(sim->num);
    s.noise = 0.1;
    s.mode = PHOSPHENE_MODE_POINTS;
    
    if (update_type == PHOSPHENE_DRAW_BACKGROUND) {
        scope_draw(&s, update_type, intensity_percent,
                   grid_horizontal, grid_vertical,
                   (unsigned char*)buffer, (unsigned int)img_width, (unsigned int)img_height);
        return;
    }
    
    for (i = 0; i < sim->num; i++)
    {
        n_int coord_x, coord_y;
        
        local_being = &(sim->beings[i]);
        
        coord_x = local_being->social_x;
        coord_y = local_being->social_y;
        
        if (coord_x < min_x)
        {
            min_x = coord_x;
        }
        else if (coord_x > max_x)
        {
            max_x = coord_x;
        }
        if (coord_y < min_y)
        {
            min_y = coord_y;
        }
        else if (coord_y > max_y)
        {
            max_y = coord_y;
        }
    }
    
    if ((max_x <= min_x) || (max_y <= min_y)) return;
    
    for (i = 0; i < sim->num; i++)
    {
        int x, y;
        
        local_being = &(sim->beings[i]);
        
        x = local_being->social_x;
        y = local_being->social_y;
        
        scope_update(&s, 0, x, min_x, max_x, (unsigned int)i);
        scope_update(&s, 1, y, min_y, max_y, (unsigned int)i);
    }
    
    scope_draw(&s, update_type, intensity_percent,
               grid_horizontal, grid_vertical,
               (unsigned char*)buffer, (unsigned int)img_width, (unsigned int)img_height);
}

/* plot the places where beings met */
void graph_meet_places(noble_simulation * sim, n_byte update_type, n_byte * buffer, n_int img_width, n_int img_height)
{
    n_uint i,index,ctr=0;
    /** dimensions of APESPACE */
    n_int min_x=0, max_x=65535, min_y=0, max_y=65535;
    scope s;
    unsigned int intensity_percent = 100;
    unsigned int grid_horizontal = 10;
    unsigned int grid_vertical = 10;
    
    if (sim->num == 0) {
        /* clear the image */
        graph_erase(buffer, img_height, img_width);
        return;
    }
    
    s = create_scope((unsigned int)1);
    s.time_ms = (unsigned int)(sim->num);
    s.noise = 0.1;
    s.mode = PHOSPHENE_MODE_POINTS;
    
    if (update_type == PHOSPHENE_DRAW_BACKGROUND) {
        scope_draw(&s, update_type, intensity_percent,
                   grid_horizontal, grid_vertical,
                   (unsigned char*)buffer, (unsigned int)img_width, (unsigned int)img_height);
        return;
    }
    
    /** count the number of locations */
    for (i = 0; i < sim->num; i++)
    {
		social_link * graph = being_social(&(sim->beings[i]));
        
        /** for each non-self social graph entry */
        for (index = 1; index < SOCIAL_SIZE_BEINGS; index++)
		{
			if (!SOCIAL_GRAPH_ENTRY_EMPTY(graph,index))
			{
                if (SOCIAL_GRAPH_ENTRY_LOCATION_EXISTS(graph,index))
                {
                    ctr++;
                }
			}
		}
    }
    
    if (ctr > 0) {
        s.time_ms = (unsigned int)ctr;
    }
    
    ctr = 0;
    for (i = 0; i < sim->num; i++)
    {
		social_link * graph = being_social(&(sim->beings[i]));
        
        /** for each non-self social graph entry */
        for (index = 1; index < SOCIAL_SIZE_BEINGS; index++)
		{
			if (!SOCIAL_GRAPH_ENTRY_EMPTY(graph,index))
			{
                if (SOCIAL_GRAPH_ENTRY_LOCATION_EXISTS(graph,index))
                {
                    scope_update(&s, 0, (int)graph[index].location[0],
                                 (int)min_x, (int)max_x, (unsigned int)ctr);
                    scope_update(&s, 1, (int)graph[index].location[1],
                                 (int)min_y, (int)max_y, (unsigned int)ctr);
                    ctr++;
                }
            }
		}
    }
    
    scope_draw(&s, update_type, intensity_percent,
               grid_horizontal, grid_vertical,
               (unsigned char*)buffer, (unsigned int)img_width, (unsigned int)img_height);
}

void graph_phasespace(noble_simulation * sim, n_byte update_type, n_byte * buffer, n_int img_width, n_int img_height, n_byte graph_type, n_byte data_type)
{
    if (graph_type==0)
    {
        graph_phasespace_dots(sim, update_type, buffer, img_width, img_height,data_type);
    }
    else
    {
        graph_phasespace_density(sim, buffer, img_width, img_height,data_type);
    }
}

/*
 Displays the braincode program for an individual
 */
void graph_braincode(noble_simulation * sim, noble_being * local_being, n_byte * buffer, n_int img_width, n_int img_height, n_byte clear)
{
#ifdef BRAINCODE_ON
#ifdef PARASITES_ON
    n_int i,x,y,n,half_width;
    n_byte * code;
    
    if (local_being!=0)
    {
        /* clear the image */
        if (clear!=0) for (i = 0; i < img_width*img_height*3; i++) buffer[i]=0;
        
        half_width = img_width/2;
        y = sim->land->time % img_height;
        n = y*img_width*3;
        for (x = 0; x < img_width; x++, n+=3)
        {
            if (x<half_width)
            {
                i = (x * ((BRAINCODE_SIZE/BRAINCODE_BYTES_PER_INSTRUCTION)-1) / half_width)*BRAINCODE_BYTES_PER_INSTRUCTION;
                code = being_braincode_internal(local_being);
            }
            else
            {
                i = ((x-half_width) * ((BRAINCODE_SIZE/BRAINCODE_BYTES_PER_INSTRUCTION)-1) / half_width)*BRAINCODE_BYTES_PER_INSTRUCTION;
                code = being_braincode_external(local_being);
            }
            buffer[n] = code[i];
            buffer[n+1] = code[i+1];
            buffer[n+2] = code[i+2];
        }
    }
    
#endif
#endif
}

/*
 Displays the preferences of the population
 */
void graph_preferences(noble_simulation * sim, n_byte update_type, n_byte * buffer, n_int img_width, n_int img_height)
{
    n_uint i;
    n_int p,x=0,y=0,half=PREFERENCES/2;
    n_int min_x=0, max_x=0, min_y=0, max_y=0;
    n_int av_x=0, av_y=0, av_dx=0, av_dy=0,dx,dy;
    const n_int dimension = 10000;
    const n_int min_variance = 20;
    noble_being * local_being;
    scope s;
    unsigned int intensity_percent = 100;
    unsigned int grid_horizontal = 10;
    unsigned int grid_vertical = 8;
    
    if (sim->num == 0) {
        /* clear the image */
        graph_erase(buffer, img_height, img_width);
        return;
    }
    
    s = create_scope((unsigned int)1);
    s.time_ms = (unsigned int)(sim->num);
    s.noise = 0.1;
    s.mode = PHOSPHENE_MODE_POINTS;
    
    if (update_type == PHOSPHENE_DRAW_BACKGROUND) {
        scope_draw(&s, update_type, intensity_percent,
                   grid_horizontal, grid_vertical,
                   (unsigned char*)buffer, (unsigned int)img_width, (unsigned int)img_height);
        return;
    }
    
    for (i = 0; i < sim->num; i++)
    {
        local_being = &(sim->beings[i]);
        x = 0;
        for (p = 0; p < half; p++)
        {
            x += local_being->learned_preference[p];
        }
        av_x += x * dimension / (half*255);
        
        y = 0;
        while (p < PREFERENCES)
        {
            y += local_being->learned_preference[p];
            p++;
        }
        av_y += y * dimension / ((PREFERENCES-half)*255);
    }
    av_x /= (n_int)sim->num;
    av_y /= (n_int)sim->num;
    
    for (i = 0; i < sim->num; i++)
    {
        local_being = &(sim->beings[i]);
        x = 0;
        for (p = 0; p < half; p++)
        {
            x += local_being->learned_preference[p];
        }
        dx = (x * dimension / (half*255)) - av_x;
        if (dx < 0) dx = -dx;
        av_dx += dx;
        
        y = 0;
        while (p < PREFERENCES)
        {
            y += local_being->learned_preference[p];
            p++;
        }
        dy = (y * dimension / ((PREFERENCES-half)*255))-av_y;
        if (dy < 0) dy = -dy;
        av_dy += dy;
    }
    av_dx /= (n_int)sim->num;
    av_dy /= (n_int)sim->num;
    if (av_dx < min_variance) av_dx = min_variance;
    if (av_dy < min_variance) av_dy = min_variance;
    
    min_x = av_x - av_dx*2 - 1;
    max_x = av_x + av_dx*2 + 1;
    min_y = av_y - av_dy*2 - 1;
    max_y = av_y + av_dy*2 + 1;
    
    if ((max_x <= min_x) || (max_y <= min_y)) return;
    
    for (i = 0; i < sim->num; i++)
    {
        local_being = &(sim->beings[i]);
        x = 0;
        for (p = 0; p < half; p++)
        {
            x += local_being->learned_preference[p];
        }
        x = x * dimension / (half*255);
        scope_update(&s, 0, x, min_x, max_x, (unsigned int)i);
        
        y = 0;
        while (p < PREFERENCES)
        {
            y += local_being->learned_preference[p];
            p++;
        }
        y = y * dimension / ((PREFERENCES-half)*255);
        scope_update(&s, 1, y, min_y, max_y, (unsigned int)i);
    }
    
    scope_draw(&s, update_type, intensity_percent,
               grid_horizontal, grid_vertical,
               (unsigned char*)buffer, (unsigned int)img_width, (unsigned int)img_height);
    
}

void graph_command(n_int gc_val)
{
    if (gc_val == GC_CLEAR_BRAINCODE)
    {
        graph_clear = 1;
        graph_state = GC_BRAINCODE;
    }
    else
    {
        graph_state = gc_val;
    }
}

void  graph_draw(noble_simulation * local_sim, n_byte * graph, n_int dim_x, n_int dim_y)
{    
    switch (graph_state)
    {
        case GC_IDEOSPHERE:
            graph_ideosphere(local_sim, graph, dim_x, dim_y);
            break;
        case GC_BRAINCODE:
            if (local_sim->select != NO_BEINGS_FOUND)
            {
                graph_braincode(local_sim, &local_sim->beings[local_sim->select],graph, dim_x, dim_y, graph_clear);
                graph_clear = 0;
            }
            break;
        case GC_HONOR:
            graph_honor_distribution(local_sim, PHOSPHENE_DRAW_ALL, graph, dim_x, dim_y);
            break;
        case GC_PATHOGENS:
            graph_pathogens(local_sim, PHOSPHENE_DRAW_ALL, graph, dim_x, dim_y);
            break;
        case GC_RELATIONSHIPS:
            graph_relationship_matrix(local_sim, graph, dim_x, dim_y);
            break;
        case GC_GENEPOOL:
            graph_genepool(local_sim, graph, dim_x, dim_y);
            break;
        case GC_PREFERENCES:
            graph_preferences(local_sim, PHOSPHENE_DRAW_ALL, graph, dim_x, dim_y);
            break;
        case GC_SOCIALSIM:
            graph_socialsim(local_sim, PHOSPHENE_DRAW_ALL, graph, dim_x, dim_y);
            break;
        case GC_MEET_PLACES:
            graph_meet_places(local_sim, PHOSPHENE_DRAW_ALL, graph, dim_x, dim_y);
            break;
        case GC_PHASESPACE:
            graph_phasespace(local_sim, PHOSPHENE_DRAW_ALL, graph, dim_x, dim_y, 0, 0);
            break;
        case GC_VASCULAR:
        default:
            if (local_sim->select != NO_BEINGS_FOUND)
            {
                /** set this to a non-zero value to show key points on the skeleton
                 which may be useful for debugging */
                
                /* doesn't work with Ofast optimization */
                n_byte show_skeleton_keypoints = 0;
                graph_vascular(&local_sim->beings[local_sim->select], graph,
                               dim_x, dim_y,
                               dim_x*10/100,dim_y*10/100,
                               dim_x*40/100,dim_y*90/100,
                               1, 1,
                               30, 0, 20, 20, 0,
                               show_skeleton_keypoints);
                
            }
            break;
    }
}


