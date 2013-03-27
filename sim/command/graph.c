/****************************************************************

 graph.c

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

/*NOBLEMAKE DEL=""*/

#ifndef	_WIN32

#include "../noble/noble.h"
#include "../universe/universe.h"
#include "../entity/entity.h"


#else

#include "..\noble\noble.h"
#include "..\universe\universe.h"
#include "..\entity\entity.h"

#endif

#include <stdio.h>
#include "command.h"

/* draws a line */
static void graph_line(n_byte * buffer, n_int img_width, n_int img_height, n_int prev_x, n_int prev_y, n_int x, n_int y, n_byte r,n_byte g,n_byte b,n_byte thickness)
{
    n_int i,max;

    n_int dx = x-prev_x;
    n_int dy = y-prev_y;
    n_int abs_dx = dx;
    n_int abs_dy = dy;


    if (dx < 0) abs_dx = -dx;
    if (dy < 0) abs_dy = -dy;

    max = abs_dx;
    if (abs_dy > max) max = abs_dy;

    for (i=0; i<max; i++)
    {
        n_int xx = prev_x + (i*(x-prev_x)/max);
        if ((xx > -1) && (xx < img_width))
        {
            n_int yy = prev_y + (i*(y-prev_y)/max);
            if ((yy > -1) && (yy < img_height))
            {
                n_int n = (yy*img_width + xx)*3;
                buffer[n] = r;
                buffer[n+1] = g;
                buffer[n+2] = b;
            }
        }
    }
}

/* Shows distribution of honor.  Note that beings are sorted in order of honor */
void graph_honor_distribution(noble_simulation * sim, n_byte * buffer, n_int img_width, n_int img_height)
{
    n_int i,j;
    n_int prev_x = -1, prev_y=-1;
    n_int x,y;
    for (i = 0; i < img_width*img_height*3; i++) buffer[i]=255;

    for (i = 0; i < sim->num; i++)
    {
        noble_being * local_being = &(sim->beings[i]);
        n_int idx = i;
        n_int max = local_being->honor;
        for (j = i+1; j < sim->num; j++)
        {
            noble_being * local_being2 = &(sim->beings[j]);
            if (local_being2->honor > max)
            {
                idx = j;
                max = local_being2->honor;
            }
        }
        if (idx != i)
        {
            noble_being * temp = &(sim->beings[idx]);
            sim->beings[idx] = *local_being;
            sim->beings[i] = *temp;
        }

        x = i*img_width/sim->num;
        y = img_height-1-(max*img_height/255);
        if (prev_x > -1) graph_line(buffer,img_width,img_height,prev_x,prev_y,x,y,0,0,0,1);
        prev_x = x;
        prev_y = y;
    }
}

static n_int graph_being_score(noble_simulation * sim, noble_being * local_being, n_byte score_type)
{
    n_int nucleotide,i,score = 0;
    n_byte * bases = (n_byte*)GET_G(local_being);

    switch (score_type)
    {
    case 0:
        for (i = 0; i < BRAINCODE_SIZE; i++)
        {
            score += GET_BRAINCODE_EXTERNAL(sim,local_being)[i] + GET_BRAINCODE_INTERNAL(sim,local_being)[i];
        }
        break;
    case 1:
        for (i = 0; i < CHROMOSOMES; i++)
        {
            for (nucleotide = 0; nucleotide < 8; nucleotide++)
            {
                score += (bases[i]>>(nucleotide*2)) & 3;
            }
        }
        break;
    }
    return score;
}

/*
  Updates an index array which is used to sort beings in order of honor value
 */
static void graph_being_index(noble_simulation * sim, n_int *index, n_byte score_type)
{
#ifdef BRAINCODE_ON
#ifdef PARASITES_ON
    n_int i,score;
    n_byte * used = (unsigned char*)io_new(sim->num);
    for (i = 0; i < sim->num; i++)
    {
        used[i] = 0;
        index[i] = i;
    }
    /* sort by honor value */
    for (i = 0; i < sim->num; i++)
    {
        if (used[i]==0)
        {
            n_int j;
            n_int max = -1;
            n_int idx = -1;
            for (j = 0; j < sim->num; j++)
            {
                if (used[j]==0)
                {
                    noble_being * local_being = &(sim->beings[j]);
                    score = graph_being_score(sim, local_being,score_type);
                    if (score>max)
                    {
                        max = score;
                        idx = j;
                    }
                }
            }
            if (idx>-1)
            {
                index[i] = idx;
                used[idx] = 1;
            }
        }
    }
    io_free((void*)used);
#endif
#endif
}

