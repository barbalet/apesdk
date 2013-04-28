/****************************************************************
 
 body.c
 
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

#include <math.h>
#include "entity.h"
#include "entity_internal.h"

enum {
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

const n_int bone_points[] = {
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

const n_int bone_arm_upper[] = {
    42,0, 53,475, /* axis */
    49,0,    16,8,     9,21,    2,37,    5,50,  10,64,  14,77,  14,95,  12,143,  12,193,
    10,241,  12,259,  18,286,  21,314,  21,343, 20,372, 17,408, 13,424,  5,438,   1,444,
    3,458,   14,470,  18,472,  28,473,  42,477, 52,476, 63,469, 75,477, 92,477,  90,458,
    99,455, 107,450, 111,440, 107,429, 100,425, 88,418, 77,407, 69,393, 65,382,  62,347,
    60,313,  56,270,  51,218,  52,174,  52,127, 54,88,  57,71,  64,68,  69,54,   75,47,
    81,36,   84,27,   77,10,   67,1
};

const n_int bone_arm_lower1[] = {
    34,4, 59,346, /* axis */
    48,0,   35,0,   21,1,   14,9,   15,15,  22,23,  24,31,  24,42,  20,67,  15,102,
    9,132,  4,157,  2,177,  3,197,   7,225, 11,258, 16,290, 18,308, 19,327, 20,337,
    19,347, 23,351, 36,349, 50,347, 60,342, 70,340, 72,336, 72,331, 67,327, 51,314,
    46,305, 40,285, 33,259, 31,233, 28,208, 28,183, 30,166, 37,131, 48,77,  52,60,
    45,45,  43,31,  45,13,  44,4
};

const n_int bone_arm_lower2[] = {
    34,4, 59,346, /* axis */
    65,0,   53,0,   48,11,  45,25,   40,40,   35,102,  35,127,  35,151,  39,178,  43,209,
    48,237, 54,266, 58,299, 60,317,  55,329,  53,339,  54,343,  69,344,  80,346,  88,349,
    91,343, 89,335, 84,324, 78,314,  75,293,  74,271,  70,239,  64,207,  60,174,  57,132,
    58,112, 62,77,  65,58,  72,48,   74,35,   85,27,   95,23,   98,16,   96,8,    86,5,
    77,2
};

const n_int bone_clavical[] = {
    23,201, 31,13, /* axis */
    26,4,    20,12,   19,22,   24,30,   30,39,   29,53,   24,77,   15,100,   10,114,   6,130,
    4,146,   5,159,   9,176,   14,189,  15,200,  18,206,  23,207,  28,206,   31,199,  31,192,
    27,182, 25,173,  22,156,   22,143,  24,126,  28,110,  39,88,   45,68,    49,52,   50,40,
    48,27,  43,12,   38,7
};

const n_int bone_leg_upper[] = {
    119,33, 80,416, /* axis */
    47,3,    36,7,   30,15,   27,22,   23,43,   19,73,   18,90,   20,112,   22,140,   23,178,
    24,223,  29,286, 32,330,  30,360,  26,378,  24,386,  28,398,  36,410,   49,419,   65,425,
    78,422,  84,417, 91,410, 102,405, 114,408, 126,408, 133,401, 134,389,  135,373,  125,362,
    114,354, 99,347, 89,330,  78,305,  71,274,  64,221,  64,174,  67,126,   73,93,    76,77,
    87,67,   99,63, 111,59,  116,61,  121,63,  126,58,  127,51,  122,38,   117,22,   109,14,
    99,13,   91,19,  86,24,   78,26,   70,25,   64,17,   66,7,    60,3
};

const n_int bone_leg_lower1[] = {
    94,5, 46,287, /* axis */
    23,26,  14,33,  12,45,  14,55,  16,68,  17,101,  18,132,  18,166,  17,195,  18,223,
    20,241, 17,251, 11,260,  5,269,  5,275, 14,280,  25,284,  33,283,  37,275,  36,261,
    32,240, 28,210, 27,171, 30,154, 31,134, 30,94,   31,69,   36,48,   36,36,   32,27
};

const n_int bone_leg_lower2[] = {
    94,5, 46,287, /* axis */
    66,1,   44,1,    28,1,    24,8,   25,24,  34,37,   41,49,   49,66,   54,85,   58,106,
    59,130, 59,165,  57,196,  51,227, 45,245, 40,254,  36,266,  39,280,  47,287,  59,287,
    77,281, 87,277,  90,272,  90,265, 84,250, 83,230,  83,188,  83,149,  85,109,  88,74,
    92,47,  100,38, 110,29,  116,18,  114,6,  98,5,    80,1
};

const n_int bone_pelvis[] = {
    214,27, 203,382, /* axis */
    213,24,    183,21,    166,22,    156,18,    139,12,    118,6,    100,6,    79,8,    68,11,    60,19,
    46,34,      35,49,    25,65,     17,81,      9,96,     6,108,    6,122,   15,136,   27,150,   44,165,
    59,181,     71,197,   75,221,    78,240,    77,255,    84,254,   97,259, 104,271,  107,284,  106,295,
    105,303,   112,315,  116,330,   118,347,   112,358,   105,365,  101,367, 102,376,  111,384,  127,393,
    141,393,   158,390,  177,387,   189,381,   201,369,   213,359,  215,336, 216,349,  218,358,  225,370,
    240,380,   254,385,  272,387,   294,387,   307,387,   316,375,  317,367, 310,357,  300,337,  300,320,
    308,308,   316,304,  321,294,   320,283,   322,275,   321,266,  330,259, 342,254,  349,245,  348,217,
    354,192,   364,176,  381,162,   402,144,   423,131,   432,121,  433,104, 427,84,   411,60,   392,37,
    377,22,    360,11,   336,7,     311,11,    277,18,    258,25,   244,26,  222,24,
    101,272, /* left leg */
    321,265  /* right leg */
};

const n_int bone_hand[] = {
    20,6, 14,100, /* axis */
    19,4,  12,6,    9,12,   9,23,   10,40,    9,59,    8,75,    6,86,    3,91,    8,98,
    9,102, 15,102, 20,101, 24,96,   27,90,   27,84,   24,76,   23,64,   24,40,   24,24,
    26,18, 29,10,  28,4,   23,3
};

const n_int bone_finger[] = {
    14,1, 15,114, /* axis */
    13,1,    6,3,   4,6,   3,10,    6,14,    6,26,    6,40,    8,52,    9,56,    8,60,
    6,66,    6,72,  8,82,  7,92,    9,98,   10,104,  11,112,  14,114,  16,113,  18,107,
    19,100, 19,92, 20,86, 20,78,   20,68,   23,60,   25,52,   26,44,   24,34,   23,18,
    23,7,   21,3,  16,2
};

const n_int bone_vertibra[] = {
    53,1, 53,47, /* axis */
    53,2,   41,2,   30,4,   22,7,   22,16,   22,24,   20,32,   14,32,   4,34,   1,36,
    1,42,   5,46,  14,48,  18,48,   18,54,   24,53,   33,50,   43,48,  53,49,  62,50,
    72,53, 81,56,  87,56,  88,50,   95,50,   98,48,  100,42,   98,38,  91,37,  84,35,
    84,33, 84,24,  86,14,  84,10,   74,6,    62,0
};

