/****************************************************************

 web.c

 =============================================================

 Copyright 2010 Bob Mottram. All rights reserved.

 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or
 sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the followingp
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

#define PNG_DEBUG 3
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <zlib.h>
#include <math.h>
#include "pnglite.h"

#ifndef	_WIN32
#ifdef __MWERKS__

#include "noble.h"

#else

#include "../../noble/noble.h"
#include "../../gui/gui.h"
#include "../../universe/universe.h"
#include "../../entity/entity.h"
#include "../../universe/universe_internal.h"
#include "../../entity/entity_internal.h"

#endif

#else

#include "..\..\noble\noble.h"
#include "..\..\gui\gui.h"
#include "..\..\universe\universe.h"
#include "..\..\entity\entity.h"
#include "..\..\universe\universe_internal.h"
#include "..\..\entity\entity_internal.h"

#endif

#include "png2.h"

#define TILE_WIDTH 32
#define TILE_RAIN                   0
#define TILE_CLOUD                  1
#define TILE_SUNNY_INTERVALS        2
#define TILE_SUNSHINE               3
#define TILE_APE_MALE_AWAKE         4
#define TILE_APE_FEMALE_AWAKE       5
#define TILE_APE_MALE_ASLEEP        6
#define TILE_APE_FEMALE_ASLEEP      7
#define TILE_APE_MALE_SWIMMING      8
#define TILE_APE_FEMALE_SWIMMING    9
#define TILE_APE_MALE_MOVING        10
#define TILE_APE_FEMALE_MOVING      11
#define TILE_APE_MALE_EATING        12
#define TILE_APE_FEMALE_EATING      13
#define TILE_APE_MALE_REPRODUCING   14
#define TILE_APE_FEMALE_REPRODUCING 15
#define TILE_APE_MALE_HUNGRY        16
#define TILE_APE_FEMALE_HUNGRY      17
#define TILE_APE_MALE_SPEAKING      18
#define TILE_APE_FEMALE_SPEAKING    19
#define TILE_APE_MALE_SHOUTING      20
#define TILE_APE_FEMALE_SHOUTING    21
#define TILE_APE_MALE_GROOMING      22
#define TILE_APE_FEMALE_GROOMING    23
#define TILE_APE_MALE_SUCKLING      24
#define TILE_APE_FEMALE_SUCKLING    25
#define TILE_APE_MALE_SHOWFORCE     26
#define TILE_APE_FEMALE_SHOWFORCE   27
#define TILE_APE_MALE_ATTACK        28
#define TILE_APE_FEMALE_ATTACK      29

#define FILE_RAIN                   "images/rain.png"
#define FILE_CLOUD                  "images/cloud.png"
#define FILE_SUNNY_INTERVALS        "images/sunny_intervals.png"
#define FILE_SUNSHINE               "images/sunshine.png"
#define FILE_APE_MALE_AWAKE         "images/ape_male_awake.png"
#define FILE_APE_FEMALE_AWAKE       "images/ape_female_awake.png"
#define FILE_APE_MALE_ASLEEP        "images/ape_male_asleep.png"
#define FILE_APE_FEMALE_ASLEEP      "images/ape_female_asleep.png"
#define FILE_APE_MALE_SWIMMING      "images/ape_male_swimming.png"
#define FILE_APE_FEMALE_SWIMMING    "images/ape_female_swimming.png"
#define FILE_APE_MALE_MOVING        "images/ape_male_moving.png"
#define FILE_APE_FEMALE_MOVING      "images/ape_female_moving.png"
#define FILE_APE_MALE_EATING        "images/ape_male_eating.png"
#define FILE_APE_FEMALE_EATING      "images/ape_female_eating.png"
#define FILE_APE_MALE_REPRODUCING   "images/ape_male_reproducing.png"
#define FILE_APE_FEMALE_REPRODUCING "images/ape_female_reproducing.png"
#define FILE_APE_MALE_HUNGRY        "images/ape_male_hungry.png"
#define FILE_APE_FEMALE_HUNGRY      "images/ape_female_hungry.png"
#define FILE_APE_MALE_SPEAKING      "images/ape_male_speaking.png"
#define FILE_APE_FEMALE_SPEAKING    "images/ape_female_speaking.png"
#define FILE_APE_MALE_SHOUTING      "images/ape_male_shouting.png"
#define FILE_APE_FEMALE_SHOUTING    "images/ape_female_shouting.png"
#define FILE_APE_MALE_GROOMING      "images/ape_male_grooming.png"
#define FILE_APE_FEMALE_GROOMING    "images/ape_female_grooming.png"
#define FILE_APE_MALE_SUCKLING      "images/ape_male_suckling.png"
#define FILE_APE_FEMALE_SUCKLING    "images/ape_female_suckling.png"
#define FILE_APE_MALE_SHOWFORCE     "images/ape_male_showforce.png"
#define FILE_APE_FEMALE_SHOWFORCE   "images/ape_female_showforce.png"
#define FILE_APE_MALE_ATTACK        "images/ape_male_attack.png"
#define FILE_APE_FEMALE_ATTACK      "images/ape_female_attack.png"

static n_byte map_tile[30][TILE_WIDTH*TILE_WIDTH*3];

void plat2_free(void * ptr)
{
    if( ptr != 0L )
    {
        free( ptr );
        ptr = 0L;
    }
}

int	plat2_file_out(char * file_name, n_byte * data, n_uint length)
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

n_int get_alpha(n_byte female, noble_simulation * sim)
{
    n_int i,alpha=-1,honor=0;
    noble_being * local_being;
    for (i = 0; i < sim->num; i++)
    {
        local_being = &sim->beings[i];
        if (((FIND_SEX(GET_I(local_being)) == SEX_FEMALE) && (female != 0)) ||
                ((FIND_SEX(GET_I(local_being)) != SEX_FEMALE) && (female == 0)))
        {
            if (local_being->honor > honor)
            {
                honor = local_being->honor;
                alpha = i;
            }
        }
    }
    return(alpha);
}

void create_alphas(n_file * fil, noble_simulation * sim)
{
    noble_being * local_being;
    n_int alpha_male, alpha_female;
    alpha_male = get_alpha(0, sim);
    alpha_female = get_alpha(1, sim);

    if ((alpha_male > -1) || (alpha_female > -1))
    {
        char str[100];
        io_write(fil, "<br>", 0);
        if (alpha_male > -1)
        {
            local_being = &sim->beings[alpha_male];
            io_write(fil, "Alpha Male: ", 0);
            being_name(
                (FIND_SEX(GET_I(local_being)) == SEX_FEMALE),
                GET_NAME(sim,local_being),
                GET_FAMILY_FIRST_NAME(sim,local_being),
                GET_FAMILY_SECOND_NAME(sim,local_being),
                str);
            io_write(fil, str, 0);
            io_write(fil, "<br>", 1);
        }
        if (alpha_female > -1)
        {
            local_being = &sim->beings[alpha_female];
            io_write(fil, "Alpha Female: ", 0);
            being_name(
                (FIND_SEX(GET_I(local_being)) == SEX_FEMALE),
                GET_NAME(sim,local_being),
                GET_FAMILY_FIRST_NAME(sim,local_being),
                GET_FAMILY_SECOND_NAME(sim,local_being),
                str);
            io_write(fil, str, 0);
            io_write(fil, "<br>", 1);
        }
        io_write(fil, "<br>", 1);
    }
}

void create_births(
    n_file * fil,
    noble_simulation * sim,
    char * births_title,
    n_int max_births)
{
    n_int i,births=0,ctr;
    char str[100];
    n_uint current_date = TIME_IN_DAYS(sim->land->date);
    for (i = 0; i < sim->num; i++)
    {
        noble_being * local_being = &sim->beings[i];
        n_uint local_dob = TIME_IN_DAYS(GET_D(local_being));
        n_int age_days = current_date - local_dob;
        if (age_days < 2)
        {
            births++;
            if (births == max_births) break;
        }
    }

    if (births > 0)
    {
        io_write(fil, "<br><br><h2>", 0);
        io_write(fil, births_title, 0);
        io_write(fil, "</h2>", 1);

        io_write(fil, "        <table style=\"text-align: center; width: 70%;\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\">", 1);
        io_write(fil, "            <tbody>", 1);

        io_write(fil, "                <tr>", 1);
        io_write(fil, "                    <td style=\"vertical-align: top; text-align: left;\">", 1);
        io_write(fil, "                    <h2>Honor</h2>", 1);
        io_write(fil, "                    </td>", 1);

        io_write(fil, "                    <td style=\"vertical-align: top; text-align: left;\">", 1);
        io_write(fil, "                    <h2>Name</h2>", 1);
        io_write(fil, "                    </td>", 1);

        io_write(fil, "                    <td style=\"vertical-align: top; text-align: left;\">", 1);
        io_write(fil, "                    <h2>Gender</h2>", 1);
        io_write(fil, "                    </td>", 1);
        io_write(fil, "                </tr>", 1);

        ctr=0;
        for (i = 0; i < sim->num; i++)
        {
            noble_being * local_being = &sim->beings[i];
            n_uint local_dob = TIME_IN_DAYS(GET_D(local_being));
            n_int age_days = current_date - local_dob;
            if (age_days < 2)
            {

                io_write(fil, "                <tr>", 1);
                io_write(fil, "                    <td style=\"vertical-align: top; text-align: left;\">", 1);
                io_write(fil, "                    ", 0);
#ifdef PARASITES_ON
                io_writenumber(fil, local_being->honor, 1, 1);
#endif
                io_write(fil, "                    </td>", 1);
                io_write(fil, "                    <td style=\"vertical-align: top; text-align: left;\">", 1);
                io_write(fil, "                    ", 0);
                io_write(fil, "                    <a href=\"ape_birth_", 0);
                io_writenumber(fil, ctr, 1, 0);
                io_write(fil, ".htm\">", 1);
                being_name((FIND_SEX(GET_I(local_being)) == SEX_FEMALE), GET_NAME(sim,local_being), GET_FAMILY_FIRST_NAME(sim,local_being), GET_FAMILY_SECOND_NAME(sim,local_being), str);
                io_write(fil, str, 1);
                io_write(fil, " ", 1);
                io_write(fil, "                    </a></td>", 1);

                io_write(fil, "                    <td style=\"vertical-align: top; text-align: left;\">", 1);
                if (FIND_SEX(GET_I(local_being)) == SEX_FEMALE)
                    io_write(fil, "                    Female", 1);
                else
                    io_write(fil, "                    Male", 1);
                io_write(fil, "                    </td>", 1);

                io_write(fil, "                </tr>", 1);

                ctr++;
                if (ctr == max_births) break;
            }
        }

        io_write(fil, "            </tbody>", 1);
        io_write(fil, "        </table>", 1);
    }
}

void create_web_ape_friends(
    n_file * fil,
    noble_simulation * sim,
    noble_being * local_being,
    n_int friend_type)
{
    char str[100];
    n_byte eliminated[SOCIAL_SIZE];
    int i,j;
    n_int f=0,attr,resp;
    social_link * local_social_graph = GET_SOC(sim, local_being);
    for (i = 0; i < SOCIAL_SIZE; i++) eliminated[i] = 0;

    for (i = 1; i < SOCIAL_SIZE; i++)
    {
        n_int winner = -1;
        n_int max_familiarity = 0;
        for (j = 1; j < SOCIAL_SIZE; j++)
        {
            if ((eliminated[j] == 0) &&
                    (!((local_social_graph[j].first_name[BEING_MET]==0)&&(local_social_graph[j].family_name[BEING_MET]==0))))
            {
                resp = (n_int)(local_social_graph[j].friend_foe) - social_respect_mean(sim,local_being);
                attr = local_social_graph[j].attraction;
                switch(friend_type)
                {
                case 0:   /* friends */
                {
                    f = local_social_graph[j].familiarity*resp;
                    if (attr != 0) f = 0;
                    break;
                }
                case 1:   /* enemies */
                {
                    f = -local_social_graph[j].familiarity*resp;
                    if (attr != 0) f = 0;
                    break;
                }
                case 2:   /* attraction */
                {
                    f = attr*resp;
                    break;
                }
                }
                if (f >= max_familiarity)
                {
                    winner = j;
                    max_familiarity = f;
                }
            }
        }
        if (winner == -1) break;
        eliminated[winner] = 1;

        if (max_familiarity > 0)
        {
            n_byte name =
                local_social_graph[winner].first_name[BEING_MET]&255;
            n_byte gender =
                local_social_graph[winner].first_name[BEING_MET]>>8;
            n_byte family0 =
                UNPACK_FAMILY_FIRST_NAME(local_social_graph[winner].family_name[BEING_MET]);
            n_byte family1 =
                UNPACK_FAMILY_SECOND_NAME(local_social_graph[winner].family_name[BEING_MET]);
            noble_being *b=0,*b2=0;
            for (j=0; j<sim->num; j++)
            {
                b = &sim->beings[j];
                if (GET_NAME(sim,b) == name)
                {
                    if ((GET_FAMILY_FIRST_NAME(sim,b)==family0) && (GET_FAMILY_SECOND_NAME(sim,b)==family1))
                    {
                        if (FIND_SEX(GET_I(b)) == gender)
                        {
                            b2 = b;
                            break;
                        }
                    }
                }
            }

            if ((b2 != 0) && (b2!=local_being) && (max_familiarity>0))
            {
                being_name(
                    gender == SEX_FEMALE,
                    GET_NAME(sim,b2), GET_FAMILY_FIRST_NAME(sim,b2), GET_FAMILY_SECOND_NAME(sim,b2), str);
                io_write(fil, str, 0);
                io_write(fil, "<br>", 1);
            }
        }
    }
}


