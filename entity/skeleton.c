/****************************************************************

 skeleton.c

 =============================================================

 Copyright 1996-2025 Tom Barbalet. All rights reserved.

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

#include <math.h>

#include "../toolkit/toolkit.h"
#include "../render/graph.h"
#include "entity.h"

#define MAX_SKELETON_DIAGRAM_POINTS 200000

#undef VASCULAR_DRAWING_READY

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
    {42, 0},    {53, 475}, /* axis */
    {49, 0},    {16, 8},    {9, 21},    {2, 37},    {5, 50},    {10, 64},  {14, 77},  {14, 95},  {12, 143},  {12, 193},
    {10, 241},  {12, 259},  {18, 286},  {21, 314},  {21, 343},  {20, 372}, {17, 408}, {13, 424}, {5, 438},   {1, 444},
    {3, 458},   {14, 470},  {18, 472},  {28, 473},  {42, 477},  {52, 476}, {63, 469}, {75, 477}, {92, 477},  {90, 458},
    {99, 455},  {107, 450}, {111, 440}, {107, 429}, {100, 425}, {88, 418}, {77, 407}, {69, 393}, {65, 382},  {62, 347},
    {60, 313},  {56, 270},  {51, 218},  {52, 174},  {52, 127},  {54, 88},  {57, 71},  {64, 68},  {69, 54},   {75, 47},
    {81, 36},   {84, 27},   {77, 10},   {67, 1}
};

const n_vect2 bone_arm_lower1[] =
{
    {34, 4},   {59, 346}, /* axis */
    {48, 0},   {35, 0},   {21, 1},   {14, 9},   {15, 15},  {22, 23},  {24, 31},  {24, 42},  {20, 67},  {15, 102},
    {9, 132},  {4, 157},  {2, 177},  {3, 197},  {7, 225},  {11, 258}, {16, 290}, {18, 308}, {19, 327}, {20, 337},
    {19, 347}, {23, 351}, {36, 349}, {50, 347}, {60, 342}, {70, 340}, {72, 336}, {72, 331}, {67, 327}, {51, 314},
    {46, 305}, {40, 285}, {33, 259}, {31, 233}, {28, 208}, {28, 183}, {30, 166}, {37, 131}, {48, 77},  {52, 60},
    {45, 45},  {43, 31},  {45, 13},  {44, 4}
};

const n_vect2 bone_arm_lower2[] =
{
    {34, 4},   {59, 346}, /* axis */
    {65, 0},   {53, 0},   {48, 11},  {45, 25},   {40, 40},   {35, 102},  {35, 127},  {35, 151},  {39, 178},  {43, 209},
    {48, 237}, {54, 266}, {58, 299}, {60, 317},  {55, 329},  {53, 339},  {54, 343},  {69, 344},  {80, 346},  {88, 349},
    {91, 343}, {89, 335}, {84, 324}, {78, 314},  {75, 293},  {74, 271},  {70, 239},  {64, 207},  {60, 174},  {57, 132},
    {58, 112}, {62, 77},  {65, 58},  {72, 48},   {74, 35},   {85, 27},   {95, 23},   {98, 16},   {96, 8},    {86, 5},
    {77, 2}
};

const n_vect2 bone_clavical[] =
{
    {23, 201}, {31, 13}, /* axis */
    {26, 4},   {20, 12},   {19, 22},   {24, 30},   {30, 39},   {29, 53},   {24, 77},   {15, 100},   {10, 114},   {6, 130},
    {4, 146},  {5, 159},   {9, 176},   {14, 189},  {15, 200},  {18, 206},  {23, 207},  {28, 206},   {31, 199},   {31, 192},
    {27, 182}, {25, 173},  {22, 156},  {22, 143},  {24, 126},  {28, 110},  {39, 88},   {45, 68},    {49, 52},    {50, 40},
    {48, 27},  {43, 12},   {38, 7}
};

const n_vect2 bone_leg_upper[] =
{
    {119, 33},  {80, 416}, /* axis */
    {47, 3},    {36, 7},   {30, 15},   {27, 22},   {23, 43},   {19, 73},   {18, 90},   {20, 112},   {22, 140},   {23, 178},
    {24, 223},  {29, 286}, {32, 330},  {30, 360},  {26, 378},  {24, 386},  {28, 398},  {36, 410},   {49, 419},   {65, 425},
    {78, 422},  {84, 417}, {91, 410},  {102, 405}, {114, 408}, {126, 408}, {133, 401}, {134, 389},  {135, 373},  {125, 362},
    {114, 354}, {99, 347}, {89, 330},  {78, 305},  {71, 274},  {64, 221},  {64, 174},  {67, 126},   {73, 93},    {76, 77},
    {87, 67},   {99, 63},  {111, 59},  {116, 61},  {121, 63},  {126, 58},  {127, 51},  {122, 38},   {117, 22},   {109, 14},
    {99, 13},   {91, 19},  {86, 24},   {78, 26},   {70, 25},   {64, 17},   {66, 7},    {60, 3}
};

const n_vect2 bone_leg_lower1[] =
{
    {94, 5},   {46, 287}, /* axis */
    {23, 26},  {14, 33},  {12, 45},  {14, 55},  {16, 68},  {17, 101},  {18, 132},  {18, 166},  {17, 195},  {18, 223},
    {20, 241}, {17, 251}, {11, 260}, {5, 269},  {5, 275},  {14, 280},  {25, 284},  {33, 283},  {37, 275},  {36, 261},
    {32, 240}, {28, 210}, {27, 171}, {30, 154}, {31, 134}, {30, 94},   {31, 69},   {36, 48},   {36, 36},   {32, 27}
};

