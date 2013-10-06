/****************************************************************

 universe_internal.h

 =============================================================

 Copyright 1996-2013 Tom Barbalet. All rights reserved.

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


/*NOBLEMAKE VAR=""*/

#ifndef NOBLEAPE_UNIVERSE_INTERNAL_H
#define NOBLEAPE_UNIVERSE_INTERNAL_H


#define USE_FIL_VER
#define USE_FIL_LAN
#define USE_FIL_BEI
#define USE_FIL_SOE
#define USE_FIL_EPI
#undef USE_FIL_WEA
#undef USE_FIL_BRA


/*	Land - References */

#undef		REDUCE_FILE

/* Time definitions */

#define GENERATIONS_BYTES (sizeof(n_byte)*4)
#define DRIVES_BYTES    (DRIVES)
#define GOALS_BYTES	(sizeof(n_byte2)*3)

#define SOCIAL_BYTES    ((SOCIAL_SIZE*sizeof(noble_social))+(2*4))

#ifdef EPISODIC_ON
#define EPISODIC_BYTES  (EPISODIC_SIZE*sizeof(noble_episodic))
#else
#define EPISODIC_BYTES  0
#endif


#ifdef BRAINCODE_ON
#define BRAINCODE_BYTES        (BRAINCODE_SIZE)
#define BRAINCODE_PROBE_BYTES  (BRAINCODE_PROBES*sizeof(noble_brain_probe))
#else
#define BRAINCODE_BYTES        0
#define BRAINCODE_PROBE_BYTES  0
#endif

#ifdef TERRITORY_ON
#define TERRITORY_BYTES        (TERRITORY_AREA*sizeof(noble_place))
#else
#define TERRITORY_BYTES        0
#endif

#ifdef PARASITES_ON
#define PARA_BYTES      2
#define PARA_ENTRIES    2
#else
#define PARA_BYTES      0
#define PARA_ENTRIES    0
#endif

#ifdef IMMUNE_ON
#define IMMUNE_BYTES    (sizeof(noble_immune_system))
#else
#define IMMUNE_BYTES    0
#endif

#ifdef METABOLISM_ON
#define VASCULAR_BYTES        (sizeof(noble_vessel))
#define HEART_RATE_BYTES      (sizeof(n_byte2))
#define BREATHING_RATE_BYTES  (sizeof(n_byte2))
#define THERMOREGULATOR_BYTES (sizeof(n_byte2))
#define METABOLISM_BYTES      (sizeof(n_byte2)*METABOLISM_SIZE)
#else
#define VASCULAR_BYTES        0
#define HEART_RATE_BYTES      0
#define BREATHING_RATE_BYTES  0
#define THERMOREGULATOR_BYTES 0
#define METABOLISM_BYTES      0
#endif



#define GENETICS_BYTES    (sizeof(n_genetics)*CHROMOSOMES)
#define GENETICS_ENTRIES  1

#define PARENT_BYTES    (9+(GENETICS_BYTES*2))

#define INVENTORY_BYTES  (INVENTORY_SIZE*sizeof(n_byte2))

/* offsets within the file */
#define INITIAL_BLOCK_BYTES	(40+INVENTORY_BYTES)
#define OFFSET_PARASITES	(INITIAL_BLOCK_BYTES+SHOUT_BYTES)

#define OFFSET_PARENTING	(OFFSET_PARASITES+PARA_BYTES)

#define OFFSET_GENETICS		(OFFSET_PARENTING+14+GENETICS_BYTES+(CHROMOSOMES*4))
#define OFFSET_SOCIAL		(OFFSET_GENETICS+GENETICS_BYTES)
#define OFFSET_EPISODIC		(OFFSET_SOCIAL+SOCIAL_BYTES)
#define OFFSET_TERRITORY	(OFFSET_EPISODIC+EPISODIC_BYTES+DRIVES_BYTES+GOALS_BYTES+PREFERENCES+GENERATIONS_BYTES)
#define OFFSET_IMMUNE		((OFFSET_GENETICS+GENETICS_BYTES+DRIVES_BYTES+6+PREFERENCES+GENERATIONS_BYTES+8)+TERRITORY_BYTES)
#define OFFSET_VASCULAR	        (OFFSET_IMMUNE+IMMUNE_BYTES)
#define OFFSET_METABOLISM       (OFFSET_VASCULAR+(VASCULAR_BYTES*VASCULAR_SIZE))

