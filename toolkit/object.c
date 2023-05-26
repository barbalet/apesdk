/****************************************************************

 object.c

 =============================================================

 Copyright 1996-2023 Tom Barbalet. All rights reserved.

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

/*! \file   object.c
 *  \brief  Provides a primitive object type to allow for dynamic
 *          object creation and also reading and writing in a
 *          JSON format.
 */

#include "toolkit.h"
#include <stdio.h>

static void object_write_object( n_file *file, n_object *start );
static void object_write_array( n_file *file, n_array *start );


#undef OBJECT_DEBUG

#undef OBJECT_RETAIN1
#define OBJECT_RETAIN2 // diff name
#define OBJECT_RETAIN3 // warfare to run but with leak


#ifdef OBJECT_RETAIN1

#define STRING_COPY1(value) io_string_copy(value)

#else

#define STRING_COPY1(value) (value)

#endif

#ifdef OBJECT_RETAIN2

#define STRING_COPY2(value) io_string_copy(value)

#else

#define STRING_COPY2(value) (value)

#endif

#ifdef OBJECT_RETAIN3

#define STRING_COPY3(value) io_string_copy(value)

#else

#define STRING_COPY3(value) (value)

#endif

#ifdef OBJECT_DEBUG

#define OBJ_DBG( test, string ) if (test == 0L) printf("%s\n", string)

#else

#define OBJ_DBG( test, string ) /* test string */

#endif

static n_object *object_file_base( n_file *file );

static number_array_list    * object_number_array_list = 0L;

static n_uint * object_hashes = 0L;
static n_uint object_hash_count = 0;


n_uint object_get_hash_count(void)
{
    return object_hash_count;
}

void object_init(n_uint * hashes, n_uint hash_count)
{
    object_number_array_list = number_array_list_new();
    object_hashes = hashes;
    object_hash_count = hash_count;
}

void object_ptr_debug(void * ptr)
{
    n_uint      ptr_repr = (n_uint)ptr;
    
    printf("%ld\n", ptr_repr);
}

void object_close(void)
{
    number_array * checkptr = (number_array *)object_number_array_list->data;
    n_int          loop = 0, array_count = object_number_array_list->count;
    printf("\n array_count %ld\n", array_count);
    while (loop < array_count)
    {
        number_array * location = &checkptr[loop];
        int_list     * il_repr = location->number;
        printf("\n");
        object_ptr_debug(location->array);
        printf("\n");
        int_list_debug(il_repr);
        
        loop ++;
    }
    
    
    number_array_list_free(&object_number_array_list);
}

void object_array_add_number(void * array, n_int number)
{
    if (object_number_array_list)
    {
        if (array)
        {
            number_array * num_array = number_array_list_find_add(object_number_array_list, array);
            if (num_array)
            {
                number_array_number(num_array, number);
                
/*                printf("number_array_number\n");*/

            }
            else
            {
                printf("no num_array\n");
            }
        }
        else
        {
/*            printf("no array\n");*/
        }
    }
    else
    {
/*        printf("no object_number_array_list\n");*/
    }
}

void object_array_not_number(void * array)
{
    if (object_number_array_list)
    {
        if (array)
        {
            number_array * num_array = number_array_list_find_add(object_number_array_list, array);
            if (num_array)
            {
                number_array_not_number(num_array);
            }
        }
    }
}

static n_object_type object_type( n_array *array )
{
    return array->type;
}

static void object_erase( n_object *object )
{
    memory_erase( ( n_byte * )object, sizeof( n_object ) );
}

static n_object *object_new( void )
{
    n_object *return_object = ( n_object * )memory_new( sizeof( n_object ) );
    if ( return_object )
    {
        object_erase( return_object );
    }
    return return_object;
}

void obj_free( n_object **object );

static void obj_free_array( n_int is_array, void **payload, n_object_type type )
{
    n_array *array = 0L;

    if ( is_array )
    {
        array = ( n_array * )( *payload );
    }
    else
    {
        n_object *array_object =  ( n_object * )( *payload );
        array = ( n_array * ) & ( array_object->primitive );
    }


    if ( type == OBJECT_NUMBER )
    {

    }
    if ( type == OBJECT_BOOLEAN )
    {

    }
    if ( type == OBJECT_STRING )
    {
        memory_free( ( void ** )&array->data );
    }
    if ( type == OBJECT_ARRAY )
    {
        n_array *local_array = obj_get_array( array->data );
        obj_free_array( 1, ( void ** )&local_array, local_array->type );
    }
    if ( type == OBJECT_OBJECT )
    {
        n_object *object = obj_get_object( array->data );
        obj_free( &object );
    }
    if ( array->next )
    {
        if ( is_array )
        {
            n_array *next_array = ( n_array * )array->next;
            obj_free_array( 1, ( void ** )&next_array, next_array->type );
        }
        else
        {
            obj_free( ( n_object ** )&array->next );
        }
    }
    memory_free( payload );
}

void obj_free( n_object **object )
{
    n_array *string_primitive = &( ( *object )->primitive );
    memory_free( ( void ** ) & ( *object )->name );
    obj_free_array( 0, ( void ** ) object, string_primitive->type );
}

