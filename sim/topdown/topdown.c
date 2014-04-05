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
    noble_image * image = image_from_file("art/bushTrunk3.png");
    
    image_add(canvas, image, 100, 100);
    image_add(canvas, image, 600, 600);
    
    image_add_alpha(canvas, image, 100, 600, 128);
    image_add_alpha(canvas, image, 600, 100, 64);
    
    image_add_alpha(canvas, image, 600, 900, 32);
    image_add_alpha(canvas, image, 900, 600, 200);

    image_add_alpha(canvas, image, 800, 500, 100);

    image_create(canvas, "bushtrunk.png");
    
    image_free(&canvas);
    image_free(&image);
    
    return 0;
}

