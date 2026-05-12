/****************************************************************

 tutorial.c

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

#ifndef    _WIN32

#include "../shared.h"

#else

#include "..\shared.h"

#endif

typedef struct
{
    n_int window;
    n_int anchor_x;
    n_int anchor_y;
    n_int anchor_width;
    n_int anchor_height;
    n_int edge;
    n_constant_string title;
    n_constant_string text;
} shared_initial_tutorial_step;

static const shared_initial_tutorial_step initial_tutorial_steps[] =
{
    {
        NUM_VIEW,
        500,
        500,
        180,
        180,
        SHARED_TUTORIAL_EDGE_MAX_Y,
        "View window",
        "This map follows the selected ape. Click near an ape to select it. Option-click or Control-click a place to move the selected ape there."
    },
    {
        NUM_VIEW,
        500,
        120,
        220,
        110,
        SHARED_TUTORIAL_EDGE_MIN_Y,
        "Map movement",
        "The arrow keys move the selected ape from this map. Left and right steer, while up and down push movement forward or back."
    },
    {
        NUM_TERRAIN,
        500,
        500,
        260,
        190,
        SHARED_TUTORIAL_EDGE_MAX_X,
        "Terrain window",
        "This is the selected ape's ground-level terrain view. Click the left or right side to turn the ape; rotate gestures also change direction."
    },
    {
        NUM_TERRAIN,
        870,
        920,
        220,
        110,
        SHARED_TUTORIAL_EDGE_MAX_Y,
        "Meters and overlays",
        "The small meters show simulation time, facing, drives, and genetics. The Control menu can show weather, territory, brain, and braincode overlays."
    },
    {
        NUM_CONTROL,
        500,
        800,
        320,
        140,
        SHARED_TUTORIAL_EDGE_MIN_X,
        "Control window",
        "This window is the live text readout for the selected ape. Blue highlighted names are selectable; click one to make that ape the focus."
    },
    {
        NUM_CONTROL,
        500,
        140,
        320,
        120,
        SHARED_TUTORIAL_EDGE_MIN_Y,
        "Working with the run",
        "Use the Control menu to pause, change apes, clear errors, or open the command line. New Simulation starts a fresh landscape."
    }
};

static const shared_initial_tutorial_step *shared_initial_tutorial_get_step( n_int step )
{
    if ( ( step < 0 ) || ( step >= shared_initial_tutorial_count() ) )
    {
        return 0L;
    }

    return &initial_tutorial_steps[step];
}

n_int shared_initial_tutorial_enabled( void )
{
#ifdef INITIAL_TUTORIAL_ON
    return 1;
#else
    return 0;
#endif
}

n_int shared_initial_tutorial_count( void )
{
    return ( n_int )( sizeof( initial_tutorial_steps ) / sizeof( initial_tutorial_steps[0] ) );
}

n_int shared_initial_tutorial_window( n_int step )
{
    const shared_initial_tutorial_step *tutorial_step = shared_initial_tutorial_get_step( step );
    return tutorial_step ? tutorial_step->window : NUM_NIL;
}

n_int shared_initial_tutorial_anchor_x( n_int step )
{
    const shared_initial_tutorial_step *tutorial_step = shared_initial_tutorial_get_step( step );
    return tutorial_step ? tutorial_step->anchor_x : 500;
}

n_int shared_initial_tutorial_anchor_y( n_int step )
{
    const shared_initial_tutorial_step *tutorial_step = shared_initial_tutorial_get_step( step );
    return tutorial_step ? tutorial_step->anchor_y : 500;
}

n_int shared_initial_tutorial_anchor_width( n_int step )
{
    const shared_initial_tutorial_step *tutorial_step = shared_initial_tutorial_get_step( step );
    return tutorial_step ? tutorial_step->anchor_width : 120;
}

n_int shared_initial_tutorial_anchor_height( n_int step )
{
    const shared_initial_tutorial_step *tutorial_step = shared_initial_tutorial_get_step( step );
    return tutorial_step ? tutorial_step->anchor_height : 80;
}

n_int shared_initial_tutorial_edge( n_int step )
{
    const shared_initial_tutorial_step *tutorial_step = shared_initial_tutorial_get_step( step );
    return tutorial_step ? tutorial_step->edge : SHARED_TUTORIAL_EDGE_MAX_Y;
}

n_constant_string shared_initial_tutorial_title( n_int step )
{
    const shared_initial_tutorial_step *tutorial_step = shared_initial_tutorial_get_step( step );
    return tutorial_step ? tutorial_step->title : "";
}

n_constant_string shared_initial_tutorial_text( n_int step )
{
    const shared_initial_tutorial_step *tutorial_step = shared_initial_tutorial_get_step( step );
    return tutorial_step ? tutorial_step->text : "";
}
