/****************************************************************
 
 test_io.c
 
 =============================================================
 
 Copyright 1996-2014 Tom Barbalet. All rights reserved.
 
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
    n_byte4     date_of_birth;
    n_byte2     seed[2];
    n_byte2     macro_state;
    n_byte2     brain_state[6];
    n_byte2     height;
    n_byte2     mass;
    n_byte2     script_overrides;
}test_being;

#define FIL_BEI (0x30)

static const noble_file_entry test_file_format[]=
{
    {"being{", FIL_BEI,  0, 0,                  "Being Definition"},
    {"locat=", FIL_BEI | FILE_TYPE_BYTE2, 2, 0, "Location in x and y coordinates"}, 	/*n_byte2	x;n_byte2	y;*/
    {"facin=", FIL_BEI | FILE_TYPE_BYTE,  1, 4, "Direction facing"},    /*n_byte	facing;*/
    {"speed=", FIL_BEI | FILE_TYPE_BYTE,  1, 5, "Speed traveling"},    /*n_byte	speed;*/
    {"energ=", FIL_BEI | FILE_TYPE_BYTE2, 1, 6, "Energy within"},   /*n_byte2	energy;*/
    {"datob=", FIL_BEI | FILE_TYPE_BYTE4, 1, 8, "Date of birth in days and millenia"},    /*n_byte4	date_of_birth;*/
    {"rando=", FIL_BEI | FILE_TYPE_BYTE2, 2, 12,"Random within"},    /*n_byte2 seed[2];*/
    {"state=", FIL_BEI | FILE_TYPE_BYTE2, 1, 16,"State description"},    /*n_byte2	state;*/
    
    {"brast=", FIL_BEI | FILE_TYPE_BYTE2, 6, 18,"Brain state values"},    /*n_byte2	brain_state[6];*/
    
    {"heigt=", FIL_BEI | FILE_TYPE_BYTE2, 1, 30, "Height"},    /*n_byte2	height;*/
    {"masss=", FIL_BEI | FILE_TYPE_BYTE2, 1, 32, "Mass"},    /*n_byte2	mass;*/
    {"overr=", FIL_BEI | FILE_TYPE_BYTE2, 1, 34, "ApeScript overrides"},
    {{0, 0, 0, 0, 0, 0, 0},0, 0, 0, 0L}
};


typedef struct test_type {
    n_byte2  defence;
    n_byte2  melee_attack;
    n_byte2  melee_damage;
    n_byte2  melee_armpie;
    
    n_byte2  missile_attack;
    n_byte2  missile_damage;
    n_byte2  missile_armpie;
    n_byte2  missile_rate;
    
    n_byte2  missile_range;
    n_byte2  speed_maximum;
    n_byte2  stature;
    n_byte2  leadership;
    
    n_byte2  wounds_per_combatant;
    n_byte2  points_per_combatant;
}
test_type;

typedef struct test_unit {
    n_byte2  morale;
    n_byte2  angle;
    
    n_byte2  average[2];
    
    n_byte2  width;
    n_byte2  number_combatants;
    
    n_byte2  alignment;
    n_byte2  missile_number;
 /*
    n_byte2  missile_timer;
    n_byte2  number_living;

    void *unit_type;
    void *combatants;
    void *unit_attacking;
 */
}
test_unit;

static n_byte2  game_vars[ 7 ] = { 12345, 4321, 5, 8000, 0xffff, 0xffff, 5 };


#define NW_TYPE  (0x10)
#define NW_UNIT  (0x20)
#define NW_GAME  (0x30)

