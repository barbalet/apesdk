/****************************************************************

 being.c

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

/*! \file   being.c
 *  \brief  Historically this represented the Simulated Ape but moreso
    now it represents the Simulated Ape's interface to something external.
    being.c also now connects to the social, brain, body and metabolim
    simulations through to external simulations.
 */

#include "entity.h"
#include "entity_internal.h"

#include <stdio.h>
#include <stdlib.h>

/*#pragma mark - macros*/

/** checks the being's line of sight to a point

    NB: As the direction of scanning can't be determined, the breaking sight point
        can't be detected through tracking the being_ground results.
*/

/** returning 0 - not in line of sight, 1 - in line of sight */

typedef    struct
{
    n_int         start_z;
    n_int         offset_x;
    n_int         offset_y;
    n_int       visibility_delta;
    n_int       visibility_total;
}
being_draw;

/*#pragma mark - can_move*/

static simulated_being_can_move *being_can_move_local = 0L;

n_byte being_can_move( n_vect2 *location, n_vect2 *delta )
{
    if ( being_can_move_local )
    {
        return being_can_move_local( location, delta );
    }
    return 1;
}

void being_can_move_override( simulated_being_can_move *new_can_move )
{
    being_can_move_local = new_can_move;
}

/*#pragma mark - wrap*/

static simulated_being_wrap *being_wrap_local = 0L;

void being_wrap( n_vect2 *location )
{
    if ( being_wrap_local )
    {
        being_wrap_local( location );
    }
    else
    {
        n_int px = location->x;
        n_int py = location->y;

        px = ( px + APESPACE_BOUNDS + 1 ) & APESPACE_BOUNDS;
        py = ( py + APESPACE_BOUNDS + 1 ) & APESPACE_BOUNDS;

        location->x = px;
        location->y = py;
    }
}

void being_wrap_override( simulated_being_wrap *new_move )
{
    being_wrap_local = new_move;
}

/*#pragma mark - initial_location*/

static simulated_being_initial_location *being_local_initial_location_local = 0L;

void being_initial_location( n_vect2 *location, n_byte2 *seed )
{
    if ( being_local_initial_location_local )
    {
        being_local_initial_location_local( location, seed );
    }
    else
    {
        n_int loop = 0;

        do
        {
            n_vect2 location_vector;

            location->x = math_random( seed ) & APESPACE_BOUNDS;
            location->y = math_random( seed ) & APESPACE_BOUNDS;

            being_wrap( &location_vector );

            loop ++;
        }
        while ( ( loop < 20 ) && ( WATER_TEST( land_location( APESPACE_TO_MAPSPACE( location->x ),
                                               APESPACE_TO_MAPSPACE( location->y ) ), land_tide_level() ) ) );

    }
}

void being_initial_location_override( simulated_being_initial_location *new_initial_location )
{
    being_local_initial_location_local = new_initial_location;
}

/**
 This checks to see if a Simulated Ape can see a particular point
 @param local The simulated_being pointer
 @param location The location of the point to be seen.
 @return 1 can see, 0 can not see
 */
static n_byte being_los( simulated_being *local, n_vect2 *location )
{
    n_int   local_facing = ( ( being_facing( local ) ) >> 5 );
    n_vect2 temp_location;

    temp_location.x = location->x;
    temp_location.y = location->y;

    /* There is probably a logical simplification of this
     as I can't think of it here is the brute force method.

     The Simulated Ape Simulation universe wraps around in all
     directions you need to calculate the line of site off the map too. */

    /*
     6
     5   7
     4       0
     3   1
     2
     */

    if ( being_los_projection( local, &temp_location ) == 1 )
    {
        return 1;
    }

    if ( ( local_facing == 6 ) || ( local_facing == 7 ) || ( local_facing == 0 ) || ( local_facing == 1 ) || ( local_facing == 2 ) )
    {
        temp_location.x = location->x + MAP_APE_RESOLUTION_SIZE;
        temp_location.y = location->y;

        if ( being_los_projection( local, &temp_location ) == 1 )
        {
            return 1;
        }
    }

    if ( ( local_facing == 7 ) || ( local_facing == 0 ) || ( local_facing == 1 ) || ( local_facing == 2 ) || ( local_facing == 3 ) )
    {
        temp_location.x = location->x + MAP_APE_RESOLUTION_SIZE;
        temp_location.y = location->y + MAP_APE_RESOLUTION_SIZE;
        if ( being_los_projection( local, &temp_location ) == 1 )
        {
            return 1;
        }
    }
    if ( ( local_facing == 0 ) || ( local_facing == 1 ) || ( local_facing == 2 ) || ( local_facing == 3 ) || ( local_facing == 4 ) )
    {
        temp_location.x = location->x;
        temp_location.y = location->y + MAP_APE_RESOLUTION_SIZE;
        if ( being_los_projection( local, &temp_location ) == 1 )
        {
            return 1;
        }
    }
    if ( ( local_facing == 1 ) || ( local_facing == 2 ) || ( local_facing == 3 ) || ( local_facing == 4 ) || ( local_facing == 5 ) )
    {
        temp_location.x = location->x - MAP_APE_RESOLUTION_SIZE;
        temp_location.y = location->y + MAP_APE_RESOLUTION_SIZE;
        if ( being_los_projection( local, &temp_location ) == 1 )
        {
            return 1;
        }
    }
    if ( ( local_facing == 2 ) || ( local_facing == 3 ) || ( local_facing == 4 ) || ( local_facing == 5 ) || ( local_facing == 6 ) )
    {
        temp_location.x = location->x - MAP_APE_RESOLUTION_SIZE;
        temp_location.y = location->y;
        if ( being_los_projection( local, &temp_location ) == 1 )
        {
            return 1;
        }
    }
    if ( ( local_facing == 3 ) || ( local_facing == 4 ) || ( local_facing == 5 ) || ( local_facing == 6 ) || ( local_facing == 7 ) )
    {
        temp_location.x = location->x - MAP_APE_RESOLUTION_SIZE;
        temp_location.y = location->y - MAP_APE_RESOLUTION_SIZE;
        if ( being_los_projection( local, &temp_location ) == 1 )
        {
            return 1;
        }
    }
    if ( ( local_facing == 4 ) || ( local_facing == 5 ) || ( local_facing == 6 ) || ( local_facing == 7 ) || ( local_facing == 0 ) )
    {
        temp_location.x = location->x;
        temp_location.y = location->y - MAP_APE_RESOLUTION_SIZE;
        if ( being_los_projection( local, &temp_location ) == 1 )
        {
            return 1;
        }
    }
    if ( ( local_facing == 5 ) || ( local_facing == 6 ) || ( local_facing == 7 ) || ( local_facing == 0 ) || ( local_facing == 1 ) )
    {
        temp_location.x = location->x + MAP_APE_RESOLUTION_SIZE;
        temp_location.y = location->y - MAP_APE_RESOLUTION_SIZE;
        if ( being_los_projection( local, &temp_location ) == 1 )
        {
            return 1;
        }
    }
    return 0;
}

/*#pragma mark - random*/

static void being_random3( simulated_being *value )
{
    math_random3( value->delta.random_seed );
}

n_byte2 being_random( simulated_being *value )
{
    return math_random( value->delta.random_seed );
}

static void being_set_random( simulated_being *value, n_byte2 *seed )
{
    value->delta.random_seed[0] = seed[0];
    value->delta.random_seed[1] = seed[1];
}

static void being_set_random1( simulated_being *value, n_byte2 seed1 )
{
    value->delta.random_seed[1] = seed1;
}

static n_byte2 *being_get_random( simulated_being *value )
{
    return value->delta.random_seed;
}

static n_uint being_genetic_count_zeros( n_genetics count )
{
    n_uint loop = 0;
    n_uint addition = 0;
    while ( loop < sizeof( n_genetics ) * 8 )
    {
        if ( ( ( count >> loop ) & 1 ) == 0 )
        {
            addition++;
        }
        loop++;
    }
    return addition;
}

