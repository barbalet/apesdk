/****************************************************************

 genealogy.c

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

/*! \file   being.c
 *  \brief  Historically this represented the Noble Ape but moreso
    now it represents the Noble Ape's interface to something external.
    being.c also now connects to the social, brain, body and metabolim
    simulations through to external simulations.
 */

/*NOBLEMAKE DEL=""*/

#include "entity.h"
#include "entity_internal.h"

#include <stdio.h>
#include <string.h>

static n_byte log_genealogy = 0;

#define GENEALOGY_XML_FILENAME "tmp_genealogy.xml"
#define GENEALOGY_GEDCOM_FILENAME "tmp_genealogy.ged"

#define GENEALOGY_EVENT_BIRTH "E1"
#define GENEALOGY_EVENT_DEATH "E2"
#define GENEALOGY_YEAR_ZERO   1900

/** Genealogy format GenXML 2.0
 http://cosoft.org/genxml */


void genealogy_log(noble_simulation * sim,n_byte value);
n_int genealogy_save(noble_simulation * sim, n_string filename);
void genealogy_birth(noble_being * child, noble_being * mother, void * sim);
void genealogy_death(noble_being * local_being, void * sim);

/**
 * Adds XML open to the named string.
 * @param file the pointer to the n_file data that is written.
 * @param name the string that is wrapped.
 * @return FILE_ERROR if there is a problem and FILE_OKAY if it is successful.
 */
n_int xml_open(n_file * file, n_string name)
{
    if (io_write(file,"<", 0) == -1) return -1;
    if (io_write(file,name, 0) == -1) return -1;
    if (io_write(file,">", 1) == -1) return -1;
    return 0;
}

/**
 * Adds XML close to the named string.
 * @param file the pointer to the n_file data that is written.
 * @param name the string that is wrapped.
 * @return FILE_ERROR if there is a problem and FILE_OKAY if it is successful.
 */
n_int xml_close(n_file * file, n_string name)
{
    if (io_write(file,"</", 0) == -1) return -1;
    if (io_write(file,name, 0) == -1) return -1;
    if (io_write(file,">", 1) == -1) return -1;
    return 0;
}

/**
 * Wraps a string with XML open and close
 * @param file the pointer to the n_file data that is written.
 * @param name the string that is the wrapper.
 * @param string the string that is wrapped.
 * @return FILE_ERROR if there is a problem and FILE_OKAY if it is successful.
 */
n_int xml_string(n_file * file, n_string name, n_string string)
{
    if (xml_open(file,name) == -1) return -1;
    if (io_write(file,string, 0) == -1) return -1;
    if (xml_close(file,name) == -1) return -1;
    return 0;
}

/**
 * Wraps an integer with XML open and close
 * @param file the pointer to the n_file data that is written.
 * @param name the string that is the wrapper.
 * @param number the integer that is wrapped.
 * @return FILE_ERROR if there is a problem and FILE_OKAY if it is successful.
 */
n_int xml_int(n_file * file, n_string name, n_int number)
{
    if (xml_open(file,name) == -1) return -1;
    if (io_writenumber(file, number, 1, 0) == -1) return -1;
    if (xml_close(file,name) == -1) return -1;
    return 0;
    
}

/**
 * Appends a file to disk.
 * @param local_file the pointer to the n_file data that is written to disk.
 * @param file_name the name of the file to be appended.
 * @return FILE_ERROR if there is a problem and FILE_OKAY if it is successful.
 */
n_int io_disk_append(n_file * local_file, n_string file_name)
{
    n_uint written_length;
#ifndef _WIN32
    FILE * out_file = fopen(file_name,"a");
#else
    FILE * out_file = 0L;
    
    fopen_s(&out_file,file_name,"a");
#endif
    
    written_length = fwrite(local_file->data,1,local_file->location, out_file);
    
    if (fclose(out_file) != 0)
    {
        return SHOW_ERROR("File could not be closed");
    }
    
    if (written_length != local_file->location)
    {
        return SHOW_ERROR("File did not complete write");
    }
    return FILE_OKAY;
}