void object_top_object( n_file *file, n_object *top_level )
{
    io_write( file, "{", 0 );
    object_write_object( file, top_level );
    io_write( file, "}", 0 );
}

static void object_top_array( n_file *file, n_array *top_level )
{
    io_write( file, "[", 0 );
    object_write_array( file, top_level );
    io_write( file, "]", 0 );
}

static void *object_write_primitive( n_file *file, n_array *primitive )
{
    switch ( object_type( primitive ) )
    {
    case OBJECT_NUMBER:
    {
        n_int *int_data = ( n_int * )&primitive->data;
        io_writenumber( file, int_data[0], 1, 0 );
    }
    break;
    case OBJECT_BOOLEAN:
    {
        n_int *int_data = ( n_int * )&primitive->data;
        if ( int_data[0] )
        {
            io_write( file, "true", 0 );
        }
        else
        {
            io_write( file, "false", 0 );
        }
    }
    break;
    case OBJECT_STRING:
        io_write( file, "\"", 0 );
        io_write( file, primitive->data, 0 );
        io_write( file, "\"", 0 );
        break;
    case OBJECT_OBJECT:
        object_top_object( file, ( n_object * )primitive->data );
        break;
    case OBJECT_ARRAY:
        object_top_array( file, ( n_array * )primitive->data );
        break;
    default:
        ( void )SHOW_ERROR( "Object kind not found" );
        return 0L;
    }
    if ( primitive->next )
    {
        io_write( file, ",", 0 );
    }
    return primitive->next;
}

n_array *obj_get_array( n_string array )
{
    return ( n_array * )array;
}

n_object *obj_get_object( n_string object )
{
    return ( n_object * )object;
}

n_int obj_get_number( n_string object )
{
    n_int *data = ( n_int * )&object;
    return data[0];
}

n_int obj_get_boolean( n_string object )
{
    return obj_get_number( object );
}

static void object_write_object( n_file *file, n_object *start )
{
    n_object *current = start;
    do
    {
        if (current)
        {
            io_write( file, "\"", 0 );
            io_write( file, current->name, 0 );
            io_write( file, "\":", 0 );
            current = ( n_object * )object_write_primitive( file, &current->primitive );
        }
    }
    while ( current );
}

static void object_write_array( n_file *file, n_array *start )
{
    n_array *current = start;
    do
    {
        current = ( n_array * )object_write_primitive( file, current );
    }
    while ( current );
}

n_file *unknown_json( void *unknown, n_object_type type )
{
    n_file *output_file = 0L;
    if ( type == OBJECT_OBJECT )
    {
        output_file = io_file_new();
        object_top_object( output_file, ( n_object * )unknown );
    }
    if ( type == OBJECT_ARRAY )
    {
        output_file = io_file_new();
        object_top_array( output_file, ( n_array * )unknown );
    }
    return output_file;
}

static n_object *object_end_or_find( n_object *object, n_string name )
{
    n_object *previous_object = 0L;
    n_int      string_length = io_length( name, STRING_BLOCK_SIZE );
    if ( string_length > 0 )
    {
        n_uint     hash = math_hash( ( n_byte * )name, ( n_uint ) string_length );
        if ( object == 0L )
        {
            return 0L;
        }
        do
        {
            if ( hash == object->name_hash )
            {
                return previous_object;
            }
            previous_object = object;
            object = object->primitive.next;
        }
        while ( object );
    }
    return previous_object;
}

static n_object *obj_get( n_object *object, n_string name )
{
    n_object *set_object;
    n_int      string_length = io_length( name, STRING_BLOCK_SIZE );

    if ( string_length > 0 )
    {
        n_uint     hash = math_hash( ( n_byte * )name, ( n_uint )string_length );
        if ( object == 0L )
        {
            object = object_new();
        }
        if ( object_type( &object->primitive ) == OBJECT_EMPTY )
        {
            set_object = object;
        }
        else
        {
            n_object *previous_object = object_end_or_find( object, name );
            if ( previous_object == 0L )
            {
                set_object = object;
            }
            else
            {
                set_object = previous_object->primitive.next;
                if ( set_object == 0L )
                {
                    set_object = object_new();
                }
                previous_object->primitive.next = set_object;
            }
        }

        set_object->name = STRING_COPY1( name );
        set_object->name_hash = hash;

        return set_object;
    }
    return 0L;
}

n_array *array_add( n_array *array, n_array * element )
{
    if ( array )
    {
        n_array *next = array;
        do
        {
            if ( next->next )
            {
                next = next->next;
            }
        }
        while ( next->next );
        next->next = element;
    }

    return element;
}

void array_add_empty(n_array ** array, n_array * element)
{
    if (*array)
    {
        array_add(*array, element);
    }
    else
    {
        *array = array_add(0L, element);
    }
}

static void *ar_pass_through( void *ptr )
{
    if ( ptr == 0L )
    {
        ptr = memory_new( sizeof( n_array ) );
        if ( ptr )
        {
            memory_erase( ( n_byte * )ptr, sizeof( n_array ) );
        }
    }
    return ptr;
}

static void *ar_number( void *ptr, n_int set_number )
{
    n_array *cleaned = ( n_array * )ar_pass_through( ptr );
    if ( cleaned )
    {
        n_int     *number;
        cleaned->type = OBJECT_NUMBER;
        number = ( n_int * )&cleaned->data;
        number[0] = set_number;
    }
    return ( void * )cleaned;
}

