/****************************************************************
 
 imagemath.h
 
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

#ifndef NOBLEAPE_IMAGEMATH_H
#define NOBLEAPE_IMAGEMATH_H

typedef struct
{
    unsigned width;
    unsigned height;
    unsigned char * image;
}noble_image;

noble_image * image_init_clear(unsigned width, unsigned height, unsigned clear);

noble_image * image_from_file(char * file_name);

noble_image * image_half(noble_image * full);
noble_image * image_third(noble_image * full);

noble_image * image_rotate_90(noble_image * full);
noble_image * image_rotate_45(noble_image * full);

void image_free(noble_image ** image_free);

void image_add(noble_image * canvas, noble_image * image, unsigned top, unsigned left);

void image_create(noble_image * image, char * filename);

void image_add_alpha(noble_image * canvas, noble_image * image, unsigned top, unsigned left, unsigned alpha);

#endif
