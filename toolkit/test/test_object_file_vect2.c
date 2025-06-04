/****************************************************************

 test_object_file.c

 =============================================================

 Copyright 1996-2025 Tom Barbalet. All rights reserved.

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

#include "../toolkit.h"

#include <stdio.h>

const n_string_block json_file_string = "[{\"quad\":[[[532911,398524],[721748,810311],[755460,426889],[116574,264735]],[[922352,505413],[580440,489118],[620286,617767],[496804,237568]],[[388567,352520],[565438,480852],[846748,800472],[812259,740070]],[[222436,693061],[979967,614417],[56338,296005],[169313,165755]],[[73828,275588],[124338,10844],[94638,372180],[467951,550143]],[[29137,639959],[244393,552413],[241100,932718],[24306,701676]],[[759579,485196],[993818,159502],[569762,619885],[425444,420339]],[[447977,650959],[677714,906136],[195771,120360],[820772,49415]],[[467033,332803],[871752,158373],[737647,818125],[393202,584360]],[[754156,237001],[731356,585458],[685869,542506],[88300,699799]],[[757707,392215],[990933,390178],[663395,439843],[822376,508005]],[[379341,485731],[779800,519219],[984460,598122],[152817,434800]],[[978186,854574],[720736,738181],[806483,351160],[485757,791990]],[[684700,379288],[361232,689432],[152025,733699],[232278,891102]],[[669198,104406],[690866,34184],[320286,99434],[813115,752872]],[[413145,187265],[926688,679911],[874662,654279],[955799,333718]],[[375896,648617],[390223,765848],[816933,463980],[429862,415070]]],\"line\":[[[168712,651317],[200564,41875]],[[868603,679432],[957389,245358]],[[240861,575877],[477972,364416]],[[13966,361230],[506414,319332]],[[96447,622649],[14412,433275]],[[266447,422887],[3466,806240]],[[550605,533825],[145462,313668]],[[88314,943376],[148798,694907]],[[186424,244553],[464,32073]],[[182225,560850],[167930,237886]],[[615344,125593],[270169,953528]],[[189,304192],[58647,515284]],[[384209,491105],[424579,863779]],[[679958,68396],[781252,628336]],[[152638,994326],[248394,983492]],[[381594,309937],[760833,471204]],[[378255,94409],[625776,229346]],[[672418,408240],[10251,677189]],[[522979,114115],[992445,727619]],[[927528,214416],[834461,470515]],[[26910,362193],[516174,421073]],[[120348,22574],[163496,866299]],[[997408,482663],[505256,127222]],[[565689,543029],[41300,80865]],[[247953,178844],[585941,664644]],[[883612,785756],[911444,321952]],[[313353,609825],[796332,900068]],[[310440,225307],[644694,691537]]]},{\"quad\":[[[890200,193621],[964527,662216],[630957,483921],[378930,710409]],[[137496,233506],[535797,993567],[912927,702531],[247315,624122]],[[187955,727298],[196616,634972],[176535,955503],[965282,142265]],[[554846,17033],[146089,453898],[981877,997807],[20973,156623]],[[419812,797234],[33863,114591],[570786,753043],[47080,66260]],[[758065,556441],[251409,635898],[913786,211243],[488989,612914]],[[879900,81655],[441215,93752],[791067,316220],[992094,54168]],[[644592,295734],[792741,876740],[900387,972465],[632793,881647]],[[108769,50346],[632779,863273],[231751,610188],[99823,864382]],[[61239,986253],[40491,597373],[733462,1070],[463489,365684]],[[700855,380613],[745681,963240],[104593,9727],[31976,553777]],[[779669,649311],[652459,744507],[374168,604800],[953570,729876]],[[453055,134482],[531913,164745],[41494,305839],[107657,426521]],[[856371,777090],[191270,526415],[1551,378016],[40840,47554]]],\"line\":[[[487161,492059],[156051,39780]],[[94331,666666],[924675,902143]],[[471306,609267],[341323,775103]],[[76228,834039],[533834,792472]],[[700034,676566],[636534,370743]],[[273797,419125],[715404,312863]],[[847309,166986],[691083,349550]],[[924201,155639],[3527,988350]],[[960090,820140],[633344,399818]],[[550211,59409],[651860,392169]],[[640852,960354],[82891,436964]],[[443633,273943],[722452,364053]],[[244030,334011],[395903,288685]],[[183857,417699],[915856,813428]],[[201077,752537],[636493,52663]],[[5221,685799],[864857,941615]],[[252870,905678],[652490,997571]],[[30752,783738],[467708,183871]],[[442417,897081],[858342,45916]],[[411420,16735],[993990,910288]],[[518754,956966],[247548,141416]],[[24350,896284],[64977,123253]],[[948524,690121],[249857,952816]],[[915677,813980],[386127,489362]]]}]";

n_int draw_error( n_constant_string error_text, n_constant_string location, n_int line_number )
{
    if ( error_text )
    {
        printf( "ERROR: %s @ %s %ld\n", ( n_constant_string )error_text, location, line_number );
    }
    return -1;
}

n_file * tof_file( n_string file_in, n_int * file_error)
{
    n_file   *in_file = io_file_new();
    *file_error = io_disk_read( in_file, file_in );

    io_whitespace_json( in_file );

    return in_file;
}

typedef struct
{
    memory_list * two;
    memory_list * four;
} n_line_quad;

n_int unwrap_two_four( n_string pass_through, n_byte * buffer)
{
    n_int something_to_copy = 0;
    n_object * object = obj_get_object(pass_through);
    n_line_quad * two_four = (n_line_quad *) buffer;

    n_string str_two = obj_contains(object, "line", OBJECT_ARRAY);
    n_string str_four = obj_contains(object, "quad", OBJECT_ARRAY);

    two_four->four = 0L;
    two_four->two = 0L;

    if (str_two)
    {
        two_four->two = object_unwrap_array(obj_get_array(str_two), sizeof(n_line), vect2_unwrap_line, OBJECT_ARRAY);
        something_to_copy = 1;
    }

    if (str_four)
    {
        two_four->four = object_unwrap_array(obj_get_array(str_four), sizeof(n_quad), vect2_unwrap_quad, OBJECT_ARRAY);
        something_to_copy = 1;
    }

    return something_to_copy;
}

n_object * tof_object_entry(n_line_quad * entry)
{
    n_object * return_object = 0L;
    if (entry)
    {
        n_array * four = vect2_memory_list_number_array(entry->four, 4);
        n_array * two  = vect2_memory_list_number_array(entry->two, 2);

        if (four)
        {
            return_object = object_array(0L, "quad", four);
        }

        if (two)
        {
            if (return_object)
            {
                object_array(return_object, "line", two);
            }
            else
            {
                return_object = object_array(0L, "line", two);
            }
        }
    }
    return return_object;
}

n_array * tof_array_process(n_array * in_array)
{
    n_array * return_array = 0L;
    memory_list * entries = object_unwrap_array(in_array, sizeof(n_line_quad), unwrap_two_four, OBJECT_OBJECT);
    n_line_quad * entry = (n_line_quad *) entries->data;
    n_int count = 0;

    while (count < entries->count)
    {
        array_add_empty(&return_array, array_object(tof_object_entry(&entry[count])));
        count ++;
    }
    memory_list_free(&entries);

    return return_array;
}

n_file * tof_file_process( n_file * in_file)
{
    n_object_type type_of;
    n_file   *output_file;

    io_file_debug(in_file);

    printf("\n --- --- ---\n");

    void * returned_blob = unknown_file_to_tree( in_file, &type_of );
    io_file_free( &in_file );

    if ( type_of != OBJECT_ARRAY )
    {
        printf( "not correct object type\n" );
        return 0L;
    }

    n_array * return_array = tof_array_process( (n_array *) returned_blob );

    returned_blob = (void *) return_array;

    if ( returned_blob )
    {
        output_file = unknown_json( returned_blob, type_of );
        unknown_free( &returned_blob, type_of );
        return output_file;
    }
    else
    {
        printf( "no returned object\n" );
    }
    return 0L;
}

void tof_gather( n_string file_in )
{
    n_int    file_error;
    n_file   *in_file = tof_file( file_in, &file_error);

    n_string file_out = io_string_copy( file_in );

    printf( "%s --- \n", file_in );
    file_out[0] = '2';
    if ( file_error != -1 )
    {
        n_file * output_file = tof_file_process( in_file );

        if ( output_file )
        {
            (void)io_disk_write( output_file, file_out );
            io_file_free( &output_file );
        }
        else
        {
            printf( "no returned output file\n" );
        }
    }
    else
    {
        printf( "reading from disk failed\n" );
    }
}

#ifndef OBJECT_TEST

int main( int argc, const char *argv[] )
{
    if ( argc == 2 )
    {
        tof_gather( ( n_string )argv[1] );
    }

    /*io_file_debug(tof_file_process(io_file_new_from_string_block((n_char *)json_file_string)));*/

    return 0;
}

#endif
