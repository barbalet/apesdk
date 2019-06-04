/****************************************************************

 test_compress.c

 =============================================================

 Copyright 1996-2019 Tom Barbalet. All rights reserved.

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

/* this doesn't currently work, it is included here for unit
    testing in the future */

#include <stdio.h>
#include <time.h>

#include "../noble/noble.h"


n_int draw_error(n_constant_string error_text, n_constant_string location, n_int line_number)
{
    if (error_text)
    {
        printf("ERROR: %s @ %s %ld\n", (n_constant_string)error_text, location, line_number);
    }
    return -1;
}

static void check_compress(void)
{
    const n_int step = 7;
    n_int loop = 0;
    n_byte decomp_string[8*7] = {0};
    n_byte sample_string[7*3] = "1 to 99 This is a";
                            /*012012012012012012012*/
                            /*   1  2  3  4  5  6  */
    printf("%s\n", sample_string);
    

    compress_buffer_run(&sample_string, decomp_string, step, 1, 3);
    compress_buffer_run(decomp_string, &sample_string, step, 0, 3);

    printf("%s\n", sample_string);
    
}

int main(int argc, const char * argv[])
{
    printf(" --- test compress --- start ------------------------------------------------\n");
    
    check_compress();
    
    printf(" --- test compress ---  end  ------------------------------------------------\n");

    return 0;
}

