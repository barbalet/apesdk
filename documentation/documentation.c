/****************************************************************

 documentation.c

 =============================================================

 Copyright 1996-2020 Tom Barbalet. All rights reserved.

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

 This software is a continuing work of Tom Barbalet, begun on
 13 June 1996. No apes or cats were harmed in the writing of
 this software.

 ****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define CONSOLE_REQUIRED
#define CONSOLE_ONLY
#define GUI_COMMAND_LINE_HYBRID

#ifndef	_WIN32

#include "../noble/noble.h"
#include "../universe/universe.h"
#include "../universe/universe_internal.h"
#include "../entity/entity.h"
#include "../entity/entity_internal.h"

#else

#include "..\noble\noble.h"
#include "..\universe\universe.h"
#include "..\universe\universe_internal.h"
#include "..\entity\entity.h"
#include "..\entity\entity_internal.h"

#endif


/* This is a cheating method initially but we need a fast converter upfront */

#define FILE_STRING_SIZE	1024
#define	NUMBER_OF_FILES		8

char	list_of_files[NUMBER_OF_FILES][FILE_STRING_SIZE]=
{
    "apescript_intro",
    "apescript_notes",
    "apescript_sim",
    "file",
    "index",
    "legal",
    "philosophic",
    "start"
};

n_int draw_error(n_constant_string error_text, n_constant_string location, n_int line_number)
{
    printf("ERROR: %s @ %s %ld\n",(const n_string) error_text, location, line_number);
    return -1;
}

void audit_file_apescripterror(FILE * html_write)
{
    n_constant_string  local_error_string = apescript_errors[0].error_string;
    n_constant_string  local_help_string  = apescript_errors[0].help_string;

    n_int   loop = 0;

    fprintf(html_write, "<CENTER>\n");
    fprintf(html_write, "<TABLE WIDTH=90%%>\n");

    fprintf(html_write, "<TD ALIGN=LEFT VALIGN=TOP WIDTH=40%% BGCOLOR=\"#eeeeee\">\n");
    fprintf(html_write, "<FONT FACE=\"Courier, Courier New\" SIZE=4>\n");
    fprintf(html_write, "<B>Error</B>\n");

    fprintf(html_write, "<TD ALIGN=LEFT VALIGN=TOP BGCOLOR=\"#eeeeee\">\n");
    fprintf(html_write, "<FONT FACE=\"Courier, Courier New\" SIZE=4>\n");
    fprintf(html_write, "<B>Help Text</B>\n");

    fprintf(html_write, "<TR>\n");


    do
    {

        local_error_string = apescript_errors[loop].error_string;
        local_help_string  = apescript_errors[loop].help_string;

        if ((local_error_string != 0L) && (local_help_string != 0L))
        {
            fprintf(html_write, "<TD ALIGN=LEFT VALIGN=TOP BGCOLOR=\"#eeeeee\">\n");
            fprintf(html_write, "<FONT FACE=\"Courier, Courier New\" SIZE=4>\n");
            fprintf(html_write, "%s\n",local_error_string);

            fprintf(html_write, "<TD ALIGN=LEFT VALIGN=TOP BGCOLOR=\"#eeeeee\">\n");
            fprintf(html_write, "<FONT FACE=\"Courier, Courier New\" SIZE=4>\n");
            fprintf(html_write, "%s\n",local_help_string);


            fprintf(html_write, "<TR>\n");
        }
        if (local_error_string != 0L)
        {
            loop++;
        }
    }
    while((local_error_string != 0L) && (local_help_string != 0L));

    fprintf(html_write, "</TABLE>\n");
    fprintf(html_write, "</CENTER>\n");
}


