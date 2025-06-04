#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include "pnglite.h"

/// Reads the PNG file
/// - Parameter filename: from the filename
/// - Parameter ptr: the png pointer
/// - Returns: The byte map where the color spread is the smallest interation in the byte.
unsigned char * read_png_file(char * filename, png_t * ptr)
{
    int retval;
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
        int i, j = 0;
        buffer2 = (unsigned char *)malloc(ptr->width * ptr->height * 3);
        
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


/// Writes a PNG file.
/// - Parameter filename: The name of the file.
/// - Parameter width: The width of the file.
/// - Parameter height: The height of the file.
/// - Parameter buffer: The image buffer where the colors are the last part of the byte division.
int write_png_file(char* filename, int width, int height, unsigned char *buffer)
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