#define OFFSET_BRAINCODE	(OFFSET_METABOLISM+METABOLISM_BYTES)

#define	NON_PTR_BEING		(648)
#define	NON_PTR_LAND		(11)

#define	LARGEST_INIT_PTR	((NON_PTR_BEING > NON_PTR_LAND) ? NON_PTR_BEING : NON_PTR_LAND)

enum file_section_type
{
    FIL_VER = (0x10),
    FIL_LAN = (0x20),
    FIL_BEI = (0x30),
    FIL_SOE = (0x40),
    FIL_EPI = (0x50),
    FIL_WEA = (0x60),
    FIL_BRA = (0x70),
    FIL_END = (0x80)
};

static const noble_file_entry noble_file_format[]=
{
#ifdef USE_FIL_VER
    {{'s', 'i', 'm', 'u', 'l', '{'}, FIL_VER,  0, 0,                  "Simulation Version Definition"},
    {{'s', 'i', 'g', 'n', 'a', '='}, FIL_VER | FILE_TYPE_BYTE2, 1, 0, "Simulation signature"},
    {{'v', 'e', 'r', 'i', 'o', '='}, FIL_VER | FILE_TYPE_BYTE2, 1, 2, "Simulation version number"},
#endif
#ifdef USE_FIL_LAN
    {{'l', 'a', 'n', 'd', 'd', '{'}, FIL_LAN,  0, 0, "land definition"},
    {{'t', 'i', 'm', 'e', 'd', '='}, FIL_LAN | FILE_TYPE_BYTE2, 1, 0,  "Time in minutes"},
    {{'d', 'a', 't', 'e', 'd', '='}, FIL_LAN | FILE_TYPE_BYTE2, 2, 2,  "Date in days and millenia"},
    {{'l', 'a', 'n', 'd', 'g', '='}, FIL_LAN | FILE_TYPE_BYTE2, 2, 6,  "Seed that created the land"},
    {{'t', 'i', 'd', 'e', 's', '='}, FIL_LAN | FILE_TYPE_BYTE,  1, 10, "Tide height value"},
#endif
    /* the line above is a substantial limit to the simulation space. The weather will limit the map area to;
     ((sizeof(n_int)/2) * (MAP_AREA)/(256*256)) <= 255
     */
#ifdef USE_FIL_WEA
    {{'w', 'e', 'a', 't', 'h', '{'}, FIL_WEA,  0, 0},
    {{'p', 'r', 'e', 's', 's', '='}, FIL_WEA | FILE_TYPE_BYTE,    sizeof(n_c_int),    0},
#endif

#ifndef REDUCE_FILE /* FILE_TYPE_PACKED has a different form - no offset and the number is the size of the PACKED_DATA_BLOCK units */
    /*	{{'a', 't', 'm', 'o', 's', '='}, FIL_WEA | DONTFILE_TYPE_PACKED, ((sizeof(n_c_int) * MAP_AREA) / (PACKED_DATA_BLOCK*2)), 1},*/
#endif

#ifdef USE_FIL_BEI
    {{'b', 'e', 'i', 'n', 'g', '{'}, FIL_BEI,  0, 0,                  "Being Definition"},
    {{'l', 'o', 'c', 'a', 't', '='}, FIL_BEI | FILE_TYPE_BYTE2, 2, 0, "Location in x and y coordinates"}, 	/*n_byte2	x;n_byte2	y;*/
    {{'f', 'a', 'c', 'i', 'n', '='}, FIL_BEI | FILE_TYPE_BYTE,  1, 4, "Direction facing"},    /*n_byte	facing;*/
    {{'s', 'p', 'e', 'e', 'd', '='}, FIL_BEI | FILE_TYPE_BYTE,  1, 5, "Speed traveling"},    /*n_byte	speed;*/
    {{'e', 'n', 'e', 'r', 'g', '='}, FIL_BEI | FILE_TYPE_BYTE2, 1, 6, "Energy within"},   /*n_byte2	energy;*/
    {{'d', 'a', 't', 'o', 'b', '='}, FIL_BEI | FILE_TYPE_BYTE2, 2, 8, "Date of birth in days and millenia"},    /*n_byte2	date_of_birth[2];*/
    {{'s', 'p', 'e', 'a', 'k', '='}, FIL_BEI | FILE_TYPE_BYTE2, 1, 12,"Speaking"},    /*n_byte2	speak;*/
    {{'r', 'a', 'n', 'd', 'o', '='}, FIL_BEI | FILE_TYPE_BYTE2, 2, 14,"Random within"},    /*n_byte2 seed[2];*/
    {{'s', 't', 'a', 't', 'e', '='}, FIL_BEI | FILE_TYPE_BYTE2, 1, 18,"State description"},    /*n_byte2	state;*/

    {{'b', 'r', 'a', 'i', 'n', '='}, FIL_BEI | FILE_TYPE_BYTE2, 6, 20,"Brain state values"},    /*n_byte2	brain_state[6];*/

    {{'h', 'e', 'i', 'g', 't', '='}, FIL_BEI | FILE_TYPE_BYTE2, 1, 32, "Height"},    /*n_byte2	height;*/
    {{'m', 'a', 's', 's', 's', '='}, FIL_BEI | FILE_TYPE_BYTE2, 1, 34, "Mass"},    /*n_byte2	mass;*/
    {{'o', 'v', 'e', 'r', 'r', '='}, FIL_BEI | FILE_TYPE_BYTE2, 1, 36, "ApeScript overrides"},    /*n_byte2  script_overrides;*/
    {{'s', 'h', 'o', 'u', 't', '='}, FIL_BEI | FILE_TYPE_BYTE,  SHOUT_BYTES, 38, "Shouting values"},    /*n_byte  shout[SHOUT_BYTES];*/
    {{'c', 'r', 'o', 'w', 'd', '='}, FIL_BEI | FILE_TYPE_BYTE,  1, 44, "Crowding"},     /*n_byte	crowding;*/
    {{'p', 'o', 's', 't', 'u', '='}, FIL_BEI | FILE_TYPE_BYTE,  1, 45, "Posture"},     /*n_byte	posture;*/
    {{'i', 'n', 'v', 'e', 'n', '='}, FIL_BEI | FILE_TYPE_BYTE2,  INVENTORY_SIZE, 46, "Inventory"},     /*n_byte2	inventory[INVENTORY_SIZE];*/

#ifdef PARASITES_ON
    {{'p', 'a', 'r', 'a', 's', '='}, FIL_BEI | FILE_TYPE_BYTE, 1, 62, "Number of parasites"},     /*n_byte  parasites;*/
    {{'h', 'o', 'n', 'o', 'r', '='}, FIL_BEI | FILE_TYPE_BYTE, 1, 63, "Honor"},     /*n_byte  honor;*/
#endif
    {{'c', 'o', 'n', 'c', 'e', '='}, FIL_BEI | FILE_TYPE_BYTE2, 2, 64, "Date of conception in days and millenia"}, /*n_byte2	date_of_conception[2];*/

    {{'a', 't', 't', 'e', 'n', '='}, FIL_BEI | FILE_TYPE_BYTE, ATTENTION_SIZE, 68, "Attention group"}, /*n_byte attention[ATTENTION_SIZE];*/
    
    {{'m', 'o', 't', 'h', 'g', '='}, FIL_BEI | FILE_TYPE_BYTE2, CHROMOSOMES*2, 76, "Mother genetics"}, /*n_genetics mother_genetics[CHROMOSOMES];*/

    {{'f', 'a', 't', 'h', 'g', '='}, FIL_BEI | FILE_TYPE_BYTE2, CHROMOSOMES*2, 92, "Father genetics"}, /*n_genetics father_genetics[CHROMOSOMES];*/

    {{'g', 'e', 'n', 'e', 't', '='}, FIL_BEI | FILE_TYPE_BYTE2, CHROMOSOMES * 2, 108, "Genetics"}, /*n_genetics   genetics[CHROMOSOMES];*/

    
    {{'f', 'a', 't', 'h', 'h', '='}, FIL_BEI | FILE_TYPE_BYTE , 1, 124, "Father honor"}, /*n_byte  father_honor; */
    
    
    {{'f', 'a', 't', 'h', 'n', '='}, FIL_BEI | FILE_TYPE_BYTE , 2, 126, "Father family names"}, /*n_byte  father_name[2];*/

    
    {{'s', 'o', 's', 'i', 'm', '='}, FIL_BEI | FILE_TYPE_BYTE2, 4, 130, "Social simulation values"}, /* n_byte2 social simulation values x, y, nx, ny */
    
    {{'d', 'r', 'i', 'v', 'e', '='}, FIL_BEI | FILE_TYPE_BYTE, DRIVES, 138, "Drives"}, /*n_byte   drives[DRIVES];*/
    {{'g', 'o', 'a', 'l', 's', '='}, FIL_BEI | FILE_TYPE_BYTE2, 4, 142, "Goals"},

    {{'p', 'r', 'e', 'f', 'e', '='}, FIL_BEI | FILE_TYPE_BYTE, PREFERENCES, 150, "Preferences"},
    {{'g', 'e', 'n', 'e', 'r', '='}, FIL_BEI | FILE_TYPE_BYTE2, 4, 164, "Generations"},
#ifdef TERRITORY_ON
    {{'t', 'e', 'r', 'i', 't', '='}, FIL_BEI | FILE_TYPE_BYTE2, TERRITORY_BYTES/2, 172, "Territory information"},
#endif
#ifdef IMMUNE_ON
    {{'i', 'm', 'm', 'u', 'n', '='}, FIL_BEI | FILE_TYPE_BYTE, IMMUNE_BYTES, 428, "Immune system information"},
#endif

#ifdef BRAINCODE_ON
    {{'b', 'r', 'r', 'e', 'g', '='}, FIL_BEI | FILE_TYPE_BYTE, BRAINCODE_PSPACE_REGISTERS, 476, "Brain code register"},
    {{'b', 'r', 'p', 'r', 'o', '='}, FIL_BEI | FILE_TYPE_BYTE, (sizeof(noble_brain_probe)*BRAINCODE_PROBES), 479, "Brain code probe"},
#endif
    
#ifdef METABOLISM_ON
    {{'m', 'e', 't', 'a', 'o', '='}, FIL_BEI | FILE_TYPE_BYTE2, METABOLISM_SIZE, 576, "Metabolism"},

    /* fixed to here */
/*
    {{'v', 'a', 's', 'c', 'a', '='}, FIL_BEI | FILE_TYPE_BYTE2, VASCULAR_BYTES, OFFSET_VASCULAR, "Vascular system 1"},
    {{'v', 'a', 's', 'c', 'b', '='}, FIL_BEI | FILE_TYPE_BYTE2, VASCULAR_BYTES, OFFSET_VASCULAR+(VASCULAR_BYTES*2), "Vascular system 2"},
    {{'v', 'a', 's', 'c', 'c', '='}, FIL_BEI | FILE_TYPE_BYTE2, VASCULAR_BYTES, OFFSET_VASCULAR+(VASCULAR_BYTES*4), "Vascular system 3"},
    {{'v', 'a', 's', 'c', 'd', '='}, FIL_BEI | FILE_TYPE_BYTE2, VASCULAR_BYTES, OFFSET_VASCULAR+(VASCULAR_BYTES*6), "Vascular system 4"},
    {{'v', 'a', 's', 'c', 'e', '='}, FIL_BEI | FILE_TYPE_BYTE2, VASCULAR_BYTES, OFFSET_VASCULAR+(VASCULAR_BYTES*8), "Vascular system 5"},
    {{'v', 'a', 's', 'c', 'f', '='}, FIL_BEI | FILE_TYPE_BYTE2, VASCULAR_BYTES, OFFSET_VASCULAR+(VASCULAR_BYTES*10), "Vascular system 6"},
    {{'v', 'a', 's', 'c', 'g', '='}, FIL_BEI | FILE_TYPE_BYTE2, VASCULAR_BYTES, OFFSET_VASCULAR+(VASCULAR_BYTES*12), "Vascular system 7"},
    {{'v', 'a', 's', 'c', 'h', '='}, FIL_BEI | FILE_TYPE_BYTE2, VASCULAR_BYTES, OFFSET_VASCULAR+(VASCULAR_BYTES*14), "Vascular system 8"},
    {{'v', 'a', 's', 'c', 'i', '='}, FIL_BEI | FILE_TYPE_BYTE2, VASCULAR_BYTES, OFFSET_VASCULAR+(VASCULAR_BYTES*16), "Vascular system 9"},
    {{'v', 'a', 's', 'c', 'j', '='}, FIL_BEI | FILE_TYPE_BYTE2, VASCULAR_BYTES, OFFSET_VASCULAR+(VASCULAR_BYTES*18), "Vascular system 10"},
    {{'v', 'a', 's', 'c', 'k', '='}, FIL_BEI | FILE_TYPE_BYTE2, VASCULAR_BYTES, OFFSET_VASCULAR+(VASCULAR_BYTES*20), "Vascular system 11"},
    {{'v', 'a', 's', 'c', 'l', '='}, FIL_BEI | FILE_TYPE_BYTE2, VASCULAR_BYTES, OFFSET_VASCULAR+(VASCULAR_BYTES*22), "Vascular system 12"},
    {{'v', 'a', 's', 'c', 'm', '='}, FIL_BEI | FILE_TYPE_BYTE2, VASCULAR_BYTES, OFFSET_VASCULAR+(VASCULAR_BYTES*24), "Vascular system 13"},
    {{'v', 'a', 's', 'c', 'n', '='}, FIL_BEI | FILE_TYPE_BYTE2, VASCULAR_BYTES, OFFSET_VASCULAR+(VASCULAR_BYTES*26), "Vascular system 14"},
*/
#endif
    
#endif

#ifdef USE_FIL_SOE
    {{'s', 'g', 'c', 'i', 'a', '{'}, FIL_SOE,  0, 0, "Social graph definition"},
    {{'s', 'g', 'l', 'o', 'c', '='}, FIL_SOE | FILE_TYPE_BYTE2, 2, 0, "Location in x and y coordinates"}, /* n_byte2 location[2];*/
    {{'s', 'g', 't', 'i', 'm', '='}, FIL_SOE | FILE_TYPE_BYTE2, 1, 4, "Time in minutes"}, /* n_byte2 time;*/
    {{'s', 'g', 'd', 'a', 't', '='}, FIL_SOE | FILE_TYPE_BYTE2, 2, 6, "Date in days and millenia"}, /* n_byte2 date[2];*/
    {{'s', 'g', 'f', 'i', 'n', '='}, FIL_SOE | FILE_TYPE_BYTE2, 1, 10, "First name"},/* n_byte2 first_name;*/
    {{'s', 'g', 'f', 'a', 'n', '='}, FIL_SOE | FILE_TYPE_BYTE2, 1, 12, "Family name"},/* n_byte2 family_name;*/

    {{'s', 'g', 'a', 't', 't', '='}, FIL_SOE | FILE_TYPE_BYTE,  1, 14, "Attraction"},/* n_byte   attraction;*/
    {{'s', 'g', 'f', 'o', 'f', '='}, FIL_SOE | FILE_TYPE_BYTE,  1, 15, "Friend or foe"},/* n_byte   friend_foe;*/
    {{'s', 'g', 'b', 'e', 'l', '='}, FIL_SOE | FILE_TYPE_BYTE2, 1, 16, "Belief"},/* n_byte2  belief;*/
    {{'s', 'g', 'f', 'a', 'm', '='}, FIL_SOE | FILE_TYPE_BYTE2, 1, 18, "Familiarity"},/* n_byte2  familiarity;*/

    {{'s', 'g', 'r', 'e', 'l', '='}, FIL_SOE | FILE_TYPE_BYTE,  1, 20, "Relationship"},/* n_byte   relationship;*/
    {{'s', 'g', 'u', 'n', 'u', '='}, FIL_SOE | FILE_TYPE_BYTE,  1, 21, "Unused"},/* n_byte   relationship;*/

#ifdef BRAINCODE_ON
    {{'s', 'g', 'b', 'r', 'c', '='}, FIL_SOE | FILE_TYPE_BYTE_EXT, BRAINCODE_SIZE, 22, "Local braincode"}, /*n_byte   braincode[BRAINCODE_SIZE];*/
#endif

#endif

#ifdef USE_FIL_EPI
    {{'e', 'p', 'i', 's', 'o', '{'}, FIL_EPI,  0, 0, "Episodic memory definition"},
    {{'e', 'p', 'l', 'o', 'c', '='}, FIL_EPI | FILE_TYPE_BYTE2, 2, 0, "Location in x and y coordinates"}, /* n_byte2 location[2];*/
    {{'e', 'p', 't', 'i', 'm', '='}, FIL_EPI | FILE_TYPE_BYTE2, 1, 4, "Time in minutes"}, /* n_byte2 time;*/
    {{'e', 'p', 'd', 'a', 't', '='}, FIL_EPI | FILE_TYPE_BYTE2, 2, 6, "Date in days and millenia"}, /* n_byte2 date[2];*/
    {{'e', 'p', 'f', 'i', 'n', '='}, FIL_EPI | FILE_TYPE_BYTE2, 1, 10, "First name"},/* n_byte2 first_name;*/
    {{'e', 'p', 'f', 'a', 'n', '='}, FIL_EPI | FILE_TYPE_BYTE2, 1, 12, "Family name"},/* n_byte2 family_name;*/

    {{'e', 'p', 'e', 'v', 'e', '='}, FIL_EPI | FILE_TYPE_BYTE,  1, 14, "Event"},/* n_byte   event;*/
    {{'e', 'p', 'f', 'o', 'o', '='}, FIL_EPI | FILE_TYPE_BYTE,  1, 15, "Food"},/* n_byte   food;*/
    {{'e', 'p', 'b', 'e', 'l', '='}, FIL_EPI | FILE_TYPE_BYTE2, 1, 16, "Belief"},/* n_byte2  affect;*/
    {{'e', 'p', 'a', 'r', 'g', '='}, FIL_EPI | FILE_TYPE_BYTE2, 1, 18, "Arg"},/* n_byte2  arg;*/
#endif

#ifndef REDUCE_FILE  /* FILE_TYPE_PACKED has a different form - no offset and the number is the size of the PACKED_DATA_BLOCK units */
    /*    {{'b', 'r', 'd', 'a', 't', '='}, FIL_BRA | DONTFILE_TYPE_PACKED, DOUBLE_BRAIN/PACKED_DATA_BLOCK, 1 },*/
#endif

    {{0, 0, 0, 0, 0, 0},0, 0, 0, 0L}

};

#define	BRAIN_LOCATION(lx, ly, lz)	(BRAIN_OFFSET((lx)|((ly)<<5)|((lz)<<10)))

#define	TRACK_BRAIN(loc,lx,ly,lz)	((loc)[BRAIN_LOCATION(lx, ly, lz)])

/* functions */

n_int sketch_input(void *code, n_byte kind, n_int value);
n_int sketch_output(void * vcode, n_byte * kind, n_int * number);

void sim_end_conditions(void * code, void * structure, n_int identifier);
void sim_start_conditions(void * code, void * structure, n_int identifier);

void console_external_watch(void);

n_int console_executing(void);


#endif /* NOBLEAPE_UNIVERSE_INTERNAL_H */

/*NOBLEMAKE END=""*/
