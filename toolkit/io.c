/****************************************************************

 io.c

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

/*! \file   io.c
 *  \brief  Covers the low level input and output relating to memory and files.
    In addition to memory and file handling, io has typically been a place holder
    for new functionality.
 */

#include "toolkit.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


/**
 * Moves the string to lower case for the length shown.
 * @param value the string with the case to be lowered.
 * @param length the number of bytes to be lowered.
 */
void io_lower( n_string value, n_int length )
{
    n_int loop = 0;
    while ( loop < length )
    {
        IO_LOWER_CHAR( value[loop] );
        loop++;
    }
}

/**
 * Read a number from a string.
 * @param number_string the string to be read from.
 * @param actual_value the actual value read.
 * @param decimal_divisor the value required to provide a decimal version.
 * @return number of characters read in condition of success and -1 in condition of failure.
 */
n_int io_number( n_string number_string, n_int *actual_value, n_int *decimal_divisor )
{
    n_uint  temp = 0;
    n_int   divisor = 0;
    n_int   ten_power_place = 0;
    n_int   string_point = 0;
    n_byte  negative = 0;

    if ( number_string == 0L )
    {
        return -1;
    }
    if ( number_string[0] == 0 )
    {
        return -1;
    }

    if ( number_string[0] == '-' )
    {
        negative = 1;
        string_point++;
    }

    while ( 1 )
    {
        n_char value = number_string[string_point++];
        n_uint mod_ten;
        if ( value == 0 )
        {
            n_int   translate;

            if ( negative == 1 )
            {
                translate = 0 - temp;
            }
            else
            {
                translate = temp;
            }

            *actual_value = translate;

            if ( divisor > 0 )
            {
                divisor--;
            }

            *decimal_divisor = divisor;
            return ten_power_place;
        }
        if ( value == '.' )
        {
            if ( divisor != 0 )
            {
                return SHOW_ERROR( "double decimal point in number" );
            }
            divisor = 1;
        }
        else
        {
            if ( !ASCII_NUMBER( value ) )
            {
                return SHOW_ERROR( "number contains non-numeric value" );
            }

            mod_ten = value - '0';
            if ( temp == 922337203685477580 )
            {
                if ( negative == 1 )
                {
                    if ( mod_ten > 8 )
                    {
                        return SHOW_ERROR( "number too small" );
                    }
                }
                else
                {
                    if ( mod_ten > 7 )
                    {
                        return SHOW_ERROR( "number too large" );
                    }
                }
            }
            if ( temp > 922337203685477580 )
            {
                return SHOW_ERROR( "number numerically too large" );
            }
            if ( divisor != 0 )
            {
                divisor++;
            }
            temp = ( temp * 10 ) + mod_ten;
            ten_power_place++;
        }
    }
}



/*
 This is too slow, consider:

 n_uint io_length(n_string s) {
 n_string start = s;
 while(*s)s++;
 return s - start;
 }
 */

/* this is used for finding the actual length of fixed length strings, max length is enforced */
n_int io_length( n_string value, n_int max )
{
    n_int return_length = -1;
    if ( value == 0L )
    {
        return 0;
    }
    if ( max < 1 )
    {
        return -1;
    }
    do
    {
        return_length++;
    }
    while ( ( value[return_length] != 0 ) && ( return_length < max ) );
    return return_length;
}

/*

 These are too slow too.

 Consider:

 return <0 if s<t, 0 if s==t, >0 if s>t


 n_int io_find(n_string s, n_string t)
 {
 for ( ; *s == *t; s++, t++)
 if (*s == '\0')
 return 0;
 return *s - *t;
 }

 */
#if 1
n_int io_find( n_string check, n_int from, n_int max, n_string value_find, n_int value_find_length )
{
    n_int  loop = from;
    n_int  check_length = 0;
    while ( loop < max )
    {
        if ( check[loop] == value_find[check_length] )
        {
            check_length++;
            if ( check_length == value_find_length )
            {
                return loop + 1;
            }
        }
        else
        {
            check_length = 0;
        }
        loop ++;
    }
    return -1;
}
#else
n_int io_find( n_string check, n_int from, n_int max, n_string value_find, n_int value_find_length )
{
    n_int loop = from;
    n_int check_length = 0;
    n_int match_start = from;
    
    while ( loop < max )
    {
        if ( check[loop] == value_find[check_length] )
        {
            if ( check_length == 0 )
            {
                match_start = loop;  // Remember where this potential match started
            }
            check_length++;
            if ( check_length == value_find_length )
            {
                return match_start;  // Return the start position of the match
            }
        }
        else
        {
            if ( check_length > 0 )
            {
                // Backtrack: restart search from position after where partial match began
                loop = match_start;
                check_length = 0;
            }
        }
        loop++;
    }
    return -1;
}
#endif