const n_vect2 bone_leg_lower2[] =
{
    {94, 5},   {46, 287}, /* axis */
    {66, 1},   {44, 1},    {28, 1},    {24, 8},   {25, 24},  {34, 37},   {41, 49},   {49, 66},   {54, 85},   {58, 106},
    {59, 130}, {59, 165},  {57, 196},  {51, 227}, {45, 245}, {40, 254},  {36, 266},  {39, 280},  {47, 287},  {59, 287},
    {77, 281}, {87, 277},  {90, 272},  {90, 265}, {84, 250}, {83, 230},  {83, 188},  {83, 149},  {85, 109},  {88, 74},
    {92, 47},  {100, 38}, {110, 29},  {116, 18},  {114, 6},  {98, 5},    {80, 1}
};

const n_vect2 bone_pelvis[] =
{
    {214, 27},  {203, 382}, /* axis */
    {213, 24},  {183, 21},    {166, 22},    {156, 18},    {139, 12},    {118, 6},    {100, 6},   {79, 8},    {68, 11},    {60, 19},
    {46, 34},   {35, 49},     {25, 65},     {17, 81},     {9, 96},      {6, 108},    {6, 122},   {15, 136},  {27, 150},   {44, 165},
    {59, 181},  {71, 197},    {75, 221},    {78, 240},    {77, 255},    {84, 254},   {97, 259},  {104, 271}, {107, 284},  {106, 295},
    {105, 303}, {112, 315},   {116, 330},   {118, 347},   {112, 358},   {105, 365},  {101, 367}, {102, 376}, {111, 384},  {127, 393},
    {141, 393}, {158, 390},   {177, 387},   {189, 381},   {201, 369},   {213, 359},  {215, 336}, {216, 349}, {218, 358},  {225, 370},
    {240, 380}, {254, 385},   {272, 387},   {294, 387},   {307, 387},   {316, 375},  {317, 367}, {310, 357}, {300, 337},  {300, 320},
    {308, 308}, {316, 304},   {321, 294},   {320, 283},   {322, 275},   {321, 266},  {330, 259}, {342, 254}, {349, 245},  {348, 217},
    {354, 192}, {364, 176},   {381, 162},   {402, 144},   {423, 131},   {432, 121},  {433, 104}, {427, 84},  {411, 60},   {392, 37},
    {377, 22},  {360, 11},    {336, 7},     {311, 11},    {277, 18},    {258, 25},   {244, 26},  {222, 24},
    {101, 272}, /* left leg */
    {321, 265} /* right leg */
};

const n_vect2 bone_hand[] =
{
    {20, 6},   {14, 100}, /* axis */
    {19, 4},   {12, 6},   {9, 12},   {9, 23},    {10, 40},   {9, 59},    {8, 75},    {6, 86},    {3, 91},    {8, 98},
    {9, 102},  {15, 102}, {20, 101}, {24, 96},   {27, 90},   {27, 84},   {24, 76},   {23, 64},   {24, 40},   {24, 24},
    {26, 18},  {29, 10},  {28, 4},   {23, 3}
};

const n_vect2 bone_finger[] =
{
    {14, 1},   {15, 114}, /* axis */
    {13, 1},   {6, 3},   {4, 6},   {3, 10},    {6, 14},    {6, 26},    {6, 40},    {8, 52},    {9, 56},    {8, 60},
    {6, 66},   {6, 72},  {8, 82},  {7, 92},    {9, 98},    {10, 104},  {11, 112},  {14, 114},  {16, 113},  {18, 107},
    {19, 100}, {19, 92}, {20, 86}, {20, 78},   {20, 68},   {23, 60},   {25, 52},   {26, 44},   {24, 34},   {23, 18},
    {23, 7},   {21, 3},  {16, 2}
};

const n_vect2 bone_vertibra[] =
{
    {53, 1},  {53, 47}, /* axis */
    {53, 2},  {41, 2},   {30, 4},   {22, 7},   {22, 16},   {22, 24},   {20, 32},   {14, 32},   {4, 34},   {1, 36},
    {1, 42},  {5, 46},   {14, 48},  {18, 48},  {18, 54},   {24, 53},   {33, 50},   {43, 48},   {53, 49},  {62, 50},
    {72, 53}, {81, 56},  {87, 56},  {88, 50},  {95, 50},   {98, 48},   {100, 42},  {98, 38},   {91, 37},  {84, 35},
    {84, 33}, {84, 24},  {86, 14},  {84, 10},  {74, 6},    {62, 0}
};

const n_vect2 bone_scapula[] =
{
    {35, 0},   {134, 214}, /* axis */
    {37, 3},    {22, 7},    {11, 15},    {6, 21},    {4, 28},    {4, 40},    {9, 58},    {19, 80},    {38, 113},    {60, 142},
    {81, 178},  {101, 204}, {114, 219},  {128, 226}, {138, 230}, {153, 221}, {165, 211}, {171, 194},  {173, 170},   {175, 143},
    {177, 116}, {176, 97},  {168, 81},   {177, 69},  {187, 60},  {188, 41},  {184, 23},  {175, 13},   {162, 11},    {151, 12},
    {146, 22},  {136, 34},  {128, 46},   {118, 59},  {113, 56},  {109, 43},  {100, 34},  {93, 34},    {84, 36},     {83, 46},
    {75, 51},   {67, 52},   {58, 42},    {58, 34},   {48, 27},   {51, 19},   {52, 12},   {45, 6}
};

