/****************************************************************

 web.h

 =============================================================

 Copyright 2010 Bob Mottram. All rights reserved.

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

#ifndef _NOBLEAPE_WEB_H_
#define _NOBLEAPE_WEB_H_

void landscape_png(
    n_int map_tx, n_int map_ty, n_int map_bx, n_int map_by,
    char* img_fil,
    n_land * local_land,
    n_weather * local_weather,
    n_int num,
    noble_being * beings,
    n_int img_width,
    n_int img_width_small,
    n_int weather_dimension,
    n_int contours,
    n_int greenblue,
    n_int weather_symbol_grid_dimension,
    n_byte show_wind,
    n_byte show_cloud,
    n_byte show_beings,
    n_byte show_weather_symbols);

void draw_vegetation(char * filename, n_int img_width, n_land * local_land, n_weather * local_weather);
void draw_brain(char * filename, noble_simulation * sim, int index, n_int img_width, n_int img_height);
void create_web_ape_profile(char * filename, char * profile_picture, char* brain_picture, noble_simulation * sim, int index);
void create_web_title(
    char * filename, char * title, char * subtitle,
    char * title_ape_map, char * fil_ape_map, char * link_ape_map,
    char * title_weather_map, char * fil_weather_map, char * link_weather_map,
    char * title_vegetation_map, char * fil_vegetation_map, char * link_vegetation_map,
    char * title_territory_map, char * fil_territory_map, char * link_territory_map,
    char * title_ideosphere, char * fil_ideosphere, char * link_ideosphere,
    char * title_braincode, char * fil_braincode, char * link_braincode,
    char * title_relationship_matrix, char * fil_relationship_matrix, char * link_relationship_matrix,
    char * title_genespace, char * fil_genespace, char * link_genespace,
    char * title_genepool, char * fil_genepool, char * link_genepool,
    char * title_pathogens, char * fil_pathogens, char * link_pathogens,
    char * title_population, char * fil_population,
    char * title_parasite_population, char * fil_parasite_population,
    char * title_average_age, char * fil_average_age,
    char * title_honor_distribution, char * fil_honor_distribution,
    char * title_demographics, char * fil_demographics,
    char * title_heights, char * fil_heights,
    char * fil_socialgraph, char * fil_mategraph,
    char * footer_text, noble_simulation * sim, char * honors_title, char * births_title,
    n_int land_random_seed, n_int being_profiles);
void update_log(char * filename, noble_simulation * sim);
void plot_log(char * log_filename, char * plot_filename, noble_simulation * sim, n_byte plot_type, n_int img_width, n_int img_height);
void plot_status_distribution(noble_simulation * sim, char * filename, n_int img_width, n_int img_height);
void plot_age_demographic(char * filename, noble_simulation * sim, n_int img_width, n_int img_height);
void plot_heights(char * filename, noble_simulation * sim, n_int img_width, n_int img_height);
void plot_ideosphere(noble_simulation * sim, char * filename, n_int img_width, n_int img_height);
void plot_genespace(noble_simulation * sim, char * filename, n_int img_width, n_int img_height);
void plot_territory(char * filename, noble_simulation * sim, n_int img_width, n_int img_height);
void plot_genepool(noble_simulation * sim, char * filename, n_int img_width, n_int img_height);
void plot_pathogens(noble_simulation * sim, char * filename, n_int img_width, n_int img_height);
void plot_braincode(noble_simulation * sim, char * filename, n_int img_width, n_int img_height);
void plot_relationshipmatrix(noble_simulation * sim, char * filename, n_int img_width, n_int img_height);
void export_social_graph(char * filename, noble_simulation * sim, n_int graph_type);

#endif