void create_web_top_apes(
    n_file * fil,
    noble_simulation * sim,
    n_int being_profiles)
{
#ifdef PARASITES_ON
    int i,j,max;
    n_byte * eliminated;
    n_uint current_date,local_dob,age_in_years,age_in_months,age_in_days;
    char str[100];
    noble_being * b;

    io_write(fil, "        <table style=\"text-align: center; width: 70%;\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\">", 1);
    io_write(fil, "            <tbody>", 1);

    io_write(fil, "                <tr>", 1);
    io_write(fil, "                    <td style=\"vertical-align: top; text-align: left;\">", 1);
    io_write(fil, "                    <h2>Honor</h2>", 1);
    io_write(fil, "                    </td>", 1);

    io_write(fil, "                    <td style=\"vertical-align: top; text-align: left;\">", 1);
    io_write(fil, "                    <h2>Name</h2>", 1);
    io_write(fil, "                    </td>", 1);

    io_write(fil, "                    <td style=\"vertical-align: top; text-align: left;\">", 1);
    io_write(fil, "                    <h2>Age</h2>", 1);
    io_write(fil, "                    </td>", 1);

    io_write(fil, "                    <td style=\"vertical-align: top; text-align: left;\">", 1);
    io_write(fil, "                    <h2>Gender</h2>", 1);
    io_write(fil, "                    </td>", 1);
    io_write(fil, "                </tr>", 1);

    eliminated = (n_byte *)malloc(sim->num*sizeof(n_byte));
    for (i = 0; i < sim->num; i++) eliminated[i] = 0;

    max = being_profiles;
    if (sim->num < max) max = sim->num;
    for (i = 0; i < max; i++)
    {
        n_int winner = -1;
        n_byte max_honor = 0;
        for (j = 0; j < sim->num; j++)
        {
            if (eliminated[j] == 0)
            {
                b = &sim->beings[j];
                if (b->honor >= max_honor)
                {
                    winner = j;
                    max_honor = b->honor;
                }
            }
        }
        eliminated[winner] = 1;
        b = &sim->beings[winner];

        io_write(fil, "                <tr>", 1);
        io_write(fil, "                    <td style=\"vertical-align: top; text-align: left;\">", 1);
        io_write(fil, "                    ", 0);
        sprintf((char*)str,"%d", (int)(b->honor));
        io_write(fil, str, 1);
        io_write(fil, "                    </td>", 1);

        io_write(fil, "                    <td style=\"vertical-align: top; text-align: left;\">", 1);
        io_write(fil, "                    ", 0);
        io_write(fil, "                    <a href=\"ape_profile_", 0);
        io_writenumber(fil, i, 1, 0);
        io_write(fil, ".htm\">", 1);
        being_name((FIND_SEX(GET_I(b)) == SEX_FEMALE), GET_NAME(sim,b), GET_FAMILY_FIRST_NAME(sim,b), GET_FAMILY_SECOND_NAME(sim,b), str);
        io_write(fil, str, 1);
        io_write(fil, " ", 1);
        io_write(fil, "                    </a></td>", 1);

        current_date = TIME_IN_DAYS(sim->land->date);
        local_dob = TIME_IN_DAYS(GET_D(b));
        age_in_years = AGE_IN_YEARS(sim,b);
        age_in_months = ((current_date - local_dob) - (age_in_years * TIME_YEAR_DAYS)) / (TIME_YEAR_DAYS/12);
        age_in_days = (current_date - local_dob) - ((TIME_YEAR_DAYS/12) * age_in_months) - (age_in_years * TIME_YEAR_DAYS);
        io_write(fil, "                    <td style=\"vertical-align: top; text-align: left;\">", 1);
        io_write(fil, "                    ", 0);
        io_writenumber(fil, age_in_years, 1, 0);
        io_write(fil, " Years ", 0);
        io_writenumber(fil, age_in_months, 1, 0);
        io_write(fil, " Months ", 0);
        io_writenumber(fil, age_in_days, 1, 0);
        io_write(fil, " Days ", 1);
        io_write(fil, "                    </td>", 1);


        io_write(fil, "                    <td style=\"vertical-align: top; text-align: left;\">", 1);
        if (FIND_SEX(GET_I(b)) == SEX_FEMALE)
            io_write(fil, "                    Female", 1);
        else
            io_write(fil, "                    Male", 1);
        io_write(fil, "                    </td>", 1);

        io_write(fil, "                </tr>", 1);
    }

    io_write(fil, "            </tbody>", 1);
    io_write(fil, "        </table>", 1);

    free(eliminated);
#endif
}

void create_web_ape_profile(
    char * filename,
    char * profile_picture,
    char* brain_picture,
    noble_simulation * sim,
    int index)
{
    int i,j;
#ifdef EPISODIC_ON
    episodic_memory * local_episodic;
#endif
    n_file fil2;
    n_file * fil;
    n_uint current_date;
    n_uint local_dob,birth_year,birth_month;
    n_uint birth_day,age_in_years,age_in_months,age_in_days;
    noble_being * local_being;
    char str[STRING_BLOCK_SIZE];
    n_byte genome[CHROMOSOMES*8+1];
    fil2 . size = 4096;
    fil2 . location = 0;
    fil2 . data = io_new(4096);
    if(fil2 . data == 0L)
        return;
    fil = &fil2;
    local_being = &(sim->beings[index]);

    current_date = TIME_IN_DAYS(sim->land->date);
    local_dob = TIME_IN_DAYS(GET_D(local_being));
    birth_year = local_dob / TIME_YEAR_DAYS;
    birth_month =
        (local_dob - (birth_year * TIME_YEAR_DAYS)) / (TIME_YEAR_DAYS/12);
    birth_day =
        local_dob - ((TIME_YEAR_DAYS/12) * birth_month) -
        (birth_year * TIME_YEAR_DAYS);

    age_in_years = (current_date - local_dob) / TIME_YEAR_DAYS;
    age_in_months =
        ((current_date - local_dob) -
         (age_in_years * TIME_YEAR_DAYS)) / (TIME_YEAR_DAYS/12);
    age_in_days =
        (current_date - local_dob) - ((TIME_YEAR_DAYS/12) * age_in_months) -
        (age_in_years * TIME_YEAR_DAYS);

    io_write(fil, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">", 1);
    io_write(fil, "<html>", 1);
    io_write(fil, "    <head>", 1);
    io_write(fil, "        <meta content=\"text/html; charset=ISO-8859-1\" http-equiv=\"Content-Type\" HTTP-EQUIV=\"Refresh\" CONTENT=\"600;\">", 1);
    io_write(fil, "        <title>Noble Ape</title>", 1);
    io_write(fil, "    </head>", 1);
    io_write(fil, "    <body>", 1);
    io_write(fil, "        <div style=\"text-align: center;\">", 1);

    io_write(fil, "        <h2><big>", 0);

    io_write(fil, "                    ", 0);

    being_name(
        (FIND_SEX(GET_I(local_being)) == SEX_FEMALE),
        GET_NAME(sim,local_being), GET_FAMILY_FIRST_NAME(sim,local_being),
        GET_FAMILY_SECOND_NAME(sim,local_being), str);
    io_write(fil, str, 1);

    io_write(fil, "        <table style=\"text-align: center; width: 100%;\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\">", 1);
    io_write(fil, "            <tbody>", 1);

    io_write(fil, "                <tr>", 1);
    io_write(fil, "                    <td style=\"vertical-align: top; text-align: left; width: 30%;\">", 1);

    io_write(fil, "</big></h2>", 1);

    io_write(fil, "                        <a href = \"", 0);
    io_write(fil, profile_picture, 0);
    io_write(fil, "\">", 1);
    io_write(fil, "                        <img style=\"width: 80%; height: 80%;\" alt=\"\" src=\"", 0);
    io_write(fil, profile_picture,0);
    io_write(fil, "\"></a>", 1);

    io_write(fil, "                    </td>", 1);
    io_write(fil, "                    <td style=\"vertical-align: top; text-align: left; width: 70%;\">", 1);


    io_write(fil, "                        <table style=\"text-align: Left;\" border=\"0\" cellpadding=\"0\" cellspacing=\"2\">", 1);
    io_write(fil, "                            <tbody>", 1);

    io_write(fil, "                                <tr>", 1);
    io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);
    io_write(fil, "                                    Gender: ", 1);
    io_write(fil, "                                    </td>", 1);
    io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);
    if (FIND_SEX(GET_I(local_being)) == SEX_FEMALE)
        io_write(fil, "                                    Female", 1);
    else
        io_write(fil, "                                    Male", 1);
    io_write(fil, "                                    </td>", 1);
    io_write(fil, "                                </tr>", 1);


    io_write(fil, "                                <tr>", 1);
    io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);
    io_write(fil, "                                    Birthday: ", 1);
    io_write(fil, "                                    </td>", 1);
    io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);
    sprintf(
        str,"%d/%d/%d", (int)birth_day+1,
        (int)birth_month+1, (int)birth_year);
    io_write(fil, str, 1);
    io_write(fil, "                                    </td>", 1);
    io_write(fil, "                                </tr>", 1);

    io_write(fil, "                                <tr>", 1);
    io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);
    io_write(fil, "                                    Age: ", 1);
    io_write(fil, "                                    </td>", 1);
    io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);
    io_writenumber(fil, age_in_years, 1, 0);
    io_write(fil, " Years ", 0);
    io_writenumber(fil, age_in_months, 1, 0);
    io_write(fil, " Months ", 0);
    io_writenumber(fil, age_in_days, 1, 0);
    io_write(fil, " Days ", 1);
    io_write(fil, "                                    </td>", 1);
    io_write(fil, "                                </tr>", 1);

    io_write(fil, "                                <tr>", 1);
    io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);
    io_write(fil, "                                    Genome: ", 1);
    io_write(fil, "                                    </td>", 1);
    io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);

    io_write(fil, "<font face='Courier'>",0);

    for (i=0; i<2; i++)
    {
        body_genome((n_byte)i, being_genetics(local_being), genome);
        for (j = 0; j < CHROMOSOMES*8; j++)
        {
            if ((j>0) && (j%8==0))
            {
                io_write(fil, "   ", 0);
            }
            switch(genome[j])
            {
            case 'A':
            {
                sprintf(str,"<span style=\"color:red\">%c</span>", genome[j]);
                break;
            }
            case 'T':
            {
                sprintf(str,"<span style=\"color:green\">%c</span>", genome[j]);
                break;
            }
            case 'C':
            {
                sprintf(str,"<span style=\"color:blue\">%c</span>", genome[j]);
                break;
            }
            case 'G':
            {
                sprintf(str,"<span style=\"color:purple\">%c</span>", genome[j]);
                break;
            }
            }
            io_write(fil, str, 0);
        }
        if (i==0) io_write(fil, "</br>", 1);
    }

    io_write(fil, "</font>",0);
    io_write(fil, " ", 1);
    io_write(fil, "                                    </td>", 1);
    io_write(fil, "                                </tr>", 1);

#ifdef PARENTING_ON

    loop = 0;

    if (GET_MI(local_being) != 0)
    {
        io_write(fil, "                                <tr>", 1);
        io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);
        io_write(fil, "                                    Mother: ", 1);
        io_write(fil, "                                    </td>", 1);
        io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);
        b = being_find_female(sim,(n_byte2*)&local_being->mother_new_genetics);
        if (b != 0L)
        {
            being_name((FIND_SEX(GET_I(b)) == SEX_FEMALE), GET_NAME(b), GET_FAMILY_FIRST_NAME(sim,b), GET_FAMILY_SECOND_NAME(sim,b), (n_byte*)str);
            io_write(fil, str, 1);
        }
        io_write(fil, "                                    </td>", 1);
        io_write(fil, "                                </tr>", 1);
    }

    if (FIND_SEX(GET_I(local_being)) == SEX_FEMALE)
    {
        io_write(fil, "                                <tr>", 1);
        io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);
        io_write(fil, "                                    Children: ", 1);
        io_write(fil, "                                    </td>", 1);
        io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);

        for (loop = 0; loop < sim->num; loop++)
        {
            b = &(sim->beings[loop]);
            if (GET_MI(b)!=0)
            {
                if (GET_MI(b)-1 == GET_I(local_being))
                {
                    being_name((FIND_SEX(GET_I(b)) == SEX_FEMALE), GET_NAME(b), GET_FAMILY_FIRST_NAME(sim,b), GET_FAMILY_SECOND_NAME(sim,b), (n_byte*)str);
                    io_write(fil, str, 0);
                    io_write(fil, "<br>", 0);
                }
            }
        }

        io_write(fil, " ", 1);
        io_write(fil, "                                    </td>", 1);
        io_write(fil, "                                </tr>", 1);
    }