const n_int bone_scapula[] = {
    35,0, 134,214, /* axis */
    37,3,     22,7,    11,15,    6,21,   4,28,    4,40,    9,58,    19,80,    38,113,    60,142,
    81,178,  101,204, 114,219, 128,226, 138,230, 153,221, 165,211, 171,194,  173,170,   175,143,
    177,116, 176,97,  168,81,  177,69,  187,60,  188,41,  184,23,  175,13,   162,11,    151,12,
    146,22,  136,34,  128,46,  118,59,  113,56,  109,43,  100,34,  93,34,    84,36,     83,46,
    75,51,    67,52,   58,42,   58,34,   48,27,   51,19,   52,12,  45,6
};

const n_int bone_ribs[] = {
    227,11, 212,350, /* axis */
    226,10,   199,7,   177,15,   152,25,   139,37,  122,52,   112,62,   104,75,   91,93,   85,108,
    76,127,   69,149,  62,169,   61,183,   53,208,  53,218,   44,230,   39,244,   40,255,  36,262,
    30,278,   28,295,  28,299,   22,317,   22,336,  23,348,   16,365,   15,389,   14,411,  10,437,
    13,449,   18,446,  18,434,   33,425,   56,407,  83,392,  116,382,  141,373,  159,362, 173,351,
    184,351, 213,348, 235,355,  242,356,  244,352, 264,363,  283,377,  304,386,  333,396, 355,409,
    375,423, 395,443, 409,458,  416,450,  419,440, 415,429,  417,404,  411,383,  408,366, 411,348,
    411,327, 409,315, 407,296,  402,278,  395,267, 393,243,  385,232,  384,213,  379,200, 371,194,
    367,172, 359,155, 357,134,  344,114,  334,90,  322,80,   318,67,   301,47,   289,30,  275,22,
    247,15,  227,12,
    35,3, 416,3, /* shoulder sockets */
    35,-20, 416,-20, /* shoulders */
};

static void outline_points(const n_int * source_points,
                           n_int no_of_source_points, n_int extra_points,
                           n_int x, n_int y,
                           n_byte mirror,
                           n_int scale_width, n_int scale_length,
                           n_int angle,
                           n_int *axis_x, n_int *axis_y,
                           n_int *extra_x1, n_int *extra_y1,
                           n_int *extra_x2, n_int *extra_y2,
                           n_int *extra_x3, n_int *extra_y3,
                           n_int *extra_x4, n_int *extra_y4,
                           n_int * points, n_int * no_of_points)
{
    n_int i, px=0,py=0,dx,dy,axis_length,point_length;
    n_int pivot_x = source_points[0];
    n_int pivot_y = source_points[1];
    float axis_angle, point_angle;
    float ang = angle*3.1415927f/3600;
    
    dx = (source_points[2] - pivot_x)*scale_width/1000;
    dy = (source_points[3] - pivot_y)*scale_length/1000;
    axis_length = (n_int)sqrt(dx*dx + dy*dy);
    if (axis_length < 1) axis_length=1;
    if (mirror != 0)
    {
        dx = -dx;
    }
    axis_angle = (float)acos(dx/(float)axis_length);
    if (dy < 0) axis_angle = (2*3.1415927f)-axis_angle;
    
    *axis_x = x + (n_int)(axis_length*sin(ang+(3.1415927f/2)-axis_angle));
    *axis_y = y + (n_int)(axis_length*cos(ang+(3.1415927f/2)-axis_angle));
    
    for (i = 2; i < no_of_source_points + 2 + extra_points; i++)
    {
        dx = (source_points[i*2]-pivot_x)*scale_width/1000;
        dy = (source_points[i*2+1]-pivot_y)*scale_length/1000;
        point_length = (n_int)sqrt(dx*dx + dy*dy);
        if (point_length < 1) point_length=1;
        if (mirror != 0)
        {
            dx = -dx;
        }
        point_angle = (float)acos(dx/(float)point_length);
        if (dy < 0) point_angle = (2*3.1415927f)-point_angle;
        
        px = x + (n_int)(point_length*sin(ang+point_angle-axis_angle));
        py = y + (n_int)(point_length*cos(ang+point_angle-axis_angle));
        
        if (i < no_of_source_points + 2)
        {
            points[(*no_of_points)*2] = px;
            points[(*no_of_points)*2+1] = py;
            *no_of_points = *no_of_points + 1;
        }
        
        *extra_x1 = *extra_x2;
        *extra_y1 = *extra_y2;
        *extra_x2 = *extra_x3;
        *extra_y2 = *extra_y3;
        *extra_x3 = *extra_x4;
        *extra_y3 = *extra_y4;
        *extra_x4 = px;
        *extra_y4 = py;
    }
    
    points[(*no_of_points)*2] = 9999;
    points[(*no_of_points)*2+1] = 9999;
    *no_of_points = *no_of_points + 1;
}