const n_vect2 bone_ribs[] =
{
    {227, 11},  {212, 350}, /* axis */
    {226, 10},  {199, 7},   {177, 15},   {152, 25},   {139, 37},  {122, 52},   {112, 62},   {104, 75},   {91, 93},   {85, 108},
    {76, 127},  {69, 149},  {62, 169},   {61, 183},   {53, 208},  {53, 218},   {44, 230},   {39, 244},   {40, 255},  {36, 262},
    {30, 278},  {28, 295},  {28, 299},   {22, 317},   {22, 336},  {23, 348},   {16, 365},   {15, 389},   {14, 411},  {10, 437},
    {13, 449},  {18, 446},  {18, 434},   {33, 425},   {56, 407},  {83, 392},   {116, 382},  {141, 373},  {159, 362}, {173, 351},
    {184, 351}, {213, 348}, {235, 355},  {242, 356},  {244, 352}, {264, 363},  {283, 377},  {304, 386},  {333, 396}, {355, 409},
    {375, 423}, {395, 443}, {409, 458},  {416, 450},  {419, 440}, {415, 429},  {417, 404},  {411, 383},  {408, 366}, {411, 348},
    {411, 327}, {409, 315}, {407, 296},  {402, 278},  {395, 267}, {393, 243},  {385, 232},  {384, 213},  {379, 200}, {371, 194},
    {367, 172}, {359, 155}, {357, 134},  {344, 114},  {334, 90},  {322, 80},   {318, 67},   {301, 47},   {289, 30},  {275, 22},
    {247, 15},  {227, 12},
    {35, 3},    {416, 3}, /* shoulder sockets */
    {35, -20},  {416, -20}, /* shoulders */
};


/**
 * @brief Returns an array of 2D points used for drawing diagrams
 * @param source_points Array of 2D points which is the template
 * @param no_of_source_points Number of 2D points in the template
 * @param extra_points The number of points to be returned via the extra parameters
 * @param x The starting x coordinate
 * @param y The starting y coordinate
 * @param mirror Flip in the vertical axis
 * @param scale Horizontal scaling factor x1000
 * @param angle Rotation angle of the result
 */

static void outline_points( const n_vect2 *source_points,
                            n_int no_of_source_points, n_int extra_points,
                            n_int x, n_int y,
                            n_byte mirror,
                            n_vect2 *scale,
                            n_int angle,
                            n_vect2 *axis,
                            n_vect2 *extra_1,
                            n_vect2 *extra_2,
                            n_vect2 *extra_3,
                            n_vect2 *extra_4,
                            n_points *collection )
{
    n_vect2  ds, location, vector;
    n_int    i, axis_length, point_length;
    n_double axis_angle, point_angle;
    n_double ang = angle * TWO_PI / 7200;

    vect2_populate( &location, x, y );
    vect2_subtract( &ds, ( n_vect2 * )&source_points[1], ( n_vect2 * )source_points );
    vect2_multiplier( &ds, &ds, scale, 1, 1000 );

    /** length of the object */
    axis_length = ( n_int )math_root( vect2_dot( &ds, &ds, 1, 1 ) );
    if ( axis_length < 1 )
    {
        axis_length = 1;
    }

    /** invert around the vertical axis if needed */
    if ( mirror != 0 )
    {
        ds.x = -ds.x;
    }

    /** find the orientation angle of the axis */
    axis_angle = ( float )acos( ds.x / ( float )axis_length );

    if ( ds.y < 0 )
    {
        axis_angle = TWO_PI - axis_angle;
    }

    vect2_populate( &vector, ( n_int )( axis_length * sin( ang + ( TWO_PI / 4 ) - axis_angle ) ),
                    ( n_int )( axis_length * cos( ang + ( TWO_PI / 4 ) - axis_angle ) ) );

    /** calculate the position of the end point of the axis */

    vect2_add( axis, &location, &vector );

    /** draw lines between each point */
    for ( i = 2; i < no_of_source_points + 2 + extra_points; i++ )
    {
        n_vect2 point;
        vect2_subtract( &ds, ( n_vect2 * )&source_points[i], ( n_vect2 * )source_points );
        vect2_multiplier( &ds, &ds, scale, 1, 1000 );
        point_length = ( n_int )math_root( vect2_dot( &ds, &ds, 1, 1 ) );
        if ( point_length < 1 )
        {
            point_length = 1;
        }

        /** invert the line around the vertical axis if necessary */
        if ( mirror != 0 )
        {
            ds.x = -ds.x;
        }

        /** angle of the line */
        point_angle = ( float )acos( ds.x / ( float )point_length );
        if ( ds.y < 0 )
        {
            point_angle = ( TWO_PI ) - point_angle;
        }

        /** position of the end of the line */
        vect2_populate( &vector, ( n_int )( point_length * sin( ang + point_angle - axis_angle ) ),
                        ( n_int )( point_length * cos( ang + point_angle - axis_angle ) ) );

        vect2_add( &point, &location, &vector );

        /** store the calculated point positions in an array */
        if ( collection->no_of_points < collection->max_points )
        {
            if ( i < no_of_source_points + 2 )
            {
                vect2_copy( &collection->points[collection->no_of_points], &point );
                collection->no_of_points++;
            }
        }
        /* TODO
        else
        {
            (void)SHOW_ERROR("Maximum number of skeleton points reached");
        }
        */
        /** This is a crude way of keeping track of the last few points
         so that they can be returned by the function */
        vect2_copy( extra_1, extra_2 );
        vect2_copy( extra_2, extra_3 );
        vect2_copy( extra_3, extra_4 );
        vect2_copy( extra_4, &point );
    }

    if ( collection->no_of_points > -1 && collection->no_of_points < collection->max_points )
    {
        collection->points[collection->no_of_points].x = 9999;
        collection->points[collection->no_of_points].y = 9999;
        collection->no_of_points++;
    }
    else
    {
        SHOW_ERROR( "Outside point range for drawing" );
    }
}


/**
 * @brief Returns vertical and horizontal scaling factors based upon the
 * genetics for a particular body segment
 * @param keypoint The index within the BONES enumeration
 * @param scale The returned scale for the bone x1000
 */
