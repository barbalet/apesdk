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
#undef SAVE_IMAGES
#define CONSOLE_REQUIRED
#undef AUDIT_FILE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#ifdef SAVE_IMAGES
#include <zlib.h>
#include "contrib/motters/pnglite.h"
#endif

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
#include "command/command.h"

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
n_string_block simulation_filename;

noble_simulation *local_sim;
n_uint itt = 0;
n_int image_ctr = 0;
const int img_width = 256;
n_string image_ctr_file = "image_counter.txt";
#ifndef CONSOLE_ONLY
const n_uint log_genealogy=GENEALOGY_GEDCOM;
#endif
n_int simulation_file_exists = 0;

#ifdef SAVE_IMAGES_REMOVE /* This is out of place here Bob, please think of another way to do this */
n_byte *  watched_ape_braincode = 0L;
n_int watched_ape_braincode_height = 60*24;
n_int watched_ape_braincode_width = 60*24;
#endif

#ifdef SAVE_IMAGES_REMOVE

extern n_byte * get_braincode_from_console(n_int * local_bc_height, n_int *local_bc_width);

#endif

#ifdef SAVE_IMAGES
n_string plot_filename[] =
{
    "ideosphere%06d.png",
    "genepool%06d.png",
    "braincode%06d.png",
    "genespace%06d.png",
    "preferences%06d.png",
    "relationships%06d.png"
};

n_string watched_ape_braincode_filename = "temporal_braincode.png";

static int write_png_file(n_string filename, int width, int height, unsigned char *buffer)
{
    png_t png;
    FILE * fp = fopen(filename, "wb");
    if (fp == NULL)
    {
        fprintf(stderr, "Could not open file %s for writing\n", filename);
        return 1;
    }
    fclose(fp);

    png_init(0,0);
    png_open_file_write(&png, filename);
    png_set_data(&png, width, height, 8, PNG_TRUECOLOR, buffer);
    png_close_file(&png);

    return 0;
}

static void plot(n_string filename, n_int img_width, n_int img_height, n_byte plot_type)
{
    n_byte * buffer;

    buffer = (n_byte*)io_new(img_width*img_height*3);
    switch (plot_type)
    {
    case PLOT_IDEOSPHERE:
        graph_ideosphere(local_sim, buffer, img_width, img_height);
        break;
    case PLOT_GENEPOOL:
        graph_genepool(local_sim, buffer, img_width, img_height);
        break;
    case PLOT_BRAINCODE:
        graph_phasespace(local_sim, buffer, img_width, img_height,1,0);
        break;
    case PLOT_GENESPACE:
        graph_phasespace(local_sim, buffer, img_width, img_height,1,1);
        break;
    case PLOT_PREFERENCES:
        graph_preferences(local_sim, buffer, img_width, img_height);
        break;
    case PLOT_RELATIONSHIPS:
        graph_relationship_matrix(local_sim, buffer, img_width, img_height);
        break;
    }
    write_png_file(filename, img_width, img_height, buffer);
    io_free((void*)buffer);
}

#endif

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
    audit_print_offset(start,(n_byte *)&(local.mother_new_genetics[0]),"mother_new_genetics[0]");
    audit_print_offset(start,(n_byte *)&(local.father_new_genetics[0]),"father_new_genetics[0]");
    
    audit_print_offset(start,(n_byte *)&(local.father_honor),"father_honor");
    audit_print_offset(start,(n_byte *)&(local.father_name[0]),"father_name[0]");
    
    audit_print_offset(start,(n_byte *)&(local.new_genetics[0]),"new_genetics[0]");
    
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

n_int draw_error(n_string error_text)
{
    printf("ERROR: %s\n",(const n_string) error_text);
    return -1;
}

static void plat_file_save_as(n_file * outfile)
{
    static n_byte	file_name_string[81]= {0};
    FILE * file_debug_out = 0L;

    time_t now;
    struct tm *date;
    now = time( NULL );
    date = localtime( &now );
    strftime( (n_string)file_name_string, 80, "as_run_%y%m%d%H%M%S.txt", date );
    file_debug_out = fopen((n_string)file_name_string,"w");
    if(file_debug_out != 0L)
    {
        fwrite(outfile->data,1,outfile->location, file_debug_out);
        fclose(file_debug_out);
    }
}

static int	plat_file_out(n_string file_name, n_byte * data, n_uint length)
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