n_uint being_genetic_comparison( n_genetics *primary, n_genetics *secondary, n_int parse_requirements )
{
    n_int   loop = 0;
    n_uint  addition = 0;

    if ( FIND_SEX( secondary[CHROMOSOME_Y] ) != SEX_FEMALE )
    {
        if ( parse_requirements == 1 )
        {
            return 0;
        }
    }
    else
    {
        if ( parse_requirements == 0 )
        {
            return 0;
        }
    }

    while ( loop < CHROMOSOMES )
    {
        n_genetics comparison = primary[loop] ^ secondary[loop];
        addition += being_genetic_count_zeros( comparison );
        loop++;
    }
    return addition;
}

static simulated_being_line_of_sight *being_line_of_sight_local = 0L;

void being_line_of_sight_override( simulated_being_line_of_sight *new_line_of_sight )
{
    being_line_of_sight_local = new_line_of_sight;
}

n_byte being_line_of_sight( simulated_being *local, n_vect2 *location )
{
    if ( being_line_of_sight_local )
    {
        return being_line_of_sight_local( local, location );
    }
    else
    {
        return being_los( local, location );
    }
}

static simulated_being_brain_cycle *being_brain_cycle_local = 0L;

void being_brain_cycle_override( simulated_being_brain_cycle *new_brain_cycle )
{
    being_brain_cycle_local = new_brain_cycle;
}

void being_brain_cycle( n_byte *local, n_byte2 *constants )
{
    if ( being_brain_cycle_local )
    {
        being_brain_cycle_local( local, constants );
    }
    else
    {
        brain_cycle( local, constants );
    }
}
/*#pragma mark - braincode*/

#ifdef BRAINCODE_ON

n_byte *being_braincode_external( simulated_being *value )
{
    simulated_isocial *social_value = being_social( value );
    return social_value[ATTENTION_EXTERNAL].braincode;
}

void being_clear_attention( simulated_being *value )
{
    memory_erase( ( n_byte * )( value->braindata.attention ), ATTENTION_SIZE );
}

void being_set_attention( simulated_being *value, n_int index, n_int attention )
{
    value->braindata.attention[index] = ( n_byte )attention;
}

n_byte being_attention( simulated_being *value, n_int index )
{
    return value->braindata.attention[index];
}

n_byte *being_braincode_internal( simulated_being *value )
{
    simulated_isocial *social_value = being_social( value );
    n_int          attention_location = being_attention( value, ATTENTION_ACTOR );
    return social_value[attention_location].braincode;
}

#else

void being_clear_attention( simulated_being *value )
{
}

void being_set_attention( simulated_being *value, n_int index, n_int attention )
{
}

n_byte being_attention( simulated_being *value, n_int index )
{
    return 0;
}

#endif

/* ape names */
#define NAMES_SURNAMES              256
#define NAMES_FIRST                 256


const n_string english_last_names[256] =
{
    "Abbott", "Adams", "Allen", "Arnold", "Ashton", "Atkins", "Austin", "Bailey", "Baird", "Baker", "Ball", "Banks", "Barker",
    "Barlow", "Barnes", "Barton", "Bates", "Baxter", "Begum", "Bell", "Benson", "Berry", "Bird", "Bishop", "Black", "Blair",
    "Blake", "Bloggs", "Bolton", "Bond", "Booth", "Bowden", "Bowen", "Boyd", "Brennan", "Briggs", "Brown", "Browne", "Bryant",
    "Bull", "Burke", "Burns", "Burton", "Butler", "Byrne", "Carr", "Carroll", "Carter", "Clark", "Clarke", "Coates", "Cole",
    "Coles", "Cook", "Cooke", "Cooper", "Cox", "Cross", "Cullen", "Curtis", "Dale", "Davey", "Davis", "Dawson", "Day", "Dean",
    "Dennis", "Dixon", "Doyle", "Duffy", "Duncan", "Dunn", "Ellis", "Evans", "Farrell", "Field", "Finch", "Fisher", "Flynn",
    "Ford", "Foster", "Fowler", "Fox", "Fraser", "French", "Frost", "Fuller", "Garner", "George", "Gibson", "Giles", "Gill",
    "Glover", "Gordon", "Gough", "Graham", "Grant", "Gray", "Green", "Haines", "Hall", "Hardy", "Harper", "Harris", "Hart",
    "Harvey", "Hayes", "Haynes", "Heath", "Hewitt", "Higgins", "Hill", "Hilton", "Hobbs", "Holden", "Holmes", "Holt", "Hooper",
    "Hope", "Howard", "Howe", "Hudson", "Hughes", "Hunt", "Hunter", "Jacobs", "James", "Jarvis", "Jones", "Jordan", "Kemp",
    "Kent", "Kerr", "King", "Kirk", "Knight", "Lane", "Lawson", "Leach", "Lee", "Leigh", "Lewis", "Little", "Lloyd", "Long",
    "Lord", "Love", "Lowe", "Lucas", "Lynch", "Mann", "Marsh", "Mason", "May", "Mellor", "Miles", "Miller", "Mills", "Moore",
    "Moran", "Morgan", "Morris", "Morton", "Moss", "Murphy", "Murray", "Nash", "Naylor", "Nelson", "Newman", "Newton", "Norman",
    "Owens", "Page", "Palmer", "Park", "Parker", "Parry", "Patel", "Payne", "Pearce", "Perry", "Peters", "Poole", "Porter",
    "Potter", "Powell", "Power", "Price", "Quinn", "Randall", "Read", "Reed", "Rees", "Reid", "Reilly", "Rhodes", "Rice", "Riley",
    "Robson", "Rogers", "Rose", "Ross", "Rossi", "Rowe", "Ryan", "Savage", "Scott", "Shah", "Sharma", "Sharp", "Shaw", "Silva",
    "Simmons", "Singh", "Slater", "Smart", "Smith", "Steele", "Stewart", "Stevens", "Stone", "Stuart", "Sutton", "Swift", "Tait",
    "Taylor", "Thomas", "Thorne", "Todd", "Tucker", "Turner", "Walker", "Wall", "Wallace", "Walsh", "Walton", "Ward", "Warren",
    "Watson", "Watts", "Weaver", "Webb", "Welch", "Wells", "West", "Whelan", "White", "Willis", "Wilson", "Wood", "Woods",
    "Wright", "Wyatt", "Yates", "Young"
};

const n_string english_female_first_names[256] =
{
    "Ada", "Agatha", "Agnes", "Aileen", "Aimee", "Alanna", "Alda", "Alice", "Alina", "Alison", "Alma", "Amanda", "Amber", "Andrea", "Angela",
    "Anita", "Anthea", "April", "Ariana", "Arleen", "Astrid", "Audrey", "Beata", "Becky", "Beryl", "Bess", "Bianca", "Blair", "Blythe",
    "Bonnie", "Brenda", "Briana", "Brooke", "Carla", "Carly", "Carmen", "Cheryl", "Chloe", "Coral", "Daphne", "Davida", "Dawn", "Denise",
    "Donna", "Dora", "Doris", "Echo", "Eda", "Edana", "Edith", "Edlyn", "Edna", "Edwina", "Effie", "Eileen", "Elaine", "Elena", "Elga",
    "Elise", "Eliza", "Ella", "Ellen", "Eloise", "Elsie", "Elvira", "Emily", "Emma", "Erika", "Erin", "Estra", "Ethel", "Eudora", "Eunice",
    "Faith", "Fannie", "Fawn", "Faye", "Fedora", "Fern", "Fiona", "Flora", "Gale", "Gaye", "Geneva", "Gilda", "Gladys", "Gloria", "Grace",
    "Gwynne", "Harley", "Hattie", "Hazel", "Hetty", "Hilda", "Holly", "Honey", "Hope", "Ingrid", "Irene", "Iris", "Ivory", "Ivy", "Jade",
    "Jane", "Janet", "Janice", "Jeanne", "Jemima", "Jewel", "Joan", "Joanna", "Joy", "June", "Kacey", "Kara", "Kate", "Kay", "Keely", "Kelsey",
    "Kendra", "Kerri", "Kyla", "Lacey", "Lane", "Lara", "Larina", "Leanne", "Leslie", "Linda", "Livia", "Lizzie", "Lois", "Lorena", "Lulu",
    "Luna", "Lynn", "Mabel", "Madge", "Maggie", "Maia", "Maisie", "Mandy", "Marcia", "Margot", "Marnia", "Mary", "Maude", "Maura", "Mavis",
    "Maxine", "Megan", "Melody", "Mercy", "Meris", "Merle", "Miriam", "Misty", "Moira", "Molly", "Mona", "Monica", "Mora", "Morgan", "Muriel",
    "Myra", "Myrtle", "Nancy", "Naomi", "Nell", "Nerita", "Nina", "Noelle", "Nola", "Norma", "Nydia", "Odette", "Olga", "Opal", "Oprah", "Orva",
    "Page", "Pamela", "Pansy", "Patty", "Pearl", "Phoebe", "Polly", "Quenna", "Questa", "Rachel", "Ramona", "Regina", "Rhea", "Rhoda", "Rita",
    "Robin", "Rosa", "Rowena", "Ruby", "Ruth", "Sacha", "Sadie", "Salena", "Sally", "Salome", "Sandra", "Sarah", "Serena", "Shana", "Sharon",
    "Sheila", "Sibley", "Silver", "Sirena", "Talia", "Tamara", "Tammy", "Tanya", "Tara", "Tasha", "Tatum", "Tess", "Thalia", "Thea", "Thelma",
    "Thora", "Tilda", "Tina", "Tracy", "Trina", "Trista", "Tyne", "Udele", "Ula", "Ulrica", "Ulva", "Una", "Unity", "Ursa", "Ursula", "Valda",
    "Vania", "Veleda", "Vera", "Verda", "Violet", "Vita", "Wanda", "Wilda", "Willa", "Willow", "Wynne", "Zea", "Zelda", "Zera", "Zoe"
};

