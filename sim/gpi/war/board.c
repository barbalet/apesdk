/****************************************************************
 
	board.c - Noble Warfare Skirmish
 
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

#include "../../gpi/gpi.h"
#include "../../noble/noble.h"
#include "battle.h"

n_byte	*board;

n_byte	board_add(n_int * ptx, n_int * pty);
n_byte	board_move(n_int frx, n_int fry, n_int * ptx, n_int * pty);

void board_remove(n_int ptx, n_int pty);

void board_remove(n_int ptx, n_int pty){
	plat_pointerase(board, ptx, pty);
}

static	n_byte	board_find(n_int * ptx, n_int * pty) {
	n_int px = *ptx;
	n_int py = *pty;
	n_uint	best_dsqu = 0xffffffff;
	n_int	best_x = 0, best_y = 0;
	n_int ly = -1;

	px = (px + GPI_DIMENSION_X) % GPI_DIMENSION_X;
	py = (py + GPI_DIMENSION_Y) % GPI_DIMENSION_Y;

	if(plat_pointget(board, px,py)==0) {
		*ptx = px;
		*pty = py;
		return 1;
	}
	while(ly<2) {
		n_int	lx = -1;
		n_int y_val = (py+ly+GPI_DIMENSION_Y)%GPI_DIMENSION_Y;
		while(lx<2) {
			n_int x_val = (px+lx+GPI_DIMENSION_X)%GPI_DIMENSION_X;
			if(plat_pointget(board, x_val,y_val)==0) {
				n_int dx = (px - lx);
				n_int dy = (py - ly);
				n_uint	dsqu = (dx*dx) + (dy*dy);
				if(dsqu<best_dsqu) {
					best_dsqu = dsqu;
					best_x = x_val;
					best_y = y_val;
				}
			}
			lx++;
		}
		ly++;
	}
	if(best_dsqu != 0xffffffff) {
		*ptx = best_x;
		*pty = best_y;
		return 1;
	}
	return 0;
}

n_byte	board_add(n_int * ptx, n_int * pty) {
	if(board_find(ptx, pty)) {
		plat_pointset(board, *ptx, *pty);
		return 1;
	}
	return 0;
}

n_byte	board_move(n_int frx, n_int fry, n_int * ptx, n_int * pty) {
	if(board_find(ptx, pty)) {
		plat_pointerase(board, frx,fry);
		plat_pointset(board, *ptx, *pty);
		return 1;
	}
	return 0;
}