/* returns a set of points corresponding to key locations on the skeleton */
n_int body_skeleton_points(noble_being * being, n_int * keypoints, n_int *points)
{
    n_int scale_width=1000, scale_length=1000, angle=0;
    n_int extra_x[4], extra_y[4];
    n_int i, vertical, vertibra_x=0, vertibra_y=0, no_of_points = 0;
    n_int arm_angle = 30; /* angle of arms in degrees */
    n_int leg_angle = 20;
    n_int knuckle_x = 0;
    n_int knuckle_y = 0;
    /* position of the bottom of the neck */
    keypoints[SKELETON_NECK*2] = 0;
    keypoints[SKELETON_NECK*2+1] = 0;
    
    /* position of the bottom of the ribs */
    outline_points(bone_ribs, bone_points[BONE_RIBS],4,
                   keypoints[SKELETON_NECK*2], keypoints[SKELETON_NECK*2+1], 0,
                   scale_width, scale_length,
                   angle,
                   &keypoints[SKELETON_LUMBAR*2],
                   &keypoints[SKELETON_LUMBAR*2+1],
                   &keypoints[SKELETON_LEFT_SHOULDER_SOCKET*2], &keypoints[SKELETON_LEFT_SHOULDER_SOCKET*2+1],
                   &keypoints[SKELETON_RIGHT_SHOULDER_SOCKET*2], &keypoints[SKELETON_RIGHT_SHOULDER_SOCKET*2+1],
                   &keypoints[SKELETON_LEFT_SHOULDER*2], &keypoints[SKELETON_LEFT_SHOULDER*2+1],
                   &keypoints[SKELETON_RIGHT_SHOULDER*2], &keypoints[SKELETON_RIGHT_SHOULDER*2+1],
                   points, &i);
    
    /* left scapula */
    outline_points(bone_scapula, bone_points[BONE_SCAPULA],0,
                   keypoints[SKELETON_LEFT_SHOULDER*2],
                   keypoints[SKELETON_LEFT_SHOULDER*2+1],
                   0, scale_width, scale_length,
                   angle-600,
                   &extra_x[0], &extra_y[0],
                   &extra_x[0], &extra_y[0],
                   &extra_x[1], &extra_y[1],
                   &extra_x[2], &extra_y[2],
                   &extra_x[3], &extra_y[3],
                   points, &no_of_points);
    
    /* right scapula */
    outline_points(bone_scapula, bone_points[BONE_SCAPULA],0,
                   keypoints[SKELETON_RIGHT_SHOULDER*2],
                   keypoints[SKELETON_RIGHT_SHOULDER*2+1],
                   1, scale_width, scale_length,
                   angle+500,
                   &extra_x[0], &extra_y[0],
                   &extra_x[0], &extra_y[0],
                   &extra_x[1], &extra_y[1],
                   &extra_x[2], &extra_y[2],
                   &extra_x[3], &extra_y[3],
                   points, &no_of_points);
    
    /* ribs */
    outline_points(bone_ribs, bone_points[BONE_RIBS],4,
                   keypoints[SKELETON_NECK*2], keypoints[SKELETON_NECK*2+1], 0,
                   scale_width, scale_length,
                   angle,
                   &keypoints[SKELETON_LUMBAR*2],
                   &keypoints[SKELETON_LUMBAR*2+1],
                   &keypoints[SKELETON_LEFT_SHOULDER_SOCKET*2], &keypoints[SKELETON_LEFT_SHOULDER_SOCKET*2+1],
                   &keypoints[SKELETON_RIGHT_SHOULDER_SOCKET*2], &keypoints[SKELETON_RIGHT_SHOULDER_SOCKET*2+1],
                   &keypoints[SKELETON_LEFT_SHOULDER*2], &keypoints[SKELETON_LEFT_SHOULDER*2+1],
                   &keypoints[SKELETON_RIGHT_SHOULDER*2], &keypoints[SKELETON_RIGHT_SHOULDER*2+1],
                   points, &no_of_points);
    
    /* position of the top of the pelvis */
    keypoints[SKELETON_PELVIS*2] = keypoints[SKELETON_LUMBAR*2];
    keypoints[SKELETON_PELVIS*2+1] =
    keypoints[SKELETON_LUMBAR*2+1] +
    ((keypoints[SKELETON_LUMBAR*2+1]-keypoints[SKELETON_NECK*2+1])*40/100);
    
    /* position of hips */
    outline_points(bone_pelvis, bone_points[BONE_PELVIS],2,
                   keypoints[SKELETON_PELVIS*2], keypoints[SKELETON_PELVIS*2+1],
                   0, scale_width, scale_length,
                   angle,
                   &extra_x[0], &extra_y[0],
                   &extra_x[0], &extra_y[0],
                   &extra_x[1], &extra_y[1],
                   &keypoints[SKELETON_LEFT_HIP*2], &keypoints[SKELETON_LEFT_HIP*2+1],
                   &keypoints[SKELETON_RIGHT_HIP*2], &keypoints[SKELETON_RIGHT_HIP*2+1],
                   points, &no_of_points);
    
    /* left upper leg */
    outline_points(bone_leg_upper, bone_points[BONE_LEG_UPPER],0,
                   keypoints[SKELETON_LEFT_HIP*2], keypoints[SKELETON_LEFT_HIP*2+1],
                   0, scale_width, scale_length,
                   angle+(leg_angle*10),
                   &keypoints[SKELETON_LEFT_KNEE*2],
                   &keypoints[SKELETON_LEFT_KNEE*2+1],
                   &extra_x[0], &extra_y[0],
                   &extra_x[1], &extra_y[1],
                   &extra_x[2], &extra_y[2],
                   &extra_x[3], &extra_y[3],
                   points, &no_of_points);
    
    /* left lower leg 1 */
    outline_points(bone_leg_lower1, bone_points[BONE_LEG_LOWER1],0,
                   keypoints[SKELETON_LEFT_KNEE*2], keypoints[SKELETON_LEFT_KNEE*2+1],
                   0, scale_width, scale_length,
                   angle,
                   &keypoints[SKELETON_LEFT_ANKLE*2],
                   &keypoints[SKELETON_LEFT_ANKLE*2+1],
                   &extra_x[0], &extra_y[0],
                   &extra_x[1], &extra_y[1],
                   &extra_x[2], &extra_y[2],
                   &extra_x[3], &extra_y[3],
                   points, &no_of_points);
    
    /* left lower leg 2 */
    outline_points(bone_leg_lower2, bone_points[BONE_LEG_LOWER2],0,
                   keypoints[SKELETON_LEFT_KNEE*2], keypoints[SKELETON_LEFT_KNEE*2+1],
                   0, scale_width, scale_length,
                   angle,
                   &keypoints[SKELETON_LEFT_ANKLE*2],
                   &keypoints[SKELETON_LEFT_ANKLE*2+1],
                   &extra_x[0], &extra_y[0],
                   &extra_x[1], &extra_y[1],
                   &extra_x[2], &extra_y[2],
                   &extra_x[3], &extra_y[3],
                   points, &no_of_points);
    
    /* right upper leg */
    outline_points(bone_leg_upper, bone_points[BONE_LEG_UPPER],0,
                   keypoints[SKELETON_RIGHT_HIP*2], keypoints[SKELETON_RIGHT_HIP*2+1],
                   1, scale_width, scale_length,
                   angle-(leg_angle*10),
                   &keypoints[SKELETON_RIGHT_KNEE*2],
                   &keypoints[SKELETON_RIGHT_KNEE*2+1],
                   &extra_x[0], &extra_y[0],
                   &extra_x[1], &extra_y[1],
                   &extra_x[2], &extra_y[2],
                   &extra_x[3], &extra_y[3],
                   points, &no_of_points);
    
    /* right lower leg 1 */
    outline_points(bone_leg_lower1, bone_points[BONE_LEG_LOWER1],0,
                   keypoints[SKELETON_RIGHT_KNEE*2], keypoints[SKELETON_RIGHT_KNEE*2+1],
                   1, scale_width, scale_length,
                   angle,
                   &keypoints[SKELETON_RIGHT_ANKLE*2],
                   &keypoints[SKELETON_RIGHT_ANKLE*2+1],
                   &extra_x[0], &extra_y[0],
                   &extra_x[1], &extra_y[1],
                   &extra_x[2], &extra_y[2],
                   &extra_x[3], &extra_y[3],
                   points, &no_of_points);
    
    /* right lower leg 2 */
    outline_points(bone_leg_lower2, bone_points[BONE_LEG_LOWER2],0,
                   keypoints[SKELETON_RIGHT_KNEE*2], keypoints[SKELETON_RIGHT_KNEE*2+1],
                   1, scale_width, scale_length,
                   angle,
                   &keypoints[SKELETON_RIGHT_ANKLE*2],
                   &keypoints[SKELETON_RIGHT_ANKLE*2+1],
                   &extra_x[0], &extra_y[0],
                   &extra_x[1], &extra_y[1],
                   &extra_x[2], &extra_y[2],
                   &extra_x[3], &extra_y[3],
                   points, &no_of_points);
    
    /* left upper arm */
    outline_points(bone_arm_upper, bone_points[BONE_ARM_UPPER],0,
                   keypoints[SKELETON_LEFT_SHOULDER_SOCKET*2],
                   keypoints[SKELETON_LEFT_SHOULDER_SOCKET*2+1],
                   0, scale_width, scale_length,
                   angle+(arm_angle*10),
                   &keypoints[SKELETON_LEFT_ELBOW*2],
                   &keypoints[SKELETON_LEFT_ELBOW*2+1],
                   &extra_x[0], &extra_y[0],
                   &extra_x[1], &extra_y[1],
                   &extra_x[2], &extra_y[2],
                   &extra_x[3], &extra_y[3],
                   points, &no_of_points);
    
    /* left lower arm 1 */
    outline_points(bone_arm_lower1, bone_points[BONE_ARM_LOWER1],0,
                   keypoints[SKELETON_LEFT_ELBOW*2],
                   keypoints[SKELETON_LEFT_ELBOW*2+1],
                   0, scale_width, scale_length,
                   angle,
                   &keypoints[SKELETON_LEFT_WRIST*2],
                   &keypoints[SKELETON_LEFT_WRIST*2+1],
                   &extra_x[0], &extra_y[0],
                   &extra_x[1], &extra_y[1],
                   &extra_x[2], &extra_y[2],
                   &extra_x[3], &extra_y[3],
                   points, &no_of_points);
    
    /* left lower arm 2 */
    outline_points(bone_arm_lower2, bone_points[BONE_ARM_LOWER2],0,
                   keypoints[SKELETON_LEFT_ELBOW*2],
                   keypoints[SKELETON_LEFT_ELBOW*2+1],
                   0, scale_width, scale_length,
                   angle,
                   &keypoints[SKELETON_LEFT_WRIST*2],
                   &keypoints[SKELETON_LEFT_WRIST*2+1],
                   &extra_x[0], &extra_y[0],
                   &extra_x[1], &extra_y[1],
                   &extra_x[2], &extra_y[2],
                   &extra_x[3], &extra_y[3],
                   points, &no_of_points);
    
    /* right upper arm */
    outline_points(bone_arm_upper, bone_points[BONE_ARM_UPPER],0,
                   keypoints[SKELETON_RIGHT_SHOULDER_SOCKET*2],
                   keypoints[SKELETON_RIGHT_SHOULDER_SOCKET*2+1],
                   1, scale_width, scale_length,
                   angle-(arm_angle*10),
                   &keypoints[SKELETON_RIGHT_ELBOW*2],
                   &keypoints[SKELETON_RIGHT_ELBOW*2+1],
                   &extra_x[0], &extra_y[0],
                   &extra_x[1], &extra_y[1],
                   &extra_x[2], &extra_y[2],
                   &extra_x[3], &extra_y[3],
                   points, &no_of_points);
    
    /* right lower arm 1 */
    outline_points(bone_arm_lower1, bone_points[BONE_ARM_LOWER1],0,
                   keypoints[SKELETON_RIGHT_ELBOW*2],
                   keypoints[SKELETON_RIGHT_ELBOW*2+1],
                   1, scale_width, scale_length,
                   angle,
                   &keypoints[SKELETON_RIGHT_WRIST*2],
                   &keypoints[SKELETON_RIGHT_WRIST*2+1],
                   &extra_x[0], &extra_y[0],
                   &extra_x[1], &extra_y[1],
                   &extra_x[2], &extra_y[2],
                   &extra_x[3], &extra_y[3],
                   points, &no_of_points);
    
    /* left lower arm 2 */
    outline_points(bone_arm_lower2, bone_points[BONE_ARM_LOWER2],0,
                   keypoints[SKELETON_RIGHT_ELBOW*2],
                   keypoints[SKELETON_RIGHT_ELBOW*2+1],
                   1, scale_width, scale_length,
                   angle,
                   &keypoints[SKELETON_RIGHT_WRIST*2],
                   &keypoints[SKELETON_RIGHT_WRIST*2+1],
                   &extra_x[0], &extra_y[0],
                   &extra_x[1], &extra_y[1],
                   &extra_x[2], &extra_y[2],
                   &extra_x[3], &extra_y[3],
                   points, &no_of_points);
    
    /* left clavical */
    outline_points(bone_clavical, bone_points[BONE_CLAVICAL],0,
                   keypoints[SKELETON_LEFT_SHOULDER*2],
                   keypoints[SKELETON_LEFT_SHOULDER*2+1],
                   0, scale_width, scale_length,
                   angle-1800,
                   &keypoints[SKELETON_LEFT_COLLAR*2],
                   &keypoints[SKELETON_LEFT_COLLAR*2+1],
                   &extra_x[0], &extra_y[0],
                   &extra_x[1], &extra_y[1],
                   &extra_x[2], &extra_y[2],
                   &extra_x[3], &extra_y[3],
                   points, &no_of_points);
    
    /* right clavical */
    outline_points(bone_clavical, bone_points[BONE_CLAVICAL],0,
                   keypoints[SKELETON_RIGHT_SHOULDER*2],
                   keypoints[SKELETON_RIGHT_SHOULDER*2+1],
                   1, scale_width, scale_length,
                   angle+1700,
                   &keypoints[SKELETON_RIGHT_COLLAR*2],
                   &keypoints[SKELETON_RIGHT_COLLAR*2+1],
                   &extra_x[0], &extra_y[0],
                   &extra_x[1], &extra_y[1],
                   &extra_x[2], &extra_y[2],
                   &extra_x[3], &extra_y[3],
                   points, &no_of_points);
    
    vertical = keypoints[SKELETON_NECK*2+1];
    for (i = 0; i < SKELETON_VERTIBRA_RIBS; i++)
    {
        outline_points(bone_vertibra, bone_points[BONE_VERTIBRA],0,
                       keypoints[SKELETON_NECK*2]+((keypoints[SKELETON_LUMBAR*2]-keypoints[SKELETON_NECK*2])*i/SKELETON_VERTIBRA_RIBS),
                       vertical,
                       0,
                       (scale_width*5/10)+((scale_width*5/10)*i/SKELETON_VERTIBRA_RIBS),
                       (scale_length*5/10)+((scale_length*5/10)*i/SKELETON_VERTIBRA_RIBS),
                       angle,
                       &vertibra_x, &vertibra_y,
                       &extra_x[0], &extra_y[0],
                       &extra_x[1], &extra_y[1],
                       &extra_x[2], &extra_y[2],
                       &extra_x[3], &extra_y[3],
                       points, &no_of_points);
        vertical = vertibra_y;
    }
    
    vertical = keypoints[SKELETON_LUMBAR*2+1];
    for (i = 0; i < SKELETON_LUMBAR_VERTIBRA; i++)
    {
        outline_points(bone_vertibra, bone_points[BONE_VERTIBRA],0,
                       keypoints[SKELETON_LUMBAR*2],
                       vertical,
                       0, scale_width, scale_length,
                       angle,
                       &vertibra_x, &vertibra_y,
                       &extra_x[0], &extra_y[0],
                       &extra_x[1], &extra_y[1],
                       &extra_x[2], &extra_y[2],
                       &extra_x[3], &extra_y[3],
                       points, &no_of_points);
        vertical = vertibra_y;
    }
    for (i = 0; i < SKELETON_LUMBAR_VERTIBRA2; i++)
    {
        outline_points(bone_vertibra, bone_points[BONE_VERTIBRA],0,
                       keypoints[SKELETON_LUMBAR*2],
                       vertical,
                       0, scale_width*(SKELETON_LUMBAR_VERTIBRA2-i)/SKELETON_LUMBAR_VERTIBRA2,
                       ((scale_length*2/3)*(SKELETON_LUMBAR_VERTIBRA2-i)/SKELETON_LUMBAR_VERTIBRA2),
                       angle,
                       &vertibra_x, &vertibra_y,
                       &extra_x[0], &extra_y[0],
                       &extra_x[1], &extra_y[1],
                       &extra_x[2], &extra_y[2],
                       &extra_x[3], &extra_y[3],
                       points, &no_of_points);
        vertical = vertibra_y;
    }
    
    for (i = 0; i < 4; i++)
    {
        /* left hand */
        outline_points(bone_hand, bone_points[BONE_HAND],0,
                       keypoints[SKELETON_LEFT_WRIST*2]-(i*15),
                       keypoints[SKELETON_LEFT_WRIST*2+1],
                       0, scale_width, scale_length,
                       angle+200-(i*400/4),
                       &knuckle_x, &knuckle_y,
                       &extra_x[0], &extra_y[0],
                       &extra_x[1], &extra_y[1],
                       &extra_x[2], &extra_y[2],
                       &extra_x[3], &extra_y[3],
                       points, &no_of_points);
        
        /* left finger */
        outline_points(bone_finger, bone_points[BONE_FINGER],0,
                       knuckle_x, knuckle_y,
                       0, scale_width, scale_length,
                       angle+400-(i*800/4),
                       &extra_x[0], &extra_y[0],
                       &extra_x[0], &extra_y[0],
                       &extra_x[1], &extra_y[1],
                       &extra_x[2], &extra_y[2],
                       &extra_x[3], &extra_y[3],
                       points, &no_of_points);
        
        /* right hand */
        outline_points(bone_hand, bone_points[BONE_HAND],0,
                       keypoints[SKELETON_RIGHT_WRIST*2]+((3-i)*15),
                       keypoints[SKELETON_RIGHT_WRIST*2+1],
                       1, scale_width, scale_length,
                       angle+200-(i*400/4),
                       &knuckle_x, &knuckle_y,
                       &extra_x[0], &extra_y[0],
                       &extra_x[1], &extra_y[1],
                       &extra_x[2], &extra_y[2],
                       &extra_x[3], &extra_y[3],
                       points, &no_of_points);
        
        /* right finger */
        outline_points(bone_finger, bone_points[BONE_FINGER],0,
                       knuckle_x, knuckle_y,
                       1, scale_width, scale_length,
                       angle+400-(i*800/4),
                       &extra_x[0], &extra_y[0],
                       &extra_x[0], &extra_y[0],
                       &extra_x[1], &extra_y[1],
                       &extra_x[2], &extra_y[2],
                       &extra_x[3], &extra_y[3],
                       points, &no_of_points);
    }
    
    /* left thumb */
    outline_points(bone_finger, bone_points[BONE_FINGER],0,
                   keypoints[SKELETON_LEFT_WRIST*2]-50,
                   keypoints[SKELETON_LEFT_WRIST*2+1],
                   0, scale_width, scale_length,
                   angle-800,
                   &extra_x[0], &extra_y[0],
                   &extra_x[0], &extra_y[0],
                   &extra_x[1], &extra_y[1],
                   &extra_x[2], &extra_y[2],
                   &extra_x[3], &extra_y[3],
                   points, &no_of_points);
    
    /* right thumb */
    outline_points(bone_finger, bone_points[BONE_FINGER],0,
                   keypoints[SKELETON_RIGHT_WRIST*2]+50,
                   keypoints[SKELETON_RIGHT_WRIST*2+1],
                   1, scale_width, scale_length,
                   angle+800,
                   &extra_x[0], &extra_y[0],
                   &extra_x[0], &extra_y[0],
                   &extra_x[1], &extra_y[1],
                   &extra_x[2], &extra_y[2],
                   &extra_x[3], &extra_y[3],
                   points, &no_of_points);
    
    /*printf("no_of_points %d\n",no_of_points*2);*/
    
    return no_of_points;
}

