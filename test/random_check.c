/****************************************************************
 
 random_check.c
 
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

/* This provides a checking utility for the first 65536, 256 and 16 values */

#include <stdio.h>

typedef	unsigned short	n_byte2;

n_byte2 math_random(n_byte2 * local)
{
    n_byte2 tmp0;
    n_byte2 tmp1;
    
    tmp0 = local[0];
    tmp1 = local[1];
    
    local[0] = tmp1;
    switch (tmp0 & 7)
    {
        case 0:
            local[1] = (unsigned short)(tmp1 ^ (tmp0 >> 1) ^ 0xd028);
            break;
        case 4:
            local[1] = (unsigned short)(tmp1 ^ (tmp0 >> 2) ^ 0xae08);
            break;
        case 8:
            local[1] = (unsigned short)(tmp1 ^ (tmp0 >> 3) ^ 0x6320);
            break;
        default:
            local[1] = (unsigned short)(tmp1 ^ (tmp0 >> 1));
            break;
    }
    return (tmp1);
}

void from_seed(unsigned short * local)
{
    unsigned long  summation = 0;
    unsigned long  odd = 0;
    unsigned long  lower256 = 0;
    unsigned long  upper256 = 0;
    int loop = 0;
    while (loop < (256*256))
    {
        unsigned short value = math_random(local);
        summation += value;
        if (value&1) odd++;
        lower256 += (value & 255);
        upper256 += (value >> 8);
        loop++;
    }
    
    printf("average %lu odd %lu first %lu second %lu\n", summation/(256*256), odd, lower256/(256*256), upper256/(256*256));
}

void from_seed256(unsigned short * local)
{
    unsigned long  summation = 0;
    unsigned long  odd = 0;
    unsigned long  lower256 = 0;
    unsigned long  upper256 = 0;
    int loop = 0;
    while (loop < 256)
    {
        unsigned short value = math_random(local);
        summation += value;
        if (value&1) odd++;
        lower256 += (value & 255);
        upper256 += (value >> 8);
        loop++;
    }
    
    printf("256 average %lu odd %lu first %lu second %lu\n", summation/(256), odd, lower256/(256), upper256/(256));
}

void from_seed16(unsigned short * local)
{
    unsigned long  summation = 0;
    unsigned long  odd = 0;
    unsigned long  lower256 = 0;
    unsigned long  upper256 = 0;
    int loop = 0;
    while (loop < 16)
    {
        unsigned short value = math_random(local);
        summation += value;
        if (value&1) odd++;
        lower256 += (value & 255);
        upper256 += (value >> 8);
        loop++;
    }
    
    printf("16 average %lu odd %lu first %lu second %lu\n", summation/(16), odd, lower256/(16), upper256/(16));
}


int main(void)
{
    unsigned short seed[2];

    printf(" --- test math --- start ----------------------------------------------\n");
    
    seed[0] = 0x625f; seed[1] = 0xe980;
    from_seed(seed);
    
    seed[0] = 0; seed[1] = 0;
    from_seed(seed);
    
    seed[0] = 0; seed[1] = 0xffff;
    from_seed(seed);
    
    seed[0] = 0; seed[1] = 0xffff;
    from_seed(seed);
    
    seed[0] = 0x8000; seed[1] = 0x8000;
    from_seed(seed);
    
    seed[0] = 0x7fff; seed[1] = 0x7fff;
    from_seed(seed);
    
    
    seed[0] = 0x625f; seed[1] = 0xe980;
    from_seed256(seed);
    
    seed[0] = 0; seed[1] = 0;
    from_seed256(seed);
    
    seed[0] = 0; seed[1] = 0xffff;
    from_seed256(seed);
    
    seed[0] = 0; seed[1] = 0xffff;
    from_seed256(seed);
    
    seed[0] = 0x8000; seed[1] = 0x8000;
    from_seed256(seed);
    
    seed[0] = 0x7fff; seed[1] = 0x7fff;
    from_seed256(seed);
    
    
    
    seed[0] = 0x625f; seed[1] = 0xe980;
    from_seed16(seed);
    
    seed[0] = 0; seed[1] = 0;
    from_seed16(seed);
    
    seed[0] = 0; seed[1] = 0xffff;
    from_seed16(seed);
    
    seed[0] = 0; seed[1] = 0xffff;
    from_seed16(seed);
    
    seed[0] = 0x8000; seed[1] = 0x8000;
    from_seed16(seed);
    
    seed[0] = 0x7fff; seed[1] = 0x7fff;
    from_seed16(seed);
    
    printf(" --- test math ---  end  ----------------------------------------------\n");
    
    return 1;
}