void audit_file_console(FILE * html_write)
{
    n_console * local_function = control_commands[0].function;
    n_string    local_command  = control_commands[0].command;

    n_int   loop = 0;

    fprintf(html_write, "<CENTER>\n");
    fprintf(html_write, "<TABLE WIDTH=90%%>\n");

    fprintf(html_write, "<TD ALIGN=LEFT VALIGN=TOP BGCOLOR=\"#eeeeee\">\n");
    fprintf(html_write, "<FONT FACE=\"Courier, Courier New\" SIZE=4>\n");
    fprintf(html_write, "<B>Command</B>\n");

    fprintf(html_write, "<TD ALIGN=LEFT VALIGN=TOP BGCOLOR=\"#eeeeee\">\n");
    fprintf(html_write, "<FONT FACE=\"Courier, Courier New\" SIZE=4>\n");
    fprintf(html_write, "<B>Addition</B>\n");

    fprintf(html_write, "<TD ALIGN=LEFT VALIGN=TOP BGCOLOR=\"#eeeeee\">\n");
    fprintf(html_write, "<FONT FACE=\"Courier, Courier New\" SIZE=4>\n");
    fprintf(html_write, "<B>Help Information</B>\n");

    fprintf(html_write, "<TR>\n");


    do
    {

        n_string    local_addition = control_commands[loop].addition;
        n_string    local_help_inf = control_commands[loop].help_information;

        local_function = control_commands[loop].function;
        local_command  = control_commands[loop].command;

        if ((local_command != 0L) && (local_help_inf != 0L))
        {
            if (local_help_inf[0] != 0)
            {
                fprintf(html_write, "<TD ALIGN=LEFT VALIGN=TOP BGCOLOR=\"#eeeeee\">\n");
                fprintf(html_write, "<FONT FACE=\"Courier, Courier New\" SIZE=4>\n");
                fprintf(html_write, "<B>%s</B>\n",local_command);

                fprintf(html_write, "<TD ALIGN=LEFT VALIGN=TOP BGCOLOR=\"#eeeeee\">\n");
                fprintf(html_write, "<FONT FACE=\"Courier, Courier New\" SIZE=4>\n");
                fprintf(html_write, "%s\n",local_addition);

                fprintf(html_write, "<TD ALIGN=LEFT VALIGN=TOP BGCOLOR=\"#eeeeee\">\n");
                fprintf(html_write, "<FONT FACE=\"Courier, Courier New\" SIZE=4>\n");
                fprintf(html_write, "<I>%s</I>\n",local_help_inf);

                fprintf(html_write, "<TR>\n");
            }
        }
        if (local_function != 0L)
        {
            loop++;
        }
    }
    while((local_function != 0L) && (local_command != 0L));

    fprintf(html_write, "</TABLE>\n");
    fprintf(html_write, "</CENTER>\n");
}

void audit_file_io(FILE * html_write)
{
    unsigned long loop = 0;
    n_byte     *local_characters = (n_byte*)noble_file_format[0].characters;
    do
    {
        n_byte   local_incl_kind  = noble_file_format[loop].incl_kind;
        n_byte   local_number     = noble_file_format[loop].number_entries;
        n_byte2  local_location   = noble_file_format[loop].start_location;
        n_string local_what_is_it = noble_file_format[loop].what_is_it;

        n_byte   printout_characters[7] = {0};
        n_byte	 file_kind = (local_incl_kind & 0xF0);
        n_byte   local_type = local_incl_kind & 0x0F;

        local_characters = (n_byte*)noble_file_format[loop].characters;

        printout_characters[0] = local_characters[0];
        printout_characters[1] = local_characters[1];
        printout_characters[2] = local_characters[2];
        printout_characters[3] = local_characters[3];
        printout_characters[4] = local_characters[4];
        printout_characters[5] = 0;

        if (local_type == 0)
        {
            if (local_what_is_it)
            {
                fprintf(html_write,"<HR><B>%s</B>, ",local_what_is_it);
                fprintf(html_write, "<B>%s</B><P>\n",printout_characters);
            }
        }
        else
        {
            unsigned long	loop2 = 0;

            fprintf(html_write,"<P><B>%s</B>, ",local_what_is_it);

            fprintf(html_write, "%s<BR>\n",printout_characters);

            if (local_type == FILE_TYPE_BYTE)
            {
                fprintf(html_write, "%d x one_byte<P>", local_number);
            }
            if (local_type == FILE_TYPE_BYTE2)
            {
                fprintf(html_write, "%d x two_bytes<P>", local_number);
            }
            if (local_type == FILE_TYPE_BYTE_EXT)
            {
                fprintf(html_write, "%d x brain_code<P>", local_number);
            }

            fprintf(html_write, "<CENTER>\n");
            fprintf(html_write, "<TABLE WIDTH=90%%>\n");
            fprintf(html_write, "<TD ALIGN=LEFT VALIGN=TOP BGCOLOR=\"#eeeeee\">\n");
            fprintf(html_write, "<FONT FACE=\"Courier, Courier New\" SIZE=4>\n");

            fprintf(html_write, "&nbsp;&nbsp;&nbsp;%s = ",printout_characters);
            while (loop2 < local_number)
            {
                if (local_type == FILE_TYPE_BYTE2)
                {
                    fprintf(html_write,"%d", rand() & 0xffff);
                }
                else
                {
                    fprintf(html_write,"%d", rand() & 0x00ff);

                }

                loop2++;
                if (loop2 == local_number)
                {
                    fprintf(html_write,";<BR>\n");
                }
                else
                {
                    fprintf(html_write,", ");

                }
            }
            fprintf(html_write, "</TABLE></CENTER>\n");

        }

        loop++;

    }
    while((local_characters[0] != 0) && (local_characters[1] != 0) &&
            (local_characters[2] != 0) && (local_characters[3] != 0));

    fprintf(html_write,"<HR>\n");

}

