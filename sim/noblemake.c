/****************************************************************

 noblemake.c

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
#include <stdlib.h>
#include <time.h>


/*	this was made a constant in case you want to change the
		watchword to something else */
#define	IMPORTANT_LETTERS		9

/*	max length of the file name */
#define	FILE_NAME_LEN			30

/*	constants based on the number of letters in the watchword
		+ intro comment */
#define	IL3						(IMPORTANT_LETTERS+3)
#define	IL8						(IMPORTANT_LETTERS+8)
#define	ILM						3


/*	input buffer - used for referential look-ups */
#define	KEEP_BUFF_LEN			(FILE_NAME_LEN+IL8+2)

/*	to make adding commands easier */
#define	NUMBER_COMMANDS			7

/*	nm_scan output */

#define	SCAN_SET				1
#define	SCAN_DIR				2
#define	SCAN_DEL				3
#define	SCAN_VAR				4
#define	SCAN_END				5
#define	SCAN_NEW				6
#define	SCAN_OUT				7

/*	state in the Noblemake parsing cycle */

#define	SECTION_GROUP			  1
#define	SECTION_REMOVE			2
#define	SECTION_VARIABLES		3
#define	SECTION_THEREST			4
#define	SECTION_NOCOMMENT		5

static int	end_wait;
static int	recur_set;
static int	known_dir = 255;

static char			last_char = 0;
static char			keep[KEEP_BUFF_LEN];

static char			outfile_name[FILE_NAME_LEN] = "output.c";
static char			infile_name[FILE_NAME_LEN] = "recipe.h";

static char			outfile_wait[FILE_NAME_LEN] = "";
static char			infile_wait[FILE_NAME_LEN] = "";

static char			directory_open[FILE_NAME_LEN] = "";
static const char		directory_delimit[4] = ":\\/";

static FILE			*re, *re2, *wr;

static unsigned char verbose = 0;

int				exit_condition = EXIT_SUCCESS;

/*
		nm_scan - strips the NOBLEMAKE comments from the source
		and returns these make commands value and stout is the
		filename etc passed after the command.
*/
static const char	compval[12+1+(NUMBER_COMMANDS<<2)] =
    "/*NOBLEMAKE SET=DIR=DEL=VAR=END=NEW=OUT=";
/*0123456789012345*/
static int	nm_scan(char * stout)
{
    int	tmp = 1, lp = 0;
    /*	bitpack these values into unsigned longs to "search" with
    		fewer cycles */
    unsigned long	* lval = (unsigned long *)compval;
    unsigned long	* lkee = (unsigned long *)keep;
    stout[0] = 0;
    stout[1] = 0;
    while(lp<ILM)
    {
        if(lval[lp] != lkee[lp])
            return 0;
        lp++;
    }
    lp = 0;
    tmp = 0;
    while(lp<NUMBER_COMMANDS)
    {
        if(lval[ILM + lp] == lkee[ILM])
        {
            tmp = ++ lp;
            lp = NUMBER_COMMANDS;
        }
        lp++;
    }
    if (tmp == 0)
        return 0;
    if (keep[IL3+4] != '"')
        return 0;
    lp = 0;
    while ((keep[IL8 + lp] != '"') && (lp < FILE_NAME_LEN))
    {
        stout[lp] = keep[IL8 + lp];
        if (keep[IL8 + lp] != '"')
            lp++;
    }
    if (lp != FILE_NAME_LEN)
    {
        stout[lp] = 0;
        stout[lp + 1] = 0;
    }
    else
        return 0;
    if (keep[IL8 + 1 + lp] != '*' || keep[IL8 + 2 + lp] != '/')
        return 0;
    if (tmp == 0)
    {
        stout[0] = 0;
        stout[1] = 0;
    }
    return(tmp);
}

static void	nm_copy(char * copy_from, char * copy_to)
{
    int	loop = 0, offset = 0;
    while(copy_to[loop] != 0)
        loop++;
    offset = loop;
    loop = 0;
    while(copy_from[loop] != 0)
    {
        copy_to[loop + offset] = copy_from[loop];
        loop++;
    }
    copy_to[loop + offset] = 0;
}

static void	nm_dir(int num, char* name)
{
    short	lp = 0;

    while (name[lp] != 0)
    {
        char	test = name[lp];
        if((test == '/') || (test == '\\') || (test ==':'))
            name[lp] = directory_delimit[num];
        lp++;
    }
    name[lp] = 0;

    /* this is for old mac dir strings */
    if(num == 0 && name[0] != ':')
    {
        while (lp>-1)
        {
            name[lp+1] = name[lp];
            lp--;
        }
        name[0]=':';
    }
}