void genealogy_log(noble_simulation * sim,n_byte value)
{
    log_genealogy = value;
    if (value == 0)
    {
        sim->ext_birth = 0L;
        sim->ext_death = 0L;
    }
    else
    {
        sim->ext_birth = &genealogy_birth;
        sim->ext_death = &genealogy_death;
    }
}

#ifdef GENEALOGY_ON

static n_byte empty_genetics(n_genetics * genetics)
{
    n_int i;
    for (i=0; i<CHROMOSOMES; i++)
    {
        if (genetics[i]!=0) return 0;
    }
    return 1;
}

static void genealogy_being_id(noble_being * local_being, n_file * fp, int parent)
{
    n_genetics * genome;
    n_int ch;
    
    switch(parent)
    {
        case 1:
            genome = local_being->mother_genetics;
            break;
        case 2:
            genome = local_being->father_genetics;
            break;
        default:
            genome = being_genetics(local_being);
    }
    
    for (ch = 0; ch < CHROMOSOMES; ch++)
    {
        io_writenumber(fp, genome[ch],1,0);
    }
}

static void genealogy_today(noble_simulation * sim, n_file * fp)
{
    n_int local_dob=0;
    
    n_int current_date = TIME_IN_DAYS(sim->land->date);
    
    n_int age_in_years = (current_date - local_dob) / TIME_YEAR_DAYS;
    n_int age_in_months = ((current_date - local_dob) - (age_in_years * TIME_YEAR_DAYS)) / (TIME_YEAR_DAYS/12);
    n_int age_in_days = (current_date - local_dob) - ((TIME_YEAR_DAYS/12) * age_in_months) - (age_in_years * TIME_YEAR_DAYS);
    
    if (age_in_months>11) age_in_months=11;
    
    io_writenumber(fp, (age_in_years+GENEALOGY_YEAR_ZERO), 1, 0);
    io_file_write(fp,'-');
    io_writenumber(fp, age_in_months, 1, 0);
    io_file_write(fp,'-');
    io_writenumber(fp, (age_in_days+1), 1, 0);
}