/*
   Displays the braincode programs (one per row) for each being in the population.
   Colors represent different instruction types, and individuals are sorted by honor.
 */
void graph_ideosphere(noble_simulation * sim, n_byte * buffer, n_int img_width, n_int img_height)
{
#ifdef BRAINCODE_ON
#ifdef PARASITES_ON
    n_int i,x,y,n,half_width,max_height=img_height;
    n_int *index = (n_int*)io_new((sim->num)*sizeof(n_int));
    noble_being * local_being;
    n_byte * code;

    graph_being_index(sim, index,0);

    /* clear the image */
    for (i = 0; i < img_width*img_height*3; i++) buffer[i]=0;

    half_width = img_width/2;
    n = 0;
    if (sim->num>0)
    {
        if (sim->num*2 < max_height)
        {
            max_height = sim->num*2;
        }
        else
        {
            if (sim->num < max_height)
            {
                max_height = sim->num;
            }
        }

        for (y = 0; y < max_height; y++)
        {
            i = index[y*(sim->num-1)/max_height];
            local_being = &(sim->beings[i]);
            for (x = 0; x < img_width; x++, n+=3)
            {
                if (x<half_width)
                {
                    i = (x * ((BRAINCODE_SIZE/BRAINCODE_BYTES_PER_INSTRUCTION)-1) / half_width)*BRAINCODE_BYTES_PER_INSTRUCTION;
                    code = GET_BRAINCODE_INTERNAL(sim,local_being);
                }
                else
                {
                    i = ((x-half_width) * ((BRAINCODE_SIZE/BRAINCODE_BYTES_PER_INSTRUCTION)-1) / half_width)*BRAINCODE_BYTES_PER_INSTRUCTION;
                    code = GET_BRAINCODE_EXTERNAL(sim,local_being);
                }
                buffer[n] = code[i];
                buffer[n+1] = code[i+1];
                buffer[n+2] = code[i+2];
            }
        }
    }

    io_free((void*)index);
#endif
#endif
}

/*
  Shows the genome for each individual in the population, with one genome per row.
  Individuals are sorted in order of honor.
 */
void graph_genepool(noble_simulation * sim, n_byte * buffer, n_int img_width, n_int img_height)
{
#ifdef PARASITES_ON
    n_int i,x,y,n,ch,idx,nucleotide,max_height=img_height;
    n_genetics * bases;
    n_int * index;
    noble_being * local_being;

    const n_byte col[] =
    {
        200,0,0,	0,200,0,	0,0,200,	200,200,0
    };

    index = (n_int*)io_new(sim->num*sizeof(n_int));
    graph_being_index(sim,index,1);

    /* clear the image */
    for (i = 0; i < img_width*img_height*3; i++) buffer[i]=0;

    n = 0;
    if (sim->num>0)
    {
        if (sim->num*2 < max_height)
        {
            max_height = sim->num*2;
        }
        else
        {
            if (sim->num < max_height)
            {
                max_height = sim->num;
            }
        }

        for (y = 0; y < max_height; y++)
        {
            i = index[y*(sim->num-1)/max_height];
            local_being = &(sim->beings[i]);
            bases = GET_G(local_being);
            for (x = 0; x < img_width; x++, n+=3)
            {
                nucleotide = x * (CHROMOSOMES*16) / img_width;
                ch = nucleotide>>4;
                idx = ((bases[ch]>>((nucleotide-(16*ch))*2))&3)*3;
                buffer[n] = col[idx++];
                buffer[n+1] = col[idx++];
                buffer[n+2] = col[idx];
            }
        }
    }

    io_free((void*)index);
#endif
}

/*
  A matrix showing the relationships between beings.
  Green squares represent friendly relationships, red represent unfriendly relationships
  and black represents "don't care"
  Individuals are plotted on each axis in the same (honor sorted) order, such that a being's
  relationship with itself is along the diagonal axis
 */