static int plat_file_in(n_string file_name, n_byte * data, n_uint * length)
{
    FILE	* infile = 0L;
    n_uint	  found_length = 0;
    int		  error_value = 0;

    infile = fopen(file_name, "rb");

    if(infile == 0L)
        return -1;

    if(data == 0L)
    {
        do
        {
            n_byte	value;
            if (fread(&value,1,1,infile) > 0)
            {
                found_length ++;
            }
        }
        while(!feof(infile));
        *length = found_length;
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

static n_int clear_files()
{
    n_int i = 0, deleted = 0;
#ifndef	_WIN32
    n_string remove_command = "rm";
#else
    n_string remove_command = "del";
#endif
    n_string_block str;
    n_string files[] =
    {
        "*.csv",
        "*.ged",
        "ideosphere*.png",
        "genepool*.png",
        "braincode*.png",
        "genespace*.png",
        "preferences*.png",
        "relationships*.png",
        "image_counter.txt",
        "realtime.txt",
        ""
    };

    while (io_length(files[i],STRING_BLOCK_SIZE)>0)
    {
        sprintf(str,"%s %s", remove_command, files[i]);
        if (system(str))
        {
            deleted++;
        }
        i++;
    }
    return deleted;
}

#ifdef SAVE_IMAGES_REMOVE

n_byte * get_braincode_from_console(n_int * local_bc_height, n_int *local_bc_width)
{
    *  local_bc_height = watched_ape_braincode_height;
    *  local_bc_width  = watched_ape_braincode_width;

    return watched_ape_braincode;
}

#endif

/* moved to console.c with major modifications */

static n_int cle_step(void * ptr, n_string response, n_console_output output_function)
{
#ifdef SAVE_IMAGES
    n_int i;
    FILE *fp;
#endif
    unsigned long length = 0;
    unsigned char *data = 0L;
    n_int done = 0;

    while (done == 0)
    {
        sim_cycle();
        watch_ape(local_sim, output_function);

#ifdef SAVE_IMAGES_REMOVE /* This is out of place here Bob, please think of another way to do this */

        /* update an image of the braincode for the watched ape */
        if (watched_ape_braincode==0)
        {
            watched_ape_braincode = (n_byte*)io_new(watched_ape_braincode_width*watched_ape_braincode_height*3);
        }
        graph_braincode(sim_sim(), local_being, watched_ape_braincode, watched_ape_braincode_width, watched_ape_braincode_height, 0);

        /* clear previous text */

        /*
         Need to think of a non printf way of doing this

         for (i = 0; i < watch_string_length; i++) printf("\b");
         fflush(stdout);
         */
#endif

        if (itt%(save_interval_steps)==0)
        {
            if (simulation_file_exists!=0)
            {
                simulation_file_exists = 0;
            }
            else
            {
                if (nolog==0)
                {
#ifndef CONSOLE_ONLY
#ifdef GENEALOGY_ON
                    sprintf((n_string)filename, "%s", "genealogy.ged");
                    genealogy_save(local_sim, (n_string)filename);
#endif
#endif
#ifdef SAVE_IMAGES
                    for (i = 0; i < PLOTS; i++)
                    {
                        sprintf((n_string)filename, plot_filename[i], image_ctr);
                        plot(filename, img_width, img_width, i);
                    }
                    image_ctr++;
#endif

                    /* save the simulation state */
                    data = sim_fileout(&length);
                    (void) plat_file_out(simulation_filename, data, length);
                    io_free(data);

#ifdef SAVE_IMAGES_REMOVE /* This needs to be reconnected out of the console code, Bob */

                    if (local_sim->select != NO_BEINGS_FOUND)
                    {
                        /* save image counter */
                        fp = fopen(image_ctr_file, "w");
                        if (fp!=0)
                        {
                            fprintf(fp,"%d", (int)image_ctr);
                            fclose(fp);
                        }
                        {
                            n_int  local_bc_height;
                            n_int  local_bc_width;
                            n_byte * local_bc = get_braincode_from_console(&local_bc_height, &local_bc_width);


                            /* save braincode image for the watched ape */
                            if (local_bc!=0L)
                            {
                                write_png_file(watched_ape_braincode_filename, local_bc_width, local_bc_height, local_bc);
                            }
                        }
                    }
#endif
                }
                done = 1;
            }
        }
        if (local_sim->num == 0)
        {
            printf("*** %d %d %d \n", local_sim->land->date[1], local_sim->land->date[0], local_sim->land->time);

            sim_init(1, rand(), MAP_AREA, 0);
#ifndef CONSOLE_ONLY
#ifdef GENEALOGY_ON
            genealogy_log(local_sim, log_genealogy);
#endif
#endif
            indicator_index++;
            local_sim->indicators_logging = indicator_index;
            itt=0;
        }
        itt++;
    }
    return 0;
}


/* moved to console.c with minor modifications */

static n_int cle_run(void * ptr, n_string response, n_console_output output_function)
{
    n_uint i,run=0;
    n_int number=0, interval=INTERVAL_DAYS;

    if (response != 0)
    {
        if (io_length(response, STRING_BLOCK_SIZE) > 0)
        {
            if (get_time_interval(response, &number, &interval) > -1)
            {
                if (number > 0)
                {
                    printf("Running for %d %s", (int)number, interval_description[interval]);
                    fflush(stdout);

                    for (i = 0; i < (number * interval_steps[interval]) / save_interval_steps; i++)
                    {
                        cle_step(ptr, 0, output_function);
                        printf(".");
                        fflush(stdout);
                    }
                    printf("\n");

                    run = 1;
                }
            }
        }
    }

    if (run == 0)
    {
        (void)SHOW_ERROR("Time not specified, examples: run 2 days, run 6 hours");
        return 0;
    }

    return 0;
}

/* moved to console.c with minor modifications */

/* load simulation data */
static n_int cle_load(void * ptr, n_string response, n_console_output output_function)
{
    n_uint length = 0;
    unsigned char *data = 0L;
    FILE * fp;
    char image_ctr_str[10];

    if (response==0) return 0;

    simulation_file_exists = 0;

    if (io_disk_check(response)!=0)
    {
        (void)plat_file_in(response,0,&length);
        data = io_new(length);
        (void)plat_file_in(response,data,&length);
        (void)sim_filein(data, length);
        io_free(data);
        simulation_file_exists = 1;
        sprintf(simulation_filename,"%s",response);
        printf("Simulation file %s loaded\n",response);

        /* read the image counter */
        fp = fopen(image_ctr_file,"r");
        if (fp!=0)
        {
            if (fgets (image_ctr_str , 10 , fp) != 0)
            {
                image_ctr = atoi(image_ctr_str);
            }
            fclose(fp);
        }
    }
    return 0;
}

/* load apescript file */
static n_int cle_script(void * ptr, n_string response, n_console_output output_function)
{
    unsigned long length = 0;
    unsigned char *data = 0L;

    if (response==0) return 0;

    if (io_disk_check(response)!=0)
    {
        (void)plat_file_in(response,0,&length);
        data = io_new(length);
        (void)plat_file_in(response,data,&length);
        (void)sim_interpret(data, length);
        io_free(data);
        printf("Apescript file %s loaded\n",response);
    }
    return 0;
}

static n_int cle_reset(void * ptr, n_string response, n_console_output output_function)
{
    clear_files();
    itt = 0;
    image_ctr = 0;
    indicator_index = 1;
    (void)console_reset(ptr, response, output_function);
    if (nolog==0)
    {
#ifndef CONSOLE_ONLY
#ifdef GENEALOGY_ON
        genealogy_log(local_sim,log_genealogy);
#endif
#endif
    }
    local_sim->indicators_logging=indicator_index;
    return 0;
}

static n_int cle_video(void * ptr, n_string response, n_console_output output_function)
{
    n_int i=0,j=0;
    n_string_block video_type_str;
    n_string_block picture_filename;
    n_string_block video_filename;
    n_string_block command_str;

    /* get the video type */
    while (i < io_length(response, STRING_BLOCK_SIZE))
    {
        if (response[i] != ' ')
        {
            video_type_str[i] = response[i];
        }
        else
        {
            if (i > 1) break;
        }
        i++;
    }
    video_type_str[i++] = '\0';

    /* get the picture name */
    picture_filename[0]='\0';
    if (io_find(video_type_str,0,io_length(video_type_str,STRING_BLOCK_SIZE),"ideosphere",10)>-1)
    {
        sprintf(picture_filename,"%s","ideosphere%06d");
    }
    if (io_find(video_type_str,0,io_length(video_type_str,STRING_BLOCK_SIZE),"genepool",8)>-1)
    {
        sprintf(picture_filename,"%s","genepool%06d");
    }
    if (io_find(video_type_str,0,io_length(video_type_str,STRING_BLOCK_SIZE),"genespace",9)>-1)
    {
        sprintf(picture_filename,"%s","genespace%06d");
    }
    if (io_find(video_type_str,0,io_length(video_type_str,STRING_BLOCK_SIZE),"pref",4)>-1)
    {
        sprintf(picture_filename,"%s","preferences%06d");
    }
    if (io_find(video_type_str,0,io_length(video_type_str,STRING_BLOCK_SIZE),"relation",8)>-1)
    {
        sprintf(picture_filename,"%s","relationships%06d");
    }

    if (picture_filename[0]=='\0')
    {
        output_function("No video type specified");
        return 0;
    }

    /* get the video filename */
    while (i < io_length(response, STRING_BLOCK_SIZE))
    {
        if ((response[i]!=10) && (response[i]!=13))
        {
            video_filename[j++] = response[i];
        }
        i++;
    }
    video_filename[j] = '\0';

    if (io_length(video_filename,STRING_BLOCK_SIZE)<2)
    {
        output_function("No video filename specified");
        return 0;
    }

    /* run the command */
    sprintf(command_str,"ffmpeg -r 5 -i %s.png %s",
            picture_filename, video_filename);

    if (system(command_str)==0)
    {
        printf("%s\n", command_str);
    }

    return 0;
}

static n_int longterm_quit(void * ptr, n_string response, n_console_output output_function)
{
#ifdef SAVE_IMAGES_REMOVE /* This is out of place, Bob */
    if (watched_ape_braincode != 0L)
    {
        io_free(watched_ape_braincode);
    }
#endif
    return io_quit(ptr, response, output_function);
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

    if (nolog==0)
    {
#ifndef CONSOLE_ONLY
#ifdef GENEALOGY_ON
        genealogy_log(local_sim,log_genealogy);
#endif
#endif
    }
    local_sim->indicators_logging=indicator_index;
    
    cle_load(local_sim, (n_string)simulation_filename, io_console_out);

#ifndef	_WIN32
    do{
        sim_thread_console();
    }while (sim_thread_console_quit() == 0);
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