static void *ar_boolean( void *ptr, n_int set_boolean )
{
    n_array *cleaned = ( n_array * )ar_pass_through( ptr );
    if ( cleaned )
    {
        n_int     *number;
        cleaned->type = OBJECT_BOOLEAN;
        number = ( n_int * )&cleaned->data;
        number[0] = set_boolean;
    }
    return ( void * )cleaned;
}

static void *ar_string( void *ptr, n_string set_string )
{
    n_array *cleaned = ( n_array * )ar_pass_through( ptr );
    if ( cleaned )
    {
        cleaned->type = OBJECT_STRING;
        cleaned->data = STRING_COPY2( set_string );
    }
    return ( void * )cleaned;
}

static void *ar_object( void *ptr, n_object *set_object )
{
    n_array *cleaned = ( n_array * )ar_pass_through( ptr );
    if ( cleaned )
    {
        cleaned->type = OBJECT_OBJECT;
        cleaned->data = ( n_string )set_object;
    }
    return ( void * )cleaned;
}

static void *ar_array( void *ptr, n_array *set_array )
{
    n_array *cleaned = ( n_array * )ar_pass_through( ptr );
    if ( cleaned )
    {
        cleaned->type = OBJECT_ARRAY;
        cleaned->data = ( n_string )set_array;
    }
    return ( void * )cleaned;
}

n_array *array_boolean( n_int set_boolean )
{
    return ar_boolean( 0L, set_boolean );
}

n_array *array_number( n_int set_number )
{
    return ar_number( 0L, set_number );
}

n_array *array_string( n_string set_string )
{
    return ar_string( 0L, set_string );
}

n_array *array_object( n_object *set_object )
{
    return ar_object( 0L, set_object );
}

n_array *array_array( n_array *set_array )
{
    return ar_array( 0L, set_array );
}

static n_object *obj_boolean( n_object *obj, n_string name, n_int boolean )
{
    return ar_boolean( obj_get( obj, name ), boolean );
}

static n_object *obj_number( n_object *obj, n_string name, n_int number )
{
    return ar_number( obj_get( obj, name ), number );
}

static n_object *obj_string( n_object *obj, n_string name, n_string string )
{
    return ar_string( obj_get( obj, name ), string );
}

static n_object *obj_object( n_object *obj, n_string name, n_object *object )
{
    return ar_object( obj_get( obj, name ), object );
}

static n_object *obj_array( n_object *obj, n_string name, n_array *array )
{
    return ar_array( obj_get( obj, name ), array );
}

n_object *object_number( n_object *obj, n_string name, n_int number )
{
    return obj_number( obj, io_string_copy( name ), number );
}

n_object *object_boolean( n_object *obj, n_string name, n_int boolean )
{
    return obj_boolean( obj, name, boolean );
}

n_object *object_string( n_object *obj, n_string name, n_string string )
{
    return obj_string( obj, io_string_copy( name ), string );
}

n_object *object_object( n_object *obj, n_string name, n_object *object )
{
    return obj_object( obj, io_string_copy( name ), object );
}

n_object *object_array( n_object *obj, n_string name, n_array *array )
{
    return obj_array( obj, io_string_copy( name ), array );
}

static n_int tracking_array_open;
static n_int tracking_object_open;
static n_int tracking_string_quote;

#define CHECK_FILE_SIZE(error_string)   if (file->location >= file->size) \
                                        { \
                                            (void)SHOW_ERROR(error_string); \
                                            return 0L; \
                                        }

static n_string object_file_read_string( n_file *file )
{
    n_string return_string = 0L;
    n_string_block block_string = {0};
    n_int          location = 0;
    if ( file->data[file->location] != '"' ) // TODO: Replace with smart char handling
    {
        ( void )SHOW_ERROR( "json not string as expected" );
        return return_string;
    }

    tracking_string_quote = 1;

    file->location ++;
    do
    {
        CHECK_FILE_SIZE( "end of json file reach unexpectedly" );
        if ( file->data[file->location] != '"' )
        {
            block_string[location] = ( n_char )file->data[file->location];
            location++;
            file->location++;
        }
    }
    while ( file->data[file->location] != '"' );
    if ( location == 0 )
    {
        ( void )SHOW_ERROR( "blank string in json file" );
        return 0L;
    }
    tracking_string_quote = 0;
    file->location++;
    CHECK_FILE_SIZE( "end of json file reach unexpectedly" );
    return_string = STRING_COPY3( block_string );
    return return_string;
}