#define	FILE_OPEN_READ		0
#define	FILE_OPEN_WRITE		1
#define	FILE_CHAR_READ		2
#define	FILE_CHAR_WRITE		3
#define	FILE_CLOSE_READ		4
#define	FILE_CLOSE_WRITE	5
#define	FILE_OPEN_READ2		6
#define	FILE_CHAR_READ2		7
#define	FILE_CLOSE_READ2	8
#define	FILE_STRING_WRITE	9
#define	FILE_REMOVE			  10

static char	nm_fileio(int num,char * filstr)
{
    char	include_dir[FILE_NAME_LEN*2];
    char	tmp = 0;
    switch (num)
    {
    case FILE_OPEN_READ:
        re = fopen(filstr, "rb");

        if (re == 0L)
        {
            exit_condition = EXIT_FAILURE;
            if (verbose == 1) fprintf(stderr,"** Can't open: %s for read\n",filstr);
        }
        break;
    case FILE_OPEN_WRITE:
        wr = fopen(filstr, "w");
        if (wr == 0L)
        {
            exit_condition = EXIT_FAILURE;
            if (verbose == 1) fprintf(stderr,"** Can't open: %s for write\n",filstr);
        }
        break;
    case FILE_CHAR_READ:
        if (!feof(re))
            fread(&tmp, 1, 1, re);
        break;
    case FILE_CHAR_WRITE:
        /* keep new lines platform specific */
        if((filstr[0]==10 || filstr[0]==13))
        {
            if((last_char!=13) && (last_char!=10))
                fprintf(wr, "\n");
        }
        else
            fprintf(wr, "%c", filstr[0]);
        last_char = filstr[0];
        break;
    case FILE_CLOSE_READ:
        fclose(re);
        break;
    case FILE_CLOSE_WRITE:
        fclose(wr);
        break;
    case FILE_OPEN_READ2:

        include_dir[0] = 0;
        nm_copy(directory_open,include_dir);
        nm_copy(filstr,include_dir);
        if(known_dir == 255)
        {
            re2 = fopen(include_dir, "rb");
            if (re2 == 0L)
            {
                nm_dir(2 , include_dir);
                re2 = fopen(include_dir , "rb");
                if (re2 == 0L)
                {
                    nm_dir(1 , include_dir);
                    re2 = fopen(include_dir , "rb");
                    if (re2 == 0L)
                    {
                        nm_dir(0 , include_dir);
                        re2 = fopen(include_dir , "rb");
                        if(re2 != 0L)
                            /*nm_dir(0 , directory_open);*/
                            known_dir = 0;
                    }
                    else
                    {
                        /*nm_dir(1 , directory_open);*/
                        known_dir = 1;
                    }
                }
                else
                {
                    /*nm_dir(2 , directory_open);*/
                    known_dir = 2;
                }
            }
        }
        else
        {
            nm_dir(known_dir , include_dir);
            /*nm_dir(known_dir , directory_open);*/

            re2 = fopen(include_dir , "rb");
        }
        if (re2 == 0L)
        {
            exit_condition = EXIT_FAILURE;
            if (verbose == 1) fprintf(stderr,"** Can't open: %s for read\n",include_dir);
        }
        else
        {
            if (verbose == 1) fprintf(stdout,"  -> %s\n",include_dir);
        }
        break;
    case FILE_CHAR_READ2:
        if (!feof(re2))
            fread(&tmp, 1, 1, re2);
        break;
    case FILE_CLOSE_READ2:
        fclose(re2);
        break;
    case FILE_STRING_WRITE:
        fprintf(wr, "%s", filstr);
        break;
    case FILE_REMOVE:
        remove(filstr);
        break;
    }
    return (tmp);
}

/* nm_read - shuffles one character down the open read file */

static char	nm_read()
{
    char	rtval = keep[0];
    int lp = 0;
    while (lp < (KEEP_BUFF_LEN-1))
    {
        keep[lp] = keep[lp + 1];
        lp++;
    }
    keep[(KEEP_BUFF_LEN-1)] = (char)nm_fileio(FILE_CHAR_READ, "");
    return rtval;
}

#define NM_START(val)	((val[0] == '/') && (val[1] == '*'))

/*		check_end - looks for end comments */
static int	nm_end()
{
    int	tmp = (keep[0] == '*') && (keep[1] == '/');
    if (tmp)
        if (nm_read() + nm_read()) {}
    return (tmp);
}