#endif

    io_write(fil, "                                <tr>", 1);
    io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);
    io_write(fil, "                                    Interested in: ", 1);
    io_write(fil, "                                    </td>", 1);
    io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);

    being_state_description(local_being->state, str);
    io_write(fil, str, 1);

    io_write(fil, "                                    </td>", 1);
    io_write(fil, "                                </tr>", 1);

#ifdef PARASITES_ON

    io_write(fil, "                                <tr>", 1);
    io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);
    io_write(fil, "                                    Honor: ", 1);
    io_write(fil, "                                    </td>", 1);
    io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);
    sprintf(str,"%d", (int)local_being->honor);
    io_write(fil, str, 1);
    io_write(fil, "                                    </td>", 1);
    io_write(fil, "                                </tr>", 1);

    io_write(fil, "                                <tr>", 1);
    io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);
    io_write(fil, "                                    Height (m): ", 1);
    io_write(fil, "                                    </td>", 1);
    io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);
    sprintf(str,"%.3f", (int)GET_BEING_HEIGHT(local_being)/1000.0f);
    io_write(fil, str, 1);
    io_write(fil, "                                    </td>", 1);
    io_write(fil, "                                </tr>", 1);

    io_write(fil, "                                <tr>", 1);
    io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);
    io_write(fil, "                                    Mass (kg): ", 1);
    io_write(fil, "                                    </td>", 1);
    io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);
    sprintf(str,"%.3f", (float)GET_M(local_being)/100.0f);
    io_write(fil, str, 1);
    io_write(fil, "                                    </td>", 1);
    io_write(fil, "                                </tr>", 1);

    io_write(fil, "                                <tr>", 1);
    io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);
    io_write(fil, "                                    Body fat (kg): ", 1);
    io_write(fil, "                                    </td>", 1);
    io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);
    sprintf(str,"%.3f", (float)GET_BODY_FAT(local_being)/100.0f);
    io_write(fil, str, 1);
    io_write(fil, "                                    </td>", 1);
    io_write(fil, "                                </tr>", 1);

    io_write(fil, "                                <tr>", 1);
    io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);
    io_write(fil, "                                    Hair (%): ", 1);
    io_write(fil, "                                    </td>", 1);
    io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);

    sprintf(str,"%d", (int)(GENE_HAIR(being_genetics(local_being))*100/16));

    io_write(fil, str, 1);
    io_write(fil, "                                    </td>", 1);
    io_write(fil, "                                </tr>", 1);

    io_write(fil, "                                <tr>", 1);
    io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);
    io_write(fil, "                                    Parasites: ", 1);
    io_write(fil, "                                    </td>", 1);
    io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);
    sprintf(str,"%d", (int)local_being->parasites);
    io_write(fil, str, 1);
    io_write(fil, "                                    </td>", 1);
    io_write(fil, "                                </tr>", 1);

#endif

    io_write(fil, "                                <tr>", 1);
    io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);
    io_write(fil, "                                    Energy: ", 1);
    io_write(fil, "                                    </td>", 1);
    io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);
    sprintf(str,"%d", (int)being_energy(local_being));
    io_write(fil, str, 1);
    io_write(fil, "                                    </td>", 1);
    io_write(fil, "                                </tr>", 1);


    io_write(fil, "                                <tr>", 1);
    io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);
    io_write(fil, "                                    Random seed: ", 1);
    io_write(fil, "                                    </td>", 1);
    io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);
    sprintf(
        str,"%d  %d",
        (int)(local_being->seed[0]), (int)(local_being->seed[1]));
    io_write(fil, str, 1);
    io_write(fil, "                                    </td>", 1);
    io_write(fil, "                                </tr>", 1);


    io_write(fil, "                                <tr>", 1);
    io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);
    io_write(fil, "                                    Brain values: ", 1);
    io_write(fil, "                                    </td>", 1);
    io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);
    sprintf(str,"Awake (%d  %d  %d)<br>Asleep (%d  %d  %d)",
            (int)(local_being->brain_state[0]),
            (int)(local_being->brain_state[1]),
            (int)(local_being->brain_state[2]),
            (int)(local_being->brain_state[3]),
            (int)(local_being->brain_state[4]),
            (int)(local_being->brain_state[5]));
    io_write(fil, str, 1);
    io_write(fil, "                                    </td>", 1);
    io_write(fil, "                                </tr>", 1);


    io_write(fil, "                                <tr>", 1);
    io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);
    io_write(fil, "                                    Speed: ", 1);
    io_write(fil, "                                    </td>", 1);
    io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);
    sprintf(str,"%d metres per min", (int)(being_speed(local_being))*10/15);
    io_write(fil, str, 1);
    io_write(fil, "                                    </td>", 1);
    io_write(fil, "                                </tr>", 1);

    io_write(fil, "                                <tr>", 1);
    io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);
    io_write(fil, "                                    Heading: ", 1);
    io_write(fil, "                                    </td>", 1);
    io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);
    sprintf(
        str,"%d degrees",
        (((int)(being_facing(local_being))*360/256) + 90) % 360);
    io_write(fil, str, 1);
    io_write(fil, "                                    </td>", 1);
    io_write(fil, "                                </tr>", 1);

    io_write(fil, "                                <tr>", 1);
    io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);
    io_write(fil, "                                    Friends: ", 1);
    io_write(fil, "                                    </td>", 1);
    io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);
    create_web_ape_friends(fil, sim, local_being, 0);
    io_write(fil, "                                    </td>", 1);
    io_write(fil, "                                </tr>", 1);

    io_write(fil, "                                <tr>", 1);
    io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);
    io_write(fil, "                                    Enemies: ", 1);
    io_write(fil, "                                    </td>", 1);
    io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);
    create_web_ape_friends(fil, sim, local_being, 1);
    io_write(fil, "                                    </td>", 1);
    io_write(fil, "                                </tr>", 1);

    if (current_date - local_dob > AGE_OF_MATURITY)
    {
        io_write(fil, "                                <tr>", 1);
        io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);
        io_write(fil, "                                    Crushes: ", 1);
        io_write(fil, "                                    </td>", 1);
        io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);
        create_web_ape_friends(fil, sim, local_being, 2);
        io_write(fil, "                                    </td>", 1);
        io_write(fil, "                                </tr>", 1);
    }

#ifdef EPISODIC_ON
    local_episodic = GET_EPI(sim, local_being);
    for (i=0; i<EPISODIC_SIZE; i++)
    {
        if (local_episodic[i].event==EVENT_MATE) break;
    }
    if (i<EPISODIC_SIZE)
    {
        io_write(fil, "                                <tr>", 1);
        io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);
        io_write(fil, "                                    Preferred mate: ", 1);
        io_write(fil, "                                    </td>", 1);
        io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);

        being_name(((
                        local_episodic[i].first_name[BEING_MET]>>8) == SEX_FEMALE),
                   local_episodic[i].first_name[BEING_MET]&255,
                   UNPACK_FAMILY_FIRST_NAME(local_episodic[i].family_name[BEING_MET]),
                   UNPACK_FAMILY_SECOND_NAME(local_episodic[i].family_name[BEING_MET]),
                   str);
        io_write(fil, "                                    ", 0);
        io_write(fil, str, 1);

        io_write(fil, "                                    </td>", 1);
        io_write(fil, "                                </tr>", 1);
    }

    io_write(fil, "                                <tr>", 1);
    io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);
    io_write(fil, "                                    Wall: ", 1);
    io_write(fil, "                                    </td>", 1);
    io_write(fil, "                                    <td style=\"vertical-align: top; text-align: left;\">", 1);

    for (i=0; i<EPISODIC_SIZE; i++)
    {
        str[0]='\0';
        episode_description(sim, local_being, i, str);
        if (io_length(str, STRING_BLOCK_SIZE)>0)
        {
			io_write(fil, str,0);
            io_write(fil, "<br>",1);
        }
    }
    io_write(fil, "                                    </td>", 1);
    io_write(fil, "                                </tr>", 1);
#endif

    io_write(fil, "                         </tbody>", 1);

    io_write(fil, "                    </td>", 1);
    io_write(fil, "                </tr>", 1);

    io_write(fil, "            </tbody>", 1);
    io_write(fil, "        </table>", 1);

    io_write(fil, "        <br>", 1);
    io_write(fil, "        <br>", 1);
    io_write(fil, "        <H2>Brain state</H2><br>", 1);

    io_write(fil, "        <a href = \"", 0);
    io_write(fil, brain_picture, 0);
    io_write(fil, "\">", 1);
    io_write(fil, "        <img style=\"width: 95%; height: 100px;\" alt=\"\" src=\"", 0);
    io_write(fil, brain_picture,0);
    io_write(fil, "\"></a>", 1);

    io_write(fil, "        <br>", 1);
    io_write(fil, "        <br>", 1);

    io_write(fil, "        </div>", 1);
    io_write(fil, "    </body>", 1);
    io_write(fil, "</html>", 1);

    (void)plat2_file_out(filename,fil2.data,fil2.location);
    plat2_free(fil2.data);
}

void export_social_graph(char * filename, noble_simulation * sim, n_int graph_type)
{
#ifdef PARASITES_ON
#ifdef PARENTING_ON
    social_link * local_social_graph;
    n_int i,index,max=1;
    int b0,b1;
    n_file	fil2;
    n_file * fil;
    noble_being * local_being;
    char str[256];
    fil2 . size = 4096;
    fil2 . location = 0;
    fil2 . data = io_new(4096);
    if(fil2 . data == 0L)
        return;
    fil = &fil2;

    io_write(fil, "digraph nobleape {", 1);
    for (index=0; index<sim->num; index++)
    {
        local_being = &(sim->beings[index]);
        local_social_graph = GET_SOC(sim, local_being);
        b0 = (GET_NAME_GENDER(local_being))|(GET_NAME_FAMILY(GET_FAMILY_FIRST_NAME(sim,local_being),GET_FAMILY_SECOND_NAME(sim,local_being))<<16);
        sprintf(str,"B%d [label=\"", b0);
        io_write(fil, str, 0);
        being_name((FIND_SEX(GET_I(local_being)) == SEX_FEMALE), GET_NAME(local_being), GET_FAMILY_FIRST_NAME(sim,local_being), GET_FAMILY_SECOND_NAME(sim,local_being), (n_byte*)str);
        io_write(fil, str, 0);
        io_write(fil, "\"];\n", 1);
        if (graph_type<3)
        {
            if (local_being->honor>max) max=local_being->honor;
        }
        else
        {
            for (i=1; i<SOCIAL_SIZE; i++)
            {
                if ((local_social_graph[i].attraction)>max)
                {
                    max=local_social_graph[i].attraction;
                }
            }
        }
    }
    for (index=0; index<sim->num; index++)
    {
        local_being = &(sim->beings[index]);
        local_social_graph = GET_SOC(sim, local_being);
        b0 = (GET_NAME_GENDER(local_being))|(GET_NAME_FAMILY(GET_FAMILY_FIRST_NAME(sim,local_being),GET_FAMILY_SECOND_NAME(sim,local_being))<<16);
        for (i=1; i<SOCIAL_SIZE; i++)
        {
            if (!((local_social_graph[i].family_name[BEING_MET] == 0) &&
                    (local_social_graph[i].first_name[BEING_MET] == 0)))
            {
                b1 = (local_social_graph[i].first_name[BEING_MET])|
                     (local_social_graph[i].family_name[BEING_MET]<<16);

                switch(graph_type)
                {
                case 0:
                {
                    sprintf(str,"B%d -> B%d [color=\"", b0, b1);
                    io_write(fil, (n_byte *)str, 0);
                    if ((local_social_graph[i].attraction)==0)
                    {
                        if ((local_social_graph[i].friend_or_foe)
                                >=127)
                        {
                            io_write(fil, "green", 0);
                        }
                        else
                        {
                            io_write(fil, "red", 0);
                        }
                    }
                    else
                    {
                        io_write(fil, "blue", 0);
                    }
                    sprintf(str,"\", penwidth=%d];\n", (int)(1+((int)(local_being->honor)*10/(int)max)));
                    io_write(fil, str, 1);

                    break;
                }
                case 1:
                {
                    if ((local_social_graph[i].friend_or_foe)
                            >=127)
                    {
                        sprintf(str,"B%d -> B%d [color=\"", b0, b1);
                        io_write(fil, str, 0);
                        io_write(fil, "green", 0);
                        sprintf(str,"\", penwidth=%d];\n",
                                (int)(1+((int)(local_being->honor)*10/
                                         (int)max)));
                        io_write(fil, str, 1);
                    }
                    break;
                }
                case 2:
                {
                    if ((local_social_graph[i].friend_or_foe)<127)
                    {
                        sprintf(str,"B%d -> B%d [color=\"", b0, b1);
                        io_write(fil, str, 0);
                        io_write(fil, "red", 0);
                        sprintf(str,"\", penwidth=%d];\n",
                                (int)(1+((int)(local_being->honor)*10/
                                         (int)max)));
                        io_write(fil, str, 1);
                    }
                    break;
                }
                case 3:
                {
                    if ((local_social_graph[i].attraction)>0)
                    {
                        sprintf(str,"B%d -> B%d [color=\"", b0, b1);
                        io_write(fil, str, 0);
                        io_write(fil, "purple", 0);
                        sprintf(str,"\", penwidth=%d];\n",
                                (int)(1+((int)(
                                             local_social_graph[i].attraction)*
                                         10/(int)max)));
                        io_write(fil, str, 1);
                    }
                    break;
                }
                }

            }
        }
    }
    io_write(fil, "}", 1);

    (void)plat2_file_out(filename,fil2.data,fil2.location);
    plat2_free(fil2.data);
#endif
#endif
}

