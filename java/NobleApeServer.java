/****************************************************************

	NobleApeServer.js

	=============================================================

 Copyright 1996-2018 Tom Barbalet. All rights reserved.

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


#define CONSOLE_ONLY /* Please maintain this define until after ALIFE XIII July 22nd */
#define CONSOLE_REQUIRED
#undef  AUDIT_FILE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

/*NOBLEMAKE DIR=""*/
/*NOBLEMAKE DIR="noble/"*/
/*NOBLEMAKE SET="noble.h"*/
/*NOBLEMAKE DIR=""*/
/*NOBLEMAKE DIR="universe/"*/
/*NOBLEMAKE SET="universe.h"*/
/*NOBLEMAKE DIR=""*/


/*NOBLEMAKE DEL=""*/

#include "noble/noble.h"
#include "universe/universe.h"
#ifdef AUDIT_FILE
#include "universe/universe_internal.h"
#endif

n_string_block simulation_filename;

noble_simulation *local_sim;

#ifdef AUDIT_FILE

static void audit_print_offset(n_byte * start, n_byte * point, char * text)
{
    printf("%s %ld\n", text, (unsigned long)(point - start));
}

static void audit_compart_offset()
{
    noble_being local;
    n_byte * start = (n_byte *)&local;
    audit_print_offset(start,(n_byte *)&(local.macro_state),"macro_state");
    audit_print_offset(start,(n_byte *)&(local.crowding),"crowding");
    audit_print_offset(start,(n_byte *)&(local.parasites),"parasites");
    audit_print_offset(start,(n_byte *)&(local.honor),"honor");

    audit_print_offset(start,(n_byte *)&(local.date_of_conception[0]),"date_of_conception[0]");

    audit_print_offset(start,(n_byte *)&(local.fetal_genetics[0]),"fetal_genetics[0]");
    audit_print_offset(start,(n_byte *)&(local.genetics[0]),"genetics[0]");

    audit_print_offset(start,(n_byte *)&(local.father_name[0]),"father_name[0]");

    audit_print_offset(start,(n_byte *)&(local.social_x),"social_x");

    audit_print_offset(start,(n_byte *)&(local.drives[0]),"drives[0]");
    audit_print_offset(start,(n_byte *)&(local.goal[0]),"goal[0]");
    audit_print_offset(start,(n_byte *)&(local.learned_preference[0]),"learned_preference[0]");

    audit_print_offset(start,(n_byte *)&(local.generation_min),"generation_min");

    audit_print_offset(start,(n_byte *)&(local.territory[0]),"territory[0]");
    audit_print_offset(start,(n_byte *)&(local.immune_system),"immune_system[0]");

    audit_print_offset(start,(n_byte *)&(local.vessel[0]),"vessel[0]");
    audit_print_offset(start,(n_byte *)&(local.metabolism[0]),"metabolism[0]");

    audit_print_offset(start,(n_byte *)&(local.braincode_register[0]),"braincode_register[0]");
    audit_print_offset(start,(n_byte *)&(local.brainprobe[0]),"brainprobe[0]");

    audit_print_offset(start,(n_byte *)&(local.vessel[0]),"vessel[0]");
    audit_print_offset(start,(n_byte *)&(local.vessel[1]),"vessel[1]");
    audit_print_offset(start,(n_byte *)&(local.vessel[2]),"vessel[2]");
    audit_print_offset(start,(n_byte *)&(local.vessel[3]),"vessel[3]");
    audit_print_offset(start,(n_byte *)&(local.vessel[4]),"vessel[4]");
    audit_print_offset(start,(n_byte *)&(local.vessel[5]),"vessel[5]");
    audit_print_offset(start,(n_byte *)&(local.vessel[6]),"vessel[6]");
    audit_print_offset(start,(n_byte *)&(local.vessel[7]),"vessel[7]");
    audit_print_offset(start,(n_byte *)&(local.vessel[8]),"vessel[8]");
    audit_print_offset(start,(n_byte *)&(local.vessel[9]),"vessel[9]");
    audit_print_offset(start,(n_byte *)&(local.vessel[10]),"vessel[10]");
    audit_print_offset(start,(n_byte *)&(local.vessel[11]),"vessel[11]");
    audit_print_offset(start,(n_byte *)&(local.vessel[12]),"vessel[12]");
    audit_print_offset(start,(n_byte *)&(local.vessel[13]),"vessel[13]");
    audit_print_offset(start,(n_byte *)&(local.vessel[14]),"vessel[14]");
    audit_print_offset(start,(n_byte *)&(local.vessel[15]),"vessel[15]");
    audit_print_offset(start,(n_byte *)&(local.vessel[16]),"vessel[16]");
    audit_print_offset(start,(n_byte *)&(local.vessel[17]),"vessel[17]");
    audit_print_offset(start,(n_byte *)&(local.vessel[18]),"vessel[18]");
    audit_print_offset(start,(n_byte *)&(local.vessel[19]),"vessel[19]");
    audit_print_offset(start,(n_byte *)&(local.vessel[20]),"vessel[20]");
    audit_print_offset(start,(n_byte *)&(local.vessel[21]),"vessel[21]");
    audit_print_offset(start,(n_byte *)&(local.vessel[22]),"vessel[22]");
    audit_print_offset(start,(n_byte *)&(local.vessel[23]),"vessel[23]");
    audit_print_offset(start,(n_byte *)&(local.vessel[24]),"vessel[24]");
    audit_print_offset(start,(n_byte *)&(local.vessel[25]),"vessel[25]");
    audit_print_offset(start,(n_byte *)&(local.vessel[26]),"vessel[26]");
    audit_print_offset(start,(n_byte *)&(local.vessel[27]),"vessel[27]");

    audit_print_offset(start,(n_byte *)&(local.brain),"brain");
}