static n_int object_file_read_number( n_file *file, n_int *with_error )
{
    n_int return_number = 0;
    n_string_block block_string = {0};
    n_int          location = 0;
    n_byte         read_char = file->data[file->location];
    n_int          char_okay = ( ASCII_NUMBER( read_char ) || ( read_char == '-' ) );
    *with_error = 1;

    if ( !char_okay )
    {
        ( void )SHOW_ERROR( "first character not number or minus" );
        return 0;
    }

    CHECK_FILE_SIZE( "end of json file reach unexpectedly for number" );

    block_string[location] = ( n_char )read_char;
    file->location++;
    location++;


    do
    {
        read_char = file->data[file->location];
        char_okay = ASCII_NUMBER( read_char );

        CHECK_FILE_SIZE( "end of json file reach unexpectedly for number" );

        if ( char_okay )
        {
            block_string[location] = ( n_char )read_char;
            location++;
            file->location++;
        }

    }
    while ( char_okay );

    {
        n_int actual_value = 1;
        n_int decimal_divisor = 1;
        n_int error_number = io_number( ( n_string )block_string, &actual_value, &decimal_divisor );

        if ( error_number == -1 )
        {
            return 0;
        }

        if ( decimal_divisor != 0 )
        {
            ( void )SHOW_ERROR( "decimal number in json file" );
            return 0;
        }
        return_number = actual_value;
    }
    *with_error = 0;
    return return_number;
}

static n_int object_file_read_boolean( n_file *file, n_int *with_error )
{
    n_int          return_number = 0;
    n_byte         read_char = file->data[file->location];
    n_int          char_okay;
    n_char        *allowed[2] = {"fals", "true"};
    n_int          allowed_advance = 1;
    *with_error = 1;

    if ( read_char == allowed[0][0] )
    {
        return_number = 0;
    }
    else if ( read_char == allowed[1][0] )
    {
        return_number = 1;
    }
    else
    {
        ( void )SHOW_ERROR( "not valid boolean (first character)" );
        return 0;
    }

    file->location++;

    while ( allowed_advance < 4 )
    {
        CHECK_FILE_SIZE( "end of json file reach unexpectedly for boolean" );

        read_char = file->data[file->location];
        char_okay = allowed[return_number][allowed_advance] == read_char;

        if ( char_okay )
        {
            file->location++;
        }
        else
        {
            ( void )SHOW_ERROR( "not valid boolean" );
            return 0;
        }
        allowed_advance++;
    }

    if ( return_number == 0 )
    {
        read_char = file->data[file->location];
        char_okay = 'e' == read_char;

        CHECK_FILE_SIZE( "end of json file reach unexpectedly for number" );

        if ( char_okay )
        {
            file->location++;
        }
        else
        {
            ( void )SHOW_ERROR( "not valid boolean (false)" );
            return 0;
        }
    }
    *with_error = 0;
    return return_number;
}

static n_object_stream_type object_stream_char( n_byte value )
{
    if ( value == '{' )
    {
        return OBJ_TYPE_OBJECT_OPEN;
    }
    if ( value == '}' )
    {
        return OBJ_TYPE_OBJECT_CLOSE;
    }
    if ( value == '[' )
    {
        return OBJ_TYPE_ARRAY_OPEN;
    }
    if ( value == ']' )
    {
        return OBJ_TYPE_ARRAY_CLOSE;
    }
    if ( ( value == 't' ) || ( value == 'f' ) )
    {
        return OBJ_TYPE_BOOLEAN;
    }
    if ( ASCII_NUMBER( value ) || ( value == '-' ) )
    {
        return OBJ_TYPE_NUMBER;
    }
    if ( ASCII_COMMA( value ) )
    {
        return OBJ_TYPE_COMMA;
    }
    if ( ASCII_COLON( value ) )
    {
        return OBJ_TYPE_COLON;
    }
    if ( ASCII_QUOTE( value ) )
    {
        return OBJ_TYPE_STRING_NOTATION;
    }
    return OBJ_TYPE_EMPTY;
}

n_array *number_base_array = 0L;


