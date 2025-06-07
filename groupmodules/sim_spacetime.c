
/****************************************************************

 spacetime.c

 =============================================================



 ****************************************************************/

#include "toolkit.h"

#include "sim.h"

void spacetime_to_string( n_string value )
{
    n_int minutes = land_time();
    n_int days = land_date();
    n_int military_time = ( minutes % 60 );
    n_int hours = ( minutes / 60 );
    n_int days_month = ( days % 28 ) + 1;
    n_int month = ( ( days / 28 ) % 13 ) + 1;
    n_int years = days / ( 28 * 13 );

    military_time += hours * 100;

    value[0] = '0' + ( military_time / 1000 ) % 10;
    value[1] = '0' + ( military_time / 100 ) % 10;
    value[2] = ':';
    value[3] = '0' + ( military_time / 10 ) % 10;
    value[4] = '0' + ( military_time / 1 ) % 10;
    value[5] = ' ';
    value[6] = '0' + ( days_month / 10 ) % 10;
    value[7] = '0' + ( days_month / 1 ) % 10;
    value[8] = '/';
    value[9] = '0' + ( month / 10 ) % 10;
    value[10] = '0' + ( month / 1 ) % 10;
    value[11] = '/';
    io_number_to_string( &value[12], ( n_uint )years );
}


n_int spacetime_after( n_spacetime *initial, n_spacetime *second )
{
    if ( initial->date < second->date )
    {
        return 0;
    }
    if ( initial->date > second->date )
    {
        return 1;
    }
    if ( initial->time > second->time )
    {
        return 1;
    }
    return 0;
}

n_int spacetime_before_now( n_spacetime *initial )
{
    if ( initial->date > land_date() )
    {
        return 0;
    }
    if ( initial->date < land_date() )
    {
        return 1;
    }
    if ( initial->time < land_time() )
    {
        return 1;
    }
    return 0;
}

void spacetime_copy( n_spacetime *to, n_spacetime *from )
{
    to->location[0] = from->location[0];
    to->location[1] = from->location[1];

    to->date = from->date;
    to->time = from->time;
}

void spacetime_set( n_spacetime *set, n_byte2 *location )
{
    set->location[0] = location[0];
    set->location[1] = location[1];
    set->time        = land_time();
    set->date        = land_date();
}

void spacetime_convert_to_map( n_vect2 *value )
{
    value->x = APESPACE_TO_MAPSPACE( value->x );
    value->y = APESPACE_TO_MAPSPACE( value->y );
}



