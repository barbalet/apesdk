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

#define GENERATIONS_BYTES (sizeof(n_uint)*GENERATION_TOTAL)
#define DRIVES_BYTES    (DRIVES)
#define GOALS_BYTES	(sizeof(n_byte2)*3)

#define SOCIAL_BYTES    ((SOCIAL_SIZE*sizeof(social_link))+(2*4))

#ifdef EPISODIC_ON
#define EPISODIC_BYTES  (EPISODIC_SIZE*sizeof(episodic_memory))
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

#define	NON_PTR_BEING		(sizeof(noble_being))
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
    {"simul{", FIL_VER,  0, 0,                  "Simulation Version Definition"},
    {"signa=", FIL_VER | FILE_TYPE_BYTE2, 1, 0, "Simulation signature"},
    {"verio=", FIL_VER | FILE_TYPE_BYTE2, 1, 2, "Simulation version number"},
#endif
#ifdef USE_FIL_LAN
    {"landd{", FIL_LAN,  0, 0, "land definition"},
    {"timed=", FIL_LAN | FILE_TYPE_BYTE2, 1, 0,  "Time in minutes"},
    {"dated=", FIL_LAN | FILE_TYPE_BYTE2, 2, 2,  "Date in days and millenia"},
    {"landg=", FIL_LAN | FILE_TYPE_BYTE2, 2, 6,  "Seed that created the land"},
    {"tides=", FIL_LAN | FILE_TYPE_BYTE,  1, 10, "Tide height value"},
#endif
    /* the line above is a substantial limit to the simulation space. The weather will limit the map area to;
     ((sizeof(n_int)/2) * (MAP_AREA)/(256*256)) <= 255
     */
#ifdef USE_FIL_WEA
    {"weath{", FIL_WEA,  0, 0},
    {"press=", FIL_WEA | FILE_TYPE_BYTE,    sizeof(n_c_int),    0},
#endif

#ifndef REDUCE_FILE /* FILE_TYPE_PACKED has a different form - no offset and the number is the size of the PACKED_DATA_BLOCK units */
    /*	{{'a', 't', 'm', 'o', 's', '='}, FIL_WEA | DONTFILE_TYPE_PACKED, ((sizeof(n_c_int) * MAP_AREA) / (PACKED_DATA_BLOCK*2)), 1},*/
#endif

#ifdef USE_FIL_BEI
    {"being{", FIL_BEI,  0, 0,                  "Being Definition"},
    {"locat=", FIL_BEI | FILE_TYPE_BYTE2, 2, 0, "Location in x and y coordinates"}, 	/*n_byte2	x;n_byte2	y;*/
    {"facin=", FIL_BEI | FILE_TYPE_BYTE,  1, 4, "Direction facing"},    /*n_byte	facing;*/
    {"speed=", FIL_BEI | FILE_TYPE_BYTE,  1, 5, "Speed traveling"},    /*n_byte	speed;*/
    {"energ=", FIL_BEI | FILE_TYPE_BYTE2, 1, 6, "Energy within"},   /*n_byte2	energy;*/
    {"datob=", FIL_BEI | FILE_TYPE_BYTE2, 2, 8, "Date of birth in days and millenia"},    /*n_byte2	date_of_birth[2];*/
    {"speak=", FIL_BEI | FILE_TYPE_BYTE2, 1, 12,"Speaking"},    /*n_byte2	speak;*/
    {"rando=", FIL_BEI | FILE_TYPE_BYTE2, 2, 14,"Random within"},    /*n_byte2 seed[2];*/
    {"state=", FIL_BEI | FILE_TYPE_BYTE2, 1, 18,"State description"},    /*n_byte2	state;*/

    {"brain=", FIL_BEI | FILE_TYPE_BYTE2, 6, 20,"Brain state values"},    /*n_byte2	brain_state[6];*/

    {"heigt=", FIL_BEI | FILE_TYPE_BYTE2, 1, 32, "Height"},    /*n_byte2	height;*/
    {"masss=", FIL_BEI | FILE_TYPE_BYTE2, 1, 34, "Mass"},    /*n_byte2	mass;*/
    {"overr=", FIL_BEI | FILE_TYPE_BYTE2, 1, 36, "ApeScript overrides"},    /*n_byte2  script_overrides;*/
    {"shout=", FIL_BEI | FILE_TYPE_BYTE,  SHOUT_BYTES, 38, "Shouting values"},    /*n_byte  shout[SHOUT_BYTES];*/
    {"crowd=", FIL_BEI | FILE_TYPE_BYTE,  1, 38+SHOUT_BYTES, "Crowding"},     /*n_byte	crowding;*/
    {"postu=", FIL_BEI | FILE_TYPE_BYTE,  1, 39+SHOUT_BYTES, "Posture"},     /*n_byte	posture;*/
    {"inven=", FIL_BEI | FILE_TYPE_BYTE2, INVENTORY_SIZE, 40+SHOUT_BYTES, "Inventory"},     /*n_byte2	inventory[INVENTORY_SIZE];*/

