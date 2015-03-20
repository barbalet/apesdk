/****************************************************************

 test_io.c

 =============================================================

 Copyright 1996-2015 Tom Barbalet. All rights reserved.

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

typedef struct
{
    n_byte2     location[2];
    n_byte      direction_facing;
    n_byte      velocity;
    n_byte2     stored_energy;
    n_byte2     date_of_birth[2];
    n_byte2     seed[2];
    n_byte2     macro_state;
    n_byte2     brain_state[6];
    n_byte2     height;
    n_byte2     mass;
    n_byte2     script_overrides;
} test_being;

#define FIL_BEI (0x30)

n_int draw_error(n_constant_string error_text, n_constant_string location, n_int line_number)
{
    if (error_text)
    {
        printf("ERROR: %s @ %s %ld\n", (n_constant_string)error_text, location, line_number);
    }
    return -1;
}

static const noble_file_entry test_file_format[]=
{
    {"being{", FIL_BEI,  0, 0,                  "Being Definition"},
    {"locat=", FIL_BEI | FILE_TYPE_BYTE2, 2, 0, "Location in x and y coordinates"}, 	/*n_byte2	x;n_byte2	y;*/
    {"facin=", FIL_BEI | FILE_TYPE_BYTE,  1, 4, "Direction facing"},    /*n_byte	facing;*/
    {"speed=", FIL_BEI | FILE_TYPE_BYTE,  1, 5, "Speed traveling"},    /*n_byte	speed;*/
    {"energ=", FIL_BEI | FILE_TYPE_BYTE2, 1, 6, "Energy within"},   /*n_byte2	energy;*/
    {"datob=", FIL_BEI | FILE_TYPE_BYTE2, 2, 8, "Date of birth in days and millenia"},    /*n_byte2	date_of_birth[2];*/
    {"rando=", FIL_BEI | FILE_TYPE_BYTE2, 2, 12,"Random within"},    /*n_byte2 seed[2];*/
    {"state=", FIL_BEI | FILE_TYPE_BYTE2, 1, 16,"State description"},    /*n_byte2	state;*/

    {"brast=", FIL_BEI | FILE_TYPE_BYTE2, 6, 18,"Brain state values"},    /*n_byte2	brain_state[6];*/

    {"heigt=", FIL_BEI | FILE_TYPE_BYTE2, 1, 30, "Height"},    /*n_byte2	height;*/
    {"masss=", FIL_BEI | FILE_TYPE_BYTE2, 1, 32, "Mass"},    /*n_byte2	mass;*/
    {"overr=", FIL_BEI | FILE_TYPE_BYTE2, 1, 34, "ApeScript overrides"},
    {{0, 0, 0, 0, 0, 0, 0},0, 0, 0, 0L}
};

static void test_pad_with_noise(n_uint random, test_being * value)
{
    n_byte2 randomizer[2];
    const n_uint sizeof_buffer = sizeof(test_being);
    n_byte * byte_pack = (n_byte *)value;
    n_uint  loop = 0;
    randomizer[0] = random & 0x0000ffff;
    randomizer[1] = (random >> 16) & 0x0000ffff;
    while(loop < sizeof_buffer)
    {
        byte_pack[loop] = math_random(randomizer) & 255;
        loop++;
    }
}

static void test_file_write(n_string file_name, n_uint random, n_byte4 * hash)
{
    test_being  check_being;
    n_file *    output_file = io_file_new();

    test_pad_with_noise(random, &check_being);

    io_write_buff(output_file, &check_being, test_file_format, FIL_BEI, 0L);

    io_disk_write(output_file, file_name);

    io_file_free(&output_file);

    if (hash)
    {
        *hash = math_hash((n_byte*)&check_being, sizeof(test_being));
    }
}

static n_int test_file_read(n_string file_name, n_byte4 * hash)
{
    test_being  check_being;
    n_file *    input_file = io_file_new();

    io_disk_read(input_file, file_name);

    io_whitespace(input_file);

    input_file->location = 0;

    if (io_read_buff(input_file, (n_byte *)&check_being, test_file_format) != FIL_BEI)
    {
        io_file_free(&input_file);

        return SHOW_ERROR("Wrong filetype found");
    }

    io_file_free(&input_file);

    if (hash)
    {
        *hash = math_hash((n_byte*)&check_being, sizeof(test_being));
    }

    return 0;
}

static void check_io(void)
{
    n_byte4 hash_write = 0;
    n_byte4 hash_read = 0;

    test_file_write("compare_file.txt", 0xf7283da, &hash_write);

    (void)test_file_read("compare_file.txt", &hash_read);

    if (hash_write != hash_read)
    {
        SHOW_ERROR("Read/write not equal");
    }

}

int main(int argc, const char * argv[])
{
    check_io();
    return 0;
}