void  process_from_source(FILE * html_write, char val1, char val2, char val3)
{
    if ((val1 == 'V') && (val2 == 'E') && (val3 == 'R'))
    {
        fprintf(html_write,"%s %s",SHORT_VERSION_NAME, FULL_DATE);
    }
    if ((val1 == 'C') && (val2 == 'O') && (val3 == 'P'))
    {
        fprintf(html_write,"%s",FULL_VERSION_COPYRIGHT);
    }
    if ((val1 == 'F') && (val2 == 'I') && (val3 == 'L'))
    {
        audit_file_io(html_write);
    }
    if ((val1 == 'C') && (val2 == 'O') && (val3 == 'N'))
    {
        audit_file_console(html_write);
    }
    if ((val1 == 'A') && (val2 == 'E') && (val3 == 'R'))
    {
        audit_file_apescripterror(html_write);
    }
}

n_int convert_txt_to_html(char * text_name, char * html_name)
{
    FILE	*text_file = fopen(text_name, "rb");
    FILE	*html_file = fopen(html_name, "wb");
    char	file_char;
    char	addition_string[FILE_STRING_SIZE];
    n_uint	addition_length;

    if (text_file == 0L)
    {
        printf("Text file: %s failed to open\n",text_name);
    }
    if (html_file == 0L)
    {
        printf("HTML file: %s failed to open\n",html_name);
    }
    if ((text_file == 0L) || (html_file == 0L))
    {
        fclose(text_file);
        fclose(html_file);
        return -1;
    }
    do
    {
        fread(&file_char,1,1,text_file);
        if (file_char == '$')
        {
            addition_length = 0;
            do
            {
                fread(&file_char,1,1,text_file);
                if (file_char == '$')
                {
                    addition_string[addition_length++] = 0;
                }
                else
                {
                    addition_string[addition_length++] = file_char;
                }
            }
            while (((file_char != '$') && (addition_length < 2048)) && (!feof(text_file)));

            if (file_char == '$')
            {
                process_from_source(html_file, addition_string[0],addition_string[1],addition_string[2]);
            }
            else
            {
                printf("*** Second $ not found in %s! ***\n", text_name);
                break;
            }
        }
        if (file_char == '@')
            printf("*** @ found ***\n");
        if (file_char == '~')
            printf("*** ~ found ***\n");
        if (file_char == '<')
            printf("*** < found ***\n");
        if (file_char == '>')
            printf("*** > found ***\n");
        if (file_char == '[')
            file_char = '<';
        if (file_char == ']')
            file_char = '>';
        if (file_char != '$')
        {
            fwrite(&file_char,1,1,html_file);
        }
    }
    while (!feof(text_file));
    fclose(text_file);
    fclose(html_file);
    return 0;
}


int main()
{
    n_uint	loop = 0;
    char    text_file_name[FILE_STRING_SIZE];
    char	html_file_name[FILE_STRING_SIZE];
    while (loop < NUMBER_OF_FILES)
    {
        sprintf(text_file_name,"documentation/convert/%s.txt",list_of_files[loop]);
        sprintf(html_file_name,"documentation/man/%s.html",list_of_files[loop]);
        convert_txt_to_html(text_file_name, html_file_name);
        loop++;
    }
    return EXIT_SUCCESS;
}