#ifdef PARASITES_ON
    {"paras=", FIL_BEI | FILE_TYPE_BYTE, 1, OFFSET_PARASITES, "Number of parasites"},     /*n_byte  parasites;*/
    {"honor=", FIL_BEI | FILE_TYPE_BYTE, 1, (OFFSET_PARASITES+1), "Honor"},     /*n_byte  honor;*/
#endif
    {"conce=", FIL_BEI | FILE_TYPE_BYTE2, 2, OFFSET_PARENTING, "Date of conception in days and millenia"}, /*n_byte2	date_of_conception[2];*/

    {"atten=", FIL_BEI | FILE_TYPE_BYTE, ATTENTION_SIZE, OFFSET_PARENTING+4, "Attention group"}, /*n_byte attention[ATTENTION_SIZE];*/

    {"mothg=", FIL_BEI | FILE_TYPE_BYTE2, CHROMOSOMES*2, (OFFSET_PARENTING+4+ATTENTION_SIZE), "Mother genetics"}, /*n_genetics mother_genetics[CHROMOSOMES];*/

    {"famil=", FIL_BEI | FILE_TYPE_BYTE,  2, (OFFSET_PARENTING+6+GENETICS_BYTES), "Family names"}, /*n_byte  family[2];*/

    {"fathg=", FIL_BEI | FILE_TYPE_BYTE2, CHROMOSOMES*2, (OFFSET_PARENTING+10+GENETICS_BYTES), "Father genetics"}, /*n_byte2 father_genetics[CHROMOSOMES];*/

    {"fathh=", FIL_BEI | FILE_TYPE_BYTE , 1, (OFFSET_PARENTING+10+GENETICS_BYTES+(CHROMOSOMES*4)), "Father honor"}, /*n_byte  father_honor; */
    {"fathn=", FIL_BEI | FILE_TYPE_BYTE , 2, (OFFSET_PARENTING+11+GENETICS_BYTES+(CHROMOSOMES*4)), "Father family names"}, /*n_byte  father_name[2];*/

    {"genet=", FIL_BEI | FILE_TYPE_BYTE2, CHROMOSOMES * 2, OFFSET_GENETICS, "Genetics"}, /*n_int   genetics[CHROMOSOMES];*/

    {"sosim=", FIL_BEI | FILE_TYPE_BYTE2, 4, (OFFSET_GENETICS+GENETICS_BYTES), "Social simulation values"}, /* n_byte2 social simulation values x, y, nx, ny */


    {"drive=", FIL_BEI | FILE_TYPE_BYTE, DRIVES, (OFFSET_GENETICS+GENETICS_BYTES+8), "Drives"}, /*n_byte   drives[DRIVES];*/
    {"goals=", FIL_BEI | FILE_TYPE_BYTE2, 3, (OFFSET_GENETICS+GENETICS_BYTES+DRIVES_BYTES+8), "Goals"},

    {"prefe=", FIL_BEI | FILE_TYPE_BYTE, PREFERENCES, (OFFSET_GENETICS+GENETICS_BYTES+DRIVES_BYTES+6+8), "Preferences"},
    {"gener=", FIL_BEI | FILE_TYPE_BYTE, GENERATIONS_BYTES, (OFFSET_GENETICS+GENETICS_BYTES+DRIVES_BYTES+6+8+PREFERENCES), "Generations"},