const noble_file_entry   battle_commands[]= { /* (number_commands<<3)|4 */
    {"ntype{", NW_TYPE | 0,              0, 0,  "Unit Type"},    /* n_type */
    {"defen=", NW_TYPE | FILE_TYPE_BYTE2, 1, 0,  "Defense Value"},/*defence;*/
    {"melat=", NW_TYPE | FILE_TYPE_BYTE2, 1, 2,  "Melee Attack"}, /*melee_attack;*/
    {"melda=", NW_TYPE | FILE_TYPE_BYTE2, 1, 4,  "Melee Damage"}, /*melee_damage;*/
    {"melap=", NW_TYPE | FILE_TYPE_BYTE2, 1, 6,  "Melee Armor Pierce"}, /*melee_armpie;*/
    
    {"misat=", NW_TYPE | FILE_TYPE_BYTE2, 1, 8,  "Missile Attack"},/*missile_attack;*/
    {"misda=", NW_TYPE | FILE_TYPE_BYTE2, 1, 10,  "Missile Damage"},/*missile_damage;*/
    {"misap=", NW_TYPE | FILE_TYPE_BYTE2, 1, 12,  "Missile Armor Pierce"},/*missile_armpie;*/
    {"misrt=", NW_TYPE | FILE_TYPE_BYTE2, 1, 14,  "Missile Rate"},/*missile_rate;*/
    
    {"misrg=", NW_TYPE | FILE_TYPE_BYTE2, 1, 16,  "Missile Range"}, /*missile_range;*/
    {"speed=", NW_TYPE | FILE_TYPE_BYTE2, 1, 18, "Speed Maximum"}, /*speed_maximum;*/
    {"statu=", NW_TYPE | FILE_TYPE_BYTE2, 1, 20, "Stature"},/*stature;*/
    {"leade=", NW_TYPE | FILE_TYPE_BYTE2, 1, 22, "Leadership"}, /*leadership;*/
    
    {"weapc=", NW_TYPE | FILE_TYPE_BYTE2, 1, 24,  "Wounds per combatant"},/*wounds_per_combatant;*/
    {"tyidn=", NW_TYPE | FILE_TYPE_BYTE2, 1, 26,  "Type ID number"}, /*type ID number*/
    
    {"nunit{", NW_UNIT | 0, 0, 0, "Specific Unit"},/* n_unit */
    
    {"tidun=", NW_UNIT | FILE_TYPE_BYTE2, 1, 0,  "Type (ID number)"}, /*type (ID number)*/
    {"angle=", NW_UNIT | FILE_TYPE_BYTE2, 1, 2,  "Angle"}, /*angle;*/
    
    {"averx=", NW_UNIT | FILE_TYPE_BYTE2, 1, 4, "Average X"}, /*average_x;*/
    {"avery=", NW_UNIT | FILE_TYPE_BYTE2, 1, 6, "Average Y"}, /*average_y;*/
    
    {"width=", NW_UNIT | FILE_TYPE_BYTE2, 1, 8, "Width"}, /*width;*/
    {"numco=", NW_UNIT | FILE_TYPE_BYTE2, 1, 10, "Number Combatants"},/*number_combatants;*/
    
    {"align=", NW_UNIT | FILE_TYPE_BYTE2, 1, 12, "Alignment"},/*alignment;*/
    {"misnu=", NW_UNIT | FILE_TYPE_BYTE2, 1, 14, "Missile Number"},/*missile_number;*/
    
    {"ngame{", NW_GAME | 0, 0, 0, "Game Variables"},/* game_vars */
    
    {"randv=", NW_GAME | FILE_TYPE_BYTE2, 2, 0,   "Random values"},/*randoms;*/
    {"vari1=", NW_GAME | FILE_TYPE_BYTE2, 1, 4,   "Value 1"}, /*value_1;*/
    {"vari2=", NW_GAME | FILE_TYPE_BYTE2, 1, 6,   "Value 2"}, /*value_2;*/
    {"vari3=", NW_GAME | FILE_TYPE_BYTE2, 1, 8,   "Value 3"}, /*value_3;*/
    {"vari4=", NW_GAME | FILE_TYPE_BYTE2, 1, 10,  "Value 4"}, /*value_4;*/
    {"vari5=", NW_GAME | FILE_TYPE_BYTE2, 1, 12,  "Value 5"}, /*value_5;*/
    
    {{0, 0, 0, 0, 0, 0},0, 0, 0} /* end commands */
};


n_int draw_error(n_constant_string error_text, n_constant_string location, n_int line_number)
{
    if (error_text)
    {
        printf("ERROR: %s @ %s %ld\n", (n_constant_string)error_text, location, line_number);
    }
    return -1;
}

