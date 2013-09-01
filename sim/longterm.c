/****************************************************************

	longterm.c

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


#define CONSOLE_ONLY /* Please maintain this define until after ALIFE XIII July 22nd */
#define CONSOLE_REQUIRED
#undef AUDIT_FILE

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

/* this was added to avoid a CPU fan */

#undef AUTO_LOAD_SCRIPT

enum
{
    PLOT_IDEOSPHERE = 0,
    PLOT_GENEPOOL,
    PLOT_BRAINCODE,
    PLOT_GENESPACE,
    PLOT_PREFERENCES,
    PLOT_RELATIONSHIPS,
    PLOTS
};

extern n_int nolog;
extern n_int indicator_index;
extern n_uint save_interval_steps;

noble_simulation *local_sim;
n_uint itt = 0;
n_int image_ctr = 0;
const int img_width = 256;
n_string image_ctr_file = "image_counter.txt";
#ifndef CONSOLE_ONLY
const n_uint log_genealogy=GENEALOGY_GEDCOM;
#endif
n_int simulation_file_exists = 0;

#ifdef AUDIT_FILE

static void audit_print_offset(n_byte * start, n_byte * point, char * text)
{
    printf("%s %ld\n", text, (unsigned long)(point - start));
}

static void audit_compart_offset()
{
    noble_being local;
    n_byte * start = (n_byte *)&local;
    audit_print_offset(start,(n_byte *)&(local.state),"state");
    audit_print_offset(start,(n_byte *)&(local.crowding),"crowding");
    audit_print_offset(start,(n_byte *)&(local.parasites),"parasites");
    audit_print_offset(start,(n_byte *)&(local.honor),"honor");

    audit_print_offset(start,(n_byte *)&(local.date_of_conception[0]),"date_of_conception[0]");

    audit_print_offset(start,(n_byte *)&(local.father_honor),"father_honor");
    audit_print_offset(start,(n_byte *)&(local.father_name[0]),"father_name[0]");

    audit_print_offset(start,(n_byte *)&(local.social_x),"social_x");

    audit_print_offset(start,(n_byte *)&(local.drives[0]),"drives[0]");
    audit_print_offset(start,(n_byte *)&(local.goal[0]),"goal[0]");
    audit_print_offset(start,(n_byte *)&(local.learned_preference[0]),"learned_preference[0]");
    audit_print_offset(start,(n_byte *)&(local.territory[0]),"territory[0]");
    audit_print_offset(start,(n_byte *)&(local.immune_system),"immune_system[0]");
    audit_print_offset(start,(n_byte *)&(local.brainprobe[0]),"brainprobe[0]");
}

static void audit(void)
{

    printf("sizeof(n_byte) %d\n",(int)sizeof(n_byte));
    printf("sizeof(n_byte2) %d\n",(int)sizeof(n_byte2));
    printf("sizeof(n_uint) %d\n",(int)sizeof(n_uint));

    printf("NON_PTR_BEING %d\n",(int)NON_PTR_BEING);

    printf("sizeof(n_byte	*)) %d \n", (int)sizeof(n_byte	*));

    io_audit_file(noble_file_format, FIL_VER);
    io_audit_file(noble_file_format, FIL_LAN);
    io_audit_file(noble_file_format, FIL_BEI);
    io_audit_file(noble_file_format, FIL_SOE);
    io_audit_file(noble_file_format, FIL_EPI);
    audit_compart_offset();
}

#endif

/*NOBLEMAKE END=""*/

/*NOBLEMAKE VAR=""*/

/*NOBLEMAKE END=""*/

n_int draw_error(n_constant_string error_text)
{
    printf("ERROR: %s\n",(const n_string) error_text);
    return -1;
}

int main(int argc, n_string argv[])
{
    printf("\n *** %sConsole, %s ***\n", SHORT_VERSION_NAME, FULL_DATE);
    printf("      For a list of commands type 'help'\n\n");

#ifdef AUDIT_FILE
    audit();
#endif

    local_sim = sim_sim();
    io_command_line_execution_set();

    srand((unsigned int) time(NULL) );
    sim_init(2,rand(),MAP_AREA,0);

    if (nolog==0)
    {
#ifndef CONSOLE_ONLY
#ifdef GENEALOGY_ON
        genealogy_log(local_sim,log_genealogy);
#endif
#endif
    }
    local_sim->indicators_logging=indicator_index;

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

