
/****************************************************************

 territory.c

 =============================================================



 ****************************************************************/

#include "toolkit.h"

#include "sim.h"

typedef enum
{
    NAME_UNKNOWN,

    NAME_ATOLL,
    NAME_BASIN,
    NAME_BAY,
    NAME_BEACH,
    NAME_CLIFF,
    NAME_FLATLAND,
    NAME_HEADLAND,
    NAME_HILL,
    NAME_LAGOON,
    NAME_LAKE,
    NAME_MOUNTAIN,
    NAME_PENINSULA,
    NAME_POND,
    NAME_RIDGE,
    NAME_RIVER,
    NAME_SPRING,
    NAME_STREAM,
    NAME_SUMMIT,
    NAME_TRENCH,
    NAME_VALLEY,
    NAME_TOTAL
} territory_name;

#if 0

static n_string territory_description( territory_name value )
{
    n_string territory_names[NAME_TOTAL] =
    {
        " ",
        "Atoll",
        "Basin",
        "Bay",
        "Beach",
        "Cliff",
        "Flatland",
        "Headland",
        "Hill",
        "Lagoon",
        "Lake",
        "Mountain",
        "Peninsula",
        "Pond",
        "Ridge",
        "River",
        "Spring",
        "Stream",
        "Summit",
        "Trench",
        "Valley"
    };
    return territory_names[value];
}

static void territory_description_init( n_byte *land, territory_name *set_names )
{
    n_int tx = 0;
    while ( tx < TERRITORY_DIMENSION )
    {
        n_int ty = 0;
        while ( ty < TERRITORY_DIMENSION )
        {
            n_byte2 count[16];

            n_int   dx = 0;
            n_int   dy = 0;

            n_byte  highest_value = 0;
            n_byte  loweest_value = 255;

            n_int   center_average = 0;

            n_byte  top_right;
            n_byte  bottom_right;

            n_byte  top_left;
            n_byte  bottom_left;

            n_int mx = 0;
            while ( mx < ( MAP_DIMENSION / TERRITORY_DIMENSION ) )
            {
                n_int my = 0;
                n_int normalx = mx + ( tx * ( MAP_DIMENSION / TERRITORY_DIMENSION ) );
                while ( my < ( MAP_DIMENSION / TERRITORY_DIMENSION ) )
                {
                    n_int normaly = my + ( ty * ( MAP_DIMENSION / TERRITORY_DIMENSION ) );
                    n_byte value = land[normalx + ( normaly * MAP_DIMENSION )];

                    count[value >> 4]++;
                    if ( value > highest_value )
                    {
                        highest_value = value;
                    }
                    if ( value < loweest_value )
                    {
                        loweest_value = value;
                    }

                    if ( mx == 0 )
                    {
                        dx -= ( n_int )value;

                        if ( my == 0 )
                        {
                            top_left = value;
                        }
                        if ( my == ( ( MAP_DIMENSION / TERRITORY_DIMENSION ) - 1 ) )
                        {
                            bottom_left = value;
                        }
                    }
                    if ( my == 0 )
                    {
                        dy -= ( n_int )value;
                    }
                    if ( mx == ( ( MAP_DIMENSION / TERRITORY_DIMENSION ) - 1 ) )
                    {
                        dx += ( n_int )value;

                        if ( my == 0 )
                        {
                            top_right = value;
                        }
                        if ( my == ( ( MAP_DIMENSION / TERRITORY_DIMENSION ) - 1 ) )
                        {
                            bottom_right = value;
                        }
                    }
                    if ( my == ( ( MAP_DIMENSION / TERRITORY_DIMENSION ) - 1 ) )
                    {
                        dy += ( n_int )value;
                    }

                    if ( ( ( mx / 2 ) == ( my / 2 ) ) && ( ( mx / 2 ) == ( ( MAP_DIMENSION / TERRITORY_DIMENSION ) / 2 ) ) )
                    {
                        center_average += ( n_int )value;
                    }

                    my++;
                }
                mx++;
            }
            /* naming logic goes here */
            {
                territory_name return_name = NAME_UNKNOWN;

                *( set_names++ ) = return_name;
            }

            ty++;
        }
        tx++;
    }
}

#endif