const n_string english_male_first_names[256] =
{
    "Aaron", "Abbott", "Abel", "Adam", "Albern", "Albert", "Alfie", "Alfred", "Alvin", "Amery", "Amos", "Andrew", "Angus", "Ansel", "Arlen",
    "Arnold", "Arvel", "Austin", "Axel", "Baird", "Barry", "Basil", "Bert", "Blair", "Blake", "Boris", "Brent", "Brian", "Brice", "Brock",
    "Bruce", "Bruno", "Bryant", "Buck", "Bud", "Burton", "Byron", "Calvin", "Carl", "Carter", "Carver", "Cary", "Casey", "Casper", "Cecil",
    "Cedric", "Claude", "Clive", "Clyde", "Colin", "Conan", "Connor", "Conrad", "Conroy", "Conway", "Corwin", "Craig", "Crosby", "Culver",
    "Curt", "Curtis", "Cyril", "Damon", "Daniel", "Darcy", "David", "Dean", "Declan", "Dennis", "Derek", "Dermot", "Derwin", "Dexter",
    "Dillon", "Dion", "Dirk", "Donald", "Dorian", "Drew", "Dudley", "Duncan", "Dwayne", "Dwight", "Dylan", "Earl", "Edgar", "Edwin", "Efrain",
    "Egbert", "Elbert", "Elmer", "Elroy", "Elton", "Elvis", "Emmett", "Emrick", "Ernest", "Errol", "Esmond", "Eugene", "Evan", "Ezra", "Fabian",
    "Farley", "Felix", "Fenton", "Ferris", "Finbar", "Floyd", "Foster", "Fox", "Frank", "Gale", "Galvin", "Garret", "Garth", "Gavin", "George",
    "Gideon", "Giles", "Gilroy", "Glenn", "Godwin", "Graham", "Grant", "Guy", "Hadden", "Hadley", "Hadwin", "Hale", "Hall", "Hamlin", "Hardy",
    "Harley", "Hector", "Henry", "Herman", "Homer", "Howard", "Hubert", "Hunter", "Ian", "Isaac", "Isaiah", "Ivan", "Ives", "Jack", "Jacob",
    "Jarvis", "Jason", "Jasper", "Jed", "Jerome", "Jesse", "John", "Joshua", "Justin", "Keaton", "Keith", "Kelsey", "Kelvin", "Kent", "Kerry",
    "Kevin", "Kirby", "Kirk", "Kit", "Kody", "Konrad", "Kurt", "Kyle", "Lamont", "Landon", "Lane", "Lars", "Lee", "Leroy", "Leslie", "Lester",
    "Lionel", "Lloyd", "Logan", "Lowell", "Lyndon", "Marcus", "Marlon", "Martin", "Marvin", "Medwin", "Melvin", "Merlin", "Miles", "Morgan",
    "Morris", "Morton", "Murray", "Neal", "Nigel", "Noel", "Norman", "Olaf", "Olin", "Oliver", "Oscar", "Oswald", "Otis", "Owen", "Paul",
    "Perry", "Peter", "Philip", "Pierce", "Quincy", "Quinn", "Ralph", "Rex", "Riley", "Rodney", "Roger", "Roland", "Rolf", "Ronald", "Rory",
    "Ross", "Roy", "Rufus", "Rupert", "Ryan", "Samson", "Samuel", "Scott", "Sean", "Seth", "Shawn", "Sidney", "Simon", "Sloane", "Stacy",
    "Thomas", "Toby", "Todd", "Tony", "Trent", "Trevor", "Troy", "Tyler", "Unwin", "Vance", "Victor", "Walter", "Warren", "Wayne", "Wilbur",
    "Willis", "Wyatt", "Wylie"
};


#define FAMILY_NAME_AND_MOD (NAMES_SURNAMES - 1)
#define FIRST_NAME_AND_MOD  (NAMES_FIRST - 1)

#define UNPACK_FAMILY_FIRST_NAME(packed_family_name)  (packed_family_name & 255)
#define UNPACK_FAMILY_SECOND_NAME(packed_family_name) ((packed_family_name >> 8) & 255)


#define GET_NAME_FAMILY(f0,f1) ((n_byte2)((f0&255)|((f1&255)<<8)))


void being_unpack_family( n_byte2 name, n_byte *values ) /* brain.c */
{
    values[0] = UNPACK_FAMILY_FIRST_NAME( name );
    values[1] = UNPACK_FAMILY_SECOND_NAME( name );
}

n_byte being_first_name( simulated_being *value )
{
    if ( value == 0L )
    {
        return 0;
    }
    {
        return value->constant.name[0] & FIRST_NAME_AND_MOD;
    }
}

static void being_set_first_name( simulated_being *value, n_byte2 name )
{
    value->constant.name[0] =  name & FIRST_NAME_AND_MOD;
}

void being_set_family_name( simulated_being *value, n_byte first, n_byte last )
{
    value->constant.name[1] =  ( n_byte2 )( first | ( last << 8 ) );
}

n_byte2 being_gender_name( simulated_being *value )
{
    if ( value == 0L )
    {
        return 0;
    }
    return ( n_byte2 )( ( being_first_name( value ) | ( FIND_SEX( GET_I( value ) ) << 8 ) ) );
}

n_byte2 being_family_name( simulated_being *value )
{
    if ( value == 0L )
    {
        return 0;
    }
    return ( GET_NAME_FAMILY( being_family_first_name( value ), being_family_second_name( value ) ) );
}

n_int being_name_comparison( simulated_being *value, n_byte2 gender_name, n_byte2 family_name )
{
    return ( ( being_gender_name( value ) == gender_name ) && ( being_family_name( value ) == family_name ) );
}

n_byte being_family_first_name( simulated_being *value )
{
    if ( value == 0L )
    {
        return 0;
    }
    {
        return UNPACK_FAMILY_FIRST_NAME( value->constant.name[1] );
    }
}

n_byte being_family_second_name( simulated_being *value )
{
    if ( value == 0L )
    {
        return 0;
    }
    {
        return UNPACK_FAMILY_SECOND_NAME( value->constant.name[1] );
    }
}

