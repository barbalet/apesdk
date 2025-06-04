/****************************************************************

 test_object_string.c

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
    exit(EXIT_FAILURE);
    return -1;
}

void tof_gather( n_string file_in )
{
    n_file   local_file;
    n_file   *output_file;
    n_int    file_error = io_disk_read( &local_file, file_in );
    n_string file_out = io_string_copy( file_in );

    io_whitespace_json( &local_file );

    printf( "%s --- \n", file_in );
    file_out[0] = '2';
    if ( file_error != -1 )
    {
        n_object_type type_of;
        void *returned_blob = unknown_file_to_tree( &local_file, &type_of );
        if ( returned_blob )
        {
            output_file = unknown_json( returned_blob, type_of );
            if ( output_file )
            {
                file_error = io_disk_write( output_file, file_out );
                io_file_free( &output_file );
                exit(EXIT_FAILURE);

            }
            unknown_free( &returned_blob, type_of );
        }
    }
    else
    {
        exit(EXIT_FAILURE);
    }
}

void tof_gather_string( n_string file_in )
{
    n_int    length = io_length( file_in, STRING_BLOCK_SIZE );
    n_file   local_file;

    local_file.data = ( n_byte * )io_string_copy( file_in );
    local_file.location = 0;
    local_file.size = length;

    io_whitespace_json( &local_file );
    {
        n_object_type type_of;
        void *returned_blob = unknown_file_to_tree( &local_file, &type_of );
        unknown_free( &returned_blob, type_of );
    }
    memory_free( ( void ** )&local_file.data );
}

#ifndef OBJECT_TEST

int main( int argc, const char *argv[] )
{
#if 0
    n_string example = "{\"general_variables\":\"test\",\"general_variables2\":{\"general_variables\":[1,2,4,5,9],\"general_variables2\":-12345,\"general_variables3\":{\"general_variables\":\"test\",\"general_variables2\":[\"hat\",\"cat\",\"horse\",\"rat\"],\"general_variables3\":\"test\"}},\"general_variables3\":\"test\"}";

    n_string example = "{\"general_variables\":\"general_test\"}";/* good */

    n_string example = "{\"general_variables\":[\"0\",\"1\",\"2\",\"3\"]}"; /* good*/

    n_string example = "{\"general_variables\":[1,2,3,4]}"; /* good */

    n_string example = "{\"general_variables\":[{\"agent\":\"yellow\"},{\"agent\":\"blue\"}]}"; /* good */


    n_string example = "{\"general_variables2\":{\"general_variables\":[1,2,4,5,9],\"general_variables2\":-12345,\"general_variables3\":{\"general_variables\":\"test\",\"general_variables2\":[\"hat\",\"cat\",\"horse\",\"rat\"],\"general_variables3\":\"test\"}}}"; /*bad */

    n_string example = "{\"general_variables2\":{\"general_variables\":[1,2,4,5,9],\"general_variables3\":{\"general_variables\":\"test\",\"general_variables2\":[\"hat\",\"cat\",\"horse\",\"rat\"]}}}";/* bad */


    n_string example = "{\"general_variables2\":{\"general_variables\":[1,2,4,5,9],\"general_variables3\":{\"general_variables2\":[\"hat\",\"cat\",\"horse\",\"rat\"]}}}"; /* bad */

    n_string example = "{\"general_variables2\":{\"general_variables3\":{\"general_variables2\":[\"hat\",\"cat\",\"horse\",\"rat\"]}}}"; /* good */

    n_string example = "{\"general_variables2\":{\"general_variables\":[1,2,4,5,9],\"general_variables3\":{\"general_variables2\":[\"hat\",\"cat\",\"horse\",\"rat\"]}}}"; /* bad */

    n_string example = "{\"general_variables\":[1,2,4,5,9],\"general_variables3\":{\"general_variables2\":[\"hat\",\"cat\",\"horse\",\"rat\"]}}"; /* bad */

    n_string example = "{\"general_variables\":[1,2,4,5,9],\"general_variables3\":{\"general_variables2\":0}}"; /* bad */

    n_string example = "{\"general_variables\":[1,2,4,5,9],\"general_variables3\":\"general_variables2\"}"; /* bad */

    n_string example = "{\"general_variables\":2,\"general_variables3\":\"general_variables2\"}"; /* bad */

    n_string example = "{\"general_variables\":2,\"general_variables3\":2}"; /* good */

#else

    n_string example = "{\"general_variables\":\"test\",\"general_variables2\":{\"general_variables\":[1,2,4,5,9],\"general_variables2\":-12345,\"general_variables3\":{\"general_variables\":\"test\",\"general_variables2\":[\"hat\",\"cat\",\"horse\",\"rat\"],\"general_variables3\":\"test\"}},\"general_variables3\":\"test\"}";

    /*
    n_string example_empty_array = "{\"type\": \"sea\",\"polygons\": {}  },  {\"type\": \"lakes\",\"polygons\": {}}";

    n_string example_decimal_numbers ="{\"geocoords\": {\"topleft\": {\"latitude\": 51.509998,\"longitude\": 0.631000},\"bottomright\": {\"latitude\":51.467999,\"longitude\": 0.543000}}}";
     */

#endif

    tof_gather_string( example );

    exit(EXIT_SUCCESS);
}

#endif