static void body_action_give(noble_simulation * sim, noble_being * local, noble_being * other, n_byte2 carrying)
{
    n_byte hand = BODY_RIGHT_HAND;
    
    if (carrying == 0)
    {
        hand = BODY_LEFT_HAND;
        carrying = OBJECTS_CARRIED(local,hand);
    }
    if ((carrying != 0) &&
        ((OBJECTS_CARRIED(other,BODY_LEFT_HAND)==0) ||
         (OBJECTS_CARRIED(other,BODY_RIGHT_HAND)==0)))
    {
        GET_A(local,ATTENTION_BODY) = BODY_RIGHT_HAND;
        GET_A(other,ATTENTION_BODY) = BODY_RIGHT_HAND;
        
        episodic_interaction(sim, local, other, EVENT_GIVEN, EPISODIC_AFFECT_ZERO, carrying);
        episodic_interaction(sim, other, local, EVENT_GIVEN_BY, AFFECT_RECEIVE, carrying);
        
        OBJECTS_DROP(local,hand);
        if (OBJECTS_CARRIED(other,BODY_RIGHT_HAND)==0)
        {
            OBJECT_TAKE(other,BODY_RIGHT_HAND,carrying);
        }
        else
        {
            OBJECT_TAKE(other,BODY_LEFT_HAND,carrying);
        }
    }
}

