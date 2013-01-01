/****************************************************************

	test.c

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

#include "../noble/noble.h"
#include "../universe/universe.h"
#include "../universe/internal.h"


/*NOBLEMAKE END=""*/

/*NOBLEMAKE VAR=""*/

/*NOBLEMAKE END=""*/

n_int draw_error(n_string error_text)
{
    printf("ERROR: %s\n",(const char *) error_text);
    return -1;
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

#define	TIME_SLICE	(60*24*1)

int main(int argc, char* argv[])
{
    unsigned long length = 0;
    unsigned char *data = 0L;

    unsigned long	ticker = 0;
    int result = 0;
    noble_simulation *local_sim = sim_sim();


    if (argc > 2)
    {
        sim_init(KIND_MEMORY_SETUP,0,MAP_AREA,0);

        result = plat_file_in(argv[2],0,&length);
        data = io_new(length);
        result = plat_file_in(argv[2],data,&length);
        result = sim_filein(data, length);
        io_free(data);
        sim_init(KIND_LOAD_FILE,0,MAP_AREA,0);
    }
    else
    {
        sim_init(KIND_START_UP,0,MAP_AREA,0);
    }


    while(ticker < TIME_SLICE)
    {
        sim_cycle();
        ticker ++;
        if((ticker&0x000000ff)==0)
        {
            n_uint		total_val, female_val, male_val;
            n_int		w_dx, w_dy;
            sim_populations(&total_val, &female_val, &male_val);
            weather_wind_vector(local_sim->weather, 0, 0, &w_dx, &w_dy);
            printf("%d to %d...t: %d f:%d m: %d  wind vector at (0,0) ( %d , %d )\n",(int)ticker,(int)TIME_SLICE, (int)total_val, (int)female_val, (int)male_val, (int)w_dx, (int)w_dy);
        }
    }

    data = sim_fileout(&length);

    if (argc >= 1)
    {
        result = plat_file_out(argv[1],data,length);
    }
    else
    {
        result = plat_file_out("realtime.txt",data,length);
    }

    io_free(data);

    sim_close();

    if(result == 0)
        printf("Ended Successfully\n");
    return(1);
}