static void body_skeleton_gene( n_genetics *genetics, n_byte keypoint, n_vect2 *scale )
{
    /** the maximum variation in body segment proportions,
     typically in the range 0-500 */
    const n_int MAX_SEGMENT_VARIANCE = 200;

    /** these are 4 bit gene values in the range 0-15 */
    n_byte gene_val1 = ( n_byte )GENE_HOX( genetics, keypoint );
    n_byte gene_val2 = ( n_byte )GENE_HOX( genetics, 20 - keypoint );

    /** convert the gene values into scaling values */
    scale->x = 1000 - MAX_SEGMENT_VARIANCE + ( ( ( n_int )gene_val1 ) * MAX_SEGMENT_VARIANCE * 2 / 15 );
    scale->y = 1000 - MAX_SEGMENT_VARIANCE + ( ( ( n_int )gene_val2 ) * MAX_SEGMENT_VARIANCE * 2 / 15 );
}

/**
 * @brief returns a set of points corresponding to key locations on the skeleton
 * @param keypoints Array which returns the x,y coordinates of each key point on the skeleton
 * @param collection The returned 2D points of the skeleton diagram
 * @param shoulder_angle Angle of the shoulders in degrees
 * @param elbow_angle Angle of the elbows in degrees
 * @param wrist_angle Angle of the wrists in degrees
 * @param hip_angle Angle of the hips in degrees
 * @param knee_angle Angle of the knees in degrees
 * @return Number of 2D points within the skeleton diagram
 */
