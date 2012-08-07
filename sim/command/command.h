/****************************************************************

 command.h

 =============================================================

 Copyright 1996-2012 Tom Barbalet. All rights reserved.

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

#ifndef _NOBLEAPE_COMMAND_H_
#define _NOBLEAPE_COMMAND_H_


#define GENEALOGY_ON

enum GENEALOGY_FORMAT
{
    GENEALOGY_NONE = 0,
    GENEALOGY_GENXML,
    GENEALOGY_GEDCOM,
    GENEALOGY_FORMATS
};

void genealogy_log(noble_simulation * sim,n_byte value);
n_int genealogy_save(noble_simulation * sim, n_string filename);
void genealogy_birth(noble_being * child, noble_being * mother, void * sim);
void genealogy_death(noble_being * local_being, void * sim);
/*
n_uint braincode_standard_deviation(noble_being * local_being);
void braincode_number_of_instructions(
    noble_being * local_being,
    n_int * no_of_sensors,
    n_int * no_of_actuators,
    n_int * no_of_operators,
    n_int * no_of_conditionals,
    n_int * no_of_data);
*/
/* graph functions */
/*void graph_line(n_byte * buffer, n_int img_width, n_int img_height, n_int prev_x, n_int prev_y, n_int x, n_int y, n_byte r,n_byte g,n_byte b,n_byte thickness);*/
void graph_honor_distribution(noble_simulation * sim, n_byte * buffer, n_int img_width, n_int img_height);
void graph_ideosphere(noble_simulation * sim, n_byte * buffer, n_int img_width, n_int img_height);
void graph_genepool(noble_simulation * sim, n_byte * buffer, n_int img_width, n_int img_height);
void graph_relationship_matrix(noble_simulation * sim, n_byte * buffer, n_int img_width, n_int img_height);
void graph_pathogens(noble_simulation * sim, n_byte * buffer, n_int img_width, n_int img_height);
void graph_age_demographic(noble_simulation * sim, n_byte * buffer, n_int img_width, n_int img_height);
void graph_heights(noble_simulation * sim, n_byte * buffer, n_int img_width, n_int img_height);
void graph_phasespace(noble_simulation * sim, n_byte * buffer, n_int img_width, n_int img_height, n_byte graph_type, n_byte data_type);
void graph_braincode(noble_simulation * sim, noble_being * local_being, n_byte * buffer, n_int img_width, n_int img_height, n_byte clear);

void graph_preferences(noble_simulation * sim, n_byte * buffer, n_int img_width, n_int img_height);

#endif /* _NOBLEAPE_COMMAND_H_ */