void create_web_title(
    char * filename, char * title, char * subtitle,
    char * title_ape_map, char * fil_ape_map, char * link_ape_map,
    char * title_weather_map, char * fil_weather_map, char * link_weather_map,
    char * title_vegetation_map, char * fil_vegetation_map, char * link_vegetation_map,
    char * title_territory_map, char * fil_territory_map, char * link_territory_map,
    char * title_ideosphere, char * fil_ideosphere, char * link_ideosphere,
    char * title_braincode, char * fil_braincode, char * link_braincode,
    char * title_relationship_matrix, char * fil_relationship_matrix, char * link_relationship_matrix,
    char * title_genespace, char * fil_genespace, char * link_genespace,
    char * title_genepool, char * fil_genepool, char * link_genepool,
    char * title_pathogens, char * fil_pathogens, char * link_pathogens,
    char * title_population, char * fil_population,
    char * title_parasite_population, char * fil_parasite_population,
    char * title_average_age, char * fil_average_age,
    char * title_honor_distribution, char * fil_honor_distribution,
    char * title_demographics, char * fil_demographics,
    char * title_heights, char * fil_heights,
    char * fil_socialgraph, char * fil_mategraph,
    char * footer_text, noble_simulation * sim, char * honors_title, char * births_title,
    n_int land_random_seed, n_int being_profiles)
{
    int i, females;
    n_byte hr,min;
    n_uint today;
    char str[50];
    n_file	fil2;
    n_file * fil;
    fil2 . size = 4096;
    fil2 . location = 0;
    fil2 . data = io_new(4096);
    if(fil2 . data == 0L)
        return;
    fil = &fil2;

    io_write(fil, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">", 1);
    io_write(fil, "<html>", 1);
    io_write(fil, "    <head>", 1);
    io_write(fil, "        <meta content=\"text/html; charset=ISO-8859-1\" http-equiv=\"Content-Type\" HTTP-EQUIV=\"Refresh\" CONTENT=\"600;\">", 1);
    io_write(fil, "        <title>Noble Ape</title>", 1);
    io_write(fil, "    </head>", 1);
    io_write(fil, "    <body>", 1);
    io_write(fil, "        <div style=\"text-align: center;\">", 1);

    if (strcmp(title,"") != 0)
    {
        io_write(fil, "        <h2><big>", 0);
        io_write(fil, title, 0);
        io_write(fil, "</big></h2>", 1);
    }
    else
    {
        io_write(fil, "        <a href=\"http://nobleape.com/sim/\" target=\"_blank\"><img src=\"http://nobleape.com/logo/160x40.jpg\" border=\"0\"></a>", 1);
        io_write(fil, "        <br><br>", 1);
    }

    io_write(fil, subtitle, 1);
    io_write(fil, "        <br><br>", 1);

    sprintf(str, "Landscape: %d<br>", (int)land_random_seed);
    io_write(fil, str, 1);
    io_write(fil, "Time: ", 0);
    hr = (sim->land->time % TIME_DAY_MINUTES) / 60;
    if (hr < 10) io_write(fil, "0", 0);
    io_writenumber(fil, hr, 1, 0);
    io_write(fil, ":", 0);
    min = (sim->land->time % TIME_DAY_MINUTES) % 60;
    if (min < 10) io_write(fil, "0", 0);
    io_writenumber(fil, min, 1, 0);
    today = TIME_IN_DAYS(sim->land->date);
    io_write(fil, "   Day: ", 0);
    io_writenumber(fil, today % TIME_YEAR_DAYS, 1, 0);
    io_write(fil, "   Year: ", 0);
    io_writenumber(fil, today / TIME_YEAR_DAYS, 1, 0);
    io_write(fil, "   Century: ", 0);
    io_writenumber(fil, sim->land->date[1], 1, 1);

    io_write(fil, "<br>", 1);

    io_write(fil, "    Population: ", 0);
    io_writenumber(fil, sim->num, 1, 0);

    females=0;
    for (i = 0; i < sim->num; i++)
    {
        if (FIND_SEX(GET_I(&sim->beings[i])) == SEX_FEMALE) females++;
    }

    io_write(fil, "    Male: ", 0);
    io_writenumber(fil, sim->num - females, 1, 0);
    io_write(fil, "    Female: ", 0);
    io_writenumber(fil, females, 1, 0);


    io_write(fil, "        <br><br>", 1);
    io_write(fil, "        <table style=\"text-align: left; width: 100%;\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\">", 1);
    io_write(fil, "            <tbody>", 1);

    io_write(fil, "                <tr>", 1);
    io_write(fil, "                <td style=\"vertical-align: top; text-align: center;\">", 1);
    io_write(fil, "                <h2>", 0);
    io_write(fil, title_ape_map, 0);
    io_write(fil, "</h2>", 1);
    io_write(fil, "                <a href = \"", 0);
    io_write(fil, link_ape_map, 0);
    io_write(fil, "\">", 1);
    io_write(fil, "                <img style=\"width: 70%; height: 70%;\" alt=\"\" src=\"",0);
    io_write(fil, fil_ape_map,0);
    io_write(fil, "\"></a><br>", 1);
    io_write(fil, "                </td>", 1);
    io_write(fil, "                <td style=\"vertical-align: top; text-align: center;\">", 1);
    io_write(fil, "                <h2>", 0);
    io_write(fil, title_weather_map, 0);
    io_write(fil, "</h2>", 1);
    io_write(fil, "                <a href = \"", 0);
    io_write(fil, link_weather_map, 0);
    io_write(fil, "\">", 1);
    io_write(fil, "                <img style=\"width: 70%; height: 70%;\" alt=\"\" src=\"", 0);
    io_write(fil, fil_weather_map,0);
    io_write(fil, "\"></a><br>", 1);
    io_write(fil, "                </td>", 1);
    io_write(fil, "                </tr>", 1);


    io_write(fil, "                <tr>", 1);

    io_write(fil, "                <td style=\"vertical-align: top; text-align: center;\">", 1);
    io_write(fil, "                <h2>", 0);
    io_write(fil, title_vegetation_map, 0);
    io_write(fil, "</h2>", 1);
    io_write(fil, "                <a href = \"", 0);
    io_write(fil, link_vegetation_map, 0);
    io_write(fil, "\">", 1);
    io_write(fil, "                <img style=\"width: 70%; height: 70%;\" alt=\"\" src=\"",0);
    io_write(fil, fil_vegetation_map,0);
    io_write(fil, "\"></a><br>", 1);
    io_write(fil, "                </td>", 1);

    io_write(fil, "                <td style=\"vertical-align: top; text-align: center;\">", 1);
    io_write(fil, "                <h2>", 0);
    io_write(fil, title_territory_map, 0);
    io_write(fil, "</h2>", 1);
    io_write(fil, "                <a href = \"", 0);
    io_write(fil, link_territory_map, 0);
    io_write(fil, "\">", 1);
    io_write(fil, "                <img style=\"width: 70%; height: 70%;\" alt=\"\" src=\"",0);
    io_write(fil, fil_territory_map,0);
    io_write(fil, "\"></a><br>", 1);
    io_write(fil, "                </td>", 1);

    io_write(fil, "                </tr>", 1);
    io_write(fil, "                <tr>", 1);

    io_write(fil, "                <td style=\"vertical-align: top; text-align: center;\">", 1);
    io_write(fil, "                <h2>", 0);
    io_write(fil, title_ideosphere, 0);
    io_write(fil, "</h2>", 1);
    io_write(fil, "                <a href = \"", 0);
    io_write(fil, link_ideosphere, 0);
    io_write(fil, "\">", 1);
    io_write(fil, "                <img style=\"width: 70%; height: 70%;\" alt=\"\" src=\"",0);
    io_write(fil, fil_ideosphere,0);
    io_write(fil, "\"></a><br>", 1);
    io_write(fil, "                </td>", 1);

    io_write(fil, "                <td style=\"vertical-align: top; text-align: center;\">", 1);
    io_write(fil, "                <h2>", 0);
    io_write(fil, title_genepool, 0);
    io_write(fil, "</h2>", 1);
    io_write(fil, "                <a href = \"", 0);
    io_write(fil, link_genepool, 0);
    io_write(fil, "\">", 1);
    io_write(fil, "                <img style=\"width: 70%; height: 70%;\" alt=\"\" src=\"",0);
    io_write(fil, fil_genepool,0);
    io_write(fil, "\"></a><br>", 1);
    io_write(fil, "                </td>", 1);


    io_write(fil, "                <tr>", 1);
    io_write(fil, "                <td style=\"vertical-align: top; text-align: center;\">", 1);
    io_write(fil, "                <h2>", 0);
    io_write(fil, title_pathogens, 0);
    io_write(fil, "</h2>", 1);
    io_write(fil, "                <a href = \"", 0);
    io_write(fil, link_pathogens, 0);
    io_write(fil, "\">", 1);
    io_write(fil, "                <img style=\"width: 70%; height: 70%;\" alt=\"\" src=\"",0);
    io_write(fil, fil_pathogens,0);
    io_write(fil, "\"></a><br>", 1);
    io_write(fil, "                </td>", 1);

    io_write(fil, "                <td style=\"vertical-align: top; text-align: center;\">", 1);

    io_write(fil, "                <h2>", 0);
    io_write(fil, title_braincode, 0);
    io_write(fil, "</h2>", 1);
    io_write(fil, "                <a href = \"", 0);
    io_write(fil, link_braincode, 0);
    io_write(fil, "\">", 1);
    io_write(fil, "                <img style=\"width: 70%; height: 70%;\" alt=\"\" src=\"",0);
    io_write(fil, fil_braincode,0);
    io_write(fil, "\"></a><br>", 1);

    io_write(fil, "                </td>", 1);


    io_write(fil, "                </tr>", 1);

    io_write(fil, "                <tr>", 1);

    io_write(fil, "                <td style=\"vertical-align: top; text-align: center;\">", 1);
    io_write(fil, "                <h2>", 0);
    io_write(fil, title_relationship_matrix, 0);
    io_write(fil, "</h2>", 1);
    io_write(fil, "                <a href = \"", 0);
    io_write(fil, link_relationship_matrix, 0);
    io_write(fil, "\">", 1);
    io_write(fil, "                <img style=\"width: 70%; height: 70%;\" alt=\"\" src=\"",0);
    io_write(fil, fil_relationship_matrix,0);
    io_write(fil, "\"></a><br>", 1);
    io_write(fil, "                </td>", 1);

    io_write(fil, "                <td style=\"vertical-align: top; text-align: center;\">", 1);
    io_write(fil, "                <h2>", 0);
    io_write(fil, title_genespace, 0);
    io_write(fil, "</h2>", 1);
    io_write(fil, "                <a href = \"", 0);
    io_write(fil, link_genespace, 0);
    io_write(fil, "\">", 1);
    io_write(fil, "                <img style=\"width: 70%; height: 70%;\" alt=\"\" src=\"",0);
    io_write(fil, fil_genespace,0);
    io_write(fil, "\"></a><br>", 1);
    io_write(fil, "                </td>", 1);

    io_write(fil, "                </tr>", 1);
    io_write(fil, "            </tbody>", 1);
    io_write(fil, "        </table>", 1);


    io_write(fil, "                            <h2>", 0);
    io_write(fil, title_population, 0);
    io_write(fil, "</h2>", 1);
    io_write(fil, "                            <a href = \"", 0);
    io_write(fil, "\">", 1);
    io_write(fil, "                            <img style=\"width: 60%; height: 20%;\" alt=\"\" src=\"", 0);
    io_write(fil, fil_population,0);
    io_write(fil, "\"></a><br>", 1);
    io_write(fil, "                            <h2>", 0);
    io_write(fil, title_parasite_population, 0);
    io_write(fil, "</h2>", 1);
    io_write(fil, "                            <a href = \"", 0);
    io_write(fil, "\">", 1);
    io_write(fil, "                            <img style=\"width: 60%; height: 20%;\" alt=\"\" src=\"", 0);
    io_write(fil, fil_parasite_population,0);
    io_write(fil, "\"></a><br>", 1);
    io_write(fil, "                            <h2>", 0);
    io_write(fil, title_average_age, 0);
    io_write(fil, "</h2>", 1);
    io_write(fil, "                            <a href = \"", 0);
    io_write(fil, "\">", 1);
    io_write(fil, "                            <img style=\"width: 60%; height: 20%;\" alt=\"\" src=\"", 0);
    io_write(fil, fil_average_age,0);
    io_write(fil, "\"></a><br>", 1);

    io_write(fil, "                <br>Download the <a href=\"noble_ape.csv\">statistics csv file</a>, ", 0);
    io_write(fil, "<a href=\"socialgraph.dot\">social graph</a> or ", 0);
    io_write(fil, "<a href=\"mategraph.dot\">mate graph</a>", 1);
    io_write(fil, "                </td>", 1);
    io_write(fil, "                <td>", 1);
    io_write(fil, "                </td>", 1);
    io_write(fil, "                </tr>", 1);


    io_write(fil, "                <tr>", 1);
    io_write(fil, "                <td style=\"vertical-align: top; text-align: center;\"><br>", 1);
    io_write(fil, "                </td>", 1);
    io_write(fil, "                <td style=\"vertical-align: top; text-align: center;\"><br>", 1);
    io_write(fil, "                </td>", 1);
    io_write(fil, "                </tr>", 1);


    create_alphas(fil, sim);

    io_write(fil, "        <h2>", 0);
    io_write(fil, honors_title, 0);
    io_write(fil, "</h2>", 1);

    io_write(fil, "        <center>", 1);
    create_web_top_apes(fil, sim, being_profiles);
    create_births(fil, sim, births_title, being_profiles);
    io_write(fil, "        </center>", 1);

    io_write(fil, "        <br>", 1);
    io_write(fil, "        <br>", 1);


    io_write(fil, "        <table style=\"text-align: left; width: 100%;\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\">", 1);
    io_write(fil, "            <tbody>", 1);

    io_write(fil, "                <tr>", 1);
    io_write(fil, "                <td style=\"vertical-align: top; text-align: center;\">", 1);

    io_write(fil, "        <H2>", 0);
    io_write(fil, title_honor_distribution,0);
    io_write(fil, "</H2><br>",1);
    io_write(fil, "        <a href=\"", 0);
    io_write(fil, fil_honor_distribution, 0);
    io_write(fil, "\"><img src=\"",0);
    io_write(fil, fil_honor_distribution, 0);
    io_write(fil, "\"/></a>", 1);
    io_write(fil, "                </td>", 1);

    io_write(fil, "                <td style=\"vertical-align: top; text-align: center;\">", 1);
    io_write(fil, "        <H2>", 0);
    io_write(fil, title_demographics,0);
    io_write(fil, "</H2><br>",1);
    io_write(fil, "        <a href=\"", 0);
    io_write(fil, fil_demographics, 0);
    io_write(fil, "\"><img src=\"",0);
    io_write(fil, fil_demographics, 0);
    io_write(fil, "\"/></a>", 1);
    io_write(fil, "                </td>", 1);

    io_write(fil, "                <td style=\"vertical-align: top; text-align: center;\">", 1);
    io_write(fil, "        <H2>", 0);
    io_write(fil, title_heights,0);
    io_write(fil, "</H2><br>",1);
    io_write(fil, "        <a href=\"", 0);
    io_write(fil, fil_heights, 0);
    io_write(fil, "\"><img src=\"",0);
    io_write(fil, fil_heights, 0);
    io_write(fil, "\"/></a>", 1);
    io_write(fil, "                </td>", 1);

    io_write(fil, "                </tr>", 1);
    io_write(fil, "            </tbody>", 1);
    io_write(fil, "        </table>", 1);

    io_write(fil, "        <br>", 1);
    io_write(fil, "        <br>", 1);

    io_write(fil, "        You can download the simulation file <a href=\"realtime.txt\">here</a> (right click and <i>Save As</i>)<br>", 1);
    io_write(fil, footer_text, 1);
    io_write(fil, "        </div>", 1);

    io_write(fil, "    </body>", 1);
    io_write(fil, "</html>", 1);

    (void)plat2_file_out(filename,fil2.data,fil2.location);
    plat2_free(fil2.data);
}