static void  being_name( n_byte female, n_int first, n_byte family0, n_byte family1, n_string name )
{
    n_int  position = 0;
    if ( first != -1 )
    {
        if ( female )
        {
            io_string_write( name, english_female_first_names[ first ], &position );
        }
        else
        {
            io_string_write( name, english_male_first_names[ first ], &position );
        }
        io_string_write( name, " ", &position );
        io_string_write( name, english_last_names[ family0 ], &position );
        io_string_write( name, "-", &position );
        io_string_write( name, english_last_names[ family1 ], &position );
    }
    else
    {
        io_string_write( name, "Unknown", &position );
    }
}

void being_name_simple( simulated_being *value, n_string str )
{
    if ( value )
    {
        n_int is_female = FIND_SEX( GET_I( value ) ) == SEX_FEMALE;
        n_byte first_name = being_first_name( value );
        n_byte first_family_name = being_family_first_name( value );
        n_byte second_family_name = being_family_second_name( value );
        being_name( is_female, first_name, first_family_name, second_family_name, str );
    }
    else
    {
        n_int position = 0;
        io_string_write( str, "Unknown", &position );
    }
}

void being_name_byte2( n_byte2 first, n_byte2 family, n_string name )
{
    being_name( ( n_byte )( ( first >> 8 ) == SEX_FEMALE ),
                ( n_int )( first & 255 ),
                ( n_byte )UNPACK_FAMILY_FIRST_NAME( family ),
                ( n_byte )UNPACK_FAMILY_SECOND_NAME( family ),
                name );
}

void being_state_description( n_byte2 state, n_string result )
{
    /* if you change this you need to change the corresponding definitions in entity.h */
    const n_string state_description[] =
    {
        "Sleeping", "Awake", "Hungry", "Swimming", "Eating", "Moving",
        "Speaking", "Shouting", "Grooming", "Suckling",
        "Showing Force", "Attacking", "No Food"
    };
    n_int string_length = 0;
    n_int n = 2;

    if ( state == BEING_STATE_ASLEEP )
    {
        io_string_write( result, state_description[0], &string_length );
        return;
    }

    while ( n < BEING_STATES )
    {
        if ( state & ( 1 << ( n - 1 ) ) )
        {
            if ( string_length > 0 )
            {
                io_string_write( result, ", ", &string_length );
            }
            io_string_write( result, state_description[n], &string_length );
        }
        n++;
    }
}

/**
 This moves the specified Simulated Ape outside the core simulation movement
 interface - think mouse movement or keyboard movement.
 @param local The pointer to the simulated_being being moved.
 @param rel_vel The movement variable used - historically the velocity hence the
 variable name.
 @param kind The kind of movement used - 0 = turning around pivoting on the
 z-axis (ie a standing turn), 1 = move forwards or backwards based on the
 direction facing, 2 = cursor like movements along the X and Y axis useful
 when operating the arrow keys (but not much else).
 */
void being_move( simulated_being *local, n_int rel_vel, n_byte kind )
{
    n_vect2 location_vector;
    n_byte2 loc[2];
    being_space( local, &location_vector );

    if ( kind == 1 )
    {
        n_vect2 facing_vector;
        being_facing_vector( local, &facing_vector, 1 );
        vect2_d( &location_vector, &facing_vector, rel_vel, 512 );
    }
    else
    {
        if ( rel_vel < 2 )
        {
            location_vector.y -= ( rel_vel * 200 ) - 100;
        }
        else
        {
            location_vector.x += 500 - ( rel_vel * 200 );
        }
    }

    being_wrap( &location_vector );

    loc[0] = ( n_byte2 )location_vector.x;
    loc[1] = ( n_byte2 )location_vector.y;
    being_set_location( local, loc );
}

n_byte being_crowding( simulated_being *value )
{
    return value->delta.crowding;
}

void being_crowding_cycle( simulated_being *value, n_int beings_in_vacinity )
{
    /** if the being is not overcrowded and its social drive is not saturated */
    if ( beings_in_vacinity < ( value->delta.crowding + SOCIAL_TOLLERANCE ) )
    {
        /** increase the social drive */
        being_inc_drive( value, DRIVE_SOCIAL );
    }
    else
    {
        /** decrease the social drive */
        being_dec_drive( value, DRIVE_SOCIAL );
    }

    /** Adjust crowding (typical expected number of neighbours). */
    if ( beings_in_vacinity < value->delta.crowding )
    {
        if ( value->delta.crowding > MIN_CROWDING )
        {
            value->delta.crowding--;
        }
    }
    if ( beings_in_vacinity > value->delta.crowding )
    {
        if ( value->delta.crowding < MAX_CROWDING )
        {
            value->delta.crowding++;
        }
    }
}


/*#pragma mark - state*/

void    being_set_state( simulated_being *value, being_state_type state )
{
    value->delta.macro_state = state;
}

void    being_add_state( simulated_being *value, being_state_type state )
{
    value->delta.macro_state |= state;

}
n_byte2 being_state( simulated_being *value )
{
    return value->delta.macro_state;
}

/*#pragma mark - brainstates*/

#ifdef BRAIN_ON

void being_set_brainstates( simulated_being *value, n_int asleep, n_byte2 val1, n_byte2 val2, n_byte2 val3 )
{
    n_int three_offset = ( asleep ? 0 : 3 );

    value->braindata.brain_state[three_offset + 0] = val1;
    value->braindata.brain_state[three_offset + 1] = val2;
    value->braindata.brain_state[three_offset + 2] = val3;
}

#endif

n_int being_brainstates( simulated_being *value, n_int asleep, n_byte2 *states )
{
    n_int three_offset = ( asleep ? 0 : 3 );

    states[0] = value->braindata.brain_state[three_offset + 0];
    states[1] = value->braindata.brain_state[three_offset + 1];
    states[2] = value->braindata.brain_state[three_offset + 2];

    return ( ( states[0] != 0 ) || ( states[1] != 1024 ) || ( states[2] != 0 ) );
}

/*#pragma mark - erase*/

void being_erase( simulated_being *value )
{
    memory_erase( ( n_byte * )value, sizeof( simulated_being ) );
}

/*#pragma mark - honor*/

void being_honor_delta( simulated_being *value, n_int delta )
{
    n_int honor_value = value->delta.honor;
    if ( delta > 0 )
    {
        if ( ( honor_value + delta ) > 255 )
        {
            value->delta.honor = 255;
            return;
        }
    }
    else
    {
        if ( ( honor_value + delta ) < 0 )
        {
            value->delta.honor = 0;
            return;
        }
    }
    value->delta.honor += ( n_byte )delta;
}

n_byte being_honor( simulated_being *value )
{
    return value->delta.honor;
}

void being_honor_inc_dec( simulated_being *inc, simulated_being *dec )
{
    if ( inc->delta.honor < 255 )
    {
        inc->delta.honor++;
    }
    if ( dec->delta.honor > 0 )
    {
        dec->delta.honor--;
    }
}

void being_honor_swap( simulated_being *victor, simulated_being *vanquished )
{
    if ( victor->delta.honor < vanquished->delta.honor )
    {
        /** swap social status */
        n_byte temp_hon = victor->delta.honor;
        victor->delta.honor = vanquished->delta.honor;
        vanquished->delta.honor = temp_hon;
    }
}

n_int being_honor_compare( simulated_being *first, simulated_being *second )
{
    if ( first->delta.honor > second->delta.honor )
    {
        return 1;
    }

    if ( first->delta.honor < second->delta.honor )
    {
        return -1;
    }

    return 0;
}

n_byte being_honor_immune( simulated_being *value )
{
    n_int local_honor = being_honor( value );
    if ( local_honor < 250 ) /* ALPHA_RANK */
    {
        return ( n_byte )( 1 + ( local_honor >> 6 ) );
    }
    return 2; /* IMMUNE_STRENGTH_ALPHA */
}

/*#pragma mark - posture*/

n_byte being_posture( simulated_being *value )
{
    return value->delta.posture;
}

void being_set_posture( simulated_being *value, n_byte post )
{
    value->delta.posture = post;
}

n_int being_posture_under( simulated_being *value, enum posture_type post )
{
    return ( value->delta.posture < post );
}

/*#pragma mark - brain*/