static void genealogy_birth_genxml(noble_being * child, noble_being * mother, noble_simulation * sim)
{
    n_file * fp;
    n_string_block str, first_name, surnames;
    n_int n;
    n_uint i;
    
    if (log_genealogy!=0)
    {
        if (!io_disk_check(GENEALOGY_XML_FILENAME))
        {
            fp = io_file_new();
            
            if (fp == 0L) return;
            
            xml_open(fp, "?xml version='1.0' encoding='ISO-8859-1'?");
            xml_open(fp, "genxml");
            
            xml_open(fp, "file");
            xml_int(fp, "version", 200);
            xml_int(fp, "level", 2);
            xml_close(fp, "file");
            
            xml_open(fp, "header");
            xml_string(fp, "exportingsystem", "Noble Ape");
            xml_string(fp, "version", "0.0");
            xml_close(fp, "header");
            
            io_write(fp,"<eventtype id=\"",0);
            io_write(fp,GENEALOGY_EVENT_BIRTH,0);
            io_write(fp,"\" class=\"birth\">",1);
            
            
            xml_string(fp, "description", "born");
            xml_string(fp, "gedcomtag", "BIRT");
            
            xml_int(fp, "roles", 1);
            xml_close(fp,  "eventtype");
            
            io_write(fp, "<eventtype id=\"",0);
            io_write(fp, GENEALOGY_EVENT_BIRTH,0);
            io_write(fp, "\" class=\"death\">",1);
            
            xml_string(fp, "description", "died");
            xml_string(fp, "gedcomtag", "DEAT");
            
            xml_int(fp, "roles", 1);
            xml_close(fp,  "eventtype");
            
            io_disk_write(fp,GENEALOGY_XML_FILENAME);
            
            io_file_free(fp);
            fp = 0L;
        }
        
        fp = io_file_new();
        
        if (fp != 0L)
        {
            
            io_write(fp, "<person id=\"", 0);
            genealogy_being_id(child,fp,0);
            io_write(fp, "\" sex=\"", 0);
            if (FIND_SEX(GET_I(child))!=SEX_FEMALE)
            {
                io_write(fp,"male",0);
            }
            else
            {
                io_write(fp,"female",0);
            }
            io_write(fp, "\">", 1);
            
            xml_open(fp, "personalname");
            
            being_name_simple(child, (n_string)str);
            
            for (i=0; i<strlen((char*)str); i++)
            {
                if (str[i]==' ') break;
                first_name[i]=str[i];
            }
            first_name[i]='\0';
            i++;
            n=0;
            while (i<strlen((char*)str))
            {
                surnames[n++]=str[i];
                i++;
            }
            surnames[n]='\0';
            
            io_write(fp,"<np tp=\"unkw\">",0);
            io_write(fp,(n_string)first_name,0);
            io_write(fp,"</np>",1);
            
            io_write(fp,"<np tp=\"surn\">",0);
            io_write(fp,(n_string)surnames,0);
            io_write(fp,"</np>",1);
            
            xml_close(fp, "personalname");
            xml_close(fp, "person");
            
            
            io_write(fp, "<assertion id=\"BIRTH",0);
            
            genealogy_being_id(child,fp,0);
            
            io_write(fp, "\">",1);
            
            
            io_write(fp, "<event type=\"",0);
            
            io_write(fp, GENEALOGY_EVENT_BIRTH,0);
            
            io_write(fp, "\">",1);
            
            
            io_write(fp, "<principal>",0);
            genealogy_being_id(child,fp,0);
            xml_close(fp, "principal");
            
            
            
            xml_open(fp, "date");
            io_write(fp, "<exact>",0);
            genealogy_today(sim,fp);
            xml_close(fp, "exact");
            xml_close(fp, "date");
            
            
            xml_open(fp, "place");
            io_write(fp, "<pnp>",0);
            
            
            io_writenumber(fp, being_location_x(child), 1, 0);
            io_file_write(fp, ' ');
            io_writenumber(fp, being_location_y(child), 1, 0);
            
            xml_close(fp, "pnp");
            xml_close(fp, "place");
            
            
            xml_close(fp, "event");
            xml_close(fp, "assertion");
            
            
            
            io_write(fp, "<assertion id=\"REL",0);
            genealogy_being_id(child,fp,0);
            io_write(fp, "\">",1);
            
            xml_open(fp, "relationship");
            
            xml_string(fp, "relation", "biological");
            
            io_write(fp, "<child>",0);
            genealogy_being_id(child,fp,0);
            xml_close(fp, "child");
            io_write(fp, "<mother>",0);
            genealogy_being_id(child,fp,1);
            xml_close(fp, "mother");
            io_write(fp, "<father>",0);
            genealogy_being_id(child,fp,2);
            
            
            xml_close(fp, "father");
            xml_close(fp, "relationship");
            xml_close(fp, "assertion");
            
            io_disk_append(fp,GENEALOGY_XML_FILENAME);
            
            io_file_free(fp);
        }
    }
}

static void genealogy_death_genxml(noble_being * local_being, noble_simulation * sim)
{
    n_file * fp;
    
    if (log_genealogy!=0)
    {
        fp = io_file_new();
        if (fp != 0L)
        {
            
            io_write(fp, "<assertion id=\"DEATH",0);
            genealogy_being_id(local_being,fp,0);
            io_write(fp, "\">",1);
            
            
            io_write(fp, "<event type=\"",0);
            
            io_write(fp, GENEALOGY_EVENT_DEATH,0);
            
            io_write(fp, "\">",1);
            
            io_write(fp, "<principal>",0);
            
            genealogy_being_id(local_being,fp,0);
            
            xml_close(fp, "principal");
            
            xml_open(fp, "date");
            io_write(fp, "<exact>",0);
            
            genealogy_today(sim,fp);
            
            xml_close(fp, "exact");
            xml_close(fp, "date");
            
            xml_close(fp, "event");
            xml_close(fp, "assertion");
            
            
            xml_open(fp, "place");
            io_write(fp, "<pnp>",0);
            
            io_writenumber(fp, being_location_x(local_being), 1, 0);
            io_file_write(fp, ' ');
            io_writenumber(fp, being_location_y(local_being), 1, 0);
            
            xml_close(fp, "pnp");
            xml_close(fp, "place");
            
            xml_close(fp, "event");
            xml_close(fp, "assertion");
            
            io_disk_append(fp,GENEALOGY_XML_FILENAME);
            io_file_free(fp);
        }
    }
}