static void body_action_bash(noble_simulation * sim, noble_being * local, noble_being * other, n_byte2 carrying)
{
    n_byte hand = BODY_RIGHT_HAND;
    n_int  index, hit = 0;
    social_link * graph;
    
    if (carrying == 0)
    {
        hand = BODY_LEFT_HAND;
        carrying = OBJECTS_CARRIED(local,hand);
    }
    if (carrying!=0)
    {
        if ((carrying & INVENTORY_BRANCH) || (carrying & INVENTORY_ROCK))
        {
            GET_A(local,ATTENTION_BODY) = BODY_RIGHT_HAND;
            GET_A(other,ATTENTION_BODY) = BODY_BACK;
            index = get_social_link(other,local,sim);
            if (index>-1)
            {
                graph = GET_SOC(sim, other);
                if (!graph) return;
                if (graph[index].friend_foe>1) graph[index].friend_foe-=2;
            }
            if ((carrying & INVENTORY_ROCK) && (math_random(other->seed)>THROW_ACCURACY))
            {
                hit=1;
                GET_E(other) -= SQUABBLE_ENERGY_ROCK_HURL;
                
                INDICATOR_ADD(sim, IT_AVERAGE_ENERGY_OUTPUT, SQUABBLE_ENERGY_ROCK_HURL);
            }
            if ((carrying & INVENTORY_BRANCH) && (math_random(other->seed)>WHACK_ACCURACY))
            {
                hit=1;
                GET_E(other) -= SQUABBLE_ENERGY_BRANCH_WHACK;
                
                INDICATOR_ADD(sim, IT_AVERAGE_ENERGY_OUTPUT, SQUABBLE_ENERGY_BRANCH_WHACK);
            }
        }
        if (carrying & INVENTORY_BRANCH)
        {
            if (hit!=0)
            {
                episodic_interaction(sim, local, other, EVENT_WHACKED, EPISODIC_AFFECT_ZERO, 0);
                episodic_interaction(sim, other, local, EVENT_WHACKED_BY, AFFECT_WHACKED, 0);
            }
        }
        if (carrying & INVENTORY_ROCK)
        {
            episodic_interaction(sim, local, other, EVENT_HURLED, EPISODIC_AFFECT_ZERO, 0);
            if (hit!=0)
            {
                episodic_interaction(sim, other, local, EVENT_HURLED_BY, AFFECT_HURL, 0);
            }
        }
    }
    
}

static void body_action_interactive(noble_simulation * sim, noble_being * local, noble_being * other,
                                    n_byte local_attention, n_byte other_attention, n_byte kind)
{
    GET_A(local,ATTENTION_BODY) = local_attention;
    GET_A(other,ATTENTION_BODY) = other_attention;
    episodic_interaction(sim, local, other, kind, EPISODIC_AFFECT_ZERO, 0);
    episodic_interaction(sim, other, local, kind+1, EPISODIC_AFFECT_ZERO, 0);
}

/*** This block should also be the same function ***/

static void body_action_interactive_change(noble_simulation * sim, noble_being * local, noble_being * other,
                                           n_byte local_attention, n_byte other_attention, n_byte kind, n_byte positive, n_int affect)
{
    n_int index;
    GET_A(local,ATTENTION_BODY) = local_attention;
    GET_A(other,ATTENTION_BODY) = other_attention;
    index = get_social_link(other,local,sim);
    if (index>-1)
    {
        social_link * graph = GET_SOC(sim, other);
        if (!graph) return;
        if (positive)
        {
            if (graph[index].friend_foe<255) graph[index].friend_foe++;
        }
        else
        {
            if (graph[index].friend_foe>0) graph[index].friend_foe--;
        }
    }
    
    episodic_interaction(sim, local, other, kind, EPISODIC_AFFECT_ZERO, 0);
    episodic_interaction(sim, other, local, kind+1, affect, 0);
}

