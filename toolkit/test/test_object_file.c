/****************************************************************

   test_object_file.c

 =============================================================

 Copyright 1996-2025 Tom Barbalet. All rights reserved.

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

#include "../toolkit.h"

#include <stdio.h>
#include <stdlib.h>

n_int draw_error( n_constant_string error_text, n_constant_string location, n_int line_number )
{
    if ( error_text )
    {
        printf( "ERROR: %s @ %s %ld\n", ( n_constant_string )error_text, location, line_number );
    }
    return -1;
}

void tof_gather( n_string file_in )
{
    n_file   *in_file = io_file_new();
    n_file   *output_file;
    n_int    file_error = io_disk_read( in_file, file_in );
    n_string file_out = io_string_copy( file_in );
    n_object_type type_of;

    io_whitespace_json( in_file );

    printf( "%s --- \n", file_in );
    file_out[0] = '2';
    if ( file_error != -1 )
    {
        void *returned_blob = unknown_file_to_tree( in_file, &type_of );
        io_file_free( &in_file );
        if ( returned_blob )
        {
            output_file = unknown_json( returned_blob, type_of );
            if ( output_file )
            {
                file_error = io_disk_write( output_file, file_out );
                io_file_free( &output_file );
            }
            else
            {
                unknown_free( &returned_blob, type_of );

                printf( "no returned output file\n" );
                exit(EXIT_FAILURE);
            }
            // unknown_free( &returned_blob, type_of );
        }
        else
        {
            printf( "no returned object\n" );
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        printf( "reading from disk failed\n" );
        exit(EXIT_FAILURE);
    }
}

#ifndef OBJECT_TEST

int main( int argc, const char *argv[] )
{
    if ( argc == 2 )
    {
        tof_gather( ( n_string )argv[1] );
    }
    exit(EXIT_SUCCESS);
}

#endif
