/****************************************************************
 
 gpi.h - The GPI Generic Header
 
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

#define GPI_WINDOW_NAME			"Noble Ape GPI Simulation"
#define GPI_PC_APP_NAME			"NobleApeGPISimulation"

#define	GPI_DIMENSION_X			512
#define	GPI_DIMENSION_Y			512

#define GPI_INIT_FUNCTION		gpi_init

#define GPI_UPDATE_FUNCTION gpi_cycle

#define	GPI_KEY_FUNCTION    gpi_key
#define	GPI_MOUSE_FUNCTION  gpi_mouse

#define GPI_EXIT_FUNCTION   gpi_exit

#define GPI_AUTO_DIRTY      1

void plat_erase(unsigned char * buffer);

int plat_pointget(unsigned char * buffer, int px, int py);

void plat_pointerase(unsigned char * buffer, int px, int py);

void plat_pointset(unsigned char * buffer, int px, int py);

/*NOBLEMAKE END=""*/