#ifdef BRAIN_ON
n_byte *being_brain( simulated_being *value )
{
    return value->braindata.brain;
}
#endif

/*#pragma mark - misc*/

simulated_iepisodic *being_episodic( simulated_being *value )
{
    return value->events.episodic;

}

simulated_isocial *being_social( simulated_being *value )
{
    return value->events.social;
}

n_int being_location_x( simulated_being *value )
{
    return ( n_int )value->delta.location[0];
}

n_int being_location_y( simulated_being *value )
{
    return ( n_int )value->delta.location[1];
}

void being_high_res( simulated_being *value, n_vect2 *vector )
{
    vector->x = APESPACE_TO_HR_MAPSPACE( being_location_x( value ) );
    vector->y = APESPACE_TO_HR_MAPSPACE( being_location_y( value ) );
}

void being_space( simulated_being *value, n_vect2 *vector )
{
    vector->x = value->delta.location[0];
    vector->y = value->delta.location[1];
}

n_byte2 *being_location( simulated_being *value )
{
    return value->delta.location;
}

void being_set_location( simulated_being *value, n_byte2 *from )
{
    value->delta.location[0] = from[0];
    value->delta.location[1] = from[1];
}

#ifdef DEBUG_LACK_OF_MOVEMENT

n_int being_total_movement( simulated_being *value )
{
    return ( n_int ) value->delta.total_movement;
}

void being_add_total_movement( simulated_being *value )
{
    value->delta.total_movement += value->delta.velocity;
}

void being_zero_total_movement( simulated_being *value )
{
    value->delta.total_movement = 0;
}

void being_register_movement( simulated_being *value, n_string comment_string )
{
    if ( ( IS_NIGHT( land_time() ) == 0 ) && ( being_total_movement( value ) == 0 ) )
    {
        n_string_block name_string, time_string;
        spacetime_to_string( time_string );
        being_name_simple( value, name_string );
        printf( "%s %s %s\n", time_string, name_string, comment_string );
    }
}

#endif

n_byte being_speed( simulated_being *value )
{
    return value->delta.velocity[0];
}

void being_set_speed( simulated_being *value, n_byte sp )
{
    value->delta.velocity[0] = sp;
}

void being_speed_advance( simulated_being *value )
{
    n_int loop = 0;
    while ( loop < 9 )
    {
        value->delta.velocity[9 - loop] = value->delta.velocity[8 - loop];
        loop++;
    }
}

n_int being_ten_minute_distance( simulated_being *value )
{
    n_int total_distance = 0;
    n_int loop = 0;
    while ( loop < 10 )
    {
        total_distance += ( n_int )value->delta.velocity[loop];
        loop++;
    }
    return total_distance;
}


void being_delta( simulated_being *primary, simulated_being *secondary, n_vect2 *delta )
{
    delta->x = primary->delta.location[0] - secondary->delta.location[0];
    delta->y = primary->delta.location[1] - secondary->delta.location[1];
}

/*#pragma mark - parasites*/

void being_add_parasites( simulated_being *value )
{
    /* maximum number of parasites in the range 0-255 */
    if ( value->delta.parasites < ( ( GENE_HAIR( being_genetics( value ) ) * 255 ) >> 4 ) )
    {
        value->delta.parasites++;
    }
}

void being_remove_parasites( simulated_being *value, n_int number_of_parasites )
{
    if ( value->delta.parasites > number_of_parasites )
    {
        value->delta.parasites -= ( n_byte )number_of_parasites;
    }
    else
    {
        value->delta.parasites = 0;
    }
}

n_int being_parasites( simulated_being *value )
{
    return value->delta.parasites;
}

void being_set_parasites( simulated_being *value, n_byte parasites )
{
    value->delta.parasites = parasites;
}

/*#pragma mark - misc part 2*/

n_int being_dob( simulated_being *value )
{
    return value->constant.date_of_birth;
}

void being_facing_towards( simulated_being *value, n_vect2 *vector )
{
    value->delta.direction_facing = math_tan( vector );
}

void being_wander( simulated_being *value, n_int wander )
{
    value->delta.direction_facing = ( n_byte )( ( value->delta.direction_facing + 256 + wander ) & 255 );
}

void being_facing_init( simulated_being *value )
{
    value->delta.direction_facing = ( n_byte )( being_random( value ) & 255 );
}

void being_facing_vector( simulated_being *value, n_vect2 *vect, n_int divisor )
{
    vect2_direction( vect, value->delta.direction_facing, divisor * 32 );
}

n_byte being_facing( simulated_being *value )
{
    return value->delta.direction_facing;
}

n_genetics *being_genetics( simulated_being *value )
{
    return value->constant.genetics;
}

n_int being_pregnant( simulated_being *value )
{
    return value->changes.date_of_conception;
}

n_int being_female( simulated_being *value )
{
    return ( FIND_SEX( GET_I( value ) ) == SEX_FEMALE );
}

n_int being_speaking( simulated_being *value )
{
    return ( value->delta.awake && ( being_state( value ) & BEING_STATE_SPEAKING ) );
}

n_genetics *being_fetal_genetics( simulated_being *value )
{
    return value->changes.fetal_genetics;
}

/* TODO: Remove this kind of access eventually */
n_int   being_energy( simulated_being *value )
{
    return value->delta.stored_energy;
}

n_int   being_energy_less_than( simulated_being *value, n_int less_than )
{
    return being_energy( value ) < less_than;
}

void  being_dead( simulated_being *value )
{
    value->delta.stored_energy = BEING_DEAD;
}

void being_living( simulated_being *value )
{
    value->delta.stored_energy = BEING_FULL;
}

void   being_energy_delta( simulated_being *value, n_int delta )
{
    n_int total = value->delta.stored_energy + delta;

    if ( total < BEING_DEAD )
    {
        total = BEING_DEAD;
    }

    value->delta.stored_energy = ( n_byte2 ) total;
}

n_byte   being_drive( simulated_being *value, enum drives_definition drive )
{
    return value->changes.drives[drive];
}

n_byte *being_drives( simulated_being *value )
{
    return value->changes.drives;
}

void    being_inc_drive( simulated_being *value, enum drives_definition drive )
{
    if ( value->changes.drives[drive] < DRIVES_MAX )
    {
        value->changes.drives[drive]++;
    }
}

void    being_dec_drive( simulated_being *value, enum drives_definition drive )
{
    if ( value->changes.drives[drive] > 0 )
    {
        value->changes.drives[drive]--;
    }
}

void    being_reset_drive( simulated_being *value, enum drives_definition drive )
{
    value->changes.drives[drive] = 0;
}

n_int   being_height( simulated_being *value )
{
    return value->delta.height;
}

void    being_set_height( simulated_being *value, n_int height )
{
    value->delta.height = ( n_byte2 )height;
}

n_int   being_mass( simulated_being *value )
{
    return value->delta.mass;
}

void being_turn_away_from_water( simulated_being *value )
{
    n_int    it_water_turn = 0;
    n_vect2 location_vector;

    being_space( value, &location_vector );

    while ( it_water_turn < 7 )
    {
        /* find higher land first */
        n_int    iturn = 8 - it_water_turn;
        n_int    loc_f = being_facing( value );
        n_int    iturn_plus  = loc_f + iturn;
        n_int    iturn_minus = loc_f + ( 256 - iturn );

        n_byte   turn_plus  = ( n_byte )( ( iturn_plus )  & 255 );
        n_byte   turn_minus = ( n_byte )( ( iturn_minus ) & 255 );
        n_vect2  temp_vector;

        n_int  z_plus;
        n_int  z_minus;

        vect2_direction( &temp_vector, turn_plus, 128 );
        vect2_add( &temp_vector, &temp_vector, &location_vector );

        spacetime_convert_to_map( &temp_vector );

        z_plus = land_location_vect( &temp_vector );

        vect2_direction( &temp_vector, turn_minus, 128 );
        vect2_add( &temp_vector, &temp_vector, &location_vector );

        spacetime_convert_to_map( &temp_vector );

        z_minus = land_location_vect( &temp_vector );

        if ( z_minus > z_plus )
        {
            being_wander( value, -iturn );
        }
        else if ( z_minus < z_plus )
        {
            being_wander( value, iturn );
        }
        it_water_turn++;
    }
}

