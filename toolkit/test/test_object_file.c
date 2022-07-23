/****************************************************************

   test_object_file.c

 =============================================================

 Copyright 1996-2022 Tom Barbalet. All rights reserved.

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
#include <time.h>

n_int draw_error( n_constant_string error_text, n_constant_string location, n_int line_number )
{
    if ( error_text )
    {
        printf( "ERROR: %s @ %s %ld\n", ( n_constant_string )error_text, location, line_number );
    }
    return -1;
}

long long current_timestamp() {
    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // calculate milliseconds
    return milliseconds;
}

void tof_gather( n_string file_in )
{
    n_file   *in_file = 0L;
    n_file   *output_file;
    n_int    file_error = 0;
    n_string file_out = io_string_copy( file_in );
    n_object_type type_of;
    long long    clock = current_timestamp();
    
    printf( "%s --- \n", file_in );

    
    in_file = io_file_new();
    
    printf("%d \t io_file_new\n", (current_timestamp() - clock));
    
    file_error = io_disk_read( in_file, file_in );
    
    printf("%d \t io_disk_read\n", (current_timestamp() - clock));

    file_out[0] = '2';
    if ( file_error != -1 )
    {
        void *returned_blob = unknown_file_to_tree( in_file, &type_of );
        
        printf("%d \t unknown_file_to_tree\n", (current_timestamp() - clock));

        io_file_free( &in_file );
        printf("%d \t io_file_free\n", (current_timestamp() - clock));
        if ( returned_blob )
        {
            output_file = unknown_json( returned_blob, type_of );
            
            printf("%d \t unknown_json\n", (current_timestamp() - clock));

            if ( output_file )
            {
                file_error = io_disk_write( output_file, file_out );
                
                printf("%d \t io_disk_write\n", (current_timestamp() - clock));

                io_file_free( &output_file );
                printf("%d \t io_file_free\n", (current_timestamp() - clock));

            }
            else
            {
                printf( "no returned output file\n" );

                unknown_free( &returned_blob, type_of );

                exit(EXIT_FAILURE);
            }
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