#ifdef TERRITORY_ON
    {"terit=", FIL_BEI | FILE_TYPE_BYTE2, TERRITORY_BYTES/2, (OFFSET_GENETICS+GENETICS_BYTES+DRIVES_BYTES+6+PREFERENCES+GENERATIONS_BYTES+8), "Territory information"},
#endif
#ifdef IMMUNE_ON
    {"immun=", FIL_BEI | FILE_TYPE_BYTE, IMMUNE_BYTES, OFFSET_IMMUNE, "Immune system information"},
#endif
#ifdef METABOLISM_ON
    {"vas01=", FIL_BEI | FILE_TYPE_BYTE, VASCULAR_BYTES*2, OFFSET_VASCULAR, "Vascular system 1"},
    {"vas02=", FIL_BEI | FILE_TYPE_BYTE, VASCULAR_BYTES*2, OFFSET_VASCULAR+(VASCULAR_BYTES*2), "Vascular system 2"},
    {"vas03=", FIL_BEI | FILE_TYPE_BYTE, VASCULAR_BYTES*2, OFFSET_VASCULAR+(VASCULAR_BYTES*4), "Vascular system 3"},
    {"vas04=", FIL_BEI | FILE_TYPE_BYTE, VASCULAR_BYTES*2, OFFSET_VASCULAR+(VASCULAR_BYTES*6), "Vascular system 4"},
    {"vas05=", FIL_BEI | FILE_TYPE_BYTE, VASCULAR_BYTES*2, OFFSET_VASCULAR+(VASCULAR_BYTES*8), "Vascular system 5"},
    {"vas06=", FIL_BEI | FILE_TYPE_BYTE, VASCULAR_BYTES*2, OFFSET_VASCULAR+(VASCULAR_BYTES*10), "Vascular system 6"},
    {"vas07=", FIL_BEI | FILE_TYPE_BYTE, VASCULAR_BYTES*2, OFFSET_VASCULAR+(VASCULAR_BYTES*12), "Vascular system 7"},
    {"vas08=", FIL_BEI | FILE_TYPE_BYTE, VASCULAR_BYTES*2, OFFSET_VASCULAR+(VASCULAR_BYTES*14), "Vascular system 8"},
    {"vas09=", FIL_BEI | FILE_TYPE_BYTE, VASCULAR_BYTES*2, OFFSET_VASCULAR+(VASCULAR_BYTES*16), "Vascular system 9"},
    {"vas10=", FIL_BEI | FILE_TYPE_BYTE, VASCULAR_BYTES*2, OFFSET_VASCULAR+(VASCULAR_BYTES*18), "Vascular system 10"},
    {"vas11=", FIL_BEI | FILE_TYPE_BYTE, VASCULAR_BYTES*2, OFFSET_VASCULAR+(VASCULAR_BYTES*20), "Vascular system 11"},
    {"vas12=", FIL_BEI | FILE_TYPE_BYTE, VASCULAR_BYTES*2, OFFSET_VASCULAR+(VASCULAR_BYTES*22), "Vascular system 12"},
    {"vas13=", FIL_BEI | FILE_TYPE_BYTE, VASCULAR_BYTES*2, OFFSET_VASCULAR+(VASCULAR_BYTES*24), "Vascular system 13"},
    {"vas14=", FIL_BEI | FILE_TYPE_BYTE, VASCULAR_BYTES*2, OFFSET_VASCULAR+(VASCULAR_BYTES*26), "Vascular system 14"},
    {"metab=", FIL_BEI | FILE_TYPE_BYTE2, METABOLISM_SIZE, OFFSET_METABOLISM, "Metabolism"},
