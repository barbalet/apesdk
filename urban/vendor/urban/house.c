/****************************************************************
 
 house.c
 
 =============================================================
 
 Copyright 1996-2023 Tom Barbalet. All rights reserved.
 
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

#include "mushroom.h"

/* 1=north, 2=south, 4=east, 8=west */


static void house_rotate_logic(n_vect2 * point, n_vect2 * points_center, n_vect2 * direction_vector, n_vect2 * center)
{
    vect2_subtract(point, point, points_center);
    vect2_rotation(point, direction_vector);
    vect2_add(point, point, center);
}

static void house_transform(simulated_building * building, n_vect2 * center, n_int direction)
{
    n_int   loop_room = 0;
    n_vect2 * min_max = vect2_min_max_init();
    n_vect2 direction_vector;
    n_vect2 points_center;

    if (min_max == 0L)
    {
        return;
    }
    while (loop_room < building->roomcount)
    {
        simulated_room * room = &building->room[loop_room++];
        vect2_min_max(room->points, POINTS_PER_ROOM_STRUCTURE, min_max);
    }
    
    vect2_center(&points_center, &min_max[1], &min_max[0]);
    vect2_direction(&direction_vector, direction, 1);
    
    memory_free((void **)&min_max);
    
    loop_room = 0;
    while (loop_room < building->roomcount)
    {
        n_int   loop = 0;
        simulated_room * room = &building->room[loop_room++];
        while (loop < 8)
        {
            house_rotate_logic(&room->points[loop], &points_center, &direction_vector, center);
            loop++;
        }
        while (loop < 16)
        {
            if (house_window_present(&room->points[loop]))
            {
                house_rotate_logic(&room->points[loop], &points_center, &direction_vector, center);
                house_rotate_logic(&room->points[loop + 1], &points_center, &direction_vector, center);
            }
            loop += 2;
        }
        while (loop < 32)
        {
            if (house_door_present(&room->points[loop]))
            {
                house_rotate_logic(&room->points[loop], &points_center, &direction_vector, center);
                house_rotate_logic(&room->points[loop + 1], &points_center, &direction_vector, center);
                house_rotate_logic(&room->points[loop + 2], &points_center, &direction_vector, center);
                house_rotate_logic(&room->points[loop + 3], &points_center, &direction_vector, center);
            }
            loop += 4;
        }
    }
}

n_int house_window_present(n_vect2 * window)
{
    if (vect2_nonzero(&window[0]) || vect2_nonzero(&window[1]))
    {
        return 1;
    }
    return 0;
}

n_int house_door_present(n_vect2 * door)
{
    if (vect2_nonzero(&door[0]) || vect2_nonzero(&door[1]) || vect2_nonzero(&door[2]) || vect2_nonzero(&door[3]))
    {
        return 1;
    }
    return 0;
}

/* 1=north, 2=south, 4=east, 8=west */
static void house_construct(simulated_room * room, n_int topx, n_int topy, n_int botx, n_int boty, n_byte window, n_byte door)
{
    n_int midx, midy;
    n_int loop = 0;

    while (loop < POINTS_PER_ROOM)
    {
        room->points[loop].data[0] = 0;
        room->points[loop].data[1] = 0;
        loop++;
    }
    
    if (topx > botx)
    {
		n_int temp = botx;
		botx = topx;
		topx = temp;
	}
	if (topy > boty)
    {
		n_int temp = boty;
		boty = topy;
		topy = temp;
	}
    
    vect2_populate(&room->points[0], topx, topy);
    vect2_populate(&room->points[1], topx, boty);
    vect2_populate(&room->points[2], botx, boty);
    vect2_populate(&room->points[3], botx, topy);
    vect2_populate(&room->points[4], topx+6, topy+6);
    vect2_populate(&room->points[5], topx+6, boty-6);
    vect2_populate(&room->points[6], botx-6, boty-6);
    vect2_populate(&room->points[7], botx-6, topy+6);
    
    if (window & DC_NORTH)
    {
        vect2_populate(&room->points[8], topx+3, topy+10);
        vect2_populate(&room->points[9], topx+3, boty-10);
    }
    if (window & DC_SOUTH)
    {
        vect2_populate(&room->points[10], botx-3, topy+10);
        vect2_populate(&room->points[11], botx-3, boty-10);
    }
    if (window & DC_EAST)
    {
        vect2_populate(&room->points[12], topx+10, boty-3);
        vect2_populate(&room->points[13], botx-10, boty-3);
    }
    if (window & DC_WEST)
    {
        vect2_populate(&room->points[14], topx+10, topy+3);
        vect2_populate(&room->points[15], botx-10, topy+3);
    }
    
    midx = (topx + botx) / 2;
    midy = (topy + boty) / 2;
    
    if (door & DC_NORTH)
    {
        vect2_populate(&room->points[16], topx + 2, midy + 5);
        vect2_populate(&room->points[17], topx + 2, midy - 5);
        vect2_populate(&room->points[18], topx - 2, midy - 5);
        vect2_populate(&room->points[19], topx - 2, midy + 5);

    }
    if (door & DC_SOUTH)
    {
        vect2_populate(&room->points[20], botx - 2, midy + 5);
        vect2_populate(&room->points[21], botx - 2, midy - 5);
        vect2_populate(&room->points[22], botx + 2, midy - 5);
        vect2_populate(&room->points[23], botx + 2, midy + 5);
    }
    if (door & DC_EAST)
    {
        vect2_populate(&room->points[24], midx + 5, boty - 2);
        vect2_populate(&room->points[25], midx - 5, boty - 2);
        vect2_populate(&room->points[26], midx - 5, boty + 2);
        vect2_populate(&room->points[27], midx + 5, boty + 2);
    }
    if (door & DC_WEST)
    {
        vect2_populate(&room->points[28], midx + 5, topy + 2);
        vect2_populate(&room->points[29], midx - 5, topy + 2);
        vect2_populate(&room->points[30], midx - 5, topy - 2);
        vect2_populate(&room->points[31], midx + 5, topy - 2);
    }
}