static void audit(void)
{

    printf("sizeof(n_byte) %d\n",(int)sizeof(n_byte));
    printf("sizeof(n_byte2) %d\n",(int)sizeof(n_byte2));
    printf("sizeof(n_uint) %d\n",(int)sizeof(n_uint));

    printf("sizeof(n_byte	*)) %d \n", (int)sizeof(n_byte	*));
    /*
    io_audit_file(noble_file_format, FIL_VER);
    io_audit_file(noble_file_format, FIL_LAN); */
    io_audit_file(noble_file_format, FIL_BEI); /*
    io_audit_file(noble_file_format, FIL_SOE);
    io_audit_file(noble_file_format, FIL_EPI); */
    audit_compart_offset();
}

#endif

/*NOBLEMAKE END=""*/

/*NOBLEMAKE VAR=""*/

/*NOBLEMAKE END=""*/

n_int draw_error(n_constant_string error_text, n_constant_string location, n_int line_number)
{
    printf("ERROR: %s @ %s %ld\n",(const n_string) error_text, location, line_number);
    return -1;
}

/* moved to console.c with minor modifications */

/* load simulation data */
static n_int cle_load(void * ptr, n_string response, n_console_output output_function)
{
    if (io_disk_check(response)!=0)
    {
        console_open(ptr, response, output_function);
        sprintf(simulation_filename,"%s",response);
        printf("Simulation file %s loaded\n",response);
    }
    return 0;
}

int main(int argc, n_string argv[])
{

    printf("\n *** %sConsole, %s ***\n", SHORT_VERSION_NAME, FULL_DATE);
    printf("      For a list of commands type 'help'\n\n");

    sprintf(simulation_filename,"%s","realtime.txt");

#ifdef AUDIT_FILE
    audit();
#endif

    local_sim = sim_sim();
    io_command_line_execution_set();

    srand((unsigned int) time(NULL) );
    sim_init(2,rand(),MAP_AREA,0);

    cle_load(local_sim, (n_string)simulation_filename, io_console_out);

#ifndef	_WIN32
    do
    {
        sim_thread_console();
    }
    while (sim_thread_console_quit() == 0);
#else
    {
        n_int return_value = 0;
        do
        {
            return_value = io_console(local_sim,
                                      (noble_console_command *)control_commands,
                                      io_console_entry,
                                      io_console_out);
        }
        while (return_value == 0);
    }
#endif

    sim_close();

    return(1);
}

