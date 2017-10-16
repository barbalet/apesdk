/****************************************************************
 
	engine.c - Noble Warfare Skirmish

 =============================================================
 
 Copyright 1996-2011 Tom Barbalet. All rights reserved.
 
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

#include "../../gpi/gpi.h"
#include "../../noble/noble.h"
#include "battle.h"

extern void sketch_number(n_byte * stat, n_uint value, n_int py);

extern    void battle_fill(n_unit * un, n_byte2 * gvar);

/*extern    void sketch_draw(n_unit * un);*/

extern    void sketch_unit(n_unit * un, n_int offset);

/* this is the battle order with the single entry function */

extern    void battle_move(n_unit *un, n_byte2 * gvar);
extern    void battle_declare(n_unit *un, n_byte2 * gvar);
extern    void battle_attack(n_unit *un, n_byte2 * gvar);
extern    void battle_remove_dead(n_unit *un, n_byte2 * gvar);
/*
extern 	  void battle_click(n_int px, n_int py);
extern 	  void battle_drag(n_int sx, n_int sy, n_int ex, n_int ey);
*/
extern 	  void  battle_loop(battle_function func, n_unit * un,
	                                   const n_uint count , n_byte2 * gvar);

extern   n_byte battle_opponent(n_unit * un, n_uint	num);

extern   n_int  read_n_file(n_file * result);

extern	 n_int	 io_read_buff(n_file * fil, n_byte * data, const noble_file_entry * commands, n_byte * unpack_data);
extern   void io_whitespace(n_file * input);


unsigned char gpi_mouse(short px, short py);
unsigned char gpi_key(unsigned short num);
void * gpi_init(n_uint random_init);
n_int gpi_cycle(n_byte update_condition);

n_int engine_conditions();

n_byte2     game_vars[ 7 ] = { 12345, 4321, 5, 0x7fff, 0xffff, 0xffff, 5};

n_unit	   *units;
n_byte2	    number_units;
n_type     *types;
n_byte2     number_types;

#define	SIZEOF_MEMORY	 ((8*1024*1024)+L_SIZE_SCREEN)

n_byte	*memory_buffer;
n_uint  memory_allocated;
n_uint  memory_used;

extern n_byte	*board;

n_int draw_error(n_byte * error_text);

void sketch_psetb(n_int px, n_int py);

void sketch_psetw(n_int px, n_int py);

void   gpi_exit();

/* DEPRECATED */
n_int draw_error(n_byte * error_text){
	OUTPUT_DEBUG(error_text);
	return -1;
}

/* DEPRECATED */
void   gpi_exit()
{
  io_free(memory_buffer);
}

/* DEPRECATED */
static n_byte * mem_use(n_uint size) {
	n_byte * val = 0L;
	if(size > (memory_allocated - memory_used)) {
		plat_close();
	}
	val = &memory_buffer[memory_used];
	memory_used += size;
	return val;
}


#define	FIL_TYPE 0x10
#define FIL_UNIT 0x20
#define FIL_VARS 0x30

