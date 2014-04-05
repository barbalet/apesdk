/****************************************************************
 
 imagemath.c
 
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

#include <string.h>
#include <stdlib.h>

#include "lodepng.h"
#include "imagemath.h"

noble_image * image_init(unsigned width, unsigned height)
{
    noble_image   * image = 0L;
    unsigned char * alloc = 0L;
    unsigned        size = width * height * 3;
    alloc = malloc(size);
    
    if (alloc == 0L)
    {
        return 0L;
    }
    
    image = (noble_image *)malloc(sizeof(noble_image));
    
    if (image == 0L)
    {
        free(alloc);
        return 0L;
    }
    
    memset(alloc, 255, size);
    
    image->height = height;
    image->width = width;
    image->image = alloc;
    return image;
}

noble_image * image_from_file(char * file_name)
{
    noble_image   * return_image = 0L;
    unsigned char* image;
    unsigned width, height;

    unsigned error;
    unsigned char* png;
    size_t pngsize;
    
    lodepng_load_file(&png, &pngsize, file_name);
    error = lodepng_decode24(&image, &width, &height, png, pngsize);
    
    free(png);

    if (error != 0)
    {
        free(image);
        return 0L;
    }

    return_image = (noble_image *)malloc(sizeof(noble_image));
    
    if (return_image == 0L)
    {
        free(image);
        return 0L;
    }
    
    return_image->width  = width;
    return_image->height = height;
    return_image->image  = image;

    return return_image;
}

void image_free(noble_image ** image_free)
{
    if (*image_free == 0L) return;
    
    free((*image_free)->image);
    free(*image_free);
    *image_free = 0L;
}

void image_add(noble_image * canvas, noble_image * image, unsigned top, unsigned left)
{
    unsigned loop_height = 0;
    while (loop_height < image->height)
    {
        unsigned loop_width = 0;
        unsigned loop = 0;
        unsigned comb_height = top + loop_height;
        unsigned comb_width = (left * 3);

        unsigned image_width3 = (image->width * 3);
        unsigned canvas_width3 = (canvas->width * 3);
        unsigned char * band_canvas = &canvas->image[comb_height * canvas->width * 3];
        unsigned char * band_image = &image->image[loop_height * image->width * 3];
        
        if (comb_height >= canvas->height)
        {
            return;
        }
        
        while (loop_width < image_width3)
        {
            if (comb_width >= canvas_width3)
            {
                break;
            }
            band_canvas[comb_width ++] = band_image[loop_width++];
        }
        
        loop_height++;
    }
}

void image_add_alpha(noble_image * canvas, noble_image * image, unsigned top, unsigned left, unsigned alpha)
{
    unsigned loop_height = 0;
    while (loop_height < image->height)
    {
        unsigned loop_width = 0;
        unsigned loop = 0;
        unsigned comb_height = top + loop_height;
        unsigned comb_width = (left * 3);
        
        unsigned image_width3 = (image->width * 3);
        unsigned canvas_width3 = (canvas->width * 3);
        unsigned char * band_canvas = &canvas->image[comb_height * canvas->width * 3];
        unsigned char * band_image = &image->image[loop_height * image->width * 3];
        
        if (comb_height >= canvas->height)
        {
            return;
        }
        
        while (loop_width < image_width3)
        {
            unsigned char canvas, image;
            if (comb_width >= canvas_width3)
            {
                break;
            }
            canvas = band_canvas[comb_width];
            
            image = band_image[loop_width++];
            
            band_canvas[comb_width ++] = ((canvas * (256 - alpha)) + (image * alpha)) >> 8;
        }
        
        loop_height++;
    }
}

void image_create(noble_image * image, char * filename)
{
    (void)lodepng_encode24_file(filename, image->image, image->width, image->height);
}