void graph_relationship_matrix(noble_simulation * sim, n_byte * buffer, n_int img_width, n_int img_height)
{
#ifdef PARASITES_ON
    n_int i,j,k,x,y;

    n_int *index = (n_int*)io_new(sim->num*sizeof(n_int));
    graph_being_index(sim,index,1);

    for (i = 0; i < img_width*img_height*3; i++) buffer[i]=255;

    for (i = 0; i < sim->num; i++)
    {
        noble_being * local_being = &(sim->beings[index[i]]);
        social_link * graph = GET_SOC(sim, local_being);
        n_uint respect_threshold = social_respect_mean(sim, local_being);
        n_int tx = i*(img_width-1)/sim->num;
        n_int bx = (i+1)*(img_width-1)/sim->num;
        for (j = 0; j < SOCIAL_SIZE; j++)
        {
            if (!SOCIAL_GRAPH_ENTRY_EMPTY(graph, j))
            {
                if (j == 0)
                {
                    k = i;
                }
                else
                {
                    for (k = 0; k < sim->num; k++)
                    {
                        noble_being * local_being2 = &(sim->beings[index[k]]);
                        if ((GET_NAME_FAMILY2(sim, local_being2) == graph[j].family_name[BEING_MET]) &&
                                (GET_NAME(sim,local_being2) == UNPACK_FAMILY_FIRST_NAME(graph[j].first_name[BEING_MET])))
                        {
                            break;
                        }
                    }
                }

                if (k < sim->num)
                {
                    n_int ty = k*(img_height-1)/sim->num;
                    n_int by = (k+1)*(img_height-1)/sim->num;

                    for (y=ty; y<by; y++)
                    {
                        for (x=tx; x<bx; x++)
                        {
                            n_int v, n = ((y*img_width)+x)*3;
                            if (graph[j].friend_foe>=respect_threshold)
                            {
                                v = (graph[j].friend_foe - respect_threshold)*8;
                                if (v>255) v=255;
                                buffer[n] = 0;
                                buffer[n+1] = v;
                                buffer[n+2] = 0;
                            }
                            else
                            {
                                v = (respect_threshold - graph[j].friend_foe)*8;
                                if (v>255) v=255;
                                buffer[n] = v;
                                buffer[n+1] = 0;
                                buffer[n+2] = 0;
                            }
                        }
                    }
                }
            }
        }
    }

    io_free((void*)index);
#endif
}

/*
  Draws the number of antigens and antibodies in the population
  There are 256 possible antigens/antibodies which are along the horizontal axis.
  Antigens are shown in red and antibodies in green.
 */
void graph_pathogens(noble_simulation * sim, n_byte * buffer, n_int img_width, n_int img_height)
{
    n_c_uint i;
    n_c_uint * antibodies;
    n_c_uint * antigens;
#ifdef IMMUNE_ON
    noble_being * local_being;
    n_int j,n,p;
    n_int max_val=1,max,x,y;
    noble_immune_system * immune;
#endif

    antibodies = (n_c_uint*)io_new(256*sizeof(n_c_uint));
    antigens = (n_c_uint*)io_new(256*sizeof(n_c_uint));

    for (i = 0; i < img_width*img_height*3; i++) buffer[i]=255;

    for (i=0; i<256; i++)
    {
        antibodies[i]=0;
        antigens[i]=0;
    }

#ifdef IMMUNE_ON
    if (sim->num>0)
    {
        /* update histograms */
        for (p=0; p<256; p++)
        {
            for (i=0; i<sim->num; i++)
            {
                local_being = &(sim->beings[i]);
                immune = &(local_being->immune_system);
                for (j=0; j<IMMUNE_POPULATION; j++)
                {
                    antibodies[immune->shape_antibody[j]]++;
                }
                for (j=0; j<IMMUNE_ANTIGENS; j++)
                {
                    antigens[immune->shape_antigen[j]]++;
                }
            }
        }

        for (p=0; p<256; p++)
        {
            if (antibodies[p]>max_val)
            {
                max_val=antibodies[p];
            }
            if (antigens[p]>max_val)
            {
                max_val=antigens[p];
            }
        }

        for (p=0; p<256; p++)
        {
            x = p*img_width/256;
            max = antibodies[p]*img_height/max_val;
            for (y=0; y<max; y++)
            {
                n = (((img_height-1-y)*img_width)+x)*3;
                buffer[n]=0;
                buffer[n+2]=0;
            }
            max = antigens[p]*img_height/max_val;
            for (y=0; y<max; y++)
            {
                n = (((img_height-1-y)*img_width)+x)*3;
                buffer[n+1]=0;
                buffer[n+2]=0;
            }
        }
    }
#endif

    io_free((void*)antibodies);
    io_free((void*)antigens);
}

/*
  Shows a histogram of ages
 */
