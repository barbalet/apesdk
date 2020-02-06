/****************************************************************
 
 compress.c
 
 =============================================================
 
 Copyright 1996-2020 Tom Barbalet. All rights reserved.
 
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

#include "toolkit.h"
#include <stdio.h>

static n_byte brain_index_compressed[384];
static n_byte brain_index[1024];
static n_byte brain_compressed[ 32 * 32 * 32 * 2];
static n_uint brain_location;

/*static n_byte brain_base85[ (((32 * 32 * 32 * 2) + 384) * 5) / 4 ];
static n_uint brain_base85_location;*/


static void compress_bytes_to_base85(n_byte * bytes, n_byte * base85)
{
    n_uint combined = bytes[0] + (bytes[1] * 256) + (bytes[2] * 256 * 256) + (bytes[3] * 256 * 256 * 256);
    base85[0] = 33 + (combined % 83);
    base85[1] = 33 + (combined / 83) % 83;
    base85[2] = 33 + (combined / (83 * 83)) % 83;
    base85[3] = 33 + (combined / (83 * 83 * 83)) % 83;
    base85[4] = 33 + (combined / (83 * 83 * 83 * 83)) % 83;
}


static void compress_base85_to_bytes(n_byte * base85, n_byte * bytes)
{
    n_uint combined;
    n_byte delta[5];
    
    delta[0] = base85[0] - 33;
    delta[1] = base85[1] - 33;
    delta[2] = base85[2] - 33;
    delta[3] = base85[3] - 33;
    delta[4] = base85[4] - 33;

    combined = delta[0] + (delta[1] * 83) + (delta[2] * 83 * 83) + (delta[3] * 83 * 83 * 83) + (delta[3] * 83 * 83 * 83 * 83);
    
    bytes[0] =  combined & 255;
    bytes[1] = (combined >> 8) & 255;
    bytes[2] = (combined >> 16) & 255;
    bytes[3] = (combined >> 24) & 255;
}

void compress_buffer(n_byte * input, n_byte * output, n_int n, n_int compressed)
{
    n_int total_bits = n * 8;
    n_int loop = 0;
    
    n_int a_result, b_result, c_result, d_result;
    
    while (loop < total_bits)
    {
        if (compressed)
        {
            a_result = loop & 7;
            b_result = loop >> 3;
            c_result = loop % n;
            d_result = loop / n;
        }
        else
        {
            a_result = loop % n;
            b_result = loop / n;
            c_result = loop & 7;
            d_result = loop >> 3;
        }

        if (c_result == 0)
        {
            output[d_result] = 0;
        }
        output[d_result] += ((input[b_result] >> a_result) & 1) << c_result;
        loop++;
    }
}

void compress_buffer_run(n_byte * input, n_byte * output, n_int n, n_int compressed, n_int number)
{
    n_int input_add, output_add;
    n_int input_loop_end, input_loop = 0, output_loop = 0;
    
    if (compressed)
    {
        input_add = n;
        output_add = 8;
    }
    else
    {
        input_add = 8;
        output_add = n;
    }
    input_loop_end = input_add * number;
    
    while (input_loop < input_loop_end)
    {
        compress_buffer(&input[input_loop], &output[output_loop], n, compressed);
        input_loop += input_add;
        output_loop += output_add;
    }
    
}


static void compress_brain_index_compress(void)
{
    n_int three_loop = 0;
    n_int eight_loop = 0;
    while (three_loop < 384)
    {
        compress_buffer(&brain_index[eight_loop], &brain_index_compressed[three_loop], 3, 1);
        eight_loop += 8;
        three_loop += 3;
    }
}


static void compress_brain_index_expand(void)
{
    n_int three_loop = 0;
    n_int eight_loop = 0;
    while (three_loop < 384)
    {
        compress_buffer(&brain_index_compressed[three_loop], &brain_index[eight_loop], 3, 0);
        eight_loop += 8;
        three_loop += 3;
    }
}


static n_byte compresed_find_level(n_byte value)
{
    if (value < 2)
    {
        return 0;
    }
    if (value < 4)
    {
        return 1;
    }
    if (value < 8)
    {
        return 2;
    }
    if (value < 16)
    {
        return 3;
    }
    if (value < 32)
    {
        return 4;
    }
    if (value < 64)
    {
        return 5;
    }
    if (value < 128)
    {
        return 6;
    }
    return 7;
}


void compress_brain_compressed(n_byte * brain)
{
    n_int pz = 0;
    brain_location = 0;
    while (pz < 8)
    {
        n_int py = 0;
        while (py < 8)
        {
            n_int px = 0;
            while (px < 8)
            {
                n_int old = 0;
                while (old < 2)
                {
                    n_byte array[4 * 4 * 4] = {0};
                    n_byte min = 255;
                    n_byte max = 0;
                    
                    n_int sz = 0;
                    while (sz < 4)
                    {
                        n_int lz = (pz * 4) + sz;
                        n_int sy = 0;
                        while (sy < 4)
                        {
                            n_int ly = (py * 4) + sy;
                            n_int sx = 0;
                            while (sx < 4)
                            {
                                n_int lx = (px * 4) + sx;
                                n_byte value = brain[lx + (ly * 32) + (lz * 1024) + (old * 32768)];
                                array[sx + (sy * 4) + (sz * 4 * 4)] = value;
                                
                                if (value > max)
                                {
                                    max = value;
                                }
                                if (value < min)
                                {
                                    min = value;
                                }
                                sx++;
                            }
                            sy++;
                        }
                        sz++;
                    }
                    
                    {
                        n_byte bits_minus_one = compresed_find_level(max);
                        compress_buffer_run(array, &brain_compressed[brain_location], bits_minus_one + 1, 1, 4 * 2);
                        brain_location += 8 * (bits_minus_one + 1);
                        brain_index[px + (py * 8) + (pz * 8 * 8)] = bits_minus_one;
                        
                    }
                    old++;
                }
                px++;
            }
            py++;
        }
        pz++;
    }
    compress_brain_index_compress();
    printf("percent %ld\n", (brain_location * 100)/(64*1024));
}

void compress_brain_expand(n_byte * brain)
{
    n_int count = 0;
    n_int pz = 0;
    while (pz < 8)
    {
        n_int py = 0;
        while (py < 8)
        {
            n_int px = 0;
            while (px < 8)
            {
                n_int old = 0;
                while (old < 2)
                {
                    n_byte array[4 * 4 * 4] = {0};
                    n_byte min = 255;
                    n_byte max = 0;
                    
                    n_int sz = 0;
                    while (sz < 4)
                    {
                        n_int lz = (pz * 4) + sz;
                        n_int sy = 0;
                        while (sy < 4)
                        {
                            n_int ly = (py * 4) + sy;
                            n_int sx = 0;
                            while (sx < 4)
                            {
                                n_int lx = (px * 4) + sx;
                                n_byte value = brain[lx + (ly * 32) + (lz * 1024) + (old * 32768)];
                                array[sx + (sy * 4) + (sz * 4 * 4)] = value;
                                
                                if (value > max)
                                {
                                    max = value;
                                }
                                if (value < min)
                                {
                                    min = value;
                                }
                                sx++;
                            }
                            sy++;
                        }
                        sz++;
                    }
                    if ( min !=0 || max !=0 )
                    {
                        printf("%ld (%ld, %ld , %ld, %ld) %d - %d\n", count++, old, px, py, pz, min, max);
                    }
                    old++;
                }
                px++;
            }
            py++;
        }
        pz++;
    }
}
