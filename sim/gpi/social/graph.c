/****************************************************************
 
	graph.c
 
 =============================================================
 
 Copyright 1996-2011 Tom Barbalet. All rights reserved.
 
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

/*NOBLEMAKE VAR=""*/


#ifndef	_WIN32

#include "../../noble/noble.h"
#include "../../universe/universe.h"

#else

#include "..\..\noble\noble.h"
#include "..\..\universe\universe.h"

#endif

#ifdef	_WIN32
#include "..\..\gpi\gpi.h"
#else
#include "../../gpi/gpi.h"
#endif

#undef  TRACE_OUTPUT

#define	SPACESIZE	(GPI_DIMENSION_X*(GPI_DIMENSION_Y>>3))

unsigned char	newSpace[SPACESIZE];

unsigned char 	gpi_key(unsigned short num);
unsigned char  	gpi_mouse(short px, short py);
void * 		    gpi_init(unsigned long kseed);
long            gpi_cycle(unsigned char first);

void        	gpi_exit();

noble_simulation * local_sim;

n_int draw_error(n_byte * error_text){
	return -1;
}

void gpi_exit()
{
	sim_close();
}


unsigned char gpi_key(unsigned short num) {
	return 0;
}

unsigned char  gpi_mouse(short px, short py){
	return 0;
}



void * gpi_init(unsigned long kseed){
  (void)(sim_init(2, kseed, MAP_AREA, 0, 0));
  plat_erase(newSpace);
	return((void *)newSpace);
}

long   gpi_cycle(unsigned char first){		
	if(first){
		noble_simulation	*local_sim = sim_sim();
		n_uint			loop = 0;
		sim_cycle();
		while (loop < local_sim->num)
		{
			n_byte2	px = (local_sim->beings[loop].social_x)>>7;
			n_byte2 py = (local_sim->beings[loop].social_y)>>7;
			plat_pointset(newSpace, px & 511, py & 511);
			loop++;
		}
	}
  else
  {
#ifndef TRACE_OUTPUT
    plat_erase(newSpace);
#endif
  }
	return 0;
}