void graph_age_demographic(noble_simulation * sim, n_byte * buffer, n_int img_width, n_int img_height)
{
    const n_int max_age = AGE_OF_MATURITY*4;
    const n_int divisor = 4;
    n_int groups = max_age/divisor;
    n_int * age_group;
    n_int i,idx,max=1;
    n_int prev_x = 0, prev_y=img_height-1;
    n_int x,y;
    n_uint current_date;

    age_group = (n_int*)io_new(groups*sizeof(n_int));
    for (i=0; i<groups; i++)
    {
        age_group[i]=0;
    }
    for (i = 0; i < img_width*img_height*3; i++) buffer[i]=255;

    current_date = TIME_IN_DAYS(sim->land->date);

    for (i = 0; i < sim->num; i++)
    {
        noble_being * local_being = &(sim->beings[i]);
        n_uint local_dob = TIME_IN_DAYS(GET_D(local_being));
        n_int age_days = current_date - local_dob;
        if (age_days >= max_age) age_days = max_age-1;
        idx = age_days/divisor;
        age_group[idx]++;
        if (age_group[idx] > max) max = age_group[idx];
    }
    max = max * 120/100;
    for (i = 0; i < groups; i++)
    {
        x = i*img_width/groups;
        y = img_height-1-(age_group[i]*img_height/max);
        graph_line(buffer,img_width,img_height,prev_x,prev_y,x,y,0,0,0,1);
        prev_x = x;
        prev_y = y;
    }
    io_free((void*)age_group);
}

/*
  Show a histogram of being heights
 */
void graph_heights(noble_simulation * sim, n_byte * buffer, n_int img_width, n_int img_height)
{
    const n_int divisor = BEING_MAX_HEIGHT/16;
    n_int groups = BEING_MAX_HEIGHT/divisor;
    n_int * height_group;
    n_int i,idx,max=1;
    n_int prev_x = 0, prev_y=img_height-1;
    n_int x,y;

    height_group = (n_int*)io_new(groups*sizeof(n_int));
    for (i=0; i<groups; i++)
    {
        height_group[i]=0;
    }
    for (i = 0; i < img_width*img_height*3; i++) buffer[i]=255;

    for (i = 0; i < sim->num; i++)
    {
        noble_being * local_being = &(sim->beings[i]);
        idx = GET_H(local_being)/divisor;
        height_group[idx]++;
        if (height_group[idx] > max) max = height_group[idx];
    }
    max = max * 120/100;
    for (i = 0; i < groups; i++)
    {
        x = i*img_width/groups;
        y = img_height-1-(height_group[i]*img_height/max);
        graph_line(buffer,img_width,img_height,prev_x,prev_y,x,y,0,0,0,1);
        prev_x = x;
        prev_y = y;
    }

    io_free((void*)height_group);
}

/* return the braincode standard deviation */
/*
static n_uint braincode_standard_deviation(noble_simulation * sim, noble_being * local_being)
{
    n_uint sd = 0;
#ifdef BRAINCODE_ON
    n_int i,av=0,diff;

    for (i=0; i<BRAINCODE_SIZE; i++)
    {
        av += GET_BRAINCODE_INTERNAL(sim,local_being)[i];
        av += GET_BRAINCODE_EXTERNAL(sim,local_being)[i];
    }
    av /= (BRAINCODE_SIZE*2);

    for (i=0; i<BRAINCODE_SIZE; i++)
    {
        diff = (n_int)(GET_BRAINCODE_INTERNAL(sim,local_being)[i]) - av;
        if (diff<0) diff=-diff;
        sd += (n_uint)(diff);
        diff = (n_int)(GET_BRAINCODE_EXTERNAL(sim,local_being)[i]) - av;
        if (diff<0) diff=-diff;
        sd += (n_uint)(diff);
    }
#endif
    return sd;
}
*/