inventory_type being_carried( simulated_being *value, BODY_INVENTORY_TYPES location )
{
    return ( value )->changes.inventory[location] & 0xfffff8;
}

void being_drop( simulated_being *value, BODY_INVENTORY_TYPES location )
{
    ( value )->changes.inventory[location] &= 7;
    being_set_attention( value, ATTENTION_BODY, location );
}

void being_take( simulated_being *value, BODY_INVENTORY_TYPES location, inventory_type object )
{
    ( value )->changes.inventory[location] |= object;
    being_set_attention( value, ATTENTION_BODY, location );
}


/**
 * @brief Check if a being is on ground or in water
 * @param px x coordinate of the being location
 * @param py y coordinate of the being location
 * @param params being_draw pointer
 * @return 1 if on ground, 0 otherwise
 */
static n_byte    being_ground( n_int px, n_int py, n_int dx, n_int dy, void *params )
{
    n_int        abs_sum = ABS( dx ) + ABS( dy );
    being_draw *being_pixel = ( being_draw * ) params;
    n_int        d_vis = being_pixel->visibility_delta;
    n_int         local_z = ( ( px * ( being_pixel->offset_x ) ) + ( py * ( being_pixel->offset_y ) ) ) >> 9;

    if ( abs_sum )
    {
        weather_values   seven_values = weather_seven_values( APESPACE_TO_MAPSPACE( px ), APESPACE_TO_MAPSPACE( py ) );
        n_int  span10 = ( ( abs_sum - 1 ) ? 1448 : 1024 );

        switch ( seven_values )
        {
        case WEATHER_SEVEN_SUNNY_DAY:
        case WEATHER_SEVEN_CLOUDY_DAY:
            being_pixel->visibility_total += ( span10 * ( d_vis + 16 ) ) >> 11;
            break;
        case WEATHER_SEVEN_RAINY_DAY:
        case WEATHER_SEVEN_DAWN_DUSK:
            being_pixel->visibility_total += ( span10 * ( ( 2 * d_vis ) + 25 ) ) >> 11;
            break;
        case WEATHER_SEVEN_CLEAR_NIGHT:
            being_pixel->visibility_total += ( span10 * ( ( 5 * d_vis ) + 65 ) ) >> 11;
        case WEATHER_SEVEN_CLOUDY_NIGHT:
            being_pixel->visibility_total += ( span10 * ( ( 8 * d_vis ) + 93 ) ) >> 11;
        case WEATHER_SEVEN_RAINY_NIGHT:
            being_pixel->visibility_total += ( span10 * ( ( 12 * d_vis ) + 145 ) ) >> 11;
            break;

        case WEATHER_SEVEN_ERROR:
        default:
            return 1;
        }
        if ( being_pixel->visibility_total > VISIBILITY_MAXIMUM )
        {
            return 1;
        }
        local_z += being_pixel->start_z;

        if ( local_z < WALK_ON_WATER( land_location( px, py ), land_tide_level() ) )
        {
            return 1;
        }
    }
    return 0;
}


n_byte being_basic_line_of_sight( simulated_being *local, n_vect2 *extern_end, n_vect2 *start, n_vect2 *delta, n_vect2 *end )
{
    n_vect2    vector_facing;
    vect2_copy( end, extern_end );
    /* TODO: Check for being awake - need a land and being based awake check */
    being_space( local, start );

    vect2_subtract( delta, end, start );
    {
        n_int distance_squared = vect2_dot( delta, delta, 1, 1 );
        if ( distance_squared > ( VISIBILITY_SPAN * VISIBILITY_SPAN ) )
        {
            return 0;
        }
    }
    /** check trivial case first - self aware */
    if ( ( delta->x == 0 ) && ( delta->y == 0 ) )
    {
        return 1;
    }
    being_facing_vector( local, &vector_facing, 16 );
    /* if it is behind, it can't be in the line of sight */
    if ( vect2_dot( &vector_facing, delta, 1, 64 ) < 0 )
    {
        return 0;
    }
    return 2;
}

n_byte being_los_projection( simulated_being *local, n_vect2 *extern_end )
{
    n_vect2    start, delta, end;
    n_byte     return_value = being_basic_line_of_sight( local, extern_end, &start, &delta, &end );

    if ( return_value != 2 )
    {
        return return_value;
    }

    /** move everything from being co-ordinates to map co-ordinates */

    spacetime_convert_to_map( &start );
    spacetime_convert_to_map( &delta );
    spacetime_convert_to_map( &end );

    /* check trivial case first - self aware (after co-ord translation) */
    if ( ( delta.x == 0 ) && ( delta.y == 0 ) )
    {
        return 1;
    }

    {
        const n_int simulated_iape_height = 3;
        n_int    start_z = ( n_int )WALK_ON_WATER( land_location_vect( &start ), land_tide_level() ) + simulated_iape_height;
        n_int    delta_z = ( n_int )WALK_ON_WATER( land_location_vect( &end ), land_tide_level() ) - start_z + simulated_iape_height;
        n_int    common_divisor = vect2_dot( &delta, &delta, 1, 1 );
        being_draw       translate;

        if ( common_divisor == 0 )
        {
            common_divisor = 1;
        }

        {
            n_vect2 offset = {0};

            vect2_d( &offset, &delta, 512 * delta_z, common_divisor );

            start_z -= vect2_dot( &start, &offset, 1, 512 );

            translate.start_z = start_z;
            translate.offset_x = offset.x;
            translate.offset_y = offset.y;

            translate.visibility_total = 100 * GENE_VISION_INITIAL( being_genetics( local ) );

            translate.visibility_delta = GENE_VISION_DELTA( being_genetics( local ) );
        }

        {
            n_join          being_point;
            being_point.information = ( void * ) &translate;
            being_point.pixel_draw  = &being_ground;

            if ( math_join_vect2( start.x, start.y, &delta, &being_point ) )
            {
                return 0;
            }
        }
    }
    return 1;
}

#ifdef BRAINCODE_ON
static void being_init_braincode_create( simulated_being *local, n_byte internal )
{
    n_byte2 *local_random = being_get_random( local );

    n_int ch = 0;
    /** initially seed the brain with instructions which are random but genetically biased */
    while ( ch < BRAINCODE_SIZE )
    {
        math_random3( local_random );
        if ( internal != 0 )
        {
#ifdef RANDOM_INITIAL_BRAINCODE
            being_braincode_internal( local )[ch] = math_random( local_random ) & 255;
#else
            being_random3( local );
            being_braincode_internal( local )[ch] = ( math_random( local_random ) & 192 ) | get_braincode_instruction( local );
#endif
            being_braincode_internal( local )[ch + 1] = math_random( local_random ) & 255;
            being_braincode_internal( local )[ch + 2] = math_random( local_random ) & 255;
        }
        else
        {
#ifdef RANDOM_INITIAL_BRAINCODE
            being_braincode_external( local )[ch] = math_random( local_random ) & 255;
#else
            being_random3( local );
            being_braincode_external( local )[ch] = ( math_random( local_random ) & 192 ) | get_braincode_instruction( local );
#endif
            being_braincode_external( local )[ch + 1] = math_random( local_random ) & 255;
            being_braincode_external( local )[ch + 2] = math_random( local_random ) & 255;
        }
        ch += 3;
    }
}


