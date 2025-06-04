/****************************************************************

 apesdk_include.h

 =============================================================

 Copyright 1996-2024 Tom Barbalet. All rights reserved.

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

/*! \file   apesdk_include.h
 *  \brief  This contains the functions exposed for dll usage.
 */

#ifndef _APESDK_INCLUDE_H_
#define _APESDK_INCLUDE_H_

#ifndef __cplusplus
#include "../toolkit/toolkit.h"
#else
typedef unsigned long long n_uint;
typedef unsigned char n_byte;
typedef n_char *n_string;
#endif

#define APESDK_API   __declspec( dllexport )

#ifdef __cplusplus
extern "C" 
{
#endif

APESDK_API void * apesdk_init( n_uint randomise, n_uint offscreen_size, n_uint landbuffer_size );

APESDK_API void apesdk_close( void );

APESDK_API void apesdk_cycle( n_uint ticks );

APESDK_API n_byte * apesdk_draw( n_int fIdentification, n_int dim_x, n_int dim_y );

APESDK_API n_string apesdk_get_output_string( void );

#ifdef __cplusplus
}
#endif
#endif  //_APESDK_INCLUDE_H_