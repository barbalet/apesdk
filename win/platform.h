/****************************************************************
 platform.h

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
#ifndef _NOBLEAPE_PLATFORM_H_
#define _NOBLEAPE_PLATFORM_H_

#define FILE_NEW_HANDLE				  40101
#define FILE_OPEN_HANDLE			  40102
#define FILE_OPEN_SCRIPT_HANDLE		  40103
#define SCRIPT_ADDITION				  1
#define FILE_CLOSE_HANDLE			  (40103 + SCRIPT_ADDITION)
#define FILE_SAVE_AS_HANDLE			  (40104 + SCRIPT_ADDITION)
#define FILE_EXIT_HANDLE			  (40105 + SCRIPT_ADDITION)

#define EDIT_UNDO_HANDLE			  40256
#define EDIT_CUT_HANDLE				  40257
#define EDIT_COPY_HANDLE			  40258
#define EDIT_PASTE_HANDLE			  40259
#define EDIT_CLEAR_HANDLE			  40260

#define CONTROL_PAUSE_HANDLE		  40017

#define CONTROL_PREV_HANDLE			  40019
#define CONTROL_NEXT_HANDLE			  40020

#define CONTROL_CLEAR_ERRORS		  40021

#define CONTROL_WEATHER_HANDLE        40022
#define CONTROL_BRAIN_HANDLE          40023
#define CONTROL_BRAINCODE_HANDLE      40024
#define CONTROL_TERRITORY_HANDLE      40025
#define CONTROL_DAYLIGHT_TIDES_HANDLE 40026

#define CONTROL_WEATHER_HANDLE        40022
#define CONTROL_WEATHER_HANDLE        40022




#define HELP_ABOUT_HANDLE			40254

#define NOBLE_APE_FILE_OPEN         "All Files (*.*)\0*.*\0"
#define	NOBLE_APE_FILE_SAVE			"All Files (*.*)\0*.*\0"
#endif