static n_array *object_file_array( n_file *file )
{
    n_array *base_array = 0L;

    n_object_stream_type stream_type;
    n_object_stream_type stream_type_in_this_array = OBJ_TYPE_EMPTY;

    if ( object_stream_char( file->data[file->location] ) != OBJ_TYPE_ARRAY_OPEN )
    {
        ( void )SHOW_ERROR( "json not array as expected" );
        return base_array;
    }

    tracking_array_open ++;

    file->location ++;
    do
    {
        CHECK_FILE_SIZE( "end of json file reach unexpectedly" );
        stream_type = object_stream_char( file->data[file->location] );

        if ( stream_type_in_this_array == OBJ_TYPE_EMPTY )
        {
            stream_type_in_this_array = stream_type;
        }
        else if ( stream_type_in_this_array != stream_type )
        {
            ( void )SHOW_ERROR( "array contains multiple types" );
            return 0L;
        }

        if ( stream_type == OBJ_TYPE_ARRAY_OPEN )
        {
            n_array *array_value = object_file_array( file );
            if ( array_value )
            {
                stream_type = object_stream_char( file->data[file->location] );

                if ( ( stream_type == OBJ_TYPE_ARRAY_CLOSE ) || ( stream_type == OBJ_TYPE_COMMA ) )
                {
                    if ( base_array == 0L )
                    {
                        base_array = array_array( array_value );
                    }
                    else
                    {
                        array_add( base_array, array_array( array_value ) );
                    }
                }
            }
            stream_type = object_stream_char( file->data[file->location] );
        }
        if ( stream_type == OBJ_TYPE_OBJECT_OPEN )
        {
            n_object *object_value = object_file_base( file );
            OBJ_DBG( object_value, "object value is nil?" );
            if ( object_value )
            {
                file->location++;
                CHECK_FILE_SIZE( "end of json file reach unexpectedly" );
                stream_type = object_stream_char( file->data[file->location] );

                if ( ( stream_type == OBJ_TYPE_ARRAY_CLOSE ) || ( stream_type == OBJ_TYPE_COMMA ) )
                {
                    if ( base_array == 0L )
                    {
                        base_array = array_object( object_value );
                    }
                    else
                    {
                        array_add( base_array, array_object( object_value ) );
                    }
                }
            }

            OBJ_DBG( base_array, "base array still nil?" );

            CHECK_FILE_SIZE( "end of json file reach unexpectedly" );
            stream_type = object_stream_char( file->data[file->location] );
        }
        if ( stream_type == OBJ_TYPE_STRING_NOTATION )
        {
            n_string string_value = object_file_read_string( file );
            if ( string_value )
            {
                stream_type = object_stream_char( file->data[file->location] );

                if ( ( stream_type == OBJ_TYPE_ARRAY_CLOSE ) || ( stream_type == OBJ_TYPE_COMMA ) )
                {
                    if ( base_array == 0L )
                    {
                        base_array = array_string( string_value );
                    }
                    else
                    {
                        array_add( base_array, array_string( string_value ) );
                    }
                }
            }
        }
        if ( stream_type == OBJ_TYPE_NUMBER )
        {
            n_int with_error;
            n_int number_value = object_file_read_number( file, &with_error );
            
            if ( with_error == 0 )
            {
                stream_type = object_stream_char( file->data[file->location] );

                if ( ( stream_type == OBJ_TYPE_ARRAY_CLOSE ) || ( stream_type == OBJ_TYPE_COMMA ) )
                {
                    if ( base_array == 0L )
                    {
                        base_array = array_number( number_value );
                    }
                    else
                    {
                        array_add( base_array, array_number( number_value ) );
                    }
                }
                
                if (number_base_array == 0L)
                {
                    number_base_array = base_array;
                }
                
                object_array_add_number(number_base_array, number_value);
            }
        }
        if ( stream_type == OBJ_TYPE_BOOLEAN )
        {
            n_int with_error;
            n_int boolean_value = object_file_read_boolean( file, &with_error );

            if ( with_error == 0 )
            {
                stream_type = object_stream_char( file->data[file->location] );

                if ( ( stream_type == OBJ_TYPE_ARRAY_CLOSE ) || ( stream_type == OBJ_TYPE_COMMA ) )
                {
                    if ( base_array == 0L )
                    {
                        base_array = array_boolean( boolean_value );
                    }
                    else
                    {
                        array_add( base_array, array_boolean( boolean_value ) );
                    }
                }
            }
        }
        if ( stream_type == OBJ_TYPE_ARRAY_CLOSE )
        {
            tracking_array_open --;
            
            /* TODO: This is based on a sign array layer then getting to the data */
            if (tracking_array_open == 2)
            {
                object_array_add_number(number_base_array, BIG_INTEGER);
            }
            if (tracking_array_open == 1)
            {
                number_base_array = 0L;
            }
        }
        file->location ++;

        OBJ_DBG( base_array, "base array nil check?" );
    }
    while ( stream_type == OBJ_TYPE_COMMA );

    OBJ_DBG( base_array, "base array really nil?" );

    return base_array;
}

static n_int object_string_key(n_string string_key)
{
    n_uint string_hash = math_hash((n_byte *)string_key, io_length(string_key, STRING_BLOCK_SIZE));
    /*
     static n_uint * object_hashes = 0L;
     static n_uint object_hash_count = 0;
     */
    
    if (object_hash_count == 0)
    {
        return -2;
    }
    if (object_hashes)
    {
        n_uint loop = 0;
        while (loop < object_hash_count)
        {
            if (object_hashes[loop] == string_hash)
            {
                return loop;
            }
            loop++;
        }
        return -3;
    }
    return -1;
}