static void body_action_hand_object(noble_simulation * sim, noble_being * local, n_byte2 carrying, n_byte hand, n_byte kind)
{
    if (carrying == 0)
    {
        hand = BODY_LEFT_HAND;
        carrying = OBJECTS_CARRIED(local,hand);
    }
    if (carrying!=0)
    {
        if (carrying & INVENTORY_BRANCH)
        {
            episodic_self(sim, local, kind, EPISODIC_AFFECT_ZERO, INVENTORY_BRANCH);
        }
        else
        {
            if (carrying & INVENTORY_TWIG)
            {
                episodic_self(sim, local, kind, EPISODIC_AFFECT_ZERO, INVENTORY_TWIG);
            }
            else
            {
                if (carrying & INVENTORY_SPEAR)
                {
                    episodic_self(sim, local, kind, EPISODIC_AFFECT_ZERO, INVENTORY_SPEAR);
                }
            }
        }
    }
}

static void body_action_jab(noble_simulation * sim, noble_being * local, n_byte2 carrying, n_byte hand)
{
    n_byte carrying2 = OBJECTS_CARRIED(local,BODY_LEFT_HAND);
    if ((carrying & INVENTORY_SPEAR) ||
        (carrying2 & INVENTORY_SPEAR))
    {
        n_int loc_f = GET_F(local);
        n_int az;
        n_vect2 location_vector,facing_vector,slope_vector;
        
        vect2_byte2(&location_vector, (n_byte2 *)&GET_X(local));
        vect2_direction(&facing_vector,(n_byte)loc_f,4);
        land_vect2(&slope_vector,&az,sim->land,&location_vector);
        
        if ((az > WATER_MAP) && (az < TIDE_MAX))
        {
            /* some probability of spearing a fish */
            if (math_random(local->seed)<FISHING_PROB)
            {
                /* carry fish */
                if (carrying & INVENTORY_SPEAR)
                {
                    OBJECT_TAKE(local,BODY_LEFT_HAND,INVENTORY_FISH);
                }
                else
                {
                    OBJECT_TAKE(local,hand,INVENTORY_FISH);
                }
                episodic_self(sim, local, EVENT_FISH, AFFECT_FISH, 0);
            }
        }
    }
}

static void body_action_bash_objects(noble_simulation * sim, noble_being * local, n_byte2 carrying, n_byte hand)
{
    n_byte carrying2 = OBJECTS_CARRIED(local,BODY_LEFT_HAND);
    if ((carrying & INVENTORY_ROCK) && (carrying2 & INVENTORY_ROCK))
    {
        /* bash two rocks to make a scraper */
        OBJECTS_DROP(local,hand);
        OBJECT_TAKE(local,hand,INVENTORY_SCRAPER);
    }
    if (((carrying & INVENTORY_ROCK) && (carrying2 & INVENTORY_NUT)) ||
        ((carrying & INVENTORY_NUT) && (carrying2 & INVENTORY_ROCK)))
    {
        /* bash nut with a rock */
        if (carrying & INVENTORY_NUT)
        {
            OBJECTS_DROP(local,hand);
            OBJECT_TAKE(local,hand,INVENTORY_NUT_CRACKED);
        }
        else
        {
            OBJECTS_DROP(local,BODY_LEFT_HAND);
            OBJECT_TAKE(local,BODY_LEFT_HAND,INVENTORY_NUT_CRACKED);
        }
    }
    if (((carrying & INVENTORY_BRANCH) && (carrying2 & INVENTORY_SCRAPER)) ||
        ((carrying & INVENTORY_SCRAPER) && (carrying2 & INVENTORY_BRANCH)))
    {
        /* use a scraper to make a spear */
        if (carrying & INVENTORY_BRANCH)
        {
            OBJECTS_DROP(local,hand);
            OBJECT_TAKE(local,hand,INVENTORY_SPEAR);
        }
        else
        {
            OBJECTS_DROP(local,BODY_LEFT_HAND);
            OBJECT_TAKE(local,BODY_LEFT_HAND,INVENTORY_SPEAR);
        }
    }
    if (((carrying & INVENTORY_BRANCH) && (carrying2 & INVENTORY_NUT)) ||
        ((carrying & INVENTORY_NUT) && (carrying2 & INVENTORY_BRANCH)))
    {
        /* whack nut with a branch */
        if (carrying & INVENTORY_NUT)
        {
            OBJECTS_DROP(local,hand);
            OBJECT_TAKE(local,hand,INVENTORY_NUT_CRACKED);
        }
        else
        {
            OBJECTS_DROP(local,BODY_LEFT_HAND);
            OBJECT_TAKE(local,BODY_LEFT_HAND,INVENTORY_NUT_CRACKED);
        }
    }
}


static void body_action_chew(noble_simulation * sim, noble_being * local, n_byte2 carrying, n_byte hand)
{
    if (!((carrying & INVENTORY_GRASS) ||
          (carrying & INVENTORY_TWIG) ||
          (carrying & INVENTORY_FISH) ||
          (carrying & INVENTORY_NUT_CRACKED)))
    {
        hand = BODY_LEFT_HAND;
        carrying = OBJECTS_CARRIED(local,hand);
    }
    if ((carrying & INVENTORY_GRASS) ||
        (carrying & INVENTORY_TWIG) ||
        (carrying & INVENTORY_FISH) ||
        (carrying & INVENTORY_NUT_CRACKED))
    {
        if (hand == BODY_RIGHT_HAND)
        {
            carrying |= 1;
        }
        episodic_self(sim, local,EVENT_CHEW, EPISODIC_AFFECT_ZERO, carrying);
    }
    if (carrying & INVENTORY_GRASS)
    {
        CONSUME(local,ENERGY_GRASS,FOOD_VEGETABLE);
        OBJECTS_DROP(local,hand);
    }
    else
    {
        if (carrying & INVENTORY_FISH)
        {
            /* consume fish */
            CONSUME(local,ENERGY_FISH,FOOD_SHELLFISH);
            OBJECTS_DROP(local,hand);
        }
        else
        {
            if (carrying & INVENTORY_NUT_CRACKED)
            {
                /* consume nut */
                CONSUME(local,ENERGY_NUT,FOOD_VEGETABLE);
                OBJECTS_DROP(local,hand);
            }
        }
    }
}

static void body_action_swap_hands(noble_simulation * sim, noble_being * local, n_byte2 carrying, n_byte hand)
{
    if ((carrying != 0) && (OBJECTS_CARRIED(local,BODY_LEFT_HAND)==0))
    {
        OBJECTS_DROP(local,hand);
        OBJECT_TAKE(local,BODY_LEFT_HAND,carrying);
    }
    else
    {
        if ((carrying == 0) && (OBJECTS_CARRIED(local,BODY_LEFT_HAND)!=0))
        {
            carrying = OBJECTS_CARRIED(local,BODY_LEFT_HAND);
            OBJECTS_DROP(local,BODY_LEFT_HAND);
            OBJECT_TAKE(local,hand,carrying);
        }
    }
}

static void body_action_drop(noble_simulation * sim, noble_being * local, n_byte2 carrying, n_byte hand)
{
    if (carrying == 0)
    {
        hand = BODY_LEFT_HAND;
        carrying = OBJECTS_CARRIED(local,hand);
    }
    if (carrying != 0)
    {
        OBJECTS_DROP(local,hand);
        episodic_self(sim, local, EVENT_DROP, EPISODIC_AFFECT_ZERO, carrying);
    }
}