static void load_tile(char* file_name, n_int index)
{
    n_int x,y,tile_x,tile_y,n,n2;
    unsigned char * buffer;
    png_t png_ptr;
    FILE * fp;

    /* does the file exist? */
    fp=fopen(file_name,"r");
    if (fp==NULL)
    {
        fprintf(stderr,"Unable to read file %s\n",file_name);
        return;
    }
    fclose(fp);

    /* read the imege */
    buffer = read_png_file(file_name,&png_ptr);

    if (buffer!=NULL)
    {
        for (tile_y=0; tile_y < TILE_WIDTH; tile_y++)
        {
            y = tile_y * (png_ptr.height-1) / TILE_WIDTH;
            for (tile_x=0; tile_x < TILE_WIDTH; tile_x++)
            {
                x = tile_x * (png_ptr.width-1) / TILE_WIDTH;
                n = (tile_y*TILE_WIDTH + tile_x)*3;
                n2 = (y*png_ptr.width+x)*png_ptr.bpp;
                map_tile[index][n++] = buffer[n2];
                map_tile[index][n++] = buffer[n2+1];
                map_tile[index][n++] = buffer[n2+2];
            }
        }
    }

    /* cleanup memory allocation */
    free(buffer);
}

/* return ape size as a value in the range 0-100 */
static n_byte get_ape_size(n_land * local_land, noble_being * local_being)
{
    return (n_byte)(30+(GET_H(local_being)*70/BEING_MAX_HEIGHT));
}

void draw_som(char * filename, n_byte * map, n_int img_width, n_int img_height, n_byte * input)
{
#ifdef SOM_ON
    unsigned char * buffer = (unsigned char*)malloc(img_width * img_height * 3);
    som_draw(buffer, img_width, img_height, map, input);
    write_png_file(filename, img_width, img_height, buffer);
    free(buffer);
#endif
}

void som_test()
{
#ifdef SOM_ON
    const int dim = 16;
    const int radius = 1;
    const n_byte input_bits = 16;
    const int inputs = input_bits>>3;
    n_byte map[dim*dim*inputs+4];
    n_byte2 seed[2];
    n_byte input[inputs];
    int i;
    n_int reinforcement = 4000;

    seed[0] =1;
    seed[1] =2;

    som_init(map, dim, radius, input_bits, seed);

    for (i=0; i<10000; i++)
    {
        input[0] = (n_byte)seed[0];
        input[1] = (n_byte)seed[1];
        io_random(seed);
        som_cycle(map, input, reinforcement, seed);
    }
    io_random(seed);
    input[0] = (n_byte)seed[0];
    input[1] = (n_byte)seed[1];
    draw_som("som.png", map, 512, 512, input);
#endif
}

void draw_brain(char * filename, noble_simulation * sim, int index, n_int img_width, n_int img_height)
{
    const int F_Y = 32;
    const int F_Z = 1024;
    int x, y, n=0;

    noble_being * b = &(sim->beings[index]);
    n_byte * brain = GET_B(sim, b);

    if (brain != 0L)
    {

        unsigned char * buffer = (unsigned char*)malloc(img_width * img_height * 3);

        n=0;
        for (y = 0; y < img_height; y++)
        {
            int fy = y * (F_Y-1) / img_height;
            for (x = 0; x < img_width; x++, n += 3)
            {
                int fz = x * (F_Z-1) / img_width;
                int n2 = fz*F_Y + fy;
                buffer[n] = brain[n2];
                buffer[n+1] = brain[n2];
                buffer[n+2] = 0;
            }
        }

        write_png_file(filename, img_width, img_height, buffer);

        free(buffer);

    }
}

void draw_tile(unsigned char* buffer, n_int img_width, n_int index, n_int x, n_int y, n_byte opacity, n_byte scale_percent)
{
    n_byte br=0, bg=0, bb=0, c;
    n_int px,py,px2,py2,n,n2;
    n_int half = TILE_WIDTH/2;
    if ((x - half >= 0) && (x - half + TILE_WIDTH < img_width) &&
            (y - half >= 0) && (y - half + TILE_WIDTH < img_width))
    {

        for (py = 0; py < TILE_WIDTH; py++)
        {
            for (px = 0; px < TILE_WIDTH; px++)
            {
                n = (py*TILE_WIDTH + px)*3;
                if ((px == 0) && (py == 0))
                {
                    br = map_tile[index][n];
                    bg = map_tile[index][n+1];
                    bb = map_tile[index][n+2];
                }
                else
                {
                    if (!((br == map_tile[index][n]) &&
                            (bg == map_tile[index][n+1]) &&
                            (bb == map_tile[index][n+2])))
                    {

                        px2 = (px-half)*scale_percent/100;
                        py2 = (py-half)*scale_percent/100;
                        n2 = ((py2+y)*img_width + px2+x)*3;
                        for (c = 0; c < 3; c++)
                        {
                            buffer[n2+c] =
                                (unsigned char)(((buffer[n2+c]*(255-opacity)) +
                                                 (map_tile[index][n+c]*opacity))/255);
                        }
                    }
                }
            }
        }
    }

}

void draw_beings(unsigned char* buffer, n_int img_width, n_int num, noble_being * beings, n_land * local_land)
{
    n_int i,px,py;
    n_byte scale_percent;

    load_tile(FILE_APE_MALE_AWAKE, TILE_APE_MALE_AWAKE);
    load_tile(FILE_APE_FEMALE_AWAKE, TILE_APE_FEMALE_AWAKE);
    load_tile(FILE_APE_MALE_ASLEEP, TILE_APE_MALE_ASLEEP);
    load_tile(FILE_APE_FEMALE_ASLEEP, TILE_APE_FEMALE_ASLEEP);
    load_tile(FILE_APE_MALE_SWIMMING, TILE_APE_MALE_SWIMMING);
    load_tile(FILE_APE_FEMALE_SWIMMING, TILE_APE_FEMALE_SWIMMING);
    load_tile(FILE_APE_MALE_MOVING, TILE_APE_MALE_MOVING);
    load_tile(FILE_APE_FEMALE_MOVING, TILE_APE_FEMALE_MOVING);
    load_tile(FILE_APE_MALE_EATING, TILE_APE_MALE_EATING);
    load_tile(FILE_APE_FEMALE_EATING, TILE_APE_FEMALE_EATING);
    load_tile(FILE_APE_MALE_REPRODUCING, TILE_APE_MALE_REPRODUCING);
    load_tile(FILE_APE_FEMALE_REPRODUCING, TILE_APE_FEMALE_REPRODUCING);
    load_tile(FILE_APE_MALE_HUNGRY, TILE_APE_MALE_HUNGRY);
    load_tile(FILE_APE_FEMALE_HUNGRY, TILE_APE_FEMALE_HUNGRY);
    load_tile(FILE_APE_MALE_SPEAKING, TILE_APE_MALE_SPEAKING);
    load_tile(FILE_APE_FEMALE_SPEAKING, TILE_APE_FEMALE_SPEAKING);
    load_tile(FILE_APE_MALE_SHOUTING, TILE_APE_MALE_SHOUTING);
    load_tile(FILE_APE_FEMALE_SHOUTING, TILE_APE_FEMALE_SHOUTING);
    load_tile(FILE_APE_MALE_GROOMING, TILE_APE_MALE_GROOMING);
    load_tile(FILE_APE_FEMALE_GROOMING, TILE_APE_FEMALE_GROOMING);
    load_tile(FILE_APE_MALE_SUCKLING, TILE_APE_MALE_SUCKLING);
    load_tile(FILE_APE_FEMALE_SUCKLING, TILE_APE_FEMALE_SUCKLING);
    load_tile(FILE_APE_MALE_SHOWFORCE, TILE_APE_MALE_SHOWFORCE);
    load_tile(FILE_APE_FEMALE_SHOWFORCE, TILE_APE_FEMALE_SHOWFORCE);
    load_tile(FILE_APE_MALE_ATTACK, TILE_APE_MALE_ATTACK);
    load_tile(FILE_APE_FEMALE_ATTACK, TILE_APE_FEMALE_ATTACK);

    for (i=0; i <num; i++)
    {
        noble_being * local_being = (noble_being *)(&(beings[i]));
        px = (n_int)(APESPACE_TO_MAPSPACE(being_location_x(local_being)))*img_width/MAP_DIMENSION;
        py = (n_int)(APESPACE_TO_MAPSPACE(being_location_y(local_being)))*img_width/MAP_DIMENSION;
        if ((px >= 0) && (px < img_width) &&
                (py >= 0) && (py < img_width))
        {
            n_int index = TILE_APE_MALE_AWAKE;
            switch(local_being->state)
            {
            case BEING_STATE_ASLEEP:
            {
                index = TILE_APE_MALE_ASLEEP;
                break;
            }
            case BEING_STATE_AWAKE:
            {
                index = TILE_APE_MALE_AWAKE;
                break;
            }
            case BEING_STATE_HUNGRY:
            {
                index = TILE_APE_MALE_SWIMMING;
                break;
            }
            case BEING_STATE_EATING:
            {
                index = TILE_APE_MALE_EATING;
                break;
            }
            case BEING_STATE_MOVING:
            {
                index = TILE_APE_MALE_MOVING;
                break;
            }
            case BEING_STATE_SPEAKING:
            {
                index = TILE_APE_MALE_SPEAKING;
                break;
            }
            case BEING_STATE_SHOUTING:
            {
                index = TILE_APE_MALE_SHOUTING;
                break;
            }
            case BEING_STATE_REPRODUCING:
            {
                index = TILE_APE_MALE_REPRODUCING;
                break;
            }
            }
            if (FIND_SEX(GET_I(local_being)) == SEX_FEMALE)
            {
                index++;
            }

            scale_percent = get_ape_size(local_land, local_being);
            draw_tile(buffer, img_width, index, px, py, 255, scale_percent);


        }
    }
}