const noble_file_entry   battle_commands[]= { /* (number_commands<<3)|4 */
    {{'t', 'y', 'p', '{'}, FIL_TYPE, 0, 0}, /* n_type */
    {{'d', 'e', 'f', '='}, FIL_TYPE | FILE_TYPE_BYTE, 1, 0}, /*defence;*/
    {{'m', 'e', 'a', '='}, FIL_TYPE | FILE_TYPE_BYTE, 1, 1}, /*melee_attack;*/
    {{'m', 'e', 'd', '='}, FIL_TYPE | FILE_TYPE_BYTE, 1, 2}, /*melee_damage;*/
    {{'m', 'e', 'r', '='}, FIL_TYPE | FILE_TYPE_BYTE, 1, 3}, /*melee_armpie;*/

    {{'m', 'i', 'a', '='}, FIL_TYPE | FILE_TYPE_BYTE, 1, 4}, /*missile_attack;*/
    {{'m', 'i', 'd', '='}, FIL_TYPE | FILE_TYPE_BYTE, 1, 5}, /*missile_damage;*/
    {{'m', 'i', 'r', '='}, FIL_TYPE | FILE_TYPE_BYTE, 1, 6}, /*missile_armpie;*/
    {{'m', 'i', 't', '='}, FIL_TYPE | FILE_TYPE_BYTE, 1, 7}, /*missile_rate;*/

    {{'m', 'i', 'g', '='}, FIL_TYPE | FILE_TYPE_BYTE2, 1, 8}, /*missile_range;*/
    {{'s', 'p', 'e', '='}, FIL_TYPE | FILE_TYPE_BYTE, 1, 10}, /*speed_maximum;*/
    {{'s', 't', 'a', '='}, FIL_TYPE | FILE_TYPE_BYTE, 1, 11}, /*stature;*/
    {{'l', 'e', 'a', '='}, FIL_TYPE | FILE_TYPE_BYTE, 1, 12}, /*leadership;*/

    {{'w', 'p', 'c', '='}, FIL_TYPE | FILE_TYPE_BYTE, 1, 13}, /*wounds_per_combatant;*/
    {{'i', 'd', 'n', '='}, FIL_TYPE | FILE_TYPE_BYTE, 1, 14}, /*type ID number*/

    {{'u', 'n', 'i', '{'}, FIL_UNIT, 0, 0}, /* n_unit */

    {{'t', 'i', 'd', '='}, FIL_UNIT | FILE_TYPE_BYTE, 1, 0},  /*type (ID number)*/
    {{'a', 'n', 'g', '='}, FIL_UNIT | FILE_TYPE_BYTE, 1, 1},  /*angle;*/

    {{'a', 'v', 'x', '='}, FIL_UNIT | FILE_TYPE_BYTE2, 1, 2}, /*average_x;*/
    {{'a', 'v', 'y', '='}, FIL_UNIT | FILE_TYPE_BYTE2, 1, 4}, /*average_y;*/

    {{'w', 'i', 'd', '='}, FIL_UNIT | FILE_TYPE_BYTE2, 1, 6}, /*width;*/
    {{'n', 'u', 'm', '='}, FIL_UNIT | FILE_TYPE_BYTE2, 1, 8}, /*number_combatants;*/

    {{'a', 'l', 'i', '='}, FIL_UNIT | FILE_TYPE_BYTE, 1, 10}, /*alignment;*/
    {{'m', 'i', 'n', '='}, FIL_UNIT | FILE_TYPE_BYTE, 1, 11}, /*missile_number;*/

    {{'g', 'a', 'm', '{'}, FIL_VARS, 0, 0}, /* game_vars */
    
    {{'r', 'n', 'd', '='}, FIL_VARS | FILE_TYPE_BYTE2, 2, 0}, /*randoms;*/
    {{'v', 'a', '1', '='}, FIL_VARS | FILE_TYPE_BYTE2, 1, 4}, /*value_1;*/
    {{'v', 'a', '2', '='}, FIL_VARS | FILE_TYPE_BYTE2, 1, 6}, /*value_2;*/
    {{'v', 'a', '3', '='}, FIL_VARS | FILE_TYPE_BYTE2, 1, 8}, /*value_3;*/
    {{'v', 'a', '4', '='}, FIL_VARS | FILE_TYPE_BYTE2, 1, 10}, /*value_4;*/
    {{'v', 'a', '5', '='}, FIL_VARS | FILE_TYPE_BYTE2, 1, 12}, /*value_5;*/

    {{0,0,0,0}, 0, 0, 0} /* end commands */
};


static n_byte	engine_filein(n_file * file_pass, n_byte val) {
	n_byte	temp_store[sizeof(n_unit)+sizeof(n_type)]={0};
	n_int   ret_val = io_read_buff(file_pass, temp_store, battle_commands, 0L);
	if(ret_val == FILE_EOF) {
		return 6;
	}

	if(val == ret_val) {
		if(val == FIL_TYPE) { /* type */
			io_copy(temp_store, (n_byte*)&types[number_types++], sizeof(n_type));
			(void)mem_use(sizeof(n_type));
		}
		if(val == FIL_UNIT) { /* unit */
			io_copy(temp_store, (n_byte *)&units[number_units++], sizeof(n_unit));
			(void)mem_use(sizeof(n_unit));
		}
		if(val == FIL_VARS) { /* game_vars */
			io_copy(temp_store, (n_byte *)game_vars, ( sizeof(n_byte2) * 7 ) );
    }
	}
	return 0;
}