static n_int graph_skeleton_points( n_genetics *genetics, n_vect2 *keypoints, n_points *collection,
                                    n_int shoulder_angle, n_int elbow_angle, n_int wrist_angle,
                                    n_int hip_angle, n_int knee_angle )
{
    n_int angle = 0;
    n_vect2 scale = {1000, 1000};
    n_vect2 extra[4];
    n_vect2 vertibra = {0, 0};
    n_vect2 knuckle = {0, 0};
    n_int i, vertical;

    collection->no_of_points = 0;

    NA_ASSERT( keypoints, "keypoints NULL" );

    /** position of the bottom of the neck */
    keypoints[SKELETON_NECK].x = 0;
    keypoints[SKELETON_NECK].y = 0;

    /** position of the bottom of the ribs */
    body_skeleton_gene( genetics, BONE_RIBS, &scale );
    outline_points( bone_ribs, bone_points[BONE_RIBS], 4,
                    keypoints[SKELETON_NECK].x, keypoints[SKELETON_NECK].y, 0,
                    &scale,
                    angle,
                    &keypoints[SKELETON_LUMBAR],
                    &keypoints[SKELETON_LEFT_SHOULDER_SOCKET],
                    &keypoints[SKELETON_RIGHT_SHOULDER_SOCKET],
                    &keypoints[SKELETON_LEFT_SHOULDER],
                    &keypoints[SKELETON_RIGHT_SHOULDER],
                    collection );

    /** left scapula */
    body_skeleton_gene( genetics, BONE_SCAPULA, &scale );
    outline_points( bone_scapula, bone_points[BONE_SCAPULA], 0,
                    keypoints[SKELETON_LEFT_SHOULDER].x,
                    keypoints[SKELETON_LEFT_SHOULDER].y,
                    0, &scale,
                    angle - 600,
                    &extra[0],
                    &extra[0],
                    &extra[1],
                    &extra[2],
                    &extra[3],
                    collection );

    /** right scapula */
    outline_points( bone_scapula, bone_points[BONE_SCAPULA], 0,
                    keypoints[SKELETON_RIGHT_SHOULDER].x,
                    keypoints[SKELETON_RIGHT_SHOULDER].y,
                    1, &scale,
                    angle + 500,
                    &extra[0],
                    &extra[0],
                    &extra[1],
                    &extra[2],
                    &extra[3],
                    collection );

    /** ribs */
    body_skeleton_gene( genetics, BONE_RIBS, &scale );
    outline_points( bone_ribs, bone_points[BONE_RIBS], 4,
                    keypoints[SKELETON_NECK].x, keypoints[SKELETON_NECK].y, 0,
                    &scale,
                    angle,
                    &keypoints[SKELETON_LUMBAR],
                    &keypoints[SKELETON_LEFT_SHOULDER_SOCKET],
                    &keypoints[SKELETON_RIGHT_SHOULDER_SOCKET],
                    &keypoints[SKELETON_LEFT_SHOULDER],
                    &keypoints[SKELETON_RIGHT_SHOULDER],
                    collection );

    /** position of the top of the pelvis */
    keypoints[SKELETON_PELVIS].x = keypoints[SKELETON_LUMBAR].x;
    keypoints[SKELETON_PELVIS].y =
        keypoints[SKELETON_LUMBAR].y +
        ( ( keypoints[SKELETON_LUMBAR].y - keypoints[SKELETON_NECK].y ) * 40 / 100 );

    /** position of hips */
    body_skeleton_gene( genetics, BONE_PELVIS, &scale );
    outline_points( bone_pelvis, bone_points[BONE_PELVIS], 2,
                    keypoints[SKELETON_PELVIS].x, keypoints[SKELETON_PELVIS].y,
                    0, &scale,
                    angle,
                    &extra[0],
                    &extra[0],
                    &extra[1],
                    &keypoints[SKELETON_LEFT_HIP],
                    &keypoints[SKELETON_RIGHT_HIP],
                    collection );

    /** left upper leg */
    body_skeleton_gene( genetics, BONE_LEG_UPPER, &scale );
    outline_points( bone_leg_upper, bone_points[BONE_LEG_UPPER], 0,
                    keypoints[SKELETON_LEFT_HIP].x, keypoints[SKELETON_LEFT_HIP].y,
                    0, &scale,
                    angle + ( hip_angle * 10 ),
                    &keypoints[SKELETON_LEFT_KNEE],
                    &extra[0],
                    &extra[1],
                    &extra[2],
                    &extra[3],
                    collection );

    /** left lower leg 1 */
    body_skeleton_gene( genetics, BONE_LEG_LOWER1, &scale );
    outline_points( bone_leg_lower1, bone_points[BONE_LEG_LOWER1], 0,
                    keypoints[SKELETON_LEFT_KNEE].x, keypoints[SKELETON_LEFT_KNEE].y,
                    0, &scale,
                    angle + ( knee_angle * 10 ),
                    &keypoints[SKELETON_LEFT_ANKLE],
                    &extra[0],
                    &extra[1],
                    &extra[2],
                    &extra[3],
                    collection );

    /** left lower leg 2 */
    body_skeleton_gene( genetics, BONE_LEG_LOWER1, &scale );
    outline_points( bone_leg_lower2, bone_points[BONE_LEG_LOWER2], 0,
                    keypoints[SKELETON_LEFT_KNEE].x, keypoints[SKELETON_LEFT_KNEE].y,
                    0, &scale,
                    angle + ( knee_angle * 10 ),
                    &keypoints[SKELETON_LEFT_ANKLE],
                    &extra[0],
                    &extra[1],
                    &extra[2],
                    &extra[3],
                    collection );

    /** right upper leg */
    body_skeleton_gene( genetics, BONE_LEG_UPPER, &scale );
    outline_points( bone_leg_upper, bone_points[BONE_LEG_UPPER], 0,
                    keypoints[SKELETON_RIGHT_HIP].x, keypoints[SKELETON_RIGHT_HIP].y,
                    1, &scale,
                    angle - ( hip_angle * 10 ),
                    &keypoints[SKELETON_RIGHT_KNEE],
                    &extra[0],
                    &extra[1],
                    &extra[2],
                    &extra[3],
                    collection );

    /** right lower leg 1 */
    body_skeleton_gene( genetics, BONE_LEG_LOWER1, &scale );
    outline_points( bone_leg_lower1, bone_points[BONE_LEG_LOWER1], 0,
                    keypoints[SKELETON_RIGHT_KNEE].x, keypoints[SKELETON_RIGHT_KNEE].y,
                    1, &scale,
                    angle - ( knee_angle * 10 ),
                    &keypoints[SKELETON_RIGHT_ANKLE],
                    &extra[0],
                    &extra[1],
                    &extra[2],
                    &extra[3],
                    collection );

    /** right lower leg 2 */
    body_skeleton_gene( genetics, BONE_LEG_LOWER1, &scale );
    outline_points( bone_leg_lower2, bone_points[BONE_LEG_LOWER2], 0,
                    keypoints[SKELETON_RIGHT_KNEE].x, keypoints[SKELETON_RIGHT_KNEE].y,
                    1, &scale,
                    angle - ( knee_angle * 10 ),
                    &keypoints[SKELETON_RIGHT_ANKLE],
                    &extra[0],
                    &extra[1],
                    &extra[2],
                    &extra[3],
                    collection );

    /** left upper arm */
    body_skeleton_gene( genetics, BONE_ARM_UPPER, &scale );
    outline_points( bone_arm_upper, bone_points[BONE_ARM_UPPER], 0,
                    keypoints[SKELETON_LEFT_SHOULDER_SOCKET].x,
                    keypoints[SKELETON_LEFT_SHOULDER_SOCKET].y,
                    0, &scale,
                    angle + ( shoulder_angle * 10 ),
                    &keypoints[SKELETON_LEFT_ELBOW],
                    &extra[0],
                    &extra[1],
                    &extra[2],
                    &extra[3],
                    collection );

    /** left lower arm 1 */
    body_skeleton_gene( genetics, BONE_ARM_LOWER1, &scale );
    outline_points( bone_arm_lower1, bone_points[BONE_ARM_LOWER1], 0,
                    keypoints[SKELETON_LEFT_ELBOW].x,
                    keypoints[SKELETON_LEFT_ELBOW].y,
                    0, &scale,
                    angle + ( elbow_angle * 10 ),
                    &keypoints[SKELETON_LEFT_WRIST],
                    &extra[0],
                    &extra[1],
                    &extra[2],
                    &extra[3],
                    collection );

    /** left lower arm 2 */
    body_skeleton_gene( genetics, BONE_ARM_LOWER1, &scale );
    outline_points( bone_arm_lower2, bone_points[BONE_ARM_LOWER2], 0,
                    keypoints[SKELETON_LEFT_ELBOW].x,
                    keypoints[SKELETON_LEFT_ELBOW].y,
                    0, &scale,
                    angle + ( elbow_angle * 10 ),
                    &keypoints[SKELETON_LEFT_WRIST],
                    &extra[0],
                    &extra[1],
                    &extra[2],
                    &extra[3],
                    collection );

    /** right upper arm */
    body_skeleton_gene( genetics, BONE_ARM_UPPER, &scale );
    outline_points( bone_arm_upper, bone_points[BONE_ARM_UPPER], 0,
                    keypoints[SKELETON_RIGHT_SHOULDER_SOCKET].x,
                    keypoints[SKELETON_RIGHT_SHOULDER_SOCKET].y,
                    1, &scale,
                    angle - ( shoulder_angle * 10 ),
                    &keypoints[SKELETON_RIGHT_ELBOW],
                    &extra[0],
                    &extra[1],
                    &extra[2],
                    &extra[3],
                    collection );

    /** right lower arm 1 */
    body_skeleton_gene( genetics, BONE_ARM_LOWER1, &scale );
    outline_points( bone_arm_lower1, bone_points[BONE_ARM_LOWER1], 0,
                    keypoints[SKELETON_RIGHT_ELBOW].x,
                    keypoints[SKELETON_RIGHT_ELBOW].y,
                    1, &scale,
                    angle - ( elbow_angle * 10 ),
                    &keypoints[SKELETON_RIGHT_WRIST],
                    &extra[0],
                    &extra[1],
                    &extra[2],
                    &extra[3],
                    collection );

    /** left lower arm 2 */
    body_skeleton_gene( genetics, BONE_ARM_LOWER1, &scale );
    outline_points( bone_arm_lower2, bone_points[BONE_ARM_LOWER2], 0,
                    keypoints[SKELETON_RIGHT_ELBOW].x,
                    keypoints[SKELETON_RIGHT_ELBOW].y,
                    1, &scale,
                    angle - ( elbow_angle * 10 ),
                    &keypoints[SKELETON_RIGHT_WRIST],
                    &extra[0],
                    &extra[1],
                    &extra[2],
                    &extra[3],
                    collection );

    /** left clavical */
    body_skeleton_gene( genetics, BONE_CLAVICAL, &scale );
    outline_points( bone_clavical, bone_points[BONE_CLAVICAL], 0,
                    keypoints[SKELETON_LEFT_SHOULDER].x,
                    keypoints[SKELETON_LEFT_SHOULDER].y,
                    0, &scale,
                    angle - 1800,
                    &keypoints[SKELETON_LEFT_COLLAR],
                    &extra[0],
                    &extra[1],
                    &extra[2],
                    &extra[3],
                    collection );

    /** right clavical */
    outline_points( bone_clavical, bone_points[BONE_CLAVICAL], 0,
                    keypoints[SKELETON_RIGHT_SHOULDER].x,
                    keypoints[SKELETON_RIGHT_SHOULDER].y,
                    1, &scale,
                    angle + 1700,
                    &keypoints[SKELETON_RIGHT_COLLAR],
                    &extra[0],
                    &extra[1],
                    &extra[2],
                    &extra[3],
                    collection );

    vertical = keypoints[SKELETON_NECK].y;
    for ( i = 0; i < SKELETON_VERTIBRA_RIBS; i++ )
    {
        n_vect2 rib_scale;

        body_skeleton_gene( genetics, BONE_RIBS, &scale );

        rib_scale.x = ( scale.x * 5 / 10 ) + ( ( scale.x * 5 / 10 ) * i / SKELETON_VERTIBRA_RIBS );
        rib_scale.y = ( scale.y * 5 / 10 ) + ( ( scale.y * 5 / 10 ) * i / SKELETON_VERTIBRA_RIBS );

        outline_points( bone_vertibra, bone_points[BONE_VERTIBRA], 0,
                        keypoints[SKELETON_NECK].x + ( ( keypoints[SKELETON_LUMBAR].x - keypoints[SKELETON_NECK].x )*i / SKELETON_VERTIBRA_RIBS ),
                        vertical,
                        0,
                        &rib_scale,
                        angle,
                        &vertibra,
                        &extra[0],
                        &extra[1],
                        &extra[2],
                        &extra[3],
                        collection );
        vertical = vertibra.y;
    }

    vertical = keypoints[SKELETON_LUMBAR].y;
    for ( i = 0; i < SKELETON_LUMBAR_VERTIBRA; i++ )
    {
        outline_points( bone_vertibra, bone_points[BONE_VERTIBRA], 0,
                        keypoints[SKELETON_LUMBAR].x,
                        vertical,
                        0, &scale,
                        angle,
                        &vertibra,
                        &extra[0],
                        &extra[1],
                        &extra[2],
                        &extra[3],
                        collection );
        vertical = vertibra.y;
    }
    for ( i = 0; i < SKELETON_LUMBAR_VERTIBRA2; i++ )
    {
        n_vect2 vertibra_scale;

        vertibra_scale.x = scale.x * ( SKELETON_LUMBAR_VERTIBRA2 - i ) / SKELETON_LUMBAR_VERTIBRA2;
        vertibra_scale.y = ( ( scale.y * 2 / 3 ) * ( SKELETON_LUMBAR_VERTIBRA2 - i ) / SKELETON_LUMBAR_VERTIBRA2 );

        outline_points( bone_vertibra, bone_points[BONE_VERTIBRA], 0,
                        keypoints[SKELETON_LUMBAR].x,
                        vertical,
                        0, &vertibra_scale,
                        angle,
                        &vertibra,
                        &extra[0],
                        &extra[1],
                        &extra[2],
                        &extra[3],
                        collection );
        vertical = vertibra.y;
    }

    for ( i = 0; i < 4; i++ )
    {
        /** left hand */
        body_skeleton_gene( genetics, BONE_HAND, &scale );
        outline_points( bone_hand, bone_points[BONE_HAND], 0,
                        keypoints[SKELETON_LEFT_WRIST].x - ( i * 15 ),
                        keypoints[SKELETON_LEFT_WRIST].y,
                        0, &scale,
                        angle + 200 - ( i * 400 / 4 ) + ( wrist_angle * 10 ),
                        &knuckle,
                        &extra[0],
                        &extra[1],
                        &extra[2],
                        &extra[3],
                        collection );

        /** left finger */
        body_skeleton_gene( genetics, BONE_FINGER, &scale );
        outline_points( bone_finger, bone_points[BONE_FINGER], 0,
                        knuckle.x, knuckle.y,
                        0, &scale,
                        angle + 400 - ( i * 800 / 4 ) + ( wrist_angle * 10 ),
                        &extra[0],
                        &extra[0],
                        &extra[1],
                        &extra[2],
                        &extra[3],
                        collection );

        /** right hand */
        body_skeleton_gene( genetics, BONE_HAND, &scale );
        outline_points( bone_hand, bone_points[BONE_HAND], 0,
                        keypoints[SKELETON_RIGHT_WRIST].x + ( ( 3 - i ) * 15 ),
                        keypoints[SKELETON_RIGHT_WRIST].y,
                        1, &scale,
                        angle + 200 - ( i * 400 / 4 ) - ( wrist_angle * 10 ),
                        &knuckle,
                        &extra[0],
                        &extra[1],
                        &extra[2],
                        &extra[3],
                        collection );

        /** right finger */
        body_skeleton_gene( genetics, BONE_FINGER, &scale );
        outline_points( bone_finger, bone_points[BONE_FINGER], 0,
                        knuckle.x, knuckle.y,
                        1, &scale,
                        angle + 400 - ( i * 800 / 4 ) - ( wrist_angle * 10 ),
                        &extra[0],
                        &extra[0],
                        &extra[1],
                        &extra[2],
                        &extra[3],
                        collection );
    }

    /** left thumb */
    body_skeleton_gene( genetics, BONE_FINGER, &scale );
    outline_points( bone_finger, bone_points[BONE_FINGER], 0,
                    keypoints[SKELETON_LEFT_WRIST].x - 50,
                    keypoints[SKELETON_LEFT_WRIST].y,
                    0, &scale,
                    angle - 800 + ( wrist_angle * 10 ),
                    &extra[0],
                    &extra[0],
                    &extra[1],
                    &extra[2],
                    &extra[3],
                    collection );

    /** right thumb */
    outline_points( bone_finger, bone_points[BONE_FINGER], 0,
                    keypoints[SKELETON_RIGHT_WRIST].x + 50,
                    keypoints[SKELETON_RIGHT_WRIST].y,
                    1, &scale,
                    angle + 800 - ( wrist_angle * 10 ),
                    &extra[0],
                    &extra[0],
                    &extra[1],
                    &extra[2],
                    &extra[3],
                    collection );

    return collection->no_of_points;
}


