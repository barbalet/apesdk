/****************************************************************

 compress.c

 =============================================================

 Copyright 1996-2017 Tom Barbalet. All rights reserved.

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

/*

 This codes is based on a modified version of:

 **
 ** Copyright (c) 1989 Mark R. Nelson
 **
 ** LZW data compression/expansion demonstration program.
 **
 ** April 13, 1989
 **

 Now in the public domain.

 */

#include "noble.h"

#define BITS 16
#define HASHING_SHIFT (BITS-8)
#define MAX_VALUE     ((1<<BITS)-1)
#define MAX_CODE      (MAX_VALUE-1)

#if BITS == 16
#define TABLE_SIZE 99991
#endif
#if BITS==14
#define TABLE_SIZE 18041
#endif
#if BITS==13
#define TABLE_SIZE 9029
#endif
#if BITS==12
#define TABLE_SIZE 5021
#endif

n_c_int   code_value[TABLE_SIZE];         /* This is code value array            */
n_byte4  prefix_code[TABLE_SIZE];        /* This array holds the prefix codes   */
n_byte    append_character[TABLE_SIZE];              /* This array holds the appended chars */

/*
 ** This is the hashing routine. It tries to find a match for the prefix+char
 ** string in the string table. If it finds it, the index is returned. If
 ** the string is not found, the first available index in the string table is
 ** returned instead.
 */
static n_c_int find_match(n_byte4 hash_prefix, n_byte4 hash_character)
{
    n_c_int offset;
    n_c_int index=(hash_character<<HASHING_SHIFT) ^ hash_prefix;
    if (index==0)
    {
        offset=1;
    }
    else
    {
        offset=TABLE_SIZE-index;
    }
    while (1)
    {
        if (code_value[index] == -1)
            return index;

        if (prefix_code[index] == hash_prefix && append_character[index] == hash_character)
            return index;

        index-=offset;
        if (index<0)
        {
            index+=TABLE_SIZE;
        }
    }
}

/*
 ** This routine simply decodes a string from the string table, storing
 ** it in buffer. The buffer can then be output in reverse order by
 ** the expansion program.
 */
static n_byte * decode_string(n_byte *buffer, n_byte4 code)
{
    n_c_int i;
    i=0;
    while (code>255)
    {
        *buffer++=append_character[code];
        code = prefix_code[code];
        if (i++>4000)
        {
            (void)SHOW_ERROR("Fatal error during code expansion.\n");
        }
    }
    *buffer = (n_byte)code;
    return buffer;
}
/*
 ** The following two routines are used to output variable negth
 ** codes. They are written strivtly for clarity and are not
 ** particularly efficent.
 */
unsigned int input_code(n_file *input)
{
    n_byte4 return_value;
    static n_c_int input_bit_count=0;
    static n_byte4 input_bit_buffer=0L;
    while (input_bit_count<=24)
    {
        n_byte byte_character;
        (void) io_read_bin(input, &byte_character);

        input_bit_buffer |= ((unsigned long) byte_character) << (24-input_bit_count);
        input_bit_count+=8;
    }
    return_value = input_bit_buffer>>(32-BITS);
    input_bit_buffer<<=BITS;
    input_bit_count-=BITS;
    return return_value;
}

void output_code(n_file *output, n_byte4 code)
{
    static n_c_int output_bit_count=0;
    static n_byte4 output_bit_buffer=0L;
    output_bit_buffer|=(n_byte4) code << (32-BITS-output_bit_count);
    output_bit_count += BITS;
    while (output_bit_count>=8)
    {
        /* putc(output_bit_buffer>>24,output);*/

        (void)io_file_write(output, output_bit_buffer>>24);

        output_bit_buffer<<=8;
        output_bit_count-=8;
    }
}

/*
 ** This is the compression routine. The code should be a fairly close
 ** match to the algorithm accompanying the article.
 */
void compress_compress(n_file *input,n_file *output)
{
    n_byte4 next_code = 256;                 /* next available string code */
    n_byte4 string_code;
    n_byte4 index = 0;
    n_byte   byte_character;

    while(index<TABLE_SIZE)     /* clear string table */
    {
        code_value[index++]=-1;
    }

    (void)io_read_bin(input, &byte_character);

    string_code = byte_character;      /* get the first code */
    /*
     ** This is the main loop where it all happens. This loop runs until all of
     ** the input has been exhausted. Note that it stops adding codes to the
     ** table after all of the possible codes have been defined.
     */

    while (io_read_bin(input, &byte_character) != -1)
    {
        n_byte4 character = byte_character;

        index = find_match(string_code, character);

        if (code_value[index]!=-1)
        {
            string_code = code_value[index];
        }
        else
        {
            if (next_code<=MAX_CODE)
            {
                code_value[index] = next_code++;
                prefix_code[index] = string_code;
                append_character[index] = (n_byte)character;
            }
            output_code(output,string_code);
            string_code=character;
        }
    }
    output_code(output,string_code);
    output_code(output,MAX_VALUE);
    output_code(output,0);
}

/*
 ** This is expansion routine. It takes an LZW format file, and expands
 ** it to an output file. The code here should be a faitly close match to
 ** the algorithm in the accompaning article.
 */
void compress_expand(n_file *input,n_file *output)
{
    n_byte4 next_code =  256;
    n_byte4 new_code;
    n_byte4 old_code=input_code(input);
    n_c_int  character=old_code;
    n_byte   *string;

    n_byte    decode_stack[4000];             /* This array holds the decoded string */


    (void)io_file_write(output, (n_byte)old_code);

    while ((new_code=input_code(input)) != (MAX_VALUE))
    {

        /*
         ** This code checks for the special STRING+CHARACTER+STRING+CHARACTER+STRING
         ** case which generates an undefined code. It handles it by decoding
         ** the last code, adding a single character to the end of the decode string.
         */

        if (new_code >= next_code)
        {
            *decode_stack = (n_byte)character;
            string = decode_string(decode_stack+1,old_code);
        }
        /*
         ** Otherwise we do a straight decode of the new code.
         */
        else
        {
            string = decode_string(decode_stack, new_code);
        }
        character = *string;
        while(string >= decode_stack)
        {
            (void)io_file_write(output, *string--);
        }
        if (next_code <= MAX_CODE)
        {
            prefix_code[next_code] = old_code;
            append_character[next_code] = (n_byte)character;
            next_code++;
        }
        old_code = new_code;
    }
}
