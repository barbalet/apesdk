/****************************************************************

	cle.c

	=============================================================

    Copyright 1996-2010 Tom Barbalet. All rights reserved.

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

#include "../../noble/noble.h"
#include "../../universe/universe.h"
#include "../../universe/universe_internal.h"
#include "../../entity/entity_internal.h"
#include "../../entity/entity.h"
#include "web.h"

#define NOBLE_WEB
#undef MICROBLOGGING_ON

extern noble_being	*ape;

extern n_byte * sim_fileout(n_uint * len);
/*extern n_byte * sim_fileoutXML(n_uint * len);*/

extern n_int sim_filein(n_byte * buff, n_uint len);

extern void * sim_init(KIND_OF_USE kind, n_uint randomise, n_uint offscreen_size, n_uint landbuffer_size);

extern void sim_close();
extern void sim_cycle();

extern void sim_ratio(n_uint * female, n_uint * male);

extern n_int sim_interpret(n_byte * buff, n_uint len);

/*NOBLEMAKE END=""*/

/*NOBLEMAKE VAR=""*/

void * plat_new(unsigned long bytes, unsigned char critical);
void plat_free(void * ptr);

void plat_copy(n_byte * from, n_byte * to, n_uint number);
void plat_cycle();

/*NOBLEMAKE END=""*/


void * plat_new(unsigned long bytes, unsigned char critical)
{
    void *	tmp = 0L;
    tmp = (void *) malloc(bytes);
    if(tmp == 0L && critical)
    {
        exit(0);
    }
    return( tmp );
}

void plat_free(void * ptr)
{
    if( ptr != 0L )
    {
        free( ptr );
        ptr = 0L;
    }
}

void plat_cycle()
{
    printf("plat_cycle called\n");
}