static n_object *object_file_base( n_file *file )
{
    n_object *base_object = 0L;
    n_object_stream_type stream_type;
    CHECK_FILE_SIZE( "file read outside end of file" );

    stream_type = object_stream_char( file->data[file->location] );

    if ( stream_type == OBJ_TYPE_OBJECT_OPEN )
    {
        tracking_object_open++;
        do
        {
            file->location++;
            CHECK_FILE_SIZE( "file read outside end of file" );
            stream_type = object_stream_char( file->data[file->location] );
            if ( stream_type == OBJ_TYPE_STRING_NOTATION )
            {
                n_string string_key = object_file_read_string( file );
                if ( string_key )
                {
                    stream_type = object_stream_char( file->data[file->location] );
                    if ( stream_type == OBJ_TYPE_COLON )
                    {
                        file->location++;
                        CHECK_FILE_SIZE( "file read outside end of file" );
                        stream_type = object_stream_char( file->data[file->location] );

                        if ( stream_type == OBJ_TYPE_OBJECT_OPEN )
                        {
                            n_object *insert_object = object_file_base( file );
                            if ( insert_object )
                            {
                                if ( base_object )
                                {
                                    obj_object( base_object, string_key, insert_object );
                                }
                                else
                                {
                                    base_object = obj_object( base_object, string_key, insert_object );
                                }
                                file->location++;
                                CHECK_FILE_SIZE( "file read outside end of file" );
                                stream_type = object_stream_char( file->data[file->location] );
                            }
                        }
                        if ( stream_type == OBJ_TYPE_NUMBER )
                        {
                            n_int number_error;
                            n_int number_value = object_file_read_number( file, &number_error );
                            if ( number_error == 0 )
                            {
                                stream_type = object_stream_char( file->data[file->location] );
                                if ( ( stream_type == OBJ_TYPE_OBJECT_CLOSE ) || ( stream_type == OBJ_TYPE_COMMA ) )
                                {
                                    if ( base_object )
                                    {
                                        obj_number( base_object, string_key, number_value );
                                    }
                                    else
                                    {
                                        base_object = obj_number( base_object, string_key, number_value );
                                    }
                                }
                                CHECK_FILE_SIZE( "file read outside end of file" );
                                stream_type = object_stream_char( file->data[file->location] );
                            }
                        }
                        if ( stream_type == OBJ_TYPE_BOOLEAN )
                        {
                            n_int number_error;
                            n_int boolean_value = object_file_read_boolean( file, &number_error );
                            if ( number_error == 0 )
                            {
                                stream_type = object_stream_char( file->data[file->location] );
                                if ( ( stream_type == OBJ_TYPE_OBJECT_CLOSE ) || ( stream_type == OBJ_TYPE_COMMA ) )
                                {
                                    if ( base_object )
                                    {
                                        obj_boolean( base_object, string_key, boolean_value );
                                    }
                                    else
                                    {
                                        base_object = obj_boolean( base_object, string_key, boolean_value );
                                    }
                                }
                                CHECK_FILE_SIZE( "file read outside end of file" );
                                stream_type = object_stream_char( file->data[file->location] );
                            }
                        }
                        if ( stream_type == OBJ_TYPE_STRING_NOTATION )
                        {
                            n_string string_value = object_file_read_string( file );
                            if ( string_value )
                            {
                                stream_type = object_stream_char( file->data[file->location] );
                                if ( ( stream_type == OBJ_TYPE_OBJECT_CLOSE ) || ( stream_type == OBJ_TYPE_COMMA ) )
                                {
                                    if ( base_object )
                                    {
                                        obj_string( base_object, string_key, string_value );
                                    }
                                    else
                                    {
                                        base_object = obj_string( base_object, string_key, string_value );
                                    }
                                }
                                CHECK_FILE_SIZE( "file read outside end of file" );
                                stream_type = object_stream_char( file->data[file->location] );
                            }
                        }
                        if ( stream_type == OBJ_TYPE_ARRAY_OPEN )
                        {
                            n_array *array_value = object_file_array( file ); // TODO: rename object_file_read_array
                            if ( array_value )
                            {
                                stream_type = object_stream_char( file->data[file->location] );
                                if ( ( stream_type == OBJ_TYPE_OBJECT_CLOSE ) || ( stream_type == OBJ_TYPE_COMMA ) )
                                {
                                    n_int string_key_output = object_string_key(string_key);
                                    
                                    if (string_key_output > -1)
                                    {
                                        object_array_add_number(number_base_array, BIG_NEGATIVE_INTEGER + string_key_output);
                                    }
                                    
                                    if ( base_object )
                                    {
                                        obj_array( base_object, string_key, array_value );
                                    }
                                    else
                                    {
                                        base_object = obj_array( base_object, string_key, array_value );
                                    }
                                }
                                CHECK_FILE_SIZE( "file read outside end of file" );
                                stream_type = object_stream_char( file->data[file->location] );
                            }
                            OBJ_DBG( array_value, "array value nil?" );
                        }
                    }
                }
            }
            if ( stream_type == OBJ_TYPE_OBJECT_CLOSE )
            {
                tracking_object_open--;
            }
        }
        while ( stream_type == OBJ_TYPE_COMMA );
    }
    return base_object;
}

void unknown_free( void **unknown, n_object_type type )
{
    if ( type == OBJECT_ARRAY )
    {
        n_array **found_array = ( n_array ** )unknown;
        obj_free_array( 1, ( void ** )found_array, ( *found_array )->type );
    }
    if ( type == OBJECT_OBJECT )
    {
        n_object **found_object = ( n_object ** )unknown;
        obj_free( found_object );
    }
}

