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

static n_byte tof_has_trailing_newline( n_file *file )
{
    if ( ( file == 0L ) || ( file->data == 0L ) || ( file->location == 0 ) )
    {
        return 0;
    }
    return IS_RETURN( file->data[file->location - 1] );
}

static n_string tof_output_name( n_string file_in )
{
    n_string file_out = io_string_copy( file_in );
    n_int string_length;
    n_int basename_start;

    if ( file_out == 0L )
    {
        return 0L;
    }

    string_length = io_length( file_out, STRING_BLOCK_SIZE );
    if ( string_length <= 0 )
    {
        return file_out;
    }

    basename_start = string_length - 1;
    while ( ( basename_start > 0 ) &&
            ( file_out[basename_start - 1] != '/' ) &&
            ( file_out[basename_start - 1] != '\\' ) )
    {
        basename_start--;
    }
    file_out[basename_start] = '2';

    return file_out;
}

void tof_gather( n_string file_in )
{
    n_file   *in_file = io_file_new();
    n_file   *output_file;
    n_int    file_error = io_disk_read( in_file, file_in );
    n_byte   trailing_newline = 0;
    n_string file_out = tof_output_name( file_in );
    n_object_type type_of;

    if ( file_error != -1 )
    {
        trailing_newline = tof_has_trailing_newline( in_file );
    }

    io_whitespace_json( in_file );

    printf( "%s --- \n", file_in );
    if ( file_error != -1 )
    {
        void *returned_blob = unknown_file_to_tree( in_file, &type_of );
        io_file_free( &in_file );
        if ( returned_blob )
        {
            output_file = unknown_json( returned_blob, type_of );
            if ( output_file )
            {
                if ( trailing_newline && ( io_write( output_file, "", 1 ) == -1 ) )
                {
                    io_file_free( &output_file );
                    memory_free( ( void ** )&file_out );
                    printf( "adding newline to output failed\n" );
                    exit( EXIT_FAILURE );
                }
                file_error = io_disk_write( output_file, file_out );
                io_file_free( &output_file );
                if ( file_error == -1 )
                {
                    memory_free( ( void ** )&file_out );
                    printf( "writing to disk failed\n" );
                    exit( EXIT_FAILURE );
                }
            }
            else
            {
                memory_free( ( void ** )&file_out );
                unknown_free( &returned_blob, type_of );

                printf( "no returned output file\n" );
                exit(EXIT_FAILURE);
            }
            // unknown_free( &returned_blob, type_of );
        }
        else
        {
            memory_free( ( void ** )&file_out );
            printf( "no returned object\n" );
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        memory_free( ( void ** )&file_out );
        printf( "reading from disk failed\n" );
        exit(EXIT_FAILURE);
    }
    memory_free( ( void ** )&file_out );
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