void io_string_write( n_string dest, n_string insert, n_int *pos )
{
    n_int  loop = 0;
    n_char character = 127;
    do
    {
        character = insert [loop++];
        if ( character )
        {
            dest[*pos] = character;
            *( pos ) += 1;
        }
    }
    while ( character );
    dest[*pos] = 0;
}

void io_three_string_combination( n_string output, n_string first, n_string second, n_string third, n_int count )
{
    n_int command_length = io_length( first, STRING_BLOCK_SIZE );
    n_int addition_length = io_length( second, STRING_BLOCK_SIZE );
    n_int total = count - ( command_length + addition_length + 1 );
    n_int loop2 = 0;
    n_int position = 0;

    io_string_write( output, " ", &position );
    io_string_write( output, first, &position );
    io_string_write( output, " ", &position );
    io_string_write( output, second, &position );
    while ( loop2 < total )
    {
        io_string_write( output, " ", &position );
        loop2++;

    }
    io_string_write( output, third, &position );
}

void io_number_to_string( n_string value, n_uint number )
{
    n_uint temp_number = number;
    n_uint digits_in_number = 0;
    n_uint multiplier = 1;
    n_uint number_location = 0;
    do
    {
        temp_number = temp_number / 10;
        digits_in_number++;
        if ( temp_number != 0 )
        {
            multiplier = multiplier * 10;
        }
    }
    while ( temp_number != 0 );
    do
    {
        value[number_location++] = '0' + ( number / multiplier ) % 10;
        multiplier = multiplier / 10;
        digits_in_number --;
    }
    while ( multiplier != 0 );
    value[number_location++] = 0;
}

void io_string_number( n_string output_string, n_string input_string, n_uint number )
{
    n_int input_length = io_length( input_string, STRING_BLOCK_SIZE );
    if ( input_length > 0 )
    {
        memory_copy( ( n_byte * )input_string, ( n_byte * )output_string, ( n_uint )input_length );
        io_number_to_string( &output_string[input_length], number );
        return ;
    }
    io_number_to_string( output_string, number );
}

void       io_three_strings( n_string output_string, n_string first_string, n_string second_string, n_string third_string, n_byte new_line )
{
    n_int first_length = io_length( first_string, STRING_BLOCK_SIZE );
    n_int second_length = io_length( second_string, STRING_BLOCK_SIZE );
    n_int third_length = io_length( third_string, STRING_BLOCK_SIZE );
    n_int carried_length = 0;

    if ( first_length > 0 )
    {
        if ( first_string != output_string )
        {
            memory_copy( ( n_byte * )first_string, ( n_byte * )output_string, ( n_uint )first_length );
        }
        carried_length += first_length;
    }
    if ( second_length > 0 )
    {
        memory_copy( ( n_byte * )second_string, ( n_byte * )&output_string[carried_length], ( n_uint )second_length );
        carried_length += second_length;
    }
    if ( third_length > 0 )
    {
        memory_copy( ( n_byte * )third_string, ( n_byte * )&output_string[carried_length], ( n_uint )third_length );
        carried_length += third_length;
    }
    if ( new_line )
    {
#ifdef    _WIN32
        output_string[carried_length++] = 13;
#endif
        output_string[carried_length++] = 10;
    }
    output_string[carried_length] = 0;
}

n_string io_string_copy( n_string string )
{
    n_string return_string = 0L;
    n_uint    string_length = ( n_uint )( io_length( string, STRING_BLOCK_SIZE ) + 1 );
    if ( string_length > 0 )
    {
        return_string = ( n_string )memory_new( string_length );
        memory_copy( ( n_byte * )string, ( n_byte * )return_string, string_length - 1 );
        return_string[string_length - 1] = 0;
    }
    return return_string;
}

void io_string_copy_buffer( n_string string, n_string buffer )
{
    n_int loop = 0;
    n_char copy_character;
    if ( string == 0L || buffer == 0L )
    {
        return;
    }
    do
    {
        copy_character = string[loop];
        buffer[loop] = copy_character;
        loop++;
    }
    while ( copy_character != 0 );
}

#ifdef SIMULATED_APE_ASSERT

void io_assert( n_string message, n_string file_loc, n_int line )
{
    printf( "Assert: %s, %s, %ld\n", message, file_loc, line );
}

#endif