void *unknown_file_to_tree( n_file *file, n_object_type *type )
{
    n_object  *base_object = 0L;
    n_array   *base_array = 0L;
    n_int      something_wrong = 0;

    n_object_stream_type stream_type;

    tracking_array_open = 0;
    tracking_object_open = 0;
    tracking_string_quote = 0;
    io_whitespace_json( file );
    file->location = 0;

    stream_type = object_stream_char( file->data[file->location] );

    if ( stream_type == OBJ_TYPE_OBJECT_OPEN )
    {
        *type = OBJECT_OBJECT;
        base_object = object_file_base( file );
    }

    if ( stream_type == OBJ_TYPE_ARRAY_OPEN )
    {
        *type = OBJECT_ARRAY;
        base_array = object_file_array( file );
    }

    if ( tracking_array_open != 0 )
    {
        ( void )SHOW_ERROR( "Array json does not match up" );
        something_wrong = 1;
    }
    if ( tracking_object_open != 0 )
    {
        ( void )SHOW_ERROR( "Object json does not match up" );
        something_wrong = 1;
    }
    if ( tracking_string_quote != 0 )
    {
        ( void )SHOW_ERROR( "String quote json does not match up" );
        something_wrong = 1;
    }
    if ( something_wrong )
    {
        printf("\n\n");

        printf("%c", file->data[file->location-3]);
        printf("%c", file->data[file->location-2]);
        printf("%c", file->data[file->location-1]);

        printf("~%c~", file->data[file->location]);
        
        printf("%c", file->data[file->location+1]);
        printf("%c", file->data[file->location+2]);
        printf("%c", file->data[file->location+3]);

        printf("\n\n");

        if ( base_object )
        {
            obj_free( &base_object );
        }
        if ( base_array )
        {
            obj_free_array( 1, ( void ** )&base_array, base_array->type );
        }
        return 0L;
    }
    if ( *type == OBJECT_ARRAY )
    {
        return ( void * )base_array;
    }

    return ( void * )base_object;
}

n_string obj_contains( n_object *base, n_string name, n_object_type type )
{
    n_object *return_object = base;
    n_int      string_length = io_length( name, STRING_BLOCK_SIZE );
    if ( string_length > 0 )
    {
        n_uint     hash = math_hash( ( n_byte * )name, ( n_uint )string_length );
        if ( return_object == 0L )
        {
            return 0L;
        }
        do
        {
            if ( ( hash == return_object->name_hash ) && ( type == object_type( &return_object->primitive ) ) )
            {
                return return_object->primitive.data;
            }
            return_object = return_object->primitive.next;
        }
        while ( return_object );
    }
    return 0L;
}

n_int obj_contains_number( n_object *base, n_string name, n_int *number )
{
    n_object *return_object = base;
    n_int      string_length = io_length( name, STRING_BLOCK_SIZE );
    if ( string_length > 0 )
    {
        n_uint     hash = math_hash( ( n_byte * )name, ( n_uint )string_length );
        if ( return_object == 0L )
        {
            return 0;
        }
        do
        {
            if ( ( hash == return_object->name_hash ) && ( OBJECT_NUMBER == object_type( &return_object->primitive ) ) )
            {
                n_int *data = ( n_int * )&return_object->primitive.data;
                number[0] = data[0];
                return 1;
            }
            return_object = return_object->primitive.next;
        }
        while ( return_object );
    }
    return 0;
}

n_array *obj_array_next( n_array *array, n_array *element )
{
    if ( element == 0L )
    {
        return array;
    }
    return ( n_array * )element->next;
}

n_int obj_array_count( n_array *array_obj )
{
    n_array *arr_second_follow = 0L;
    n_int     count = 0;
    if ( array_obj )
    {
        while ( ( arr_second_follow = obj_array_next( array_obj, arr_second_follow ) ) )
        {
            count++;
        }
    }
    return count;
}


n_int obj_contains_array_nbyte2( n_object *base, n_string name, n_byte2 *array_numbers, n_int size )
{
    n_string array_string;
    if ( ( array_string = obj_contains( base, name, OBJECT_ARRAY ) ) )
    {
        n_array *array_obj = obj_get_array( array_string );
        n_array *arr_follow = 0L;
        n_int     count = 0;
        n_int     estimated_size = obj_array_count( array_obj );
        if ( estimated_size != size )
        {
            return -1;
        }
        while ( ( arr_follow = obj_array_next( array_obj, arr_follow ) ) )
        {
            n_int follow_number = obj_get_number( arr_follow->data );
            array_numbers[count++] = follow_number;
            if ( count == size )
            {
                return 1;
            }
        }
    }
    return 0;
}

n_int obj_contains_array_numbers( n_object *base, n_string name, n_int *array_numbers, n_int size )
{
    n_string array_string;
    if ( ( array_string = obj_contains( base, name, OBJECT_ARRAY ) ) )
    {
        n_array *array_obj = obj_get_array( array_string );
        n_array *arr_follow = 0L;
        n_int     count = 0;
        n_int     estimated_size = obj_array_count( array_obj );
        if ( estimated_size != size )
        {
            return 0;
        }
        while ( ( arr_follow = obj_array_next( array_obj, arr_follow ) ) )
        {
            if (arr_follow->type == OBJECT_NUMBER)
            {
                n_int follow_number = obj_get_number( arr_follow->data );
                array_numbers[count++] = follow_number;
                if ( count == size )
                {
                    return 1;
                }
            }
            else
            {
                return -1;
            }
        }
    }
    return 0;
}

n_array * object_vect2_array(n_vect2 * value)
{
    n_array  * point = array_number(value->x);
    array_add( point, array_number(value->y) );
    return point;
}

n_array * object_vect2_pointer(n_vect2 * vect_array, n_uint count)
{
    n_array * json_array = 0L;
    n_int loop = 0;
    while (loop < count)
    {
        array_add_empty( &json_array, array_array(object_vect2_array(&vect_array[loop])));
        loop++;
    }
    return json_array;
}

n_int object_unwrap_four_vect2( n_string pass_through , n_byte * buffer)
{
    return vect2_unwrap_number_entry( pass_through, buffer, 4);
}