static n_int genealogy_save_genxml(noble_simulation * sim, n_string filename)
{
    n_file * fp;
    
    if (log_genealogy!=0)
    {
        fp = io_file_new();
        if (io_disk_check(GENEALOGY_XML_FILENAME))
        {
            io_disk_read(fp, GENEALOGY_XML_FILENAME);
        }
        
        xml_close(fp, "genxml");
        io_disk_write(fp, filename);
        io_file_free(fp);
        return 1;
    }
    return 0;
}

/** GEDCOM 5.5 http://en.wikipedia.org/wiki/GEDCOM */

static void genealogy_today_gedcom(noble_simulation * sim, n_file * fp)
{
    n_string month[] =
    {
        "JAN","FEB","MAR",
        "APR","MAY","JUN",
        "JUL","AUG","SEP",
        "OCT","NOV","DEC"
    };
    n_int	current_date = TIME_IN_DAYS(sim->land->date);
    n_int days_month[12] = {31, 28, 31, 30,
        31, 30, 31, 31,
        30, 31, 30, 31
    };
    
    n_int years = current_date / TIME_YEAR_DAYS;
    n_int days = current_date % TIME_YEAR_DAYS;
    n_int loop = 0;
    while (loop < 12)
    {
        n_int days_in_month = days_month[ loop ];
        if (days >= days_in_month)
        {
            days -= days_in_month;
        }
        else
        {
            io_writenumber(fp, (years+GENEALOGY_YEAR_ZERO), 1, 0);
            io_file_write(fp,' ');
            
            io_write(fp, month[loop], 0);
            
            io_file_write(fp,' ');
            io_writenumber(fp, (days+1), 1, 0);
            return;
        }
        loop++;
    }
}

static void genealogy_birth_gedcom(noble_being * child, noble_being * mother, noble_simulation * sim)
{
    n_string_block str,first_name,surnames;
    n_int n;
    n_uint i;
    n_file * fp;
    
    if (log_genealogy!=0)
    {
        if (!io_disk_check(GENEALOGY_GEDCOM_FILENAME))
        {
            fp = io_file_new();
            if (fp == 0L) return;
            
            io_write(fp, "0 HEAD", 1);
            io_write(fp, "1 SOUR Noble Ape", 1);
            io_write(fp, "2 VERS V0.0", 1);
            io_write(fp, "1 DEST Noble Ape", 1);
            io_write(fp, "1 FILE ", 0);
            io_write(fp, GENEALOGY_GEDCOM_FILENAME, 1);
            io_write(fp, "1 GEDC", 1);
            io_write(fp, "2 VERS 5.5", 1);
            io_write(fp, "1 CHAR ASCII", 1);
            
            io_disk_write(fp,GENEALOGY_GEDCOM_FILENAME);
            
            io_file_free(fp);
            fp = 0L;
        }
        
        fp = io_file_new();
        
        if (fp != 0L)
        {
            being_name_simple(child, (n_string)str);
            
            for (i=0; i<strlen((char*)str); i++)
            {
                if (str[i]==' ') break;
                first_name[i]=str[i];
            }
            first_name[i]='\0';
            i++;
            n=0;
            while (i<strlen((char*)str))
            {
                surnames[n++]=str[i];
                i++;
            }
            surnames[n]='\0';
            
            io_write(fp, "0 @I", 0);
            genealogy_being_id(child,fp,0);
            
            io_write(fp, "@ INDI", 1);
            
            
            io_write(fp, "1 NAME ", 0);
            io_write(fp, (n_string)first_name, 0);
            io_write(fp, " /", 0);
            io_write(fp, (n_string)surnames, 0);
            io_write(fp, "/", 1);
            
            io_write(fp, "1 SEX ", 0);
            
            if (FIND_SEX(GET_I(child))!=SEX_FEMALE)
            {
                io_write(fp, "M", 1);
            }
            else
            {
                io_write(fp, "F", 1);
            }
            io_write(fp, "1 BIRT", 1);
            io_write(fp, "2 DATE ", 0);
            genealogy_today_gedcom(sim,fp);
            io_write(fp, "", 1);
            /*
             io_write(fp, "2 PLAC ", 0);
             genealogy_place_name(child,fp);
             io_write(fp, "", 1);
             */
            
            if (!empty_genetics(child->father_genetics))
            {
                io_write(fp, "1 FAMC @F", 0);
                genealogy_being_id(child,fp,0);
                io_write(fp, "@", 1);
                
                io_write(fp, "0 @F", 0);
                genealogy_being_id(child,fp,0);
                io_write(fp, "@ FAM", 1);
                
                io_write(fp, "1 HUSB @I", 0);
                genealogy_being_id(child,fp,2);
                io_write(fp, "@", 1);
                io_write(fp, "1 WIFE @I", 0);
                genealogy_being_id(child,fp,1);
                io_write(fp, "@", 1);
                
                io_write(fp, "1 MARR", 1);
                io_write(fp, "2 DATE ", 0);
                genealogy_today_gedcom(sim,fp);
                io_write(fp, "", 1);
                
                io_write(fp, "1 CHIL @I", 0);
                genealogy_being_id(child,fp,0);
                io_write(fp, "@", 1);
            }
            
            io_disk_append(fp,GENEALOGY_GEDCOM_FILENAME);
            
            io_file_free(fp);
        }
    }
}