void draw_cloud(unsigned char * buffer, n_int img_width, n_int p, n_int tx, n_int ty, n_int bx, n_int by, n_byte opacity)
{
    n_int px,py,n,r;
    for (py = ty; py < by; py++)
    {
        for (px = tx; px < bx; px++)
        {
            r = rand() % 30000;
            if (r < p)
            {
                n = (py*img_width + px)*3;
                buffer[n] = ((buffer[n]*(255-opacity)) + (255*opacity))/255;
                buffer[n+1] = ((buffer[n+1]*(255-opacity)) + (255*opacity))/255;
                buffer[n+2] = ((buffer[n+2]*(255-opacity)) + (255*opacity))/255;
            }
        }
    }

}

void draw_vegetation(char * filename, n_int img_width, n_land * local_land, n_weather * local_weather)
{
    n_int seaweed,rockpool,beach,grass,trees,bush;
    n_int px,py,map_x,map_y,mx,my,n,edible,az;
    unsigned char * buffer;
    buffer = (unsigned char*)malloc(img_width * img_width * 3);

    for (py = 1; py < img_width-1; py++)
    {
        map_y = py * MAP_DIMENSION / img_width;
        my = py * MAP_APE_RESOLUTION_SIZE / img_width;
        for (px = 1; px < img_width-1; px++)
        {
            map_x = px * MAP_DIMENSION / img_width;
            mx = px * MAP_APE_RESOLUTION_SIZE / img_width;

            n = (py*img_width + px)*3;

            az = QUICK_LAND(local_land, map_x, map_y);
            if (az < local_land->tide_level)
            {
                edible = 0;
            }
            else
            {
#ifdef VEGETATION_ON
                edible = local_land->edible[map_y*MAP_DIMENSION+map_x];
#else
                edible = 200;
#endif
            }

            seaweed = land_operator_interpolated(local_land, local_weather, mx, my, (n_byte*)&operators[VARIABLE_BIOLOGY_SEAWEED - VARIABLE_BIOLOGY_AREA]);
            rockpool = land_operator_interpolated(local_land, local_weather, mx, my, (n_byte*)&operators[VARIABLE_BIOLOGY_ROCKPOOL - VARIABLE_BIOLOGY_AREA]);
            beach = land_operator_interpolated(local_land, local_weather, mx, my, (n_byte*)&operators[VARIABLE_BIOLOGY_BEACH - VARIABLE_BIOLOGY_AREA]);
            buffer[n] = 0;
            buffer[n+1] = 0;
            buffer[n+2] = 0;
            if ((seaweed==0) && (rockpool==0) && (beach==0))
            {
                grass = land_operator_interpolated(local_land, local_weather, mx, my, (n_byte*)&operators[VARIABLE_BIOLOGY_GRASS - VARIABLE_BIOLOGY_AREA])+OFFSET_GRASS;
                trees = land_operator_interpolated(local_land, local_weather, mx, my, (n_byte*)&operators[VARIABLE_BIOLOGY_TREE - VARIABLE_BIOLOGY_AREA]);
                bush = land_operator_interpolated(local_land, local_weather, mx, my, (n_byte*)&operators[VARIABLE_BIOLOGY_BUSH - VARIABLE_BIOLOGY_AREA])+OFFSET_BUSH;
                grass += LAND_DITHER(grass, trees, bush);
                if ((trees > grass) && (trees > bush))
                {
                    buffer[n] = (unsigned char)(edible>>2);
                    buffer[n+1] = (unsigned char)(edible>>1);
                }
                else
                {
                    if (grass > bush)
                    {
                        buffer[n+1] = (unsigned char)(edible);
                    }
                    else
                    {
                        buffer[n+2] = (unsigned char)(edible>>2);
                        buffer[n] = (unsigned char)(edible>>1);
                    }
                }
            }
            else
            {
                beach += LAND_DITHER(seaweed, rockpool, beach);
                if ((seaweed > rockpool) && (seaweed > beach))
                {
                    buffer[n+2] = (unsigned char)(edible);
                }
                else
                {
                    if (rockpool > beach)
                    {
                        buffer[n] = (unsigned char)(edible);
                    }
                    else
                    {
                        buffer[n] = 200;
                        buffer[n+1] = 200;
                    }
                }
            }

        }
    }

    write_png_file(filename, img_width, img_width, buffer);

    free(buffer);
}

void draw_weather(unsigned char * buffer, n_int img_width, n_weather * local_weather, n_int weather_dimension, n_int symbol_grid_dimension, n_byte show_wind, n_byte show_cloud, n_byte show_symbols)
{
    n_int step,px,py,px2,py2,map_x,map_y,x,y,p,n,i;
    step = weather_dimension/symbol_grid_dimension;
    if (step < 1) step = 1;

    /* show wind directions */
    if (show_wind != 0)
    {
        n_int mag, w_dx=0, w_dy=0;
        for (map_y = 2; map_y < weather_dimension-2; map_y++)
        {
            py = (map_y*2+1)*img_width / (weather_dimension*2);
            for (map_x = 2; map_x < weather_dimension-2; map_x++)
            {
                px = (map_x*2+1)*img_width / (weather_dimension*2);

                weather_wind_vector(local_weather, map_x, map_y, &w_dx, &w_dy);
                mag = 0;
                if (w_dx > 0)
                    mag += w_dx;
                else
                    mag -= w_dx;
                if (w_dy > 0)
                    mag += w_dy;
                else
                    mag -= w_dy;

                if (mag > 10)
                {
                    mag /= 2048;
                    w_dx /= 2048;
                    w_dy /= 2048;
                    i = 0;
                    for (i = 0; i < mag; i++)
                    {
                        x = px + w_dx*i/mag;
                        y = py + w_dy*i/mag;
                        if ((x < 0) || (y < 0) || (x >= img_width) || (y >= img_width)) break;
                        n = (y*img_width + x)*3;
                        buffer[n] = 0;
                        buffer[n+1] = 0;
                        buffer[n+2] = 0;
                    }
                }
            }
        }
    }

    if (show_symbols == 0)
    {
        if (show_cloud != 0)
        {
            for (map_y = 0; map_y < weather_dimension-1; map_y++)
            {
                py = map_y*img_width / weather_dimension;
                for (map_x = 0; map_x < weather_dimension-1; map_x++)
                {
                    px = map_x*img_width / weather_dimension;
                    p = weather_pressure(local_weather, map_x, map_y);

                    px2 = (map_x+1)*img_width / weather_dimension;
                    py2 = (map_y+1)*img_width / weather_dimension;

                    if (p > WEATHER_RAIN)
                    {
                        draw_cloud(buffer, img_width, p - WEATHER_CLOUD, px, py, px2, py2, 80);
                    }
                    else
                    {
                        if (p > WEATHER_CLOUD)
                        {
                            draw_cloud(buffer, img_width, p - WEATHER_CLOUD, px, py, px2, py2, 70);
                        }
                    }
                }
            }
        }
    }
    else
    {

        load_tile(FILE_RAIN, TILE_RAIN);
        load_tile(FILE_CLOUD, TILE_CLOUD);
        load_tile(FILE_SUNNY_INTERVALS, TILE_SUNNY_INTERVALS);
        load_tile(FILE_SUNSHINE, TILE_SUNSHINE);

        /* show weather symbols */
        for (map_y = 0; map_y < weather_dimension; map_y+=step)
        {
            py = (map_y*2+1)*img_width / (weather_dimension*2);
            for (map_x = 0; map_x < weather_dimension; map_x+=step)
            {
                px = (map_x*2+1)*img_width / (weather_dimension*2);
                p = weather_pressure(local_weather, map_x, map_y);

                if (p > WEATHER_RAIN)
                {
                    draw_tile(buffer, img_width, TILE_RAIN, px, py, 210, 100);
                }
                else
                {
                    if (p > WEATHER_CLOUD)
                    {
                        draw_tile(buffer, img_width, TILE_CLOUD, px, py, 128, 100);
                    }
                    else
                    {
                        if (p > WEATHER_CLOUD * 90/100)
                        {
                            draw_tile(buffer, img_width, TILE_SUNNY_INTERVALS, px, py, 128, 100);
                        }
                        else
                        {
                            if ((p > 0) && (p < WEATHER_CLOUD * 20/100))
                            {
                                draw_tile(buffer, img_width, TILE_SUNSHINE, px, py, 200, 100);
                            }
                        }
                    }
                }

            }
        }
    }

}

void landscape_png(
    n_int map_tx, n_int map_ty, n_int map_bx, n_int map_by, /* observation window */
    char* img_fil, /* filename to save as */
    n_land * local_land, /* landscape data */
    n_weather * local_weather, /* the weather */
    n_int num, /* number of beings */
    noble_being * beings, /* the apes */
    n_int img_width, /* width of the image to save as */
    n_int img_width_small, /* smaller width for drawing individual apes */
    n_int weather_dimension,  /* dimension of the weather map, typically 128 */
    n_int contours, /* show contours */
    n_int greenblue, /* show simple green and blue map */
    n_int weather_symbol_grid_dimension, /* grid used to show weather symbols, typically 20 */
    n_byte show_wind, /* show wind directions */
    n_byte show_cloud, /* show cloud as particles */
    n_byte show_beings, /* show the apes */
    n_byte show_weather_symbols /* show weather symbols or not */
)
{
    const n_int mult = 1024;
    n_int px, py, map_x, map_y, i = 0,j;
    unsigned char r,g,b;
    n_int denom, az;
    n_byte az0,az1,az2;
    unsigned char * buffer;

    buffer = (unsigned char*) malloc(3 * img_width * img_width);
    if (buffer==NULL)
    {
        fprintf(stderr,"landscape_png: Unable to allocate memory for image of dimensions %dx%d\n",(int)img_width,(int)img_width);
        return;
    }

    for (py = 0; py < img_width; py++)
    {

        map_y = py * MAP_DIMENSION / img_width;
        for (px = 0; px < img_width; px++, i += 3)
        {
            map_x = px * MAP_DIMENSION / img_width;

            az0 = local_land->tide_level;
            if (map_x > 0) az0 = QUICK_LAND(local_land, map_x-1, map_y);
            az1 = QUICK_LAND(local_land, map_x, map_y);
            az2 = local_land->tide_level;
            if (map_x < MAP_DIMENSION) az2 = QUICK_LAND(local_land, map_x+1, map_y);

            denom = 2 * ((n_int)az0*mult - (2*(n_int)az1*mult) + (n_int)az2*mult);
            az = az1*mult;
            if (denom != 0)
            {
                n_int numer = (n_int)az0*mult - (n_int)az2*mult;
                az += (numer*mult)/denom;
            }

            az0 = local_land->tide_level;
            if (map_y > 0) az0 = QUICK_LAND(local_land, map_x, map_y-1);
            az2 = local_land->tide_level;
            if (map_y < MAP_DIMENSION) az2 = QUICK_LAND(local_land, map_x, map_y+1);

            denom = 2 * ((n_int)az0*mult - (2*(n_int)az1*mult) + (n_int)az2*mult);
            if (denom != 0)
            {
                n_int numer = (n_int)az0*mult - (n_int)az2*mult;
                az += numer*mult/denom;
            }

            az0 = local_land->tide_level;
            if ((map_x > 0) && (map_y > 0)) az0 = QUICK_LAND(local_land, map_x-1, map_y-1);
            az2 = local_land->tide_level;
            if ((map_x < MAP_DIMENSION) && (map_y < MAP_DIMENSION)) az2 = QUICK_LAND(local_land, map_x+1, map_y+1);

            denom = 2 * ((n_int)az0*mult - (2*(n_int)az1*mult) + (n_int)az2*mult);
            if (denom != 0)
            {
                n_int numer = (n_int)az0*mult - (n_int)az2*mult;
                az += numer*mult/denom;
            }

            az0 = local_land->tide_level;
            if ((map_x < MAP_DIMENSION) && (map_y > 0)) az0 = QUICK_LAND(local_land, map_x+1, map_y-1);
            az2 = local_land->tide_level;
            if ((map_x > 0) && (map_y < MAP_DIMENSION)) az2 = QUICK_LAND(local_land, map_x-1, map_y+1);

            denom = 2 * ((n_int)az0*mult - (2*(n_int)az1*mult) + (n_int)az2*mult);
            if (denom != 0)
            {
                n_int numer = (n_int)az0*mult - (n_int)az2*mult;
                az += numer*mult/denom;
            }

            if (az <= (local_land->tide_level+1)*mult)
            {
                if (contours == 0)
                {
                    if (greenblue == 0)
                    {
                        r = g = 0;
                        b = 255 - local_land->tide_level + az/mult;
                    }
                    else
                    {
                        r = g = 0;
                        b = greenblue;
                    }
                }
                else
                {
                    r = g = b = 255;
                }
            }
            else
            {
                n_int v = az / mult;
                if (contours == 0)
                {
                    if (greenblue == 0)
                    {
                        r = b = 0;
                        g = 255 - (local_land->tide_level/2) + v;
                        if (v - WATER_MAP < 15)
                        {
                            r = 80 - (v - WATER_MAP)*3;
                            g = r;
                        }
                    }
                    else
                    {
                        r = b = 0;
                        g = greenblue;
                    }
                }
                else
                {
                    r = g = b = 200 + (55 * ((v - WATER_MAP)/contours) / (WATER_MAP/contours));
                }
            }

            buffer[i] = r;
            buffer[i+1] = g;
            buffer[i+2] = b;

        }
    }

    if (contours != 0)
    {
        for (py = 0; py < img_width; py++)
        {
            for (px = 1; px < img_width; px++)
            {
                i = (py*img_width + px)*3;
                if ((buffer[i] != buffer[i-3]) && (buffer[i-3] != 0))
                {
                    buffer[i] = 0;
                    buffer[i+1] = 0;
                    buffer[i+2] = 0;
                }
            }
        }
        for (px = 0; px < img_width; px++)
        {
            for (py = 1; py < img_width; py++)
            {
                i = (py*img_width + px)*3;
                if ((buffer[i] != 0) &&
                        (buffer[i-img_width*3] != 0) &&
                        (buffer[i] != buffer[i-img_width*3]))
                {
                    buffer[i] = 0;
                    buffer[i+1] = 0;
                    buffer[i+2] = 0;
                }
            }
        }
    }

    if (show_beings != 0) draw_beings(buffer, img_width, num, beings, local_land);

    draw_weather(buffer, img_width, local_weather, weather_dimension, weather_symbol_grid_dimension, show_wind, show_cloud, show_weather_symbols);

    if (!((map_tx == 0) && (map_ty == 0) && (map_bx == img_width) && (map_by == img_width)))
    {
        /* subsample */
        unsigned char *smallbuff = (unsigned char*) malloc(3 * img_width_small * img_width_small);
        i = 0;
        for (py = 0; py < img_width_small; py++)
        {
            n_int py2 = map_ty + (py * (map_by - map_ty) / img_width_small);
            for (px = 0; px < img_width_small; px++, i += 3)
            {
                n_int px2 = map_tx + (px * (map_bx - map_tx) / img_width_small);
                j = (py2*img_width + px2)*3;
                smallbuff[i] = buffer[j];
                smallbuff[i+1] = buffer[j+1];
                smallbuff[i+2] = buffer[j+2];
            }
        }
        write_png_file(img_fil, img_width_small, img_width_small, smallbuff);
        free(smallbuff);
    }
    else
    {
        write_png_file(img_fil, img_width, img_width, buffer);
    }

    free(buffer);
}