n_int object_unwrap_two_vect2( n_string pass_through , n_byte * buffer)
{
    return vect2_unwrap_number_entry( pass_through, buffer, 2);
}

n_int object_unwrap_vect2( n_string pass_through , n_byte * buffer)
{
    return vect2_unwrap_number_entry( pass_through, buffer, 1);
}

n_int object_count_name_vect2(n_vect2 * vect_array, n_uint count, object_unwrap * wrap_func, n_string name, n_object * object)
{
    n_string array_string;
    n_int out_value = 0;
    if ( ( array_string = obj_contains( object, name, OBJECT_ARRAY ) ) )
    {
        n_array * array_obj = obj_get_array( array_string );
        memory_list * mem_list = object_unwrap_array(array_obj, sizeof(n_vect2), wrap_func, OBJECT_ARRAY);
                
        if (mem_list->count == count)
        {
            n_int index = 0;
            n_vect2 * value = (n_vect2 *)mem_list->data;
            while (index < (count * 2))
            {
                vect_array[index].data[0] = value[index].data[0];
                vect_array[index].data[1] = value[index].data[1];
                index ++;

                vect_array[index].data[0] = value[index].data[0];
                vect_array[index].data[1] = value[index].data[1];
                index ++;
            }
            out_value = 1;
        }
        memory_list_free(&mem_list);
    }
    return out_value;
}

static n_array * object_vect2_list(memory_list * vect_list)
{
    n_array * json_array = 0L;
    if (vect_list->unit_size == sizeof(n_vect2))
    {
        return object_vect2_pointer((n_vect2 *)vect_list->data, vect_list->count);
    }
    return json_array;
}


n_string object_type_string(n_object_type objtype)
{
    return n_object_type_string[objtype];
}

n_int object_vect2_from_array(n_array * vect_element, n_vect2 * vect_list)
{        
    n_int   loop = 0;
    n_array * arr_follow = 0L;
    while ( ( arr_follow = obj_array_next( vect_element, arr_follow ) ) )
    {
        if (arr_follow->type == OBJECT_NUMBER)
        {
            n_int number = obj_get_number( arr_follow->data );
            vect_list->data[loop++] = number;
        }else{
            printf("Expected OBJECT_NUMBER not %s\n\n", object_type_string(arr_follow->type));
            return -1;
        }
        
        if (loop == 2)
        {
            return 1;
        }
    }
    return 0;
}


memory_list * object_list_vect2(n_array * vect_array)
{
    memory_list * vect_list = 0L;
    if (vect_array)
    {
        n_int count_in_array = obj_array_count(vect_array);
        n_array * arr_follow = 0L;
        vect_list = memory_list_new(sizeof(n_vect2), count_in_array);
        while ( ( arr_follow = obj_array_next( vect_array, arr_follow ) ) )
        {
            if (arr_follow->type == OBJECT_ARRAY)
            {
                n_vect2 temp_vect;
                n_array * vect_element = obj_get_array( arr_follow->data );
                n_int error_response = object_vect2_from_array(vect_element, &temp_vect);
                if (error_response == 1)
                {
                    memory_list_copy(vect_list, (n_byte*)&temp_vect);
                }
                
                if (error_response == -1)
                {
                    memory_list_free(&vect_list);
                    return 0L;
                }
            }
        }
        vect_list->count = count_in_array;
    }
    return vect_list;
}

n_object * object_vect2_names(n_string names, memory_list * vect_array)
{
    return object_array(0L, names, object_vect2_list(vect_array));
}

n_object * object_vect2_name( n_object *obj, n_string name, n_vect2 * value)
{
    return object_array( obj, name, object_vect2_array(value));
}

n_int   object_name_vect2(n_string name, n_vect2 * value, n_object * input_json)
{
    n_string str_point = obj_contains(input_json, name, OBJECT_ARRAY);
    if (str_point)
    {
        n_array * point_array = obj_get_array(str_point);
        n_int number_elements = obj_array_count(point_array);
        if (number_elements == 2)
        {
            n_int return_value = obj_contains_array_numbers(input_json, name, (n_int *)value, 2);
            if (return_value == 0)
            {
                return SHOW_ERROR("expecting two integers for point");
            }
            if (return_value == -1)
            {
                return SHOW_ERROR("expecting integers for point");
            }
            return 1;
        }
        return SHOW_ERROR("expecting two integers for point");
    }
    return SHOW_ERROR("expecting point element");
}

memory_list * object_unwrap_array(n_array * general_array, n_uint size, object_unwrap wrap_func , n_object_type type)
{
    memory_list * return_list = 0L;
    if (general_array)
    {
        n_int     general_array_count = obj_array_count(general_array);
        n_array * arr_follow = 0L;
        return_list = memory_list_new(size, general_array_count);
        while ( ( arr_follow = obj_array_next( general_array, arr_follow ) ) )
        {
            if (arr_follow->type == type)
            {
                n_byte * temp_buffer = memory_new(size);
                if (1 == wrap_func(arr_follow->data, temp_buffer))
                {
                    memory_list_copy(return_list, temp_buffer);
                }
                memory_free((void**)&temp_buffer);
            }
        }
    }
    return return_list;
}