unsigned char gpi_key(unsigned short num) {return 0;}


/* DEPRECATED */
n_int engine_conditions() {
	n_byte    		ret_val;
	n_file 	file_pass;

	number_units = 0;
	number_types = 0;

	memory_used = 0;

	board = (n_byte *)mem_use(L_SIZE_SCREEN);

	file_pass.size = L_SIZE_SCREEN >> 2;
	file_pass.location = 0;
	file_pass.data = (n_byte *)memory_buffer;

	if(read_n_file(&file_pass) != 0) {
		return SHOW_ERROR("Read file failed");
	}
	
	io_whitespace(&file_pass);
  
  /* load the types first */
	types = (n_type *) mem_use(0);
	do {
		ret_val = engine_filein(&file_pass, FIL_TYPE);
	} while(ret_val == 0);

	/* back to the start of the file */
	file_pass.location = 0;

	/* load the units next */
	units = (n_unit *) mem_use(0);

	do {
		ret_val = engine_filein(&file_pass, FIL_UNIT);
	} while(ret_val == 0);

	if((number_types == 0) || (number_units == 0) || (number_types > 255)) {
		SHOW_ERROR("Type/Unit Logic Failed");
	}

	/* load the game variables last */
  /* back to the start of the file */
  file_pass.location = 0;
  
  do {
    ret_val = engine_filein(&file_pass, FIL_VARS);
  } while(ret_val == 0);
        
  /* resolve the units with types and check the alignments */
	{
		n_byte	resolve[256] = {0};
		n_uint	check_alignment[2] = {0};
		n_byte	loop = 0;
		while(loop < number_types) {
			resolve[types[loop].points_per_combatant] = loop;
			loop++;
		}
		loop = 0;
		while(loop < number_units) {
			n_byte2	local_combatants = units[loop].number_combatants;
			units[loop].unit_type = &types[resolve[units[loop].morale]];
			units[loop].morale = 255;
			units[loop].number_living = local_combatants;
			units[loop].combatants = (n_combatant *)mem_use(sizeof(n_combatant)*local_combatants);
			check_alignment[ (units[loop].alignment) & 1 ]++;
			loop++;
		}

		/* if there are none of one of the alignments, there can be no battle */
		if((check_alignment[0] == 0) || (check_alignment[1] == 0)) {
			SHOW_ERROR("Alignment Logic Failed");
		}
	}

	plat_erase(board);

	/* get the drawing ready, fill the units with spaced combatants and draw it all */

	battle_loop(&battle_fill, units, number_units, 0L);
	gpi_cycle(1);
	return 0;
}

void * gpi_init(n_uint random_init) {

	game_vars[ GVAR_RANDOM_0 ] = (n_byte2) (random_init & 0xFFFF);
	game_vars[ GVAR_RANDOM_1 ] = (n_byte2) (random_init >> 16);

	memory_allocated = SIZEOF_MEMORY;
	memory_buffer = io_new_range(512*784, &memory_allocated);

	if (memory_buffer == 0L)
		return 0L;

	memory_used = 0;
	
	if(engine_conditions() != 0)
		return 0L;

	return ((void *) memory_buffer);
}

n_byte sm_down = 0, sm_last = 0;
n_int startx = 0, starty = 0, endx = 0, endy = 0;
/*
static void engine_drag() {}
*/

unsigned char gpi_mouse(short px, short py) {
	sm_down = 1;

	if(sm_last == 0) {
		startx = px;
		starty = py;
		endx = px;
		endy = py;
	} else {
		endx = px;
		endy = py;
	}
	return 1;
}


n_int gpi_cycle(n_byte update_condition) {
	if(update_condition == 1){
		n_byte	result = battle_opponent(units, number_units);

		if(result != 0){
			if(engine_conditions() != 0){
				return SHOW_ERROR("Update conditions failed");
			}
		}
		battle_loop(&battle_declare,    units, number_units, game_vars);
		battle_loop(&battle_move,       units, number_units, game_vars);
		battle_loop(&battle_attack,     units, number_units, game_vars);
		battle_loop(&battle_remove_dead,units, number_units, 0L);
	}
	return 0;
}


