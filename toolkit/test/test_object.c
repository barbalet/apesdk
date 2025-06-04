/****************************************************************

 test_object.c

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

static n_int check_reader( n_string entry, n_object_type  * type_of, n_vect2 * values)
{
    n_file * entry_file = io_file_new_from_string_block((n_char *)entry);

    if ( entry_file )
    {
        io_whitespace_json( entry_file );
        void *returned_blob = unknown_file_to_tree( entry_file, type_of );

        io_file_free( &entry_file );
        if ( returned_blob )
        {
            if (*type_of == OBJECT_ARRAY)
            {
                n_vect2 vect_value;
                n_array * returned_array = (n_array *) returned_blob;
                n_int return_vect = object_vect2_from_array( returned_array, &vect_value);

                printf("return_vect %ld\n", return_vect);

                printf("comparison ( %ld, %ld ) , ( %ld, %ld ) \n", vect_value.x, vect_value.y, values->x, values->y);


            }
            else
            {
                printf( "no returned array\n" );
                return 1;
            }
        }
        else
        {
            printf( "no returned data\n" );
            return 1;

        }
    }
    else
    {
        printf( "reading from disk failed\n" );
        return 1;

    }
    return 0;
}

static n_int check_vector_from_array(void)
{
    n_object_type  type_of;
    n_int          return_information;
    n_vect2        values;


    values.x = -3029;
    values.y = -2134;

    return_information |= check_reader("[[-3029,-2134],[-2781,-2040],[-2831,-1911],[-3078,-2007]]", &type_of, &values);

    values.x = -2781;
    values.y = -2040;

    return_information |= check_reader("[[-2781,-2040],[-2831,-1911],[-3078,-2007]]", &type_of, &values);

    values.x = -2831;
    values.y = -1911;

    return_information |= check_reader("[[-2831,-1911],[-3078,-2007]]", &type_of, &values);

    values.x = -3078;
    values.y = -2007;

    return_information |= check_reader("[[-3078,-2007]]", &type_of, &values);

    values.x = -2781;
    values.y = -2040;

    return_information |= check_reader("[-2781,-2040]", &type_of, &values);

    values.x = -2831;
    values.y = -1911;

    return_information |= check_reader("[-2831,-1911]", &type_of, &values);

    values.x = -3078;
    values.y = -2007;

    return_information |= check_reader("[-3078,-2007]", &type_of, &values);

    if(return_information)
    {
        exit(EXIT_FAILURE);
    }
}

static n_object *check_element( n_int value )
{
    n_string_block string_value = "0 The good ship";

    string_value[0] += value;

    {
        n_object *element_obj = object_string( 0L, "name", string_value );
        object_number( element_obj, "number", value );
        object_number( element_obj, "constant", 4321 );

        return element_obj;
    }
}

static void check_object( void )
{
    n_object *new_object = object_number( 0L, "index", 1 );

    n_object *sub_object = object_number( 0L, "index", 2 );
    n_array   *new_array = array_number( -1 );

    array_add( new_array, array_number( -1 ) );
    array_add( new_array, array_number( -1 ) );
    array_add( new_array, array_string( "hello" ) );
    array_add( new_array, array_number( 2 ) );
    array_add( new_array, array_string( "is" ) );
    array_add( new_array, array_number( 4 ) );
    array_add( new_array, array_string( "it me" ) );
    array_add( new_array, array_number( 50 ) );

    object_number( sub_object, "top", 3 );

    object_array( sub_object, "array", new_array );

    array_add( new_array, array_number( 10 ) );
    array_add( new_array, array_number( 20 ) );
    array_add( new_array, array_number( 30 ) );

    io_file_debug( unknown_json( sub_object, OBJECT_OBJECT ) );

    object_number( new_object, "top", 2 );
    object_string( new_object, "name", "Benson" );

    object_string( new_object, "another", "corner" );

    io_file_debug( unknown_json( new_object, OBJECT_OBJECT ) );

    object_string( new_object, "name", "Kevin" );

    io_file_debug( unknown_json( sub_object, OBJECT_OBJECT ) );
    io_file_debug( unknown_json( new_object, OBJECT_OBJECT ) );

    {
        n_uint        count = 1;
        n_object    *being_object = check_element( 0 );
        n_array     *beings = array_object( being_object );
        while ( count < 9 )
        {
            being_object = check_element( count++ );
            array_add( beings, array_object( being_object ) );
        }
        object_array( new_object, "beings", beings );
    }

    io_file_debug( unknown_json( new_object, OBJECT_OBJECT ) );

    object_object( new_object, "example", sub_object );

    io_file_debug( unknown_json( new_object, OBJECT_OBJECT ) );

    object_number( sub_object, "top", 4 );

    io_file_debug( unknown_json( new_object, OBJECT_OBJECT ) );
    io_file_debug( unknown_json( sub_object, OBJECT_OBJECT ) );

    obj_free( &new_object );
}

int main( int argc, const char *argv[] )
{
    n_int return_value = 0;

    printf( " --- test object --- start --------------------------------------------\n" );

    check_object();

    printf( " --- test object ---  end  --------------------------------------------\n" );
    printf( " --- test check_vector_from_array ---  start  --------------------------------------------\n" );


    return_value = check_vector_from_array();

    printf( " --- test check_vector_from_array ---  end  --------------------------------------------\n" );

    exit(EXIT_SUCCESS);
}

