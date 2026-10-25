/****************************************************************
 
 tree.c
 
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

void tree_create(simulated_tree * tree, n_byte2 * seed, n_vect2 * center)
{
    n_int loop = 0;
    n_int wandering = (math_random(seed) % 11) - 5;
    
    while (loop < POINTS_PER_TREE)
    {
        wandering += (math_random(seed) % 11) - 5;
        tree->points[loop] = 60-wandering;
        
        loop++;
    }
    
    loop = 0;
    while (loop < POINTS_PER_TREE)
    {
        tree->points[loop] = (tree->points[(loop + POINTS_PER_TREE - 1) & (POINTS_PER_TREE - 1)] +
                              tree->points[loop] + tree->points[(loop + 1) & (POINTS_PER_TREE - 1)])/3;
        loop++;
    }
    
    tree->center.x = center->x;
    tree->center.y = center->y;
    tree->radius = (math_random(seed) % 8) + 5;
}

static n_int tree_offset(n_byte2 * seed)
{
    return (math_random(seed) % 61) - 30;
}

static void tree_clear(simulated_tree * trees)
{
    n_int loop = 0;
    while (loop < 4)
    {
        simulated_tree * tree = &trees[loop];
        tree->center.x = 0;
        tree->center.y = 0;
        tree->radius = 0;
        loop++;
    }
}

n_int tree_populated(simulated_tree * tree)
{
    if ((tree->center.x | tree->center.y) | tree->radius)
    {
        return 1;
    }
    return 0;
}

void tree_init(simulated_tree * trees, n_byte2 * seed, n_vect2 * edge)
{
    n_byte mod = math_random(seed) & 15;
    n_int count = 0;
    n_int px = -1;
        
    tree_clear(trees);
    
    while (px < 1)
    {
        n_int py = -1;
        while (py < 1)
        {
            if ((mod >> count) & 1)
            {
                n_vect2 local_center;
                simulated_tree * tree = &trees[count++];
                vect2_populate(&local_center,
                               (px * TREE_SPACE) + (TREE_SPACE / 2) + tree_offset(seed),
                               (py * TREE_SPACE) + (TREE_SPACE / 2) + tree_offset(seed));
                vect2_delta(&local_center, edge);
                tree_create(tree, seed, &local_center);
            }
            py++;
        }
        px++;
    }
}
