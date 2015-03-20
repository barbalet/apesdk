/****************************************************************

 topdown.c

 =============================================================

 Copyright 1996-2015 Tom Barbalet. All rights reserved.

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

#include "imagemath.h"

#include <stdio.h>
#include <stdlib.h>


int main(int argc, char *argv[])
{
    noble_image * canvas = image_init_clear(1024, 1024, 1);

    noble_image * branch1_full = image_from_file("art/branches1.png");
    noble_image * branch1_half = image_half(branch1_full);
    noble_image * branch1_0  = image_third(branch1_full);
    noble_image * branch1_45_full = image_grayscale(image_rotate_45(branch1_half));
    noble_image * branch1_45 = image_cut(branch1_45_full, 55, 20, 260, 280);

    noble_image * branch1_90 = image_rotate_90(branch1_0);
    noble_image * branch1_135 = image_rotate_90(branch1_45);

    noble_image * grass1 = image_from_file("art/grass1.png");

    noble_image * grass2 = image_from_file("art/grass2.png");
    noble_image * grass3 = image_from_file("art/grass3.png");

    noble_image * grass_rotate;
    noble_image * grass_rotate2;

    noble_image * retain;

    unsigned loopx = 0;

    srand(12123123);

    retain = image_half(grass1);
    image_free(&grass1);
    grass1 = retain;

    retain = image_half(grass2);
    image_free(&grass2);
    grass2 = retain;

    retain = image_half(grass3);
    image_free(&grass3);
    grass3 = retain;

    grass_rotate = image_rotate_90(grass3);
    grass_rotate2 = image_rotate_90(grass_rotate);

    image_add(canvas, branch1_0, 50, 50);
    image_add(canvas, branch1_45, 220, 220);
    image_add(canvas, branch1_90, 440, 440);

    image_add(canvas, branch1_135, 700, 700);

    image_free(&branch1_45_full);

    while (loopx < 16)
    {
        unsigned loopy = 0;
        while (loopy < 16)
        {
            unsigned random = (rand())%5;
            noble_image *image_here = 0L;

            switch(random)
            {
            case 2:
                image_here = grass1;
                break;
            case 1:
                image_here = grass2;
                break;
            case 3:
                image_here = grass3;
                break;
            case 4:
                image_here = grass_rotate2;
                break;
            default:
                image_here = grass_rotate;
                break;
            }

            if (image_here)
            {
                image_combination(canvas, image_here, (loopx*64) + (rand()%50), (loopy*64) + (rand()%50));
            }

            loopy++;
        }
        loopx++;
    }

    image_free(&branch1_full);
    image_free(&branch1_half);

    image_free(&branch1_0);
    image_free(&branch1_45);
    image_free(&branch1_90);
    image_free(&branch1_135);

    image_free(&grass1);
    image_free(&grass2);
    image_free(&grass3);

    image_free(&grass_rotate);
    image_free(&grass_rotate2);

    image_create(canvas, "bushtrunk.png");

    image_free(&canvas);

    return 0;
}

