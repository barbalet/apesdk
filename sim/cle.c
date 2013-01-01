/****************************************************************

	cle.c

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
#include "universe/universe_internal.h"

#undef AUTO_LOAD_SCRIPT


/*NOBLEMAKE END=""*/

/*NOBLEMAKE VAR=""*/


/*NOBLEMAKE END=""*/

n_int draw_error(n_string error_text)
{
    printf("ERROR: %s\n",(const char *) error_text);
    return -1;
}

void plat_file_save_as(n_file * outfile)
{
    static n_byte	file_name_string[81]= {0};
    FILE * file_debug_out = 0L;

    time_t now;
    struct tm *date;
    now = time( NULL );
    date = localtime( &now );
    strftime( (char *)file_name_string, 80, "as_run_%y%m%d%H%M%S.txt", date );
    file_debug_out = fopen((char *)file_name_string,"w");
    if(file_debug_out != 0L)
    {
        fwrite(outfile->data,1,outfile->location, file_debug_out);
        fclose(file_debug_out);
    }
}

int	plat_file_out(char * file_name, n_byte * data, n_uint length)
{
    FILE  * outfile = 0L;
    int	    error_value = -1;

    outfile = fopen(file_name,"wb");
    if(outfile == 0L)
        return -1;
    if(fwrite(data,1,length,outfile) == length)
        error_value = 0;
    fclose(outfile);
    return error_value;
}

int plat_file_in(char * file_name, n_byte * data, n_uint * length)
{
    FILE	* infile = 0L;
    n_uint	  found_length = 0;
    int		  error_value = 0;
    int retval = 0;

    infile = fopen(file_name, "rb");

    if(infile == 0L)
        return -1;

    if(data == 0L)
    {
        do
        {
            n_byte	value;
            retval = fread(&value,1,1,infile);
            found_length ++;
        }
        while(!feof(infile));
        *length = found_length - 1;
    }
    else
    {
        n_uint	local_length = *length;
        if( fread(data,1,local_length,infile) != local_length)
            error_value = -1;
    }

    fclose(infile);

    return error_value;
}

n_uint	plat_variable_hash(n_byte * data, n_uint length, n_byte shift)
{
    n_uint	return_value = 0xf7462ad3;
    n_uint	loop = 0;

    while(loop<length)
    {
        n_uint	byte_data = data[loop++];
        return_value = ((return_value << shift) | (return_value >> (31-shift))) ^ byte_data;
    }
    return return_value;
}


void plat_print_offset(n_byte * start, n_byte * point, char * text)
{
    unsigned long delta = (unsigned long)(point - start);
    printf("%s %ld\n", text, delta);
}

void plat_compart_offset()
{
    noble_being local;
    n_byte * start = (n_byte *)&local;
    plat_print_offset(start,(n_byte *)&(local.state),"state");
    plat_print_offset(start,(n_byte *)&(local.crowding),"crowding");
    plat_print_offset(start,(n_byte *)&(local.parasites),"parasites");
    plat_print_offset(start,(n_byte *)&(local.honor),"honor");

    plat_print_offset(start,(n_byte *)&(local.date_of_conception[0]),"date_of_conception[0]");
    plat_print_offset(start,(n_byte *)&(local.mother_new_genetics[0]),"mother_new_genetics[0]");
    plat_print_offset(start,(n_byte *)&(local.father_new_genetics[0]),"father_new_genetics[0]");

    plat_print_offset(start,(n_byte *)&(local.father_honor),"father_honor");
    plat_print_offset(start,(n_byte *)&(local.father_name[0]),"father_name[0]");

    plat_print_offset(start,(n_byte *)&(local.new_genetics[0]),"new_genetics[0]");

    plat_print_offset(start,(n_byte *)&(local.social_x),"social_x");

    plat_print_offset(start,(n_byte *)&(local.drives[0]),"drives[0]");
    plat_print_offset(start,(n_byte *)&(local.goal[0]),"goal[0]");
    plat_print_offset(start,(n_byte *)&(local.learned_preference[0]),"learned_preference[0]");
    plat_print_offset(start,(n_byte *)&(local.territory[0]),"territory[0]");
    plat_print_offset(start,(n_byte *)&(local.immune_system),"immune_system[0]");
    plat_print_offset(start,(n_byte *)&(local.brainprobe[0]),"brainprobe[0]");

}

n_int cle_log(void * ptr, n_string response, n_console_output output_function)
{
    printf("log %s\n", response);
    return 0;
}

n_int cle_load(void * ptr, n_string response, n_console_output output_function)
{
    printf("load %s\n", response);
    return 0;
}

n_int cle_run(void * ptr, n_string response, n_console_output output_function)
{
    printf("run %s\n", response);
    return 0;
}