/* return the number of instruction_types in the braincode */
/*
static void braincode_number_of_instructions(
   noble_simulation * sim,
   noble_being * local_being,
   n_int * no_of_sensors,
   n_int * no_of_actuators,
   n_int * no_of_operators,
   n_int * no_of_conditionals,
   n_int * no_of_data)
{
#ifdef BRAINCODE_ON
   n_int i,j,instruction;

   *no_of_sensors = 0;
   *no_of_actuators = 0;
   *no_of_operators = 0;
   *no_of_conditionals = 0;
   *no_of_data = 0;

   for (i=0; i<BRAINCODE_SIZE; i+=3)
   {
       for (j=0; j<2; j++)
       {
           if (j==0)
           {
               instruction = GET_BRAINCODE_INTERNAL(sim,local_being)[i] & 63;
           }
           else
           {
               instruction = GET_BRAINCODE_EXTERNAL(sim,local_being)[i] & 63;
           }
           if ((instruction >= BRAINCODE_SENSORS_START) && (instruction < BRAINCODE_ACTUATORS_START))
           {
               *no_of_sensors = *no_of_sensors + 1;
           }
           if ((instruction >= BRAINCODE_ACTUATORS_START) && (instruction < BRAINCODE_OPERATORS_START))
           {
               *no_of_actuators = *no_of_actuators + 1;
           }
           if ((instruction >= BRAINCODE_OPERATORS_START) && (instruction < BRAINCODE_CONDITIONALS_START))
           {
               *no_of_operators = *no_of_operators + 1;
           }
           if ((instruction >= BRAINCODE_CONDITIONALS_START) && (instruction < BRAINCODE_DATA_START))
           {
               *no_of_conditionals = *no_of_conditionals + 1;
           }
           if ((instruction >= BRAINCODE_DATA_START) && (instruction < BRAINCODE_INSTRUCTIONS))
           {
               *no_of_data = *no_of_data + 1;
           }
       }
   }
#endif
}
*/
/* return coordinates of the braincode system for phase space plot */
static void graph_braincode_coords(noble_simulation * sim, noble_being * local_being, n_uint * x, n_uint * y)
{
    n_int i;
    *x=0;
    *y=0;
    for (i=0; i<BRAINCODE_SIZE; i++)
    {
        *x = *x + GET_BRAINCODE_INTERNAL(sim,local_being)[i];
        *y = *y + GET_BRAINCODE_EXTERNAL(sim,local_being)[i];
    }
}

/* return coordinates of the genome for phase space plot */
static void graph_genespace_coords(noble_being * local_being, n_uint * x, n_uint * y)
{
    n_int ch,b;
    *x=0;
    *y=0;
    for (ch=0; ch<CHROMOSOMES; ch++)
    {
        for (b=0; b<8; b++)
        {
            *x = *x + ((GET_G(local_being)[ch]>>(b*2))&3);
            *y = *y + ((GET_G(local_being)[ch]>>(16+(b*2)))&3);
        }
    }
}

static void graph_phasespace_dots(noble_simulation * sim, n_byte * buffer, n_int img_width, n_int img_height, n_byte graph_type)
{
#ifdef PARASITES_ON
    const int grid = 32;
    n_uint i,x=0,y=0,n;

    /* clear the image */
    for (i = 0; i < img_width*img_height*3; i++) buffer[i]=255;

    for (i=0; i<sim->num; i++)
    {
        switch(graph_type)
        {
        case 0:
            graph_braincode_coords(sim, &(sim->beings[i]), &x, &y);
            x = x * (img_width-1) / (256*BRAINCODE_SIZE);
            y = (img_height-1) - (y * (img_height-1) / (255*BRAINCODE_SIZE));
            break;
        case 1:
            graph_genespace_coords(&(sim->beings[i]), &x, &y);
            x = x * img_width / (4*8*CHROMOSOMES);
            y = img_height - 1 - (y * img_height / (4*8*CHROMOSOMES));
            break;
        }

        n = ((y*img_width)+x)*3;
        buffer[n] = 0;
        buffer[n+1] = 0;
        buffer[n+2] = 0;
        if (x>0)
        {
            buffer[n-3] = 0;
            buffer[n-2] = 0;
            buffer[n-1] = 0;
        }
    }
#endif
}