static void body_action_pickup(noble_simulation * sim, noble_being * local, n_byte2 carrying, n_byte hand)
{
    if ((carrying != 0) && (GET_PS(local)>=POSTURE_CROUCHING))
    {
        hand = BODY_LEFT_HAND;
        carrying = OBJECTS_CARRIED(local,hand);
    }
    if (carrying == 0)
    {
        n_int loc_f = GET_F(local);
        n_int az;
        n_vect2 location_vector,facing_vector,slope_vector;
        vect2_byte2(&location_vector, (n_byte2 *)&GET_X(local));
        vect2_direction(&facing_vector,(n_byte)loc_f,4);
        land_vect2(&slope_vector,&az,sim->land,&location_vector);
        
        if (az > WATER_MAP)
        {
            if (az > TIDE_MAX)
            {
                n_int grass, trees, bush;
                food_values(sim->land, sim->weather, local->x, local->y, &grass, &trees, &bush);
                
                if ((grass>bush) && (grass>trees))
                {
                    OBJECT_TAKE(local,hand, INVENTORY_GRASS);
                    episodic_self(sim, local, EVENT_PICKUP,EPISODIC_AFFECT_ZERO, INVENTORY_GRASS);
                }
                if ((trees>grass) && (trees>bush))
                {
                    if (GET_PS(local) < POSTURE_UPRIGHT)
                    {
                        OBJECT_TAKE(local,hand, INVENTORY_BRANCH);
                        episodic_self(sim, local, EVENT_PICKUP,EPISODIC_AFFECT_ZERO, INVENTORY_BRANCH);
                    }
                    else
                    {
                        OBJECT_TAKE(local,hand, INVENTORY_NUT);
                        episodic_self(sim, local, EVENT_PICKUP,EPISODIC_AFFECT_ZERO, INVENTORY_NUT);
                    }
                }
                if ((bush>grass) && (bush>trees))
                {
                    OBJECT_TAKE(local,hand, INVENTORY_TWIG);
                    episodic_self(sim, local, EVENT_PICKUP,EPISODIC_AFFECT_ZERO, INVENTORY_TWIG);
                }
            }
            else
            {
                OBJECT_TAKE(local,hand, INVENTORY_ROCK);
                episodic_self(sim, local, EVENT_PICKUP,EPISODIC_AFFECT_ZERO, INVENTORY_ROCK);
            }
        }
    }
}

void social_action(
                   noble_simulation * sim,
                   noble_being * local,
                   noble_being * other,
                   n_byte action)
{
    n_byte2 carrying;
    n_byte hand = BODY_RIGHT_HAND;
    
    if (being_awake_local(sim, local) == FULLY_ASLEEP)
    {
        return;
    }
    
    carrying = OBJECTS_CARRIED(local,hand);
    if (other == 0L)
    {
        /* individual action */
        switch(action%INDIVIDUAL_ACTIONS)
        {
            case ACTION_JAB:
                body_action_jab(sim, local, carrying, hand);
                break;
            case ACTION_BASH_OBJECTS:
                body_action_bash_objects(sim, local, carrying, hand);
                break;
            case ACTION_CHEW:
                body_action_chew(sim, local, carrying, hand);
                break;
            case ACTION_BRANDISH:
                body_action_hand_object(sim, local, carrying, hand, EVENT_BRANDISH);
                break;
            case ACTION_DRAG:
                body_action_hand_object(sim, local, carrying, hand, EVENT_DRAG);
                break;
            case ACTION_SWAP_HANDS:
                body_action_swap_hands(sim, local, carrying, hand);
                break;
            case ACTION_DROP:
                body_action_drop(sim, local, carrying, hand);
                break;
            case ACTION_PICKUP:
                body_action_pickup(sim, local, carrying, hand);
                break;
        }
    }
    else
    {
        /* social action */
        switch(action%SOCIAL_ACTIONS)
        {
            case ACTION_PROD:
                body_action_interactive_change(sim, local, other, BODY_RIGHT_HAND, BODY_FRONT,
                                               EVENT_PRODDED, 0, AFFECT_PRODDED);
                break;
            case ACTION_HUG:
                body_action_interactive_change(sim, local, other, BODY_FRONT, BODY_FRONT,
                                               EVENT_HUGGED, 1, AFFECT_HUGGED);
                break;
            case ACTION_SMILE:
                body_action_interactive_change(sim, local, other, BODY_TEETH, BODY_TEETH,
                                               EVENT_SMILED, 1, AFFECT_SMILED);
                break;
            case ACTION_GLOWER:
                body_action_interactive_change(sim, local, other, BODY_HEAD, BODY_HEAD,
                                               EVENT_GLOWERED, 0, AFFECT_GLOWER);
                break;
            case ACTION_TICKLE:
                body_action_interactive(sim, local, other, BODY_RIGHT_HAND, BODY_FRONT, EVENT_TICKLED);
                break;
            case ACTION_POINT:
                body_action_interactive(sim, local, other, BODY_RIGHT_HAND, BODY_RIGHT_HAND, EVENT_POINT);
                break;
            case ACTION_PAT:
                body_action_interactive(sim, local, other, BODY_RIGHT_HAND, BODY_BACK, EVENT_PATTED);
                break;
            case ACTION_BASH:
                body_action_bash(sim, local, other, carrying);
                break;
            case ACTION_GIVE:
                body_action_give(sim, local, other, carrying);
                break;
        }
    }
}


n_int genetics_compare(n_genetics * genetics_a, n_genetics * genetics_b)
{
    n_int loop = 0;
    while (loop < CHROMOSOMES)
    {
        if (genetics_b == 0L)
        {
            if (genetics_a[loop] != 0)
            {
                return 0;
            }
        }
        else if (genetics_a[loop] != genetics_b[loop])
        {
            return 0;
        }
        loop++;
    }
    return 1;
}

void genetics_set(n_genetics * genetics_a, n_genetics * genetics_b)
{
    n_int loop = 0;
    while (loop < CHROMOSOMES)
    {
        genetics_a[loop] = genetics_b[loop];
        loop++;
    }
}

void genetics_zero(n_genetics * genetics_a)
{
    n_int loop = 0;
    while (loop < CHROMOSOMES)
    {
        genetics_a[loop] = 0;
        loop++;
    }
}

static n_int genetics_unique(noble_simulation * local, n_genetics * genetics)
{
    n_uint         loop = 0;
    noble_being	* local_being = local->beings;
    
    if (local->num == 0)
    {
        return 1;
    }
    
    while (loop < local->num)
    {
        if (genetics_compare(local_being[loop].new_genetics, genetics))
        {
            return 0;
        }
        loop++;
    }
    return 1;
}

/* Returns the 2 bit gene value from the given point within a diploid chromosome.
 This includes mutations */