/**
 * @brief Draws a the skeleton
 * @param buffer Image buffer (3 bytes per pixel)
 * @param img Image dimensions
 * @param tp Top coordinate of the bounding box
 * @param bp Bottom coordinate of the bounding box
 * @param thickness Thickness of the outline in pixels
 * @param shoulder_angle Angle of the shoulders in degrees
 * @param elbow_angle Angle of the elbows in degrees
 * @param wrist_angle Angle of the wrists in degrees
 * @param hip_angle Angle of the hips in degrees
 * @param knee_angle Angle of the knees in degrees
 * @param show_keypoints If non-zero show key points on the skeleton
 */
static void skeleton_draw( n_genetics *genetics,
                           n_byte *buffer,
                           n_vect2 *img,
                           n_vect2 *tp, n_vect2 *bp,
                           n_byte thickness,
                           n_vect2 *keypoints,
                           n_int shoulder_angle, n_int elbow_angle, n_int wrist_angle,
                           n_int hip_angle, n_int knee_angle,
                           n_byte show_keypoints )
{
    n_vect2 skeleton_points[MAX_SKELETON_DIAGRAM_POINTS];

    n_vect2 min = {99999, 99999};
    n_vect2 max = {-99999, -99999};
    n_vect2 current;
    n_vect2 previous = {0, 0};
    n_int i, no_of_points, first_point = 0, ctr = 0;
    n_rgba32 bone_shade = {220, 220, 220, 0};
    n_rgba32 color = {150, 150, 150, 0};
    n_points collection;

    collection.max_points = MAX_SKELETON_DIAGRAM_POINTS;
    collection.points = skeleton_points;
    collection.no_of_points = 0;

    /** get points on the skeleton */
    no_of_points = graph_skeleton_points( genetics, keypoints, &collection,
                                          shoulder_angle, elbow_angle, wrist_angle,
                                          hip_angle, knee_angle );

    /** get the bounding box for the points */
    for ( i = 0; i < no_of_points; i++ )
    {
        vect2_copy( &current, &skeleton_points[i] );

        if ( ( current.x == 9999 ) || ( current.y == 9999 ) )
        {
            continue;
        }
        if ( current.x < min.x )
        {
            min.x = current.x;
        }
        if ( current.y < min.y )
        {
            min.y = current.y;
        }
        if ( current.x > max.x )
        {
            max.x = current.x;
        }
        if ( current.y > max.y )
        {
            max.y = current.y;
        }
    }

    if ( ( max.x - min.x ) && ( max.y - min.y ) )
    {
        /** rescale the skeleton keypoints into the bounding box */
        for ( i = 0; i < SKELETON_POINTS; i++ )
        {
            if ( keypoints[i].x != 9999 )
            {
                keypoints[i].x = tp->x + ( ( keypoints[i].x - min.x ) * ( bp->x - tp->x ) / ( max.x - min.x ) );
                keypoints[i].y = tp->y + ( ( keypoints[i].y - min.y ) * ( bp->y - tp->y ) / ( max.y - min.y ) );
            }
        }
    }

    if ( ( max.x - min.x ) && ( max.y - min.y ) )
    {
        /** rescale the drawing points into the bounding box */
        for ( i = 0; i < no_of_points; i++ )
        {
            if ( skeleton_points[i].x != 9999 )
            {
                skeleton_points[i].x = tp->x + ( ( skeleton_points[i].x - min.x ) * ( bp->x - tp->x ) / ( max.x - min.x ) );
                skeleton_points[i].y = tp->y + ( ( skeleton_points[i].y - min.y ) * ( bp->y - tp->y ) / ( max.y - min.y ) );
            }
        }
    }
    /** do the drawing */
    for ( i = 0; i < no_of_points; i++ )
    {
        vect2_copy( &current, &skeleton_points[i] );
        if ( i > 0 )
        {
            if ( ( current.x != 9999 ) && ( previous.x != 9999 ) )
            {
                if ( first_point == -1 )
                {
                    first_point = i;
                }
                graph_line( buffer, img,
                            &previous, &current,
                            &color, thickness );
            }
            else
            {
                if ( ( first_point > -1 ) && ( i - first_point > 2 ) )
                {
                    graph_fill_polygon( &skeleton_points[first_point], i - first_point,
                                        &bone_shade, 127,
                                        buffer, img );
                    ctr++;
                }
                first_point = -1;
            }
        }
        vect2_copy( &previous, &current );
    }
    /** optionally show the keypoints on the skeleton */
    if ( show_keypoints != 0 )
    {
        for ( i = 0; i < SKELETON_POINTS; i++ )
        {
            if ( keypoints[i].x != 9999 )
            {
                n_vect2 negative_x, positive_x, negative_y, positive_y;
                n_rgba32   green = {0, 255, 0, 0};

                vect2_copy( &negative_x, &keypoints[i] );
                vect2_copy( &positive_x, &keypoints[i] );
                vect2_copy( &negative_y, &keypoints[i] );
                vect2_copy( &positive_y, &keypoints[i] );

                negative_x.x -= 10;
                negative_y.y -= 10;

                positive_x.x += 10;
                positive_y.y += 10;

                graph_line( buffer, img,
                            &negative_x, &positive_x,
                            &green, 1 );
                graph_line( buffer, img,
                            &negative_y, &positive_y,
                            &green, 1 );
            }
        }
    }
}