void update_log(char * filename, noble_simulation * sim)
{
    n_int alpha_male=-1, alpha_female=-1;
    n_int alpha_male_state=-1,alpha_female_state=-1;
    n_byte alpha_male_family[2];
    n_byte alpha_female_family[2];
    n_int map_x,map_y,n;
    n_int edible_biomass;
    FILE * fp;
    n_byte file_exists = 1;
    if (!fopen (filename,"r"))
    {
        file_exists=0;
    }

    if ((fp = fopen (filename,"a")))
    {

        n_uint current_date = TIME_IN_DAYS(sim->land->date);
        n_int i, parasites=0, av_parasites=0, honor=0, av_honor=0, av_age=0;
        n_int infants=0,juveniles=0,males=0,females=0,male_honor=0,female_honor=0;
        n_int animal_biomass=0;
        for (i = 0; i < sim->num; i++)
        {
            noble_being * local_being = &(sim->beings[i]);
            n_uint local_dob = TIME_IN_DAYS(GET_D(local_being));
            n_int age_days = current_date - local_dob;
            av_age += age_days;
            if (FIND_SEX(GET_I(local_being)) == SEX_FEMALE)
            {
                females++;
            }
            else
            {
                males++;
            }
            animal_biomass += being_energy(local_being);

#ifdef PARASITES_ON
            parasites += local_being->parasites;
            honor += local_being->honor;
            if (FIND_SEX(GET_I(local_being)) == SEX_FEMALE)
            {
                male_honor += local_being->honor;
            }
            else
            {
                female_honor += local_being->honor;
            }
#endif
#ifdef PARENTING_ON
            if (age_days < CARRYING_DAYS)
            {
                infants++;
            }
            else
            {
                if (age_days < WEANING_DAYS)
                {
                    juveniles++;
                }
            }
#endif
        }
        if (sim->num > 0)
        {
            av_age /= sim->num;
            av_parasites = parasites/sim->num;
            av_honor = honor/sim->num;
            male_honor /= sim->num;
            female_honor /= sim->num;
        }

        n=0;
        edible_biomass=0;
        for (map_y = 0; map_y < MAP_DIMENSION; map_y++)
        {
            for (map_x = 0; map_x < MAP_DIMENSION; map_x++,n++)
            {
#ifdef VEGETATION_ON
                edible_biomass += sim->land->edible[n];
#endif
            }
        }

        alpha_male=-1;
        alpha_female=-1;
        alpha_male_state=-1;
        alpha_female_state=-1;
        alpha_male = get_alpha(0, sim);
        alpha_female = get_alpha(1, sim);
        alpha_male_family[0] = 0;
        alpha_male_family[1] = 0;
        if (alpha_male>-1)
        {
            noble_being * local_being = &(sim->beings[alpha_male]);
            alpha_male_state = local_being->state;
#ifdef PARENTING_ON
            alpha_male = GET_NAME(local_being);
            alpha_male_family[0] = GET_FAMILY_FIRST_NAME(sim,local_being);
            alpha_male_family[1] = GET_FAMILY_SECOND_NAME(sim,local_being);
#endif
        }
        alpha_female_family[0] = 0;
        alpha_female_family[1] = 0;
        if (alpha_female>-1)
        {
            noble_being * local_being = &(sim->beings[alpha_female]);
            alpha_female_state = local_being->state;
#ifdef PARENTING_ON
            alpha_female = GET_NAME(local_being);
            alpha_female_family[0] = GET_FAMILY_FIRST_NAME(sim,local_being);
            alpha_female_family[1] = GET_FAMILY_SECOND_NAME(sim,local_being);
#endif
        }

        if (file_exists == 0)
        {
            fprintf(fp, "Mins,Date,Time,Population,Average Age,Parasites,");
            fprintf(fp, "Average Parasites,Honor,Average Honor,Infants,Juveniles,");
            fprintf(fp, "Males,Females,Male Honor,Female Honor,");
            fprintf(fp, "Edible Biomass,Animal Biomass,");
            fprintf(fp, "Alpha Male Name,Alpha Male Family0,Alpha Male Family1,Alpha Male State,");
            fprintf(fp, "Alpha Female Name,Alpha Female Family0,Alpha Female Family1,Alpha Female State\n");
        }

        fprintf(fp, "%d,%d,%d,%d,%d,%d,",(int)(current_date*24*60)+(int)(sim->land->time),(int)current_date,(int)sim->land->time,(int)sim->num,(int)av_age,(int)parasites);
        fprintf(fp, "%d,%d,%d,%d,%d,",(int)av_parasites,(int)honor, (int)av_honor, (int)infants, (int)juveniles);
        fprintf(fp, "%d,%d,%d,%d,",(int)males, (int)females, (int)male_honor, (int)female_honor);
        fprintf(fp, "%d,%d,",(int)edible_biomass, (int)animal_biomass);
        fprintf(fp, "%d,%d,%d,%d,",(int)alpha_male,(int)alpha_male_family[0],(int)alpha_male_family[1],(int)alpha_male_state);
        fprintf(fp, "%d,%d,%d,%d\n",(int)alpha_female,(int)alpha_female_family[0],(int)alpha_female_family[1], (int)alpha_female_state);
        fclose (fp);
    }

}

void draw_line(unsigned char * buffer, n_int img_width, n_int img_height, n_int prev_x, n_int prev_y, n_int x, n_int y, n_byte r,n_byte g,n_byte b,n_byte thickness)
{
    n_int i,n,dx,dy,max,xx,yy;
    n_int abs_dx,abs_dy;
    dx = x-prev_x;
    dy = y-prev_y;

    abs_dx = dx;
    if (dx < 0) abs_dx = -dx;
    abs_dy = dy;
    if (dy < 0) abs_dy = -dy;

    max = abs_dx;
    if (abs_dy > max) max = abs_dy;

    for (i=0; i<max; i++)
    {
        xx = prev_x + (i*(x-prev_x)/max);
        if ((xx > -1) && (xx < img_width))
        {
            yy = prev_y + (i*(y-prev_y)/max);
            if ((yy > -1) && (yy < img_height))
            {
                n = (yy*img_width + xx)*3;
                buffer[n] = r;
                buffer[n+1] = g;
                buffer[n+2] = b;
            }
        }
    }
}

void plot_status_distribution(noble_simulation * sim, char * filename, n_int img_width, n_int img_height)
{
    n_byte * buffer;

    buffer = (n_byte*)io_new(img_width*img_height*3);
    graph_honor_distribution(sim, buffer,img_width,img_height);
    write_png_file(filename, img_width, img_height, buffer);
    io_free((void*)buffer);
}

void plot_ideosphere(noble_simulation * sim, char * filename, n_int img_width, n_int img_height)
{
    n_byte * buffer;

    buffer = (n_byte*)io_new(img_width*img_height*3);
    graph_ideosphere(sim, buffer, img_width, img_height);
    write_png_file(filename, img_width, img_height, buffer);
    io_free((void*)buffer);
}

void plot_genespace(noble_simulation * sim, char * filename, n_int img_width, n_int img_height)
{
    n_byte * buffer;

    buffer = (n_byte*)io_new(img_width*img_height*3);
    graph_phasespace(sim, buffer, img_width, img_height,1,1);
    write_png_file(filename, img_width, img_height, buffer);
    io_free((void*)buffer);
}

void plot_braincode(noble_simulation * sim, char * filename, n_int img_width, n_int img_height)
{
    n_byte * buffer;

    buffer = (n_byte*)io_new(img_width*img_height*3);
    graph_phasespace(sim, buffer, img_width, img_height, 1,0);
    write_png_file(filename, img_width, img_height, buffer);
    io_free((void*)buffer);
}

void plot_genepool(noble_simulation * sim, char * filename, n_int img_width, n_int img_height)
{
    n_byte * buffer;

    buffer = (n_byte*)io_new(img_width*img_height*3);
    graph_genepool(sim, buffer, img_width, img_height);
    write_png_file(filename, img_width, img_height, buffer);
    io_free((void*)buffer);
}

void plot_relationshipmatrix(noble_simulation * sim, char * filename, n_int img_width, n_int img_height)
{
    n_byte * buffer;

    buffer = (n_byte*)io_new(img_width*img_height*3);
    graph_relationship_matrix(sim, buffer, img_width, img_height);
    write_png_file(filename, img_width, img_height, buffer);
    io_free((void*)buffer);
}

void plot_pathogens(noble_simulation * sim, char * filename, n_int img_width, n_int img_height)
{
    n_byte * buffer;

    buffer = (n_byte*)io_new(img_width*img_height*3);
    graph_pathogens(sim, buffer, img_width, img_height);
    write_png_file(filename, img_width, img_height, buffer);
    io_free((void*)buffer);
}

void plot_territory(char * filename, noble_simulation * sim, n_int img_width, n_int img_height)
{
#ifdef TERRITORY_ON
    n_int i,x,y,n,max,idx,*hist;
    n_int px,py,map_x,map_y;
    unsigned char * buffer;
    noble_being * local_being;

    const n_byte col[] =
    {
        255,0,0,	0,255,0,	0,0,255,	255,255,0,	255,0,255,
        0,255,255,	255,255,255,	200,150,150,	150,200,150,	150,150,200,
        200,200,150,	200,150,200,	150,200,200,	150,150,150,	100,50,50,
        50,100,50,	50,50,100,	100,100,50,	100,50,100,	50,100,100,
        50,50,50,	78,234,162,	186,76,23,	245,134,189,	10,23,72,
        121,168,251
    };

    buffer = (unsigned char*)malloc(img_width*img_height*3);
    hist = (n_int*)malloc(256*sizeof(n_int));
    for (i = 0; i < img_width*img_height*3; i++) buffer[i]=0;

    i=0;
    for (py = 0; py < img_width; py++)
    {
        y = py * (TERRITORY_DIMENSION-1) / img_height;
        map_y = py * MAP_DIMENSION / img_width;
        for (px = 0; px < img_width; px++, i += 3)
        {
            map_x = px * MAP_DIMENSION / img_width;
            x = px * (TERRITORY_DIMENSION-1) / img_width;

            if (QUICK_LAND(sim->land, map_x, map_y) > sim->land->tide_level)
            {

                for (n = 0; n < 256; n++) hist[n]=0;
                for (n = 0; n < sim->num; n++)
                {
                    local_being = &(sim->beings[n]);
                    idx = y*TERRITORY_DIMENSION+x;
                    hist[local_being->territory[idx].name]++;
                }
                max=5;
                idx = -1;
                for (n = 1; n < 256; n++)
                {
                    if (hist[n]>max)
                    {
                        max = hist[n];
                        idx=n;
                    }
                }
                if (idx>0)
                {
                    idx=(idx%26)*3;
                    buffer[i] = col[idx++];
                    buffer[i+1] = col[idx++];
                    buffer[i+2] = col[idx];
                }
                else
                {
                    buffer[i] = 0;
                    buffer[i+1] = 120;
                    buffer[i+2] = 0;
                }
            }
            else
            {
                buffer[i] = 0;
                buffer[i+1] = 0;
                buffer[i+2] = 120;
            }
        }
    }


    write_png_file(filename, img_width, img_height, buffer);
    free(buffer);
    free(hist);
#endif
}


