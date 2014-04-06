/****************************************************************
 
 topdown.c
 
 =============================================================
 
 Copyright 1996-2014 Tom Barbalet. All rights reserved.
 
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
    noble_image * canvas = image_init(1024, 1024);
    
    noble_image * grass1 = image_from_file("art/grass1.png");
    noble_image * grass2 = image_from_file("art/grass2.png");
    noble_image * grass3 = image_from_file("art/grass3.png");
    
    noble_image * grass_rotate;
    noble_image * grass_rotate2;
    
    noble_image * branch1 = image_from_file("art/branches1.png");
    noble_image * branch2 = image_from_file("art/branches2.png");
    noble_image * branch3 = image_from_file("art/branches3.png");
    
    noble_image * retain;
    
    retain = image_half(grass1);
    image_free(&grass1);
    grass1 = retain;

    retain = image_half(grass2);
    image_free(&grass2);
    grass2 = retain;

    retain = image_half(grass3);
    image_free(&grass3);
    grass3 = retain;
    
    grass_rotate = image_rotate(grass3);
    grass_rotate2 = image_rotate(grass_rotate);

    unsigned loopx = 0;
    while (loopx < 8)
    {
        unsigned loopy = 0;
        while (loopy < 8)
        {
            unsigned random = ((loopx)+(loopy*2))%5;
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
                image_add(canvas, image_here, loopx*128, loopy*128);
            }
            
            loopy++;
        }
        loopx++;
    }

    image_add_alpha(canvas, branch3, 10, 123, 130);
    image_add_alpha(canvas, branch2, 210, 450, 130);
    image_add_alpha(canvas, branch1, 570, 210, 130);

    image_add_alpha(canvas, branch1, 930, 403, 130);
    image_add_alpha(canvas, branch3, 450, 570, 130);
    image_add_alpha(canvas, branch2, 323, 720, 130);
    
    image_add_alpha(canvas, branch3, 830, 723, 130);
    image_add_alpha(canvas, branch2, 123, 623, 130);
    image_add_alpha(canvas, branch1, 920, 293, 130);

    image_create(canvas, "bushtrunk.png");
    
    image_free(&canvas);
    
    image_free(&grass1);
    image_free(&grass2);
    image_free(&grass3);
    
    image_free(&grass_rotate);
    image_free(&grass_rotate2);

    image_free(&branch1);
    image_free(&branch2);
    image_free(&branch3);
    
    return 0;
}