/** initialise inner or outer braincode */
void being_init_braincode( simulated_being *local,
                           simulated_being *other,
                           n_byte friend_foe,
                           n_byte internal )
{
    n_uint i, most_similar_index, diff, min, actor_index;
    simulated_isocial *graph;
    if ( other == 0L )
    {
        being_init_braincode_create( local, internal );
    }
    else
    {
        /** initialise based upon a similar being */
        graph = being_social( local );

        if ( graph == 0L )
        {
            return;
        }

        most_similar_index = 0;
        min = 99999;
        actor_index = being_attention( local, ATTENTION_ACTOR );

        /** Find the entry in the social graph with the most similar friend or foe value.
         The FOF value is used because when two beings meet for the first time this
         value is calculated based upon a variety of genetic and learned dispositions.
         Notice also that the search includes index zero, which is the self. */
        for ( i = 0; i < SOCIAL_SIZE; i++ )
        {
            if ( ( i != actor_index ) && ( !SOCIAL_GRAPH_ENTRY_EMPTY( graph, i ) ) )
            {
                n_int signed_diff = ( n_int )graph[i].friend_foe - ( n_int )friend_foe;
                if ( signed_diff < 0 )
                {
                    diff = ( n_uint )( 0 - signed_diff );
                }
                else
                {
                    diff = ( n_uint )signed_diff;
                }
                if ( diff < min )
                {
                    min = diff;
                    most_similar_index = i;
                }
            }
        }
        /** Copy braincode for the most similar individual */
        memory_copy( graph[most_similar_index].braincode, graph[actor_index].braincode, BRAINCODE_SIZE );
    }
}
#endif


/** Assign a unique name to the given being, based upon the given family names */
static n_int being_set_unique_name( simulated_being *beings,
                                    n_int number,
                                    simulated_being *local_being,
                                    n_byte2   mother_family_name,
                                    n_byte2   father_family_name )
{
    n_int     i;
    n_int     samples = 0, found = 0;
    n_byte2   possible_family_name;
    n_byte2   possible_first_name;

    /** random number initialization */
    being_random3( local_being );
    being_random3( local_being );


    /** if no mother and father are specified then randomly create names */
    if ( ( mother_family_name == 0 ) && ( father_family_name == 0 ) )
    {
        n_byte2 *random_factor = being_get_random( local_being );

        mother_family_name =
            GET_NAME_FAMILY( ( random_factor[0] & FAMILY_NAME_AND_MOD ),
                             ( random_factor[1] & FAMILY_NAME_AND_MOD ) );

        math_random3( random_factor );

        father_family_name =
            GET_NAME_FAMILY( ( random_factor[0] & FAMILY_NAME_AND_MOD ),
                             ( random_factor[1] & FAMILY_NAME_AND_MOD ) );
    }

    /** conventional family name */
    possible_family_name =
        GET_NAME_FAMILY( UNPACK_FAMILY_FIRST_NAME( mother_family_name ),
                         UNPACK_FAMILY_FIRST_NAME( father_family_name ) );

    while ( ( found == 0 ) && ( samples < 2048 ) )
    {
        n_byte2 *random_factor = being_get_random( local_being );

        being_random3( local_being );

        /** choose a first_name at random */
        possible_first_name = ( n_byte2 )( ( random_factor[0] & 255 ) | ( FIND_SEX( GET_I( local_being ) ) << 8 ) );


        /** avoid the same two family names */
        if ( UNPACK_FAMILY_FIRST_NAME( mother_family_name ) ==
                UNPACK_FAMILY_SECOND_NAME( father_family_name ) )
        {

            being_random3( local_being );

            random_factor = being_get_random( local_being );

            possible_family_name =
                GET_NAME_FAMILY( ( random_factor[0] & FAMILY_NAME_AND_MOD ),
                                 ( random_factor[1] & FAMILY_NAME_AND_MOD ) );
        }
        if ( samples == 1024 )
        {
            being_random3( local_being );

            random_factor = being_get_random( local_being );

            possible_family_name =
                GET_NAME_FAMILY( ( random_factor[0] & FAMILY_NAME_AND_MOD ),
                                 ( random_factor[1] & FAMILY_NAME_AND_MOD ) );
        }

        /** avoid the same two family names */
        if ( UNPACK_FAMILY_SECOND_NAME( mother_family_name ) ==
                UNPACK_FAMILY_FIRST_NAME( father_family_name ) )
        {
            being_random3( local_being );

            random_factor = being_get_random( local_being );

            possible_family_name =
                GET_NAME_FAMILY( ( random_factor[0] & FAMILY_NAME_AND_MOD ),
                                 ( random_factor[1] & FAMILY_NAME_AND_MOD ) );
        }

        being_set_first_name( local_being, possible_first_name );

        being_set_family_name( local_being,
                               UNPACK_FAMILY_FIRST_NAME( possible_family_name ),
                               UNPACK_FAMILY_SECOND_NAME( possible_family_name ) );

        /** does the name already exist in the population */
        found = 1;
        for ( i = 0; i < number; i++ )
        {
            simulated_being *other_being = &beings[i];
            if ( being_name_comparison( local_being, being_gender_name( other_being ), being_family_name( other_being ) ) )
            {
                found = 0;
                break;
            }
        }
        samples++;
    }

    return found;
}

static void being_random_genetics( n_genetics *value, n_byte2 *random, n_int male )
{
    n_int loop = 0;
    math_random3( random );
    while ( loop < CHROMOSOMES )
    {
        n_uint loop2 = 0;

        value[loop] = 0;

        while ( loop2 < ( sizeof( n_genetics ) * 8 ) )
        {
            if ( math_random( random ) & 1 )
            {
                value[loop] |= 1 << loop2;
            }
            loop2++;
        }
        loop++;
    }
    value[CHROMOSOME_Y] = ( n_genetics )( value[CHROMOSOME_Y] & 0xfffffffe );
    value[CHROMOSOME_Y] |= ( male ? 2 : 3 );
}

n_uint being_init_group( simulated_being *beings, n_byte2 *local_random, n_uint count_to, n_uint max )
{
    n_uint num = 0;
    math_random3( local_random );

    while ( num < count_to )
    {
        math_random3( local_random );
        if ( ( num + 1 ) < max )
        {
            if ( being_init( beings, ( n_int )num, &beings[num], 0L, local_random ) != 0 )
            {
                being_erase( &beings[num] );
                break;
            }
            else
            {
                num++;
            }
        }
    }
    return num;
}


/**
 * Initialise the ape's variables and clear its brain
\ * @param mother Pointer to the mother
 * @param random_factor Random seed
 * @return 0
 */