#ifdef VASCULAR_DRAWING_READY

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
void point_relative_to_skeleton( n_int *keypoints,
                                 n_int start_keypoint_index,
                                 n_int end_keypoint_index,
                                 n_int distance_along_axis_percent,
                                 n_int distance_from_axis_percent,
                                 n_vect2 *pt )
{
    /** length of the axis */
    n_int dx = keypoints[end_keypoint_index * 2] - keypoints[start_keypoint_index * 2];
    n_int dy = keypoints[end_keypoint_index * 2 + 1] - keypoints[start_keypoint_index * 2 + 1];

    /** point position on the axis */
    n_int axis_x = keypoints[start_keypoint_index * 2] + ( dx * distance_along_axis_percent / 100 );
    n_int axis_y = keypoints[start_keypoint_index * 2 + 1] + ( dy * distance_along_axis_percent / 100 );

    /** point position */
    pt->x = axis_x + ( dy * distance_from_axis_percent / 100 );
    pt->y = axis_y + ( dx * distance_from_axis_percent / 100 );
}
#endif

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
 * @param buffer Image buffer (3 bytes per pixel)
 * @param img Image dimesions
 * @param tp Top coordinate of the bounding box
 * @param bp Bottom coordinate of the bounding box
 * @param thickness Thickness of the outline in pixels
 * @param clear Non zero if the image should be cleared before drawing
 * @param shoulder_angle Angle of the shoulders in degrees
 * @param elbow_angle Angle of the elbows in degrees
 * @param wrist_angle Angle of the wrists in degrees
 * @param hip_angle Angle of the hips in degrees
 * @param knee_angle Angle of the knees in degrees
 * @param show_skeleton_keypoints If non-zero show key points on the skeleton
 */