static void genealogy_death_gedcom(noble_being * local_being, noble_simulation * sim)
{
    if (log_genealogy!=0)
    {
        n_file * fp = io_file_new();
        
        if (fp == 0L) return;
        
        io_write(fp, "0 @I", 0);
        genealogy_being_id(local_being,fp,0);
        io_write(fp, "@ INDI", 1);
        
        io_write(fp, "1 DEAT", 1);
        
        io_write(fp, "2 DATE ", 0);
        genealogy_today_gedcom(sim,fp);
        io_write(fp, "", 1);
        /*
         io_write(fp, "2 PLAC ", 0);
         genealogy_place_name(local_being,fp);
         io_write(fp, "", 1);
         */
        io_disk_append(fp,GENEALOGY_GEDCOM_FILENAME);
        
        io_file_free(fp);
    }
}

static n_int genealogy_save_gedcom(noble_simulation * sim, n_string filename)
{
    n_file * fp;
    
    if (log_genealogy!=0)
    {
        fp = io_file_new();
        if (io_disk_check(GENEALOGY_GEDCOM_FILENAME))
        {
            io_disk_read(fp, GENEALOGY_GEDCOM_FILENAME);
        }
        io_write(fp, "0 TRLR",1);
        io_disk_write(fp, filename);
        io_file_free(fp);
        return 1;
    }
    return 0;
}

void genealogy_birth(noble_being * child, noble_being * mother, void * sim)
{
    switch(log_genealogy)
    {
        case GENEALOGY_GENXML:
            genealogy_birth_genxml(child, mother, (noble_simulation *)sim);
            break;
        case GENEALOGY_GEDCOM:
            genealogy_birth_gedcom(child, mother, (noble_simulation *)sim);
            break;
    }
}

void genealogy_death(noble_being * local_being, void * sim)
{
    switch(log_genealogy)
    {
        case GENEALOGY_GENXML:
            genealogy_death_genxml(local_being, (noble_simulation *)sim);
            break;
        case GENEALOGY_GEDCOM:
            genealogy_death_gedcom(local_being, (noble_simulation *)sim);
            break;
    }
}

n_int genealogy_save(noble_simulation * sim, n_string filename)
{
    n_int retval=0;
    
    switch(log_genealogy)
    {
        case GENEALOGY_GENXML:
            retval = genealogy_save_genxml(sim, filename);
            break;
        case GENEALOGY_GEDCOM:
            retval = genealogy_save_gedcom(sim, filename);
            break;
    }
    return retval;
}

#endif