static void graph_phasespace_density(noble_simulation * sim, n_byte * buffer, n_int img_width, n_int img_height, n_byte graph_type)
{
#ifdef BRAINCODE_ON
    const n_int grid = 32;
    n_uint x=0,y=0,n0,n1,i,tx,ty,bx,by,xx,yy;
    n_uint density[32*32],max=1;
    n_byte r,b;

    for (i = 0; i < img_width*img_height*3; i+=3)
    {
        buffer[i]=0;
        buffer[i+1]=0;
        buffer[i+2]=255;
    }
    for (i = 0; i < grid*grid; i++) density[i]=0;

    for (i=0; i < sim->num; i++)
    {
        switch(graph_type)
        {
        case 0:
            graph_braincode_coords(sim, &(sim->beings[i]), &x, &y);
            x = x * (grid-1) / (256*BRAINCODE_SIZE);
            y = (grid-1) - (y * (grid-1) / (255*BRAINCODE_SIZE));
            break;
        case 1:
            graph_genespace_coords(&(sim->beings[i]), &x, &y);
            x = x * (grid-1) / (4*8*CHROMOSOMES);
            y = (grid-1) - (y * (grid-1) / (4*8*CHROMOSOMES));
            break;
        }
        density[y*grid+x]++;
    }

    for (i = 0; i < grid*grid; i++)
    {
        if (density[i] > max) max = density[i];
    }

    n0 = 0;
    for (y = 0; y < grid; y++)
    {
        for (x = 0; x < grid; x++, n0++)
        {
            if (density[n0] > 0)
            {
                r = density[n0]*255/max;
                b = 255-r;

                tx = x * img_width / grid;
                bx = (x+1) * img_width / grid;
                ty = y * img_height / grid;
                by = (y+1) * img_height / grid;

                for (yy = ty; yy < by; yy++)
                {
                    n1 = ((yy*img_width)+tx)*3;
                    for (xx = tx; xx < bx; xx++, n1+=3)
                    {
                        buffer[n1] = r;
                        buffer[n1+1] = 0;
                        buffer[n1+2] = b;
                    }
                }
            }
        }
    }
#endif
}

void graph_phasespace(noble_simulation * sim, n_byte * buffer, n_int img_width, n_int img_height, n_byte graph_type, n_byte data_type)
{
    if (graph_type==0)
    {
        graph_phasespace_dots(sim, buffer, img_width, img_height,data_type);
    }
    else
    {
        graph_phasespace_density(sim, buffer, img_width, img_height,data_type);
    }
}

/*
   Displays the braincode program for an individual
 */
void graph_braincode(noble_simulation * sim, noble_being * local_being, n_byte * buffer, n_int img_width, n_int img_height, n_byte clear)
{
#ifdef BRAINCODE_ON
#ifdef PARASITES_ON
    n_int i,x,y,n,half_width;
    n_byte * code;

    if (local_being!=0)
    {
        /* clear the image */
        if (clear!=0) for (i = 0; i < img_width*img_height*3; i++) buffer[i]=0;

        half_width = img_width/2;
        y = sim->land->time % img_height;
        n = y*img_width*3;
        for (x = 0; x < img_width; x++, n+=3)
        {
            if (x<half_width)
            {
                i = (x * ((BRAINCODE_SIZE/BRAINCODE_BYTES_PER_INSTRUCTION)-1) / half_width)*BRAINCODE_BYTES_PER_INSTRUCTION;
                code = GET_BRAINCODE_INTERNAL(sim,local_being);
            }
            else
            {
                i = ((x-half_width) * ((BRAINCODE_SIZE/BRAINCODE_BYTES_PER_INSTRUCTION)-1) / half_width)*BRAINCODE_BYTES_PER_INSTRUCTION;
                code = GET_BRAINCODE_EXTERNAL(sim,local_being);
            }
            buffer[n] = code[i];
            buffer[n+1] = code[i+1];
            buffer[n+2] = code[i+2];
        }
    }

#endif
#endif
}

/*
   Displays the preferences of the population
 */
void graph_preferences(noble_simulation * sim, n_byte * buffer, n_int img_width, n_int img_height)
{
    n_int i,p,x=0,y=0,n,half=PREFERENCES/2;
    noble_being * local_being;

    /* clear the image */
    for (i = 0; i < img_width*img_height*3; i++) buffer[i]=255;

    for (i = 0; i < sim->num; i++)
    {
        local_being = &(sim->beings[i]);
        for (p = 0; p < half; p++)
        {
            x += local_being->learned_preference[p];
        }
        x = x * img_width / (half*255);
        if (x >= img_width-2) x = img_width-2;

        while (p < PREFERENCES)
        {
            y += local_being->learned_preference[p];
            p++;
        }
        y = y * img_height / ((PREFERENCES-half)*255);
        if (y >= img_height-2) y = img_height-2;

        n = (y*img_width+x)*3;

        buffer[n] = 0;
        buffer[n+1] = 0;
        buffer[n+2] = 0;
        buffer[n+3] = 0;
        buffer[n+4] = 0;
        buffer[n+5] = 0;

        n += img_width*3;

        buffer[n] = 0;
        buffer[n+1] = 0;
        buffer[n+2] = 0;
        buffer[n+3] = 0;
        buffer[n+4] = 0;
        buffer[n+5] = 0;
    }
}