void vascular_draw( n_genetics *genetics,
                    n_byte *buffer,
                    n_vect2 *img,
                    n_vect2 *tp, n_vect2 *bp,
                    n_byte thickness,
                    n_byte clear,
                    n_int shoulder_angle, n_int elbow_angle, n_int wrist_angle,
                    n_int hip_angle, n_int knee_angle,
                    n_byte show_skeleton_keypoints )
{
    n_vect2 keypoints[SKELETON_POINTS];
#ifdef VASCULAR_DRAWING_READY
    n_int i, vascular_model_index, previous_index;
    n_uint start_thickness = 4, end_thickness = 4;
    n_vect2 pt[3];
    n_rgba32 blood = {255, 0, 0, 0};
#endif
    /** clear the image if necessary */
    if ( clear != 0 )
    {
        n_rgba32 white = {255, 255, 255, 0};
        graph_erase( buffer, img, &white );
    }

    /** draw the skeleton */
    skeleton_draw( genetics, buffer, img,
                   tp, bp, thickness, keypoints,
                   shoulder_angle, elbow_angle, wrist_angle,
                   hip_angle, knee_angle, show_skeleton_keypoints );

#ifdef VASCULAR_DRAWING_READY
    for ( i = 0; i < no_of_vascular_diagram_points; i++ )
    {
        /** index of the previous segment */
        previous_index = vascular_diagram_points[i * vascular_diagram_fields];
        pt[0].x = -1;
        if ( previous_index > -1 )
        {
            /** end point of the vessel */
            point_relative_to_skeleton( ( n_int * )keypoints,
                                        vascular_diagram_points[previous_index * vascular_diagram_fields + 5],
                                        vascular_diagram_points[previous_index * vascular_diagram_fields + 6],
                                        vascular_diagram_points[previous_index * vascular_diagram_fields + 7],
                                        vascular_diagram_points[previous_index * vascular_diagram_fields + 8],
                                        &pt[0] );
        }

        /** beginning point of the vessel */
        point_relative_to_skeleton( ( n_int * )keypoints,
                                    vascular_diagram_points[i * vascular_diagram_fields + 1],
                                    vascular_diagram_points[i * vascular_diagram_fields + 2],
                                    vascular_diagram_points[i * vascular_diagram_fields + 3],
                                    vascular_diagram_points[i * vascular_diagram_fields + 4],
                                    &pt[1] );

        /** end point of the vessel */
        point_relative_to_skeleton( ( n_int * )keypoints,
                                    vascular_diagram_points[i * vascular_diagram_fields + 5],
                                    vascular_diagram_points[i * vascular_diagram_fields + 6],
                                    vascular_diagram_points[i * vascular_diagram_fields + 7],
                                    vascular_diagram_points[i * vascular_diagram_fields + 8],
                                    &pt[2] );

        /** compartment index within the vascular simulation */
        vascular_model_index = vascular_diagram_points[i * vascular_diagram_fields + 9];

        if ( pt[0].x != -1 )
        {
            /* draw a curve */
            graph_curve( buffer, img,
                         &pt[0], &pt[1], &pt[2],
                         &blood,
                         10, start_thickness, end_thickness );
        }
        else
        {
            /* draw a line */
            graph_line( buffer, img,
                        &pt[1], &pt[2],
                        &blood, end_thickness );
        }
    }
#endif
}