#endif
#ifdef BRAINCODE_ON
    {"brreg=", FIL_BEI | FILE_TYPE_BYTE, BRAINCODE_PSPACE_REGISTERS, OFFSET_BRAINCODE, "Brain code register"},
    {"brpro=", FIL_BEI | FILE_TYPE_BYTE, (sizeof(noble_brain_probe)*BRAINCODE_PROBES), (OFFSET_BRAINCODE+BRAINCODE_PSPACE_REGISTERS), "Brain code probe"},
#endif
#endif

#ifdef USE_FIL_SOE
    {"sgcia{", FIL_SOE,  0, 0, "Social graph definition"},
    {"sgloc=", FIL_SOE | FILE_TYPE_BYTE2, 2, 0, "Location in x and y coordinates"}, /* n_byte2 location[2];*/
    {"sgtim=", FIL_SOE | FILE_TYPE_BYTE2, 1, 4, "Time in minutes"}, /* n_byte2 time;*/
    {"sgdat=", FIL_SOE | FILE_TYPE_BYTE2, 2, 6, "Date in days and millenia"}, /* n_byte2 date[2];*/
    {"sgfin=", FIL_SOE | FILE_TYPE_BYTE2, 1, 10, "First name"},/* n_byte2 first_name;*/
    {"sgfan=", FIL_SOE | FILE_TYPE_BYTE2, 1, 12, "Family name"},/* n_byte2 family_name;*/

    {"sgatt=", FIL_SOE | FILE_TYPE_BYTE,  1, 14, "Attraction"},/* n_byte   attraction;*/
    {"sgfof=", FIL_SOE | FILE_TYPE_BYTE,  1, 15, "Friend or foe"},/* n_byte   friend_foe;*/
    {"sgbel=", FIL_SOE | FILE_TYPE_BYTE2, 1, 16, "Belief"},/* n_byte2  belief;*/
    {"sgfam=", FIL_SOE | FILE_TYPE_BYTE2, 1, 18, "Familiarity"},/* n_byte2  familiarity;*/

    {"sgrel=", FIL_SOE | FILE_TYPE_BYTE,  1, 20, "Relationship"},/* n_byte   relationship;*/

#ifdef BRAINCODE_ON
    {"sgbrc=", FIL_SOE | FILE_TYPE_BYTE_EXT, BRAINCODE_SIZE, 21, "Local braincode"}, /*n_byte   braincode[BRAINCODE_SIZE];*/
#endif

#endif

#ifdef USE_FIL_EPI
    {"episo{", FIL_EPI,  0, 0, "Episodic memory definition"},
    {"eploc=", FIL_EPI | FILE_TYPE_BYTE2, 2, 0,  "Location in x and y coordinates"}, /* n_byte2 location[2];*/
    {"eptim=", FIL_EPI | FILE_TYPE_BYTE2, 1, 4,  "Time in minutes"}, /* n_byte2 time;*/
    {"epdat=", FIL_EPI | FILE_TYPE_BYTE2, 2, 6,  "Date in days and millenia"}, /* n_byte2 date[2];*/
    {"epfin=", FIL_EPI | FILE_TYPE_BYTE2, 1, 10, "First name"},/* n_byte2 first_name;*/
    {"epfan=", FIL_EPI | FILE_TYPE_BYTE2, 1, 12, "Family name"},/* n_byte2 family_name;*/
    {"epeve=", FIL_EPI | FILE_TYPE_BYTE,  1, 14, "Event"},/* n_byte   event;*/
    {"epfoo=", FIL_EPI | FILE_TYPE_BYTE,  1, 15, "Food"},/* n_byte   food;*/
    {"epbel=", FIL_EPI | FILE_TYPE_BYTE2, 1, 16, "Belief"},/* n_byte2  affect;*/
    {"eparg=", FIL_EPI | FILE_TYPE_BYTE2, 1, 18, "Arg"},/* n_byte2  arg;*/

#endif

    {{0, 0, 0, 0, 0, 0, 0},0, 0, 0, 0L}

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