static n_int genetics_child_gene(n_int chromosome, n_int point, n_byte2 mutation_prob, n_byte2 * random)
{
    n_byte2 mutation_type;
    n_int child_gene = 0;
    
    math_random3(random);
    if (random[0] < mutation_prob)
    {
        mutation_type = (random[1] & 7);
        switch(mutation_type)
        {
            case MUTATION_MATERNAL:
                child_gene = DIPLOID(
                                     (math_random(random) & 3),
                                     ((CHROMOSOME_FROM_FATHER(chromosome) >> point ) & 3));
                break;
            case MUTATION_PATERNAL:
                child_gene = DIPLOID(
                                     ((CHROMOSOME_FROM_MOTHER(chromosome) >> point ) & 3),
                                     (math_random(random) & 3));
                break;
            case MUTATION_MATERNAL_DUPLICATE:
                child_gene = DIPLOID(
                                     ((CHROMOSOME_FROM_MOTHER(chromosome) >> point ) & 3),
                                     ((CHROMOSOME_FROM_MOTHER(chromosome) >> point ) & 3));
                break;
            case MUTATION_PATERNAL_DUPLICATE:
                child_gene = DIPLOID(
                                     ((CHROMOSOME_FROM_FATHER(chromosome) >> point ) & 3),
                                     ((CHROMOSOME_FROM_FATHER(chromosome) >> point ) & 3));
                break;
            default:
                math_random3(random);
                child_gene = DIPLOID(
                                     (random[0] & 3), (random[1] & 3));
        }
    }
    else
    {
        /* normal gene */
        if ((math_random(random) & 1)!=0)
        {
            child_gene = DIPLOID(
                                 ((CHROMOSOME_FROM_MOTHER(chromosome) >> point ) & 3),
                                 ((CHROMOSOME_FROM_FATHER(chromosome) >> point ) & 3));
        }
        else
        {
            child_gene = DIPLOID(
                                 ((CHROMOSOME_FROM_FATHER(chromosome) >> point ) & 3),
                                 ((CHROMOSOME_FROM_MOTHER(chromosome) >> point ) & 3));
        }
    }
    return child_gene;
}

/* Performs crossover and mutation */
static n_genetics	genetics_crossover(n_genetics mother, n_genetics father, n_byte2 * random)
{
    n_int loop = 0;
    n_genetics result = 0;
    n_int point, point2, parent;
    n_int deletion_point = 16;
    n_byte2 prob;
    
    /* randomly select a crossover point */
    n_int crossover_point = (math_random(random) >> 13) << 1;
    
    /* gene insertion/deletion */
    if (math_random(random) < MUTATION_DELETION_PROB)
    {
        deletion_point = (math_random(random) >> 13) << 1;
    }
    
    point = point2 = crossover_point - 8;
    while(loop< 16)
    {
        if (loop == deletion_point) point2 -= 2;
        
        /* equal genetic contribution from mother and father */
        if (point2 < 0)
        {
            point2 += 16;
        }
        else
        {
            if (point2 > 15) point2 -= 16;
        }
        
        if (loop < 8)
        {
            parent=father;
            /* higher mutation probability in males */
            prob = MUTATION_CROSSOVER_PROB*50;
        }
        else
        {
            parent=mother;
            prob = MUTATION_CROSSOVER_PROB;
        }
        
        result |= ( genetics_child_gene(parent, point2, prob, random) << point );
        loop += 2;
        point += 2;
        point2 += 2;
    }
    return result;
}

/* Mutates a single chromosome, without crossover */
static n_genetics	genetics_mutate(n_genetics chromosome, n_byte2 * random)
{
    n_genetics result = 0;
    n_int point = 0;
    n_int loop = 0;
    n_int deletion_point = 16;
    
    /* gene insertion/deletion */
    if (math_random(random) < MUTATION_DELETION_PROB)
    {
        deletion_point = (math_random(random) >> 13) << 1;
    }
    
    point = 0;
    while(loop< 16)
    {
        if (loop == deletion_point)
        {
            point -= 2;
            if (point < 0)
            {
                point += 16;
            }
        }
        if (point > 15) point -= 16;
        
        result |= ( genetics_child_gene(chromosome, point, MUTATION_CROSSOVER_PROB, random) << point );
        loop += 2;
        point += 2;
    }
    return result;
}

/* Transposes segments of the genome between chromosomes or within the same chromosome.
 This is the main cause of variation between siblings. */
static void genetics_transpose(noble_being * local, n_byte2 * local_random)
{
    math_random3(local_random);
    
    if (local_random[0] < MUTATION_TRANSPOSE_PROB)
    {
        /* number of bits to transpose */
        /* chromosome numbers */
        /* locations within the chromosomes */
        n_byte source_offset = (local_random[0]>>8)&31;
        n_byte dest_offset   = local_random[1]&31;
        /* whether to invert the sequence */
        n_byte inversion     = (local_random[0]>>13) & 1;
        n_byte source_ch     = (local_random[1]>>5) % CHROMOSOMES;
        n_byte dest_ch       = (local_random[1]>>7) % CHROMOSOMES;
        n_int  ctr1          = source_offset;
        n_byte p             = 0;
        math_random3(local_random);
        while (p < (local_random[1]&15))
        {
            n_int ctr2;
            ctr1 = (ctr1 & 31);
            
            if (inversion==0)
            {
                ctr2=(n_int)(dest_offset+p);
            }
            else
            {
                /* inverted sequence */
                ctr2=(n_int)dest_offset-p+32;
            }
            ctr2 = (ctr2 & 31);
            /* clear destination bit */
            if ((GET_G(local)[dest_ch] & (1<<ctr2)) != 0)
            {
                GET_G(local)[dest_ch] ^= (1 << ctr2);
            }
            /* set destination bit */
            if ((GET_G(local)[source_ch] & (1<<ctr1)) != 0)
            {
                GET_G(local)[dest_ch] |= (1 << ctr2);
            }
            p++;
            ctr1++;
        }
    }
}

void body_genetics(noble_simulation * sim, noble_being * local, noble_being * mother, n_byte2 * local_random)
{
    n_c_uint ch;
    n_byte sex = 2;
    
    /* determine the sex */
    math_random3(local_random);
    sex |= (local_random[0]&1);
    
    do
    {
        /* crossover and mutation */
        for (ch = 0; ch < CHROMOSOMES; ch++)
        {
            if (ch != CHROMOSOME_Y)
            {
                GET_G(local)[ch] = genetics_crossover(GET_G(mother)[ch], mother->father_new_genetics[ch], local_random);
            }
        }
        /* Y chromosome does not undergo crossover and passes from father to son */
        if (sex != SEX_FEMALE)
        {
            GET_I(local) = genetics_mutate(mother->father_new_genetics[CHROMOSOME_Y], local_random);
        }
        else
        {
            GET_I(local) = genetics_mutate(mother->mother_new_genetics[CHROMOSOME_Y], local_random);
        }
        /* transpose genes between chromosomes */
        genetics_transpose(local, local_random);
        /* align the sex genetics */
        GET_I(local) &= ~1;
        GET_I(local) |= sex;
    }
    while (genetics_unique(sim, GET_G(local)) == 0);
}

void body_genome_random(noble_simulation * sim, noble_being * local, n_byte2 * local_random)
{
    do
    {
        n_int ch,loop,gene;
        
        for (ch = 0; ch < CHROMOSOMES; ch++)
        {
            GET_G(local)[ch] = 0;
            for (loop=0; loop<16; loop+=2)
            {
                math_random3(local_random);
                gene = DIPLOID((local_random[0] & 3), (local_random[1] & 3));
                GET_G(local)[ch] |= ( gene << loop );
            }
        }
        /* align the sex genetics */
        GET_G(local)[CHROMOSOME_Y] |= 2;
    }
    while (genetics_unique(sim, GET_G(local)) == 0);
}

void body_genome(n_byte maternal, n_genetics * genome, n_byte * genome_str)
{
    n_byte string_point = 0;
    n_int ch, value;
    
    n_byte nucleotide[] = { 'A', 'T', 'C', 'G' };
    for (ch = 0; ch < CHROMOSOMES; ch++)
    {
        n_byte gene_point = 0;
        while (gene_point < 16)
        {
            if (maternal!=0)
            {
                value = ( CHROMOSOME_FROM_MOTHER(genome[ch]) >> gene_point ) & 3;
            }
            else
            {
                value = ( CHROMOSOME_FROM_FATHER(genome[ch]) >> gene_point ) & 3;
            }
            genome_str[string_point++] = nucleotide[value];
            gene_point += 2;
        }
    }
    genome_str[string_point] = 0;
}