void plat_copy(n_byte * from, n_byte * to, n_uint number)
{
    memcpy(to, from, number);
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
    n_uint found_length = 0;
    int error_value = 0;
    n_byte value;
    n_uint local_length = *length;

    infile = fopen(file_name, "rb");

    if(infile == 0L)
        return -1;

    if(data == 0L)
    {
        do
        {
            if (fread(&value,1,1,infile)>0)
            {
                found_length ++;
            }
        }
        while(!feof(infile));
        *length = found_length - 1;
    }
    else
    {
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


noble_being check_delta = {0};

void plat_debug_being(noble_being * test, n_byte initial)
{
    if(initial)
        plat_copy((n_byte*)test,(n_byte *)&check_delta,sizeof(noble_being));
    else
    {
        FILE * append_write = 0L;

        append_write = fopen("data_delta.txt","a");

        if(append_write != 0L)
        {

            if (being_location_x(&check_delta) != being_location_x(test))
                fprintf(append_write, "new x = %d;\n",(int)being_location_x(test));
            if (being_location_y(&check_delta) != being_location_y(test))
                fprintf(append_write, "new y = %d;\n",(int)being_location_y(test));
            if (being_facing(&check_delta) != being_facing(test))
                fprintf(append_write, "new facing = %d;\n",(int)being_facing(test));

            if(being_speed(&check_delta) != being_speed(test))
                fprintf(append_write, "new speed = %d;\n",(int)being_speed(test));
            if(being_energy(&check_delta) != being_energy(test))
                fprintf(append_write, "new energy = %d;\n",(int)being_energy(test));

            if (genetics_compare(being_genetics(&check_delta), being_genetics(test)) != 0)
            {
                n_int i;
                fprintf(append_write, "new new_genetics = ");
                for (i = 0; i < CHROMOSOMES; i++)
                {
                    fprintf(append_write, "%d ",(int)being_genetics(test)[i]);
                }
                fprintf(append_write, "\n");
            }

            if(being_dob(&check_delta) != being_dob(test))
                fprintf(append_write, "new date_of_birth = %d;\n",(int)being_dob(test));

            if(check_delta.state != test->state)
                fprintf(append_write, "new state = %d;\n",test->state);

            fprintf(append_write,"---\n");
            fclose(append_write);
        }
    }
}

#ifdef NOBLE_WEB

void sim_fileoutImage(char* img_fil, noble_simulation * sim)
{
    n_int img_width = 512;
    landscape_png(0,0,img_width,img_width,img_fil, sim->land, sim->weather, sim->num, sim->beings, img_width, img_width, 128, 0, 0, 18, 0, 1, 1, 0);
}

void sim_fileoutImageApe(
    char* img_fil,
    noble_simulation * sim,
    n_int index,
    n_byte width_percent)
{
    n_int tx,ty,bx,by;

    noble_being * local_being = &(sim->beings[index]);

    n_int img_width = 512;
    n_int img_width_small = 320;
    n_int px,py;

    px = (n_int)(APESPACE_TO_MAPSPACE(being_location_x(local_being)))*
         img_width/MAP_DIMENSION;
    py = (n_int)(APESPACE_TO_MAPSPACE(being_location_x(local_being)))*
         img_width/MAP_DIMENSION;

    tx = px - img_width * width_percent / 200;
    ty = py - img_width * width_percent / 200;
    bx = tx + img_width * width_percent / 100;
    by = ty + img_width * width_percent / 100;

    if (tx < 0) tx = 0;
    if (ty < 0) ty = 0;
    if (bx >= img_width) bx = img_width-1;
    if (by >= img_width) by = img_width-1;

    landscape_png(tx,ty,bx,by,img_fil, sim->land, sim->weather, sim->num, sim->beings, img_width, img_width_small, 128, 0, 0, 18, 0, 1, 1, 0);
}


void sim_fileoutImageContours(char* img_fil, noble_simulation * sim)
{
    n_int img_width = 512;
    landscape_png(0,0,img_width,img_width,img_fil, sim->land, sim->weather, sim->num, sim->beings, img_width, img_width, 128, 100, 0, 18, 1, 0, 0, 0);
}

void sim_fileoutImageWeather(char* img_fil, noble_simulation * sim)
{
    n_int img_width = 512;
    landscape_png(0,0,img_width,img_width,img_fil, sim->land, sim->weather, sim->num, sim->beings, img_width, img_width, 128, 0, 80, 12, 1, 1, 0, 1);
}

void sim_fileoutImageVegetation(char* img_fil, noble_simulation * sim)
{
    n_int img_width = 512;
    draw_vegetation(img_fil, img_width, sim->land, sim->weather);
}

void sim_fileoutImageTerritory(char* img_fil, noble_simulation * sim)
{
    n_int img_width = 512;
    plot_territory(img_fil, sim_sim(), img_width, img_width);
}

void sim_fileoutImageIdeosphere(char* img_fil, noble_simulation * sim)
{
    n_int img_width = 256;
    plot_ideosphere(sim_sim(), img_fil, img_width, img_width);
}

void sim_fileoutImageBraincode(char* img_fil, noble_simulation * sim)
{
    n_int img_width = 256;
    plot_braincode(sim_sim(), img_fil, img_width, img_width);
}

void sim_fileoutImageRelationshipMatrix(char* img_fil, noble_simulation * sim)
{
    n_int img_width = 256;
    plot_relationshipmatrix(sim, img_fil, img_width, img_width);
}

void sim_fileoutImageGenespace(char* img_fil, noble_simulation * sim)
{
    n_int img_width = 256;
    plot_genespace(sim, img_fil, img_width, img_width);
}

void sim_fileoutImageGenepool(char* img_fil, noble_simulation * sim)
{
    n_int img_width = 256;
    plot_genepool(sim, img_fil, img_width, img_width);
}

void sim_fileoutImagePathogens(char* img_fil, noble_simulation * sim)
{
    n_int img_width = 256;
    plot_pathogens(sim, img_fil, img_width, img_width);
}

void sim_web(noble_simulation * sim, n_byte being_profiles, n_byte animate, int land_seed)
{
    struct tm *tmp;
    char img_str[50];
    char weather_str[50];
    char vegetation_str[50];
    char territory_str[50];
    char ideosphere_str[50];
    char braincode_str[50];
    char relationshipmatrix_str[50];
    char genespace_str[50];
    char genepool_str[50];
    char pathogens_str[50];
    char img_fil[70];
    char brain_fil[70];
    char profile_fil[70];
    char img_str2[50];
    char weather_str2[50];
    char vegetation_str2[50];
    char territory_str2[50];
    char ideosphere_str2[50];
    char braincode_str2[50];
    char relationshipmatrix_str2[50];
    char genespace_str2[50];
    char genepool_str2[50];
    char pathogens_str2[50];
    noble_being * b;
    n_int i,j;
    n_uint current_date;
    n_byte * eliminated;
    n_int births;
    n_uint local_dob;
    n_int age_days;
    /* append the hour to file names */
    time_t now;
    time(&now);
    tmp = localtime(&now);
    sprintf(img_str, "noble_ape_%d.png", tmp->tm_hour);
    sprintf(weather_str, "noble_weather_%d.png", tmp->tm_hour);
    sprintf(vegetation_str, "noble_vegetation_%d.png", tmp->tm_hour);
    sprintf(territory_str, "noble_territory_%d.png", tmp->tm_hour);
    sprintf(ideosphere_str, "noble_ideosphere_%d.png", tmp->tm_hour);
    sprintf(braincode_str, "noble_braincode_%d.png", tmp->tm_hour);
    sprintf(relationshipmatrix_str, "noble_relationshipmatrix_%d.png", tmp->tm_hour);
    sprintf(genespace_str, "noble_genespace_%d.png", tmp->tm_hour);
    sprintf(genepool_str, "noble_genepool_%d.png", tmp->tm_hour);
    sprintf(pathogens_str, "noble_pathogens_%d.png", tmp->tm_hour);

    /* create main image */
    sim_fileoutImage(img_str, sim);

    /* create weather image */
    sim_fileoutImageWeather(weather_str, sim);

    /* create vegetation image */
    sim_fileoutImageVegetation(vegetation_str, sim);

    /* create territory image */
    sim_fileoutImageTerritory(territory_str, sim);

    /* create ideosphere image */
    sim_fileoutImageIdeosphere(ideosphere_str, sim);

    /* create braincode image */
    sim_fileoutImageBraincode(braincode_str, sim);

    /* create relationship matrix image */
    sim_fileoutImageRelationshipMatrix(relationshipmatrix_str, sim);

    /* create genespace matrix image */
    sim_fileoutImageGenespace(genespace_str, sim);

    /* create genepool image */
    sim_fileoutImageGenepool(genepool_str, sim);

    /* create pathogens image */
    sim_fileoutImagePathogens(pathogens_str, sim);

    /* create a profile page for each ape */
    eliminated = (n_byte*)malloc(sim->num*sizeof(n_byte));
    for (i = 0; i < sim->num; i++) eliminated[i] = 0;

    if (sim->num < being_profiles) being_profiles = sim->num;
    for (i = 0; i < being_profiles; i++)
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
        sprintf(img_fil, "ape_profile_%d.png", (int)i);
        sim_fileoutImageApe(img_fil, sim, winner, 20);

        sprintf(brain_fil, "ape_brain_%d.png", (int)i);
        draw_brain(brain_fil, sim, winner, 1024, 32);

        sprintf(profile_fil, "ape_profile_%d.htm", (int)i);
        create_web_ape_profile(profile_fil, img_fil, brain_fil, sim, winner);
    }
    free(eliminated);


    current_date = TIME_IN_DAYS(sim->land->date);
    births = 0;
    for (i = 0; i < sim->num; i++)
    {
        b = &sim->beings[i];
        local_dob = TIME_IN_DAYS(b->date_of_birth);
        age_days = current_date - local_dob;
        if (age_days < 2)
        {
            sprintf(img_fil, "ape_birth_%d.png", (int)births);
            sim_fileoutImageApe(img_fil, sim, i, 20);

            sprintf(brain_fil, "ape_birth_brain_%d.png", (int)births);
            draw_brain(brain_fil, sim, i, 1024, 32);

            sprintf(profile_fil, "ape_birth_%d.htm", (int)births);
            create_web_ape_profile(profile_fil, img_fil, brain_fil, sim, i);
            births++;
            if (births == being_profiles) break;
        }
    }


    memcpy(img_str2, img_str, 50);
    memcpy(weather_str2, weather_str, 50);
    memcpy(vegetation_str2, vegetation_str, 50);
    memcpy(territory_str2, territory_str, 50);
    memcpy(ideosphere_str2, ideosphere_str, 50);
    memcpy(braincode_str2, braincode_str, 50);
    memcpy(relationshipmatrix_str2, relationshipmatrix_str, 50);
    memcpy(genespace_str2, genespace_str, 50);
    memcpy(genepool_str2, genepool_str, 50);
    memcpy(pathogens_str2, pathogens_str, 50);


    if (animate != 0)
    {
        /* create an animated gif
           sudo apt-get install gifsicle imagemagick
        */
        i = system("mogrify -format gif noble_*.png");
        i = system("gifsicle --colors=256 --delay=50 --loop noble_ape_*.gif > noble_ape.gif");
        i = system("gifsicle --colors=256 --delay=50 --loop noble_weather_*.gif > noble_weather.gif");
        i = system("gifsicle --colors=256 --delay=50 --loop noble_vegetation_*.gif > noble_vegetation.gif");
        i = system("gifsicle --colors=256 --delay=50 --loop noble_territory_*.gif > noble_territory.gif");
        i = system("gifsicle --colors=256 --delay=50 --loop noble_ideosphere_*.gif > noble_ideosphere.gif");
        i = system("gifsicle --colors=256 --delay=50 --loop noble_braincode_*.gif > noble_braincode.gif");
        i = system("gifsicle --colors=256 --delay=50 --loop noble_relationshipmatrix_*.gif > noble_relationshipmatrix.gif");
        i = system("gifsicle --colors=256 --delay=50 --loop noble_genespace_*.gif > noble_genespace.gif");
        i = system("gifsicle --colors=256 --delay=50 --loop noble_genepool_*.gif > noble_genepool.gif");

        sprintf(img_str2,"%s","noble_ape.gif");
        sprintf(weather_str2,"%s","noble_weather.gif");
        sprintf(vegetation_str2,"%s","noble_vegetation.gif");
        sprintf(territory_str2,"%s","noble_territory.gif");
        sprintf(ideosphere_str2,"%s","noble_ideosphere.gif");
        sprintf(braincode_str2,"%s","noble_braincode.gif");
        sprintf(relationshipmatrix_str2,"%s","noble_relationshipmatrix.gif");
        sprintf(genespace_str2,"%s","noble_genespace.gif");
        sprintf(genepool_str2,"%s","noble_genepool.gif");
        sprintf(pathogens_str2,"%s","noble_pathogens.gif");
    }


    /* create the home page */
    create_web_title(
        "index.htm", "", "By Tom Barbalet",
        "Apes", img_str2, img_str,
        "Weather", weather_str2, weather_str,
        "Vegetation", vegetation_str2, vegetation_str,
        "Territory", territory_str2, territory_str,
        "Ideosphere", ideosphere_str2, ideosphere_str,
        "Braincode", braincode_str2, braincode_str,
        "Relationship Matrix", relationshipmatrix_str2, relationshipmatrix_str,
        "Genespace", genespace_str2, genespace_str,
        "Gene Pool", genepool_str2, genepool_str,
        "Pathogens / Immune response", pathogens_str2, pathogens_str,
        "Ape Population", "population.png",
        "Parasite Population", "population_parasites.png",
        "Average Age", "average_age.png",
        "Honor distribution", "honor_distribution.png",
        "Age Demographic", "age_demographic.png",
        "Ape Heights", "heights.png",
        "socialgraph.dot", "mategraph.dot",
        "<i>No apes or cats were harmed in the writing of this software</i>",
        sim, "Top Apes", "New Births",
        land_seed, being_profiles);
}

#define	TIME_SLICE	(60)

#else
#define	TIME_SLICE	(60*24*28*13)
#endif

int main(int argc, char* argv[])
{
    unsigned long length = 0;
    unsigned char *data = 0L;

    unsigned long	ticker = 0;
    int result = 0;
    int i,land_seed = 6342;
    char * simulation_file = 0;
    char * apescript_file = 0;
    char * output_file = "realtime.txt";
    /*char * output_xml = "realtime.xml";*/
    FILE * fp;
    srand( time(NULL) );

    for (i=1; i<argc; i+=2)
    {
        if ((strcmp(argv[i],"-s")==0) || ((strcmp(argv[i],"--sim")==0)))
        {
            simulation_file = argv[i+1];
        }
        else
        {
            if ((strcmp(argv[i],"-a")==0) || ((strcmp(argv[i],"--apescript")==0)))
            {
                apescript_file = argv[i+1];
            }
            else
            {
                if ((strcmp(argv[i],"-l")==0) || ((strcmp(argv[i],"--land")==0)))
                {
                    land_seed = atoi(argv[i+1]);
                }
                else
                {
                    if ((strcmp(argv[i],"-o")==0) || ((strcmp(argv[i],"--output")==0)))
                    {
                        output_file = argv[i+1];
                    }
                    /*
                    else {
                      if ((strcmp(argv[i],"-x")==0) || ((strcmp(argv[i],"--xml")==0))) {
                        output_xml = argv[i+1];
                      }
                    }
                    */
                }
            }
        }
    }

    /* added logging prior to sim_init */
    genealogy_log(sim_sim(), 0);

    sim_init(2,land_seed,(MAP_DIMENSION*MAP_DIMENSION),0);

    if (simulation_file != 0)
    {
        result = plat_file_in(simulation_file,0,&length);
        data = plat_new(length,1);
        result = plat_file_in(simulation_file,data,&length);
        result = sim_filein(data, length);
        plat_free(data);
    }

    if (apescript_file != 0)
    {
        fp = fopen(apescript_file,"r");
        if (fp != 0)
        {
            fclose(fp);
            result = plat_file_in(apescript_file,0,&length);
            data = plat_new(length,1);
            result = plat_file_in(apescript_file,data,&length);
            result = sim_interpret(data, length);
            plat_free(data);
        }
        else
        {
            printf("No apescript file found: %s\n", apescript_file);
        }
    }

    while(ticker < TIME_SLICE)
    {
        sim_cycle();
        ticker ++;
        if((ticker&0x000000ff)==0)
        {
            n_uint		total_val, female_val, male_val;
            n_int			w_dx, w_dy;
            noble_simulation *local_sim = sim_sim();

            sim_populations(&total_val, &female_val, &male_val);
            weather_wind_vector(local_sim->land, local_sim->weather, 0, 0, &w_dx, &w_dy);

            printf("%d to %d...t: %d f:%d m: %d  wind vector at (0,0) ( %d , %d )\n",(int)ticker,(int)TIME_SLICE, (int)total_val, (int)female_val, (int)male_val, (int)w_dx, (int)w_dy);
        }
    }

    data = sim_fileout(&length);
    result = plat_file_out(output_file,data,length);
    plat_free(data);

    /*
    data = sim_fileoutXML(&length);
    result = plat_file_out(output_xml,data,length);
    plat_free(data);
    */

#ifdef NOBLE_WEB
    export_social_graph("socialgraph.dot", sim_sim(),0);
    export_social_graph("mategraph.dot", sim_sim(),3);
    printf("Logging\n");
    update_log("noble_ape.csv", sim_sim());
    printf("Plotting population\n");
    plot_log("noble_ape.csv", "population.png", sim_sim(), 3, 512, 100);
    plot_log("noble_ape.csv", "population_parasites.png", sim_sim(), 5, 512, 100);
    plot_log("noble_ape.csv", "average_age.png", sim_sim(), 4, 512, 100);
    printf("Creating web pages\n");
    sim_web(sim_sim(), 5, 0, land_seed);
    printf("Plotting demographics\n");
    plot_status_distribution(sim_sim(), "honor_distribution.png", 256, 256);
    plot_age_demographic("age_demographic.png", sim_sim(), 256, 256);
    plot_heights("heights.png", sim_sim(), 256, 256);

#ifdef MICROBLOGGING_ON
    printf("Microblogging\n");
    update_microblog("noble_ape.csv", sim_sim());
#endif

#endif


    sim_close();
    if(result == 0)
        printf("Ended Successfully\n");
    return(1);
}