#define	KILL keep[IL3] = '_'; keep[IL3+1] = '_'; keep[IL3+2] = '_'

static int	nm_section(int	dupl, char* filnam2)
{
    char	tmp, pnt, filnam[FILE_NAME_LEN];
# ifndef S_SPLINT_S
    time_t	now;
    struct tm *date;
#endif
    int lp = 0, typ = 0;
    (void) nm_fileio(FILE_OPEN_READ, filnam2);
    if(exit_condition != EXIT_SUCCESS)
        return exit_condition;

    if (dupl == SECTION_NOCOMMENT)
    {
        now = time( NULL );
        date = localtime( &now );
        (void) nm_fileio(FILE_STRING_WRITE, "#ifndef NOBLEMADE\n#define NOBLEMADE ");
        strftime( filnam, FILE_NAME_LEN, " %y%m%d %H%M ", date );
        filnam[0] = '"';
        filnam[12] = '"';
        (void) nm_fileio(FILE_STRING_WRITE, filnam);
        (void) nm_fileio(FILE_STRING_WRITE, "\n#endif\n");
        if (verbose == 1) fprintf(stdout," Noble-made %s\n",filnam);
    }
    while (lp < KEEP_BUFF_LEN)
    {
        tmp = nm_read();
        lp++;
    }
    tmp = 1;
    while (tmp != 0)
    {
        if (NM_START(keep))
        {
            if (dupl != SECTION_NOCOMMENT)
            {
                typ = nm_scan( filnam );

                switch(typ)
                {
                case SCAN_DIR:
                    if(filnam[0]==0)
                        directory_open[0] = 0;
                    else
                        nm_copy(filnam, directory_open);
                    break;
                case SCAN_NEW:
                case SCAN_OUT:
                    if(typ == SCAN_NEW)
                    {
                        infile_wait[0] = 0;
                        nm_copy(filnam, infile_wait);
                    }
                    else
                    {
                        outfile_wait[0] = 0;
                        nm_copy(filnam, outfile_wait);
                    }
                    /* kill the OUT so it isn't reused */
                    KILL;
                    end_wait = 1;
                    break;
                }
            }
            switch((dupl<<8) | typ)
            {
            case ((SECTION_GROUP<<8) | SCAN_SET):
                (void) nm_fileio(FILE_OPEN_READ2, filnam);
                if(exit_condition != EXIT_SUCCESS)
                    return exit_condition;
                pnt = 1;
                while (pnt != 0)
                {
                    pnt = (char)nm_fileio(FILE_CHAR_READ2, "");
                    if (pnt != 0)
                    {
                        filnam[0] = pnt;
                        (void) nm_fileio(FILE_CHAR_WRITE, filnam);
                    }
                }
                if (nm_fileio(FILE_CLOSE_READ2, "")) {}
                /* kill the SET so it isn't reused */
                KILL;
                recur_set++;
                break;
            case ((SECTION_REMOVE<<8) | SCAN_DEL):
            case ((SECTION_THEREST<<8) | SCAN_VAR):
                while ((typ != SCAN_END) && (tmp != 0))
                {
                    tmp = nm_read();
                    if (NM_START(keep))
                        typ = nm_scan(filnam);
                }
                if (typ == SCAN_END)
                {
                    KILL;
                }
                break;

            case ((SECTION_VARIABLES<<8) | SCAN_VAR):
                while ((typ != SCAN_END) && (tmp != 0))
                {
                    tmp = nm_read();
                    if (NM_START(keep))
                        typ = nm_scan(filnam);
                    if (tmp != 0)
                    {
                        filnam[0] = tmp;
                        (void) nm_fileio(FILE_CHAR_WRITE, filnam);
                    }
                }
                break;
            }
            if (dupl == SECTION_NOCOMMENT)
                while ((!nm_end()) && (tmp != 0))
                    tmp = nm_read();
        }
        tmp = nm_read();
        if (dupl != SECTION_VARIABLES)
        {
            if (tmp != 0)
            {
                filnam[0] = tmp;
                (void) nm_fileio(FILE_CHAR_WRITE, filnam);
            }
        }
    }
    (void) nm_fileio(FILE_CLOSE_READ, "");
    return EXIT_SUCCESS;
}