static void test_pad_with_noise(n_uint random, n_byte * byte_pack, n_uint sizeof_buffer)
{
    n_byte2 randomizer[2];
    n_uint  loop = 0;
    randomizer[0] = random & 0x0000ffff;
    randomizer[1] = (random >> 16) & 0x0000ffff;
    while(loop < sizeof_buffer)
    {
        byte_pack[loop] = math_random(randomizer) & 255;
        loop++;
    }
}


static void check_io_war_type(n_uint random)
{
    test_type   check_one;
    test_type   check_two;
    n_file *     output_file = io_file_new();
    n_file *     input_file = io_file_new();
    
    test_pad_with_noise(random, (n_byte*)&check_one, sizeof(test_type));
    
    io_write_buff(output_file, &check_one, battle_commands, NW_TYPE, 0L);
    
    io_disk_write(output_file, "compare_file_type.txt");
    
    io_file_free(&output_file);
    
    io_disk_read(input_file, "compare_file_type.txt");
    
    io_whitespace(input_file);
        
    if (io_read_buff(input_file, (n_byte *)&check_two, battle_commands) != NW_TYPE)
    {
        SHOW_ERROR("Wrong filetype found");
    }
    
    if (math_hash((n_byte*)&check_one, sizeof(test_type)) !=
        math_hash((n_byte*)&check_two, sizeof(test_type)))
    {
        SHOW_ERROR("Read/write not equal");
    }
    io_file_free(&input_file);
}

static void check_io_war_unit(n_uint random)
{
    test_unit    check_one;
    test_unit    check_two;
    n_file *     output_file = io_file_new();
    n_file *     input_file = io_file_new();
    
    test_pad_with_noise(random, (n_byte*)&check_one, sizeof(test_unit));
    
    io_write_buff(output_file, &check_one, battle_commands, NW_UNIT, 0L);
    
    io_disk_write(output_file, "compare_file_unit.txt");
    
    io_file_free(&output_file);
    
    io_disk_read(input_file, "compare_file_unit.txt");
    
    io_whitespace(input_file);
    
    if (io_read_buff(input_file, (n_byte *)&check_two, battle_commands) != NW_UNIT)
    {
        SHOW_ERROR("Wrong filetype found");
    }
    
    if (math_hash((n_byte*)&check_one, sizeof(test_unit)) !=
        math_hash((n_byte*)&check_two, sizeof(test_unit)))
    {
        n_file * check_file = io_file_new();
        
        io_write_buff(check_file, &check_two, battle_commands, NW_UNIT, 0L);

        io_output_contents(check_file);
        io_output_contents(input_file);
        
        SHOW_ERROR("Read/write not equal");
        
        io_file_free(&check_file);

    }
    io_file_free(&input_file);

}


static void check_io(n_uint random)
{
    test_being   check_one;
    test_being   check_two;
    n_file *     output_file = io_file_new();
    n_file *     input_file = io_file_new();
    
    test_pad_with_noise(random, (n_byte*)&check_one, sizeof(test_being));
    
    io_write_buff(output_file, &check_one, test_file_format, FIL_BEI, 0L);
    
    io_disk_write(output_file, "compare_file.txt");
    
    io_file_free(&output_file);
    
    io_disk_read(input_file, "compare_file.txt");
    
    io_whitespace(input_file);
    
    if (io_read_buff(input_file, (n_byte *)&check_two, test_file_format) != FIL_BEI)
    {
        SHOW_ERROR("Wrong filetype found");
    }
    
    if (math_hash((n_byte*)&check_one, sizeof(test_being)) !=
        math_hash((n_byte*)&check_two, sizeof(test_being)))
    {
        SHOW_ERROR("Read/write not equal");
    }
    io_file_free(&input_file);
}

int main(int argc, const char * argv[])
{
    check_io_war_unit(0xf7283da);
    check_io_war_type(0xf7283da);
    
    check_io(0x12345678);
    check_io(0x87654321);
    /* this final check is compared in a file diff */
    check_io(0xf7283da);
    return 0;
}

