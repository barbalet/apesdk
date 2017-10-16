/****************************************************************
 
	stdio_file.c - Noble Warfare STDIO File Handling

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
 
#include "../../noble/noble.h"

#include "battle.h"
#include <stdio.h>
#include <stdlib.h>




n_int  read_n_file(n_file * result);

void debug_file(char * filename, int number, char * string)
{
  FILE * debug = fopen("debug.txt", "a");
  fprintf(debug, "%s %d %s\n",filename, number, string);
  fclose(debug);
}

/* DEPRECATED */
n_int  read_n_file(n_file * result) {
	n_uint		loc_length = 0;
	n_uint		compare_size = result->size;
	n_byte	   *vc = result->data;

	FILE	   *read = NULL;
	read = fopen("battle.txt","rb");

	if(read == NULL)
		return SHOW_ERROR("File didn't open");
	do {
		fread(&vc[loc_length++],1,1,read);
	} while(!feof(read) && (loc_length < compare_size));
	fclose(read);
	if(loc_length == compare_size) {
		return SHOW_ERROR("File larger than allocated");
	}
	result->size = loc_length;
	result->location = 0;
	return 0;
}