n_int being_init( simulated_being *beings, n_int number,
                  simulated_being *local, simulated_being *mother,
                  n_byte2 *random_factor )
{
    n_int        loop = 0;
#ifdef EPISODIC_ON
    simulated_isocial *local_social_graph = being_social( local );
    simulated_iepisodic *local_episodic = being_episodic( local );

    if ( local_social_graph == 0L )
    {
        return SHOW_ERROR( "Social memory not available" );
    }
    if ( local_episodic == 0L )
    {
        return SHOW_ERROR( "Episodic memory not available" );
    }
#endif
    being_erase( local );

#ifdef BRAIN_ON
    {
        n_byte       *brain_memory = being_brain( local );
        if ( brain_memory != 0L )
        {
            memory_erase( brain_memory, DOUBLE_BRAIN );
        }
        else
        {
            return SHOW_ERROR( "Brain memory not available" );
        }
    }
#endif

    being_set_goal_none( local );

    /** Set learned preferences to 0.5 (no preference in either direction.
     This may seem like tabla rasa, but there are genetic biases */

    while ( loop < PREFERENCES )
    {
        local->changes.learned_preference[loop++] = 127;
    }

    immune_init( &( local->immune_system ), being_get_random( local ) );
    being_clear_attention( local );

    /** clear the generation numbers for mother and father */
    if ( mother )
    {
        local->constant.generation_max = mother->changes.child_generation_max + 1;
        local->constant.generation_min = mother->changes.child_generation_min + 1;
    }
    else
    {
        local->constant.generation_max = 0;
        local->constant.generation_min = 0;
    }
    local->changes.child_generation_max = 0;
    local->changes.child_generation_min = 0;

    /** initially seed the brain with instructions which
     are genetically biased */


    if ( random_factor )
    {
        being_set_random( local, random_factor );

        being_random3( local );
        being_random3( local );
    }
    else if ( mother )
    {
        ( void )being_random( mother );

        being_set_random( local, being_get_random( mother ) );

        being_random3( local );

        being_set_random1( local, being_get_random( mother )[0] );

        being_random3( local );

        being_set_random1( local, ( n_byte2 )land_time() );

        being_random3( local );
    }
    else
    {
        NA_ASSERT( random_factor, "Random factor not set" );
        NA_ASSERT( mother, "Mother not set" );
        return SHOW_ERROR( "No correct being interface provided" );
    }
#ifdef BRAINCODE_ON

    being_random3( local );

#ifdef EPISODIC_ON
    /** has no social connections initially */
    memory_erase( ( n_byte * )local_social_graph, sizeof( simulated_isocial )*SOCIAL_SIZE );

    loop = 0;
    while ( loop < EPISODIC_SIZE )
    {
        local_episodic[loop++].affect = EPISODIC_AFFECT_ZERO;
    }

    local_social_graph[0].relationship = RELATIONSHIP_SELF;
    loop = 0;
    while ( loop < SOCIAL_SIZE )
    {
        /** default type of entity */
        local_social_graph[loop].entity_type = ENTITY_BEING;
        /** friend_or_foe can be positive or negative,
         with SOCIAL_RESPECT_NORMAL as the zero point */
        local_social_graph[loop].friend_foe = SOCIAL_RESPECT_NORMAL;
        loop++;
    }
#endif

    /* TODO: Apply fitness function around the braincode generation */

    being_init_braincode( local, 0L, 0, BRAINCODE_INTERNAL );
    being_init_braincode( local, 0L, 0, BRAINCODE_EXTERNAL );

    /** randomly initialize registers */
    loop = 0;
    while ( loop < BRAINCODE_PSPACE_REGISTERS )
    {
        being_random3( local );
        local->braindata.braincode_register[loop++] = ( n_byte )being_random( local ) & 255;
    }

    /** initialize brainprobes */
    loop = 0;
    while ( loop < BRAINCODE_PROBES )
    {
        being_random3( local );
        if ( being_random( local ) & 1 )
        {
            local->braindata.brainprobe[loop].type = INPUT_SENSOR;
        }
        else
        {
            local->braindata.brainprobe[loop].type = OUTPUT_ACTUATOR;
        }
        local->braindata.brainprobe[loop].frequency = ( n_byte )1 + ( being_random( local ) % BRAINCODE_MAX_FREQUENCY );
        being_random3( local );
        local->braindata.brainprobe[loop].address = ( n_byte )being_random( local ) & 255;
        local->braindata.brainprobe[loop].position = ( n_byte )being_random( local ) & 255;
        being_random3( local );
        local->braindata.brainprobe[loop].offset = ( n_byte )being_random( local ) & 255;
        loop++;
    }
#endif

    being_facing_init( local );

    if ( random_factor )
    {
        n_byte2 location[2];
        n_vect2 location_vector;
        being_random3( local );

        being_initial_location( &location_vector, being_get_random( local ) );

        location[0] = ( n_byte2 )location_vector.x;
        location[1] = ( n_byte2 )location_vector.y;

        being_set_location( local, location );

        {
            n_genetics mother_genetics[CHROMOSOMES];
            n_genetics father_genetics[CHROMOSOMES];
            n_byte2    gene_random[2];

            being_random3( local );

            gene_random[0] = being_random( local );

            being_random3( local );
            being_random3( local );

            gene_random[1] = being_random( local );

            being_random_genetics( mother_genetics, gene_random, 0 );

            being_random3( local );

            gene_random[0] = being_random( local );
            being_random3( local );
            being_random3( local );
            being_random3( local );

            gene_random[1] = being_random( local );

            being_random_genetics( father_genetics, gene_random, 1 );
            being_random3( local );

            body_genetics( beings, number, being_genetics( local ), mother_genetics, father_genetics, gene_random );

            being_set_unique_name( beings, number, local, 0L, 0L );
        }
        local->delta.social_coord_x = local->delta.social_coord_nx =
                                          ( math_random( local->delta.random_seed ) & 32767 ) + 16384;
        local->delta.social_coord_y = local->delta.social_coord_ny =
                                          ( math_random( local->delta.random_seed ) & 32767 ) + 16384;

        local->constant.date_of_birth = 0;
    }
    else
    {
        being_set_location( local, being_location( mother ) );

        /** this is the same as equals */
        being_wander( local, being_facing( mother ) - being_facing( local ) );

        ( void ) being_random( local );
        local->delta.social_coord_x = local->delta.social_coord_nx = mother->delta.social_coord_x;
        local->delta.social_coord_y = local->delta.social_coord_ny = mother->delta.social_coord_y;

        genetics_set( being_genetics( local ), being_fetal_genetics( mother ) );

        /** ascribed social status */
        local->delta.honor = ( n_byte )being_honor( mother );

        being_set_unique_name( beings, number, local,
                               being_family_name( mother ),
                               mother->changes.father_name[1] );

        local->constant.date_of_birth = land_date();
    }

    being_living( local );

    if ( random_factor )
    {
        being_set_height( local, BIRTH_HEIGHT );

        GET_M( local ) = BIRTH_MASS;
    }
    else
    {
        /** produce an initial distribution of heights and masses*/
        being_random3( local );
        being_set_height( local, BIRTH_HEIGHT +
                          ( local->delta.random_seed[0] % ( BEING_MAX_HEIGHT - BIRTH_HEIGHT ) ) );

        GET_M( local ) = BIRTH_MASS +
                         ( local->delta.random_seed[1] % ( BEING_MAX_MASS_G - BIRTH_MASS ) );
    }

    local->delta.crowding = MIN_CROWDING;
#ifdef BRAIN_ON
    if ( being_brain( local ) )
    {
        /** These magic numbers were found in March 2001 -
         feel free to change them! */
        being_set_brainstates( local, 0, 171, 0, 146 );
        being_set_brainstates( local, 1, 86, 501, 73 );

    }
#endif
    return 0;
}

n_int being_move_energy( simulated_being *local_being, n_int *conductance )
{
    n_int      local_s  = being_speed( local_being );
    n_int      delta_e = 0;
    n_vect2    location_vector;
    n_vect2    facing_vector;
    n_genetics  *genetics = being_genetics( local_being );

    being_space( local_being, &location_vector );

    being_facing_vector( local_being, &facing_vector, 1 );

    if ( local_s > 0 )
    {
        n_byte2 location[2];
        vect2_d( &location_vector, &facing_vector, local_s, 512 );

        /* vector to n_byte2 may do incorrect wrap around MUST be improved */

        being_wrap( &location_vector );

        location[0] = ( n_byte2 )location_vector.x;
        location[1] = ( n_byte2 )location_vector.y;

        being_set_location( local_being, location );
#ifdef DEBUG_LACK_OF_MOVEMENT
        being_add_total_movement( local_being );
#endif
    }

    {
        n_int delta_z;
        n_int delta_energy;
        n_int local_z;
        n_vect2 slope_vector;

        land_vect2( &slope_vector, &local_z, &location_vector );

        delta_z = vect2_dot( &slope_vector, &facing_vector, 1, 96 );
        delta_energy = ( ( 512 - delta_z ) * local_s ) / 80;

        if ( WATER_TEST( local_z, land_tide_level() ) )
        {
            n_int insulation = 0;
            /** the more body fat, the less energy is lost whilst swimming */
            n_int fat_mass = GET_BODY_FAT( local_being );
            delta_energy = ( ( delta_energy * delta_energy ) >> 9 );
            if ( fat_mass > BEING_MAX_MASS_FAT_G )
            {
                fat_mass = BEING_MAX_MASS_FAT_G;
            }
            insulation = fat_mass * 5 / BEING_MAX_MASS_FAT_G;
            delta_e += ( delta_energy + 10 - insulation ) >> 3;
            *conductance = 4;
        }
        else
        {
            if ( delta_z > 0 )
            {
                /** going uphill */
                delta_energy += GENE_HILL_CLIMB( genetics );
            }

            delta_energy = ( ( delta_energy * delta_energy ) >> 9 );

            /* the more massive the more energy consumed when moving */
            delta_e += ( delta_energy + 4 + ( GET_M( local_being ) * 5 / BEING_MAX_MASS_G ) ) >> 2;
        }
    }
    return delta_e;
}
