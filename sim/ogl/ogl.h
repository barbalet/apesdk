/****************************************************************

 ogl.h

 =============================================================

 Copyright 1996-2016 Tom Barbalet. All rights reserved.

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

/*! \file   ogl.h
 *  \brief  The OpenGL interface to the Simulation.
 */

/*NOBLEMAKE VAR=""*/

#ifndef _NOBLEAPE_OGL_H_
#define _NOBLEAPE_OGL_H_

void polygonal_init(n_int value);
n_int polygonal_entry(n_int value);
void polygonal_close(void);

void polygonal_color(n_byte value, n_byte alpha);
void polygonal_line(n_int px, n_int py, n_int dx, n_int dy, n_byte value);
void polygonal_line_vect(n_int px, n_int py, n_vect2* vector, n_byte value);


#endif /* _NOBLEAPE_OGL_H_ */

/*NOBLEMAKE END=""*/