static n_int house_plusminus(n_byte2 * seed)
{
	n_int	tmp=0;
	while ((tmp > -3) && (tmp < 3))
    {
		tmp = (math_random(seed) % 11) - 5;
	}
	return (tmp);
}

static n_int house_genetics(n_int * house, n_byte2 * seed)
{
	n_int	count = 1;
	n_int   lpend = house[0] = 5 + (math_random(seed) % 11); /* 5 to 15 */
	n_int   lp = (math_random(seed) % 3) + 2;
	while (lp < lpend)
    {
		house[(count * 3) - 1] = 0;
		house[count * 3] = lp;
		house[(count * 3) + 1] = house_plusminus(seed);
		count++;
        lp += (math_random(seed) % 3) + 2;
	}
    house[0] ++;
	return count;
}

#define HOUSE_SIZE (30)

void house_create(simulated_building * building, n_byte2 * seed, n_vect2 * center)
{
    n_int            rooms = house_genetics(building->house, seed);
    n_int            count = 0;
    n_int            pointp = -4;
    n_int            pointm = -4;
    n_int            loop = 2;
    simulated_room * room = building->room;
    n_int          * house = building->house;
    
	if (house[4] > 1)
    {
        house_construct(&room[count++], -8, -4, 8+(house[4]*HOUSE_SIZE), -4+(house[3]*HOUSE_SIZE), DC_SOUTH/*10*/,/*DC_NONE*/ DC_WEST);//north + east
        pointp = (house[3] * HOUSE_SIZE) - 4;
    }
    else
    {
        house_construct(&room[count++], -8 + (house[4]*HOUSE_SIZE), -4, 8, -4+(house[3]*HOUSE_SIZE), DC_NORTH /*| DC_WEST*/, DC_WEST);//north + west
        pointm = (house[3] * HOUSE_SIZE) - 4;
    }
    
    while(loop < (rooms - 1))
    {
        if (house[(loop * 3) + 1] > 1)
        {
            house_construct(&room[count++], 8, -4+(house[loop*3]*HOUSE_SIZE), 8+(house[(loop*3)+1]*HOUSE_SIZE), pointp, DC_SOUTH, /*DC_NORTH*/ DC_EAST);//east
            pointp = (house[loop * 3] * HOUSE_SIZE) - 4;
        }
        else
        {
            house_construct(&room[count++], -8 + (house[(loop*3)+1]*HOUSE_SIZE), -4+(house[loop*3]*HOUSE_SIZE), -8, pointm, DC_NORTH, /*DC_SOUTH*/ DC_EAST);//west
            pointm = (house[loop * 3] * HOUSE_SIZE) - 4;
        }
        loop++;
    }

	if (house[((rooms - 1) * 3) + 1] > 1)
    {
        house_construct(&room[count++], -8, (house[0]*HOUSE_SIZE)+4, 8+(house[((rooms-1)*3)+1]*HOUSE_SIZE), -4+(house[(rooms-1)*3]*HOUSE_SIZE), DC_EAST, DC_WEST);
        house_construct(&room[count++], -8, pointm, -8-(house[((rooms-1)*3)+1]*HOUSE_SIZE), (house[0]*HOUSE_SIZE)+4, DC_NORTH, DC_SOUTH);
        if (rooms != loop)
        {
            house_construct(&room[count++], 8, -4+(house[(rooms-1)*3]*HOUSE_SIZE),8+(house[((rooms-1)*3)+1]*HOUSE_SIZE), pointp, DC_SOUTH, DC_NORTH);
        }
    }
    else
    {
        house_construct(&room[count++], 8, (house[0]*HOUSE_SIZE)+4, 8+(house[((rooms-1)*3)+1]*HOUSE_SIZE), -4+(house[(rooms-1)*3]*HOUSE_SIZE), DC_EAST, DC_WEST);
        house_construct(&room[count++], 8, pointp, 8-(house[((rooms-1)*3)+1]*HOUSE_SIZE), (house[0]*HOUSE_SIZE)+4, DC_SOUTH, DC_NORTH);
        if (rooms != loop)
        {
            house_construct(&room[count++], -8, -4+(house[(rooms-1)*3]*HOUSE_SIZE), -8+(house[((rooms-1)*3)+1]*HOUSE_SIZE),pointm, DC_NORTH, DC_SOUTH);
        }
    }
    
    if (rooms > 2)
    {
        house_construct(&room[count++], 8, -4+(house[3]*HOUSE_SIZE), -8, -4+(house[(rooms-1)*3]*HOUSE_SIZE), DC_NONE, DC_EAST | DC_WEST);
    }
    building->roomcount = count;
    
    house_transform(building, center, math_random(seed) & 255);
}

void house_init(n_byte2 * seed, n_vect2 * location, simulated_building * buildings)
{
    n_int count = 0;
    n_int px = 0;
    
    while (px < 4)
    {
        n_int py = 0;
        while (py < 4)
        {
            n_vect2 local_center;
            simulated_building * building = &buildings[count++];
            vect2_populate(&local_center, (px * RESIDENCE_SPACE) + location->x, (py * RESIDENCE_SPACE) + location->y);
            house_create(building, seed, &local_center);
            tree_init(building->trees, seed, &local_center);
            py++;
        }
        px++;
    }
}