void plot_age_demographic(char * filename, noble_simulation * sim, n_int img_width, n_int img_height)
{
    n_byte * buffer;

    buffer = (n_byte*)io_new(img_width*img_height*3);
    graph_age_demographic(sim, buffer, img_width, img_height);
    write_png_file(filename, img_width, img_height, buffer);
    io_free((void*)buffer);
}

void plot_heights(char * filename, noble_simulation * sim, n_int img_width, n_int img_height)
{
    n_byte * buffer;

    buffer = (n_byte*)io_new(img_width*img_height*3);
    graph_heights(sim, buffer, img_width, img_height);
    write_png_file(filename, img_width, img_height, buffer);
    io_free((void*)buffer);
}

void plot_log(char * log_filename, char * plot_filename, noble_simulation * sim, n_byte plot_type, n_int img_width, n_int img_height)
{
    FILE * fp;
    char line[256];
    int pctr,entries=0;
    int param[30],max=1;
    int first = 1;
    unsigned char * buffer;
    int i,x,y,prev_x,prev_y,entry;
    if ((fp = fopen (log_filename,"r")))
    {
        while (!feof(fp))
        {
            char ch = 0;
            int ch_ctr = 0;
            pctr=0;
            while ((ch!='\n') && (!feof(fp)))
            {
                ch = fgetc(fp);
                if (ch !=',')
                {
                    line[ch_ctr++] = ch;
                }
                else
                {
                    line[ch_ctr] = 0;
                    if (first == 0) param[pctr] = atoi(line);
                    ch_ctr=0;
                    pctr++;
                }
            }
            if (first==0)
            {
                if (param[plot_type] > max) max = param[plot_type];
                entries++;
            }
            first = 0;
        }
        fclose(fp);
    }

    if ((entries > 1) && (fp = fopen (log_filename,"r")))
    {

        max = max * 120/100;

        buffer = (unsigned char*)malloc(img_width*img_height*3);
        for (i = 0; i < img_width*img_height*3; i++) buffer[i]=255;

        prev_x=-1;
        prev_y=-1;
        entry=0;
        first = 1;
        while (!feof(fp))
        {
            char ch = 0;
            int ch_ctr = 0;
            pctr=0;
            while ((ch!='\n') && (!feof(fp)))
            {
                ch = fgetc(fp);
                if (ch !=',')
                {
                    line[ch_ctr++] = ch;
                }
                else
                {
                    line[ch_ctr] = 0;
                    if (first == 0) param[pctr] = atoi(line);
                    ch_ctr=0;
                    pctr++;
                }
            }
            if (first==0)
            {
                x = entry*(img_width-1)/entries;
                y = img_height - (param[plot_type]*img_height/max);
                if (prev_x > -1) draw_line(buffer,img_width,img_height,prev_x,prev_y,x,y,0,0,0,1);
                prev_x = x;
                prev_y = y;
                entry++;
            }
            first = 0;
        }
        fclose(fp);

        write_png_file(plot_filename, img_width, img_height, buffer);

        free(buffer);
    }
}

void update_microblog(char * log_filename, noble_simulation * sim)
{
    n_byte new_msg;
    int name_index,ch_ctr;
    n_int dominance_strings=12;
    FILE * fp;
    char ch,line[256],line2[256];
    n_int alpha_male[5];
    n_int alpha_female[5];
    n_int i,pctr,male_dominion=-1,female_dominion=-1;
    n_int param[30];
    n_int first = 1;

    char *dominance_str[] =
    {
        "%s becomes the new alpha %s",
        "%s is the new leading %s",
        "%s is now the dominant %s",
        "%s is %s leader of the group",
        "%s becomes alpha %s",
        "%s succeeds as the dominant %s",
        "%s is the most honorable %s",
        "%s is the highest ranking %s",
        "%s is the highest status %s",
        "%s is the top ranking %s",
        "%s is the dominant %s",
        "%s is top ranked %s"
    };

    char *male_str[] =
    {
        "male", "male ape", "Noble Ape male", "male Noble ape", "male simian", "simian male", "male primate"
    };
    char *female_str[] =
    {
        "female", "female ape", "Noble Ape female", "female Noble ape", "female simian", "simian female", "female primate"
    };
    char *sleep_str[] =
    {
        "sleeping", "snoozing", "snoring", "settling down to sleep", "finding a place to sleep","dreaming",
        "feeling sleepy", "nestling down to sleep"
    };
    char *hungry_str[] =
    {
        "is hungry", "wants to eat", "is looking for food", "is searching for food", "is foraging for food"
    };
    char *swimming_str[] =
    {
        "swimming","paddling","splashing in water","wading in shallow water","splashing","an aquatic ape","drinking"
    };
    char *eating_str[] =
    {
        "eating","munching","grazing","chewing on vegetation","chewing nonchalantly","grazing nonchalantly"
    };
    char *grooming_str[] =
    {
        "grooming","grooming other apes","looking for fleas","picking off fleas","social networking","socialising",
        "scratching"
    };
    char *speaking_str[] =
    {
        "speaking","chatting","chattering","bantering","in discussion","chuntering","communicating",
        "talking to themselves","chuntering to themselves"
    };
    char *showforce_str[] =
    {
        "makes an angry display", "is threatening", "looks threatening", "sounds an alarm call", "looks aggressive",
        "is alerted to danger","is startled","quickly looks around","is on the alert"
    };
    char *attack_str[] =
    {
        "is attacking", "launches an attack", "attacks a rival",
        "is competing for dominance", "is wrestling","squabbles with a rival",
        "is squabbling"
    };
    char *infant_str[] =
    {
        "new baby", "new born", "new infant", "offspring", "young infant"
    };
    char *parenting_str[] =
    {
        "holding", "caring for", "nurturing", "playing with",
        "teaching", "carrying", "protecting", "feeding",
        "attending to", "inspecting", "grooming"
    };

    for (i=0; i<5; i++)
    {
        alpha_male[i]=-1;
        alpha_female[i]=-1;
    }
    name_index = 17;
    male_dominion=-1;
    female_dominion=-1;
    first = 1;
    if ((fp = fopen (log_filename,"r")))
    {
        while (!feof(fp))
        {
            ch = 0;
            ch_ctr = 0;
            pctr=0;
            while ((ch!='\n') && (!feof(fp)))
            {
                ch = fgetc(fp);
                if (ch !=',')
                {
                    line[ch_ctr++] = ch;
                }
                else
                {
                    line[ch_ctr] = 0;
                    if (first == 0) param[pctr] = atoi(line);
                    ch_ctr=0;
                    pctr++;
                }
            }
            if ((first==0) && (!feof(fp)))
            {
                alpha_male[4] = alpha_male[3];
                alpha_male[3] = param[name_index+3];
                alpha_female[4] = alpha_female[3];
                alpha_female[3] = param[name_index+7];

                if ((param[name_index] != alpha_male[0]) ||
                        (param[name_index+1] != alpha_male[1]) ||
                        (param[name_index+2] != alpha_male[2]))
                {
                    male_dominion=0;
                    alpha_male[0] = param[name_index];
                    alpha_male[1] = param[name_index+1];
                    alpha_male[2] = param[name_index+2];
                }
                else
                {
                    male_dominion++;
                }
                if ((param[name_index+4] != alpha_female[0]) ||
                        (param[name_index+5] != alpha_female[1]) ||
                        (param[name_index+6] != alpha_female[2]))
                {
                    female_dominion=0;
                    alpha_female[0] = param[name_index+4];
                    alpha_female[1] = param[name_index+5];
                    alpha_female[2] = param[name_index+6];
                }
                else
                {
                    female_dominion++;
                }
            }
            first = 0;
        }
        fclose(fp);
    }


    new_msg = 0;
    if ((alpha_male[0] >-1) && (male_dominion == 1))
    {
        being_name(0, (n_int)alpha_male[0], (n_byte)alpha_male[1], (n_byte)alpha_male[2], line2);
        sprintf(line,dominance_str[rand()%dominance_strings], line2, male_str[rand()%7]);
        new_msg=1;
    }
    else
    {
        if ((alpha_female[0] >-1) && (female_dominion == 1))
        {
            being_name(1, alpha_female[0], alpha_female[1], alpha_female[2], line2);
            sprintf(line,dominance_str[rand()%dominance_strings], line2, female_str[rand()%7]);
            new_msg=1;
        }
        else
        {

            n_byte2 v = 0;
            n_byte state_changed=0;
            if ((alpha_male[3] >-1) && (alpha_male[4] >-1) && (alpha_male[3] != alpha_male[4]))
            {
                being_name(0, (n_int)alpha_male[0], (n_byte)alpha_male[1], (n_byte)alpha_male[2], line2);
                v = alpha_male[3];
                state_changed=1;
            }
            if ((alpha_female[3] >-1) && (alpha_female[4] >-1) && (alpha_female[3] != alpha_female[4]))
            {
                being_name(1, (n_int)alpha_female[0], (n_byte)alpha_female[1], (n_byte)alpha_female[2], line2);
                v = alpha_female[3];
                state_changed=1;
            }
            if (state_changed == 0)
            {
                if (alpha_female[0] >-1)
                {
                    n_uint current_date = TIME_IN_DAYS(sim->land->date);
                    for (i=0; i<sim->num; i++)
                    {
                        noble_being * local_being = &(sim->beings[i]);
                        if ((GET_FAMILY_FIRST_NAME(sim,local_being) == alpha_female[1]) &&
                                (GET_FAMILY_SECOND_NAME(sim,local_being) == alpha_female[2]) &&
                                (GET_NAME(sim,local_being) == alpha_female[0]))
                        {
                            n_int conception_date = TIME_IN_DAYS(local_being->date_of_conception);
                            if (conception_date != 0)
                            {
                                if ((current_date - conception_date > GESTATION_DAYS) &&
                                        (current_date - conception_date < WEANING_DAYS) &&
                                        ((rand()%100)<10))
                                {
                                    being_name(1, (n_int)alpha_female[0], (n_byte)alpha_female[1], (n_byte)alpha_female[2], line2);
                                    sprintf(line,"%s is %s her %s", line2, parenting_str[rand()%11],infant_str[rand()%5]);
                                    new_msg=1;
                                }
                            }
                            break;
                        }
                    }
                }
            }
            else
            {
                if (v & BEING_STATE_ASLEEP)
                {
                    sprintf(line,"%s is %s", line2, sleep_str[rand()%8]);
                    new_msg=1;
                }
                else
                {
                    if (v & BEING_STATE_HUNGRY)
                    {
                        sprintf(line,"%s %s", line2, hungry_str[rand()%5]);
                        new_msg=1;
                    }
                    else
                    {
                        if (v & BEING_STATE_SWIMMING)
                        {
                            sprintf(line,"%s is %s", line2, swimming_str[rand()%7]);
                            new_msg=1;
                        }
                        else
                        {
                            if (v & BEING_STATE_EATING)
                            {
                                sprintf(line,"%s is %s", line2, eating_str[rand()%6]);
                                new_msg=1;
                            }
                            else
                            {
                                if (v & BEING_STATE_GROOMING)
                                {
                                    sprintf(line,"%s is %s", line2, grooming_str[rand()%7]);
                                    new_msg=1;
                                }
                                else
                                {
                                    if ((v & BEING_STATE_SPEAKING) &&
                                            (!(v & BEING_STATE_ATTACK)) &&
                                            (!(v & BEING_STATE_SHOWFORCE)))
                                    {
                                        sprintf(line,"%s is %s", line2, speaking_str[rand()%9]);
                                        new_msg=1;
                                    }
                                    else
                                    {
                                        if (v & BEING_STATE_SHOWFORCE)
                                        {
                                            sprintf(line,"%s %s", line2, showforce_str[rand()%9]);
                                            new_msg=1;
                                        }
                                        else
                                        {
                                            if (v & BEING_STATE_ATTACK)
                                            {
                                                sprintf(line,"%s %s", line2, attack_str[rand()%7]);
                                                new_msg=1;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (new_msg !=0)
    {
        sprintf(line2, "twidge update \"%s #nobleape\"\n", line);
        /*printf("%s",line2);*/
        i = system(line2);
    }
}