static int	nm_file(char*	INPUTFILE, char * OUTPUTFILE)
{
#define	BUFFERFILE	"buffer.txt"

    if (verbose == 1) fprintf(stdout,"%s -> %s\n",INPUTFILE, OUTPUTFILE);
    if (verbose == 1) fprintf(stdout," Group Files\n");

    /* (1) Group Files */

    recur_set=0; /* set the global -> recursive SET to false */
    (void) nm_fileio(FILE_OPEN_WRITE, BUFFERFILE);

    if(nm_section(SECTION_GROUP, INPUTFILE) != EXIT_SUCCESS)
        return(EXIT_FAILURE);

    (void) nm_fileio(FILE_CLOSE_WRITE, "");
    if (verbose == 1)
    {
        if(recur_set==1)
            fprintf(stdout,"  -> ->\n");
    }
    if (recur_set) do
        {
            /* has there been a recursive SET in the last grouping */
            recur_set=0;
            (void) nm_fileio(FILE_OPEN_WRITE, OUTPUTFILE);
            if(nm_section(SECTION_GROUP, BUFFERFILE) != EXIT_SUCCESS)
                return(EXIT_FAILURE);
            (void) nm_fileio(FILE_CLOSE_WRITE, "");
            if (verbose == 1)
            {
                if(recur_set==1)
                    fprintf(stdout,"  -> ->\n");
            }
            (void) nm_fileio(FILE_OPEN_WRITE, BUFFERFILE);
            if(nm_section(SECTION_GROUP, OUTPUTFILE) != EXIT_SUCCESS)
                return(EXIT_FAILURE);
            /* Swap files or repeat Assemble */
            (void) nm_fileio(FILE_CLOSE_WRITE, "");
            if (verbose == 1)
            {
                if(recur_set==2)
                    fprintf(stdout,"  -> ->\n");
            }

        }
        while (recur_set==2);
    /* repeat if a recursive SET in the last two cycles...*/
    (void) nm_fileio(FILE_OPEN_WRITE, OUTPUTFILE);
    /* (2) Remove Sections */

    if (verbose == 1) fprintf(stdout," Remove Sections\n");

    if(nm_section(SECTION_REMOVE, BUFFERFILE) != EXIT_SUCCESS)
        return(EXIT_FAILURE);
    (void) nm_fileio(FILE_CLOSE_WRITE, "");
    (void) nm_fileio(FILE_OPEN_WRITE, BUFFERFILE);
    if (verbose == 1) fprintf(stdout," Variable Code Division\n  -> Variables\n");

    /* (3) Variable Code Division (Variables) */
    if(nm_section(SECTION_VARIABLES, OUTPUTFILE) != EXIT_SUCCESS)
        return(EXIT_FAILURE);

    if (verbose == 1) fprintf(stdout,"  -> the Rest\n");

    /* (3) Variable Code Division (the Rest) */
    if(nm_section(SECTION_THEREST, OUTPUTFILE) != EXIT_SUCCESS)
        return(EXIT_FAILURE);
    (void) nm_fileio(FILE_CLOSE_WRITE, "");
    (void) nm_fileio(FILE_OPEN_WRITE, OUTPUTFILE);

    if (verbose == 1) fprintf(stdout," Remove Comments\n");

    /* (4) Remove Comments */
    if(nm_section(SECTION_NOCOMMENT, BUFFERFILE) != EXIT_SUCCESS)
        return(EXIT_FAILURE);
    (void) nm_fileio(FILE_CLOSE_WRITE, "");
    (void) nm_fileio(FILE_REMOVE, BUFFERFILE);
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
    end_wait = 0;


    if (argc >3)
    {
        if (((argv[3][0] == '-') && (argv[3][1] == 'v'))&& (argv[3][2] == 0))
        {
            verbose = 1;
        }
    }

    if (argc > 2)
    {
        outfile_name[0]=0;
        nm_copy(argv[2],outfile_name);
    }

    if (argc > 1)
    {
        infile_name[0]=0;
        nm_copy(argv[1],infile_name);
    }


    if(nm_file(infile_name, outfile_name) != EXIT_SUCCESS)
        return	EXIT_FAILURE;


    if(end_wait) while(end_wait)
        {
            if (verbose == 1) fprintf(stdout," Next...\n\n");
            if(infile_wait[0]!=0)
            {
                infile_name[0]=0;
                nm_copy(infile_wait,infile_name);
            }
            if(outfile_wait[0]!=0)
            {
                outfile_name[0]=0;
                nm_copy(outfile_wait,outfile_name);
            }
            end_wait = 0;
            if(nm_file(infile_name, outfile_name) != EXIT_SUCCESS)
                return EXIT_FAILURE;
        }
    if (verbose == 1) fprintf(stdout,"\nDone\n");

    return exit_condition;
}



