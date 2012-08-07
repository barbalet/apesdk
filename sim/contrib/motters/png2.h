/****************************************************************

 png2.h

 =============================================================

 Copyright 2011 Bob Mottram. All rights reserved.

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

#include <stdio.h>
#include <zlib.h>
#include "pnglite.h"

static unsigned char * read_png_file(char * filename, png_t * ptr)
{
    int i,j,retval;
    unsigned char * buffer = NULL, * buffer2 = NULL;

    png_init(0,0);
    retval = png_open_file(ptr, filename);
    if (retval != PNG_NO_ERROR)
    {
        printf("Failed to open file %s\n", filename);

        switch(retval)
        {
        case PNG_FILE_ERROR:
        {
            printf("File error\n");
            break;
        }
        case PNG_HEADER_ERROR:
        {
            printf("Header error\n");
            break;
        }
        case PNG_IO_ERROR:
        {
            printf("IO error\n");
            break;
        }
        case PNG_EOF_ERROR:
        {
            printf("EOF error\n");
            break;
        }
        case PNG_CRC_ERROR:
        {
            printf("CRC error\n");
            break;
        }
        case PNG_MEMORY_ERROR:
        {
            printf("Memory error\n");
            break;
        }
        case PNG_ZLIB_ERROR:
        {
            printf("Zlib error\n");
            break;
        }
        case PNG_UNKNOWN_FILTER:
        {
            printf("Unknown filter\n");
            break;
        }
        case PNG_NOT_SUPPORTED:
        {
            printf("PNG format not supported\n");
            break;
        }
        case PNG_WRONG_ARGUMENTS:
        {
            printf("Wrong arguments\n");
            break;
        }
        }
        return buffer;
    }

    if (ptr->bpp < 3)
    {
        printf("Not enought bytes per pixel\n");
        return buffer;
    }

    buffer = (unsigned char *)malloc(ptr->width * ptr->height * ptr->bpp);
    png_get_data(ptr, buffer);

    if (ptr->bpp > 3)
    {
        buffer2 = (unsigned char *)malloc(ptr->width * ptr->height * 3);
        j=0;
        for (i=0; i<ptr->width * ptr->height * ptr->bpp; i+=ptr->bpp,j+=3)
        {
            buffer2[j] = buffer[i];
            buffer2[j+1] = buffer[i+1];
            buffer2[j+2] = buffer[i+2];
        }
        free(buffer);
        buffer = buffer2;
    }

    return buffer;
}

static int write_png_file(char* filename, int width, int height, unsigned char *buffer)
{
    png_t png;
    FILE * fp = fopen(filename, "wb");
    if (fp == NULL)
    {
        fprintf(stderr, "Could not open file %s for writing\n", filename);
        return 1;
    }
    fclose(fp);

    png_init(0,0);
    png_open_file_write(&png, filename);
    png_set_data(&png, width, height, 8, PNG_TRUECOLOR, buffer);
    png_close_file(&png);

    return 0;
}