n_int cle_help(void * ptr, n_string response, n_console_output output_function)
{
    printf("Commands\n");
    printf(" file <something> - save output to specified file (toggles with out)\n");
    printf(" list <something> - list specified values or groups, all if not specified\n");
    printf(" log <something>- track specified values or groups (toggled)\n");
    printf(" load <something> - load the file specified\n");
    printf(" out - return output to this console (toggles with file)\n");
    printf(" run <sometime> - run simulation for specified time \n");
    printf(" quit - exit this command line\n");
    return 0;
}

n_int cle_file(void * ptr, n_string response, n_console_output output_function)
{
    printf("file %s\n", response);
    return 0;
}

n_int cle_out(void * ptr, n_string response, n_console_output output_function)
{
    printf("out %s\n", response);
    return 0;
}

noble_console_command cle_commands[] =
{
    {&cle_log,  "log"},
    {&cle_load, "load"},
    {&cle_run,  "run"},
    {&cle_help, "help"},
    {&io_quit,  "quit"},
    {&cle_file, "file"},
    {&cle_out,  "out"},
    {0L, 0L},
};


#define	TIME_SLICE	(60*24*1)


/* 1 for console */
#if 1

int main()
{
    n_int return_value = 0;
    do
    {
        return_value = io_console(0L,cle_commands, io_console_entry, io_console_out);
    }
    while(return_value == 0);

    return 1;
}

#else

int main(int argc, char* argv[])
{
    unsigned long length = 0;
    unsigned char *data = 0L;

    unsigned long	ticker = 0;
    int result = 0;
    noble_simulation *local_sim = sim_sim();
    n_file    debug_output;

    printf("---------------------------------------\n");
    io_search_file_format(noble_file_format, 0L);
    printf("---------------------------------------\n");
    io_search_file_format(noble_file_format, "being");
    printf("---------------------------------------\n");
    io_search_file_format(noble_file_format, "datob");
    printf("---------------------------------------\n");
    io_search_file_format(noble_file_format, "some?");
    printf("---------------------------------------\n");
    io_search_file_format(noble_file_format, "failure text here?");
    printf("---------------------------------------\n");

    printf("sizeof(n_byte) %d\n",(int)sizeof(n_byte));
    printf("sizeof(n_byte2) %d\n",(int)sizeof(n_byte2));
    printf("sizeof(n_uint) %d\n",(int)sizeof(n_uint));

    printf("END_BEFORE_BRAIN_PTR %d\n",(int)END_BEFORE_BRAIN_PTR);
    printf("NON_PTR_BEING %d\n",(int)NON_PTR_BEING);

    printf("sizeof(n_byte	*)) %d \n", (int)sizeof(n_byte	*));

    if (END_BEFORE_BRAIN_PTR != NON_PTR_BEING)
    {
        io_audit_file(noble_file_format, FIL_BEI);
        plat_compart_offset();
    }

    debug_output.data = io_new(2048);
    if (debug_output.data == 0L)
    {
        return (1);
    }
    debug_output.size = 2048;
    debug_output.location = 0;

    srand( time(NULL) );
    sim_init(2,rand(),MAP_AREA,0);

    if (argc > 1)
    {
        result = plat_file_in(argv[1],0,&length);
        data = io_new(length);
        result = plat_file_in(argv[1],data,&length);
        result = sim_filein(data, length);
        io_free(data);
        sim_init(0,0,MAP_AREA,0);
        if(argc == 3)
        {
            result = plat_file_in(argv[2],0,&length);
            data = io_new(length);
            result = plat_file_in(argv[2],data,&length);
            result = sim_interpret(data, length);
            io_free(data);
        }
    }

#ifdef AUTO_LOAD_SCRIPT
    result = plat_file_in(AUTO_LOAD_SCRIPT,0,&length);
    data = io_new(length);
    result = plat_file_in(AUTO_LOAD_SCRIPT,data,&length);
    result = sim_interpret(data, length);
    io_free(data);
#endif

    sim_debug_csv(&debug_output, 1);

    while(ticker < TIME_SLICE)
    {
        sim_cycle();
        ticker ++;
        if((ticker&0x0000000f)==0)
        {
            n_uint		total_val, female_val, male_val;
            n_int			w_dx, w_dy;
            sim_populations(&total_val, &female_val, &male_val);
            weather_wind_vector(local_sim->weather, 0, 0, &w_dx, &w_dy);
            printf("%d to %d...t: %d f:%d m: %d  wind vector at (0,0) ( %d , %d )\n",(int)ticker,(int)TIME_SLICE, (int)total_val, (int)female_val, (int)male_val, (int)w_dx, (int)w_dy);
        }
        sim_debug_csv(&debug_output, 0);
    }

    data = sim_fileout(&length);
    result = plat_file_out("realtime.txt",data,length);
    io_free(data);

    data = sim_fileoutXML(&length);
    result = plat_file_out("realtime.xml",data,length);
    io_free(data);

    result = plat_file_out("realtime.csv",debug_output.data,debug_output.location);

    io_free(debug_output.data);

    sim_close();
    if(result == 0)
        printf("Ended Successfully\n");
    return(1);
}

#endif
