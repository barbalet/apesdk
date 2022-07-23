/****************************************************************

    useful.c

    =============================================================

 Copyright 1996-2022 Tom Barbalet. All rights reserved.

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

// command line interface

#include "cli.h"

const n_string RUN_STEP_CONST = "RSC";

/** How many steps at which to periodically save the simulation */
static n_uint save_interval_steps = 60;

static n_int simulation_running = 1;

static n_int simulation_executing = 0;

/*
     {&command_reset,         "reset",          "",                     "Reset the simulation"},
     {&command_reset,         "clear"           "",                     ""},

     {&command_open,          "open",           "[file]",               "Load a simulation file"},
     {&command_open,          "load",           "",                     ""},

     {&useful_quit,           "quit",           "",                     "Quits the console"},
     {&useful_quit,           "exit",           "",                     ""},
     {&useful_quit,           "close",          "",                     ""},

     {&useful_stop,          "stop",           "",                     "Stop the simulation during step or run"},

     {&useful_run,           "run",            "(time format)|forever", "Simulate for a given number of days or forever"},
     {&useful_step,          "step",           "",                     "Run for a single logging interval"},
     {&useful_interval,      "interval",       "(days)",               "Set the simulation logging interval in days"},
 
     {&useful_simulation,    "simulation",     "",                     ""},
     {&useful_simulation,    "sim",            "",                     "Show simulation parameters"},
 
     {&command_list,          "list",           "",                     "List all ape names"},
     {&command_list,          "ls",             "",                     ""},
     {&command_list,          "dir",            "",                     ""},

     {&useful_next,          "next",           "",                     "Next ape"},

     {&useful_previous,      "previous",       "",                     "Previous ape"},
     {&useful_previous,      "prev",           "",                     ""},


     {0L, 0L},
 };

 
 */


/**
 * gets the number of mins/hours/days/months/years
 * @param str text to be processed
 * @param number the number of the time units
 * @param interval the time units
 * @return number of mins/hours/days/months/years
 */
static n_int get_time_interval( n_string str, n_int *number, n_int *interval )
{
    n_int i, index = 0, ctr = 0, result = 0, divisor = 0;
    char c;
    n_string_block buf;
    n_int retval = -1;
    n_int length = io_length( str, 256 );

    for ( i = 0; i < length; i++ )
    {
        if ( str[i] != ' ' )
        {
            buf[ctr++] = str[i];
        }

        if ( ( str[i] == ' ' ) || ( i == ( length - 1 ) ) )
        {
            buf[ctr] = 0;

            switch ( index )
            {
            case 0:
            {
                io_number( ( n_string )buf, &result, &divisor );
                *number = result;
                retval = 0;
                break;
            }
            case 1:
            {
                if ( ctr == 1 )
                {
                    char lower_c;
                    lower_c = c = buf[0];
                    IO_LOWER_CHAR( lower_c );
                    if ( c == 'm' )
                    {
                        *interval = INTERVAL_MINS;
                    }
                    if ( lower_c == 'h' )
                    {
                        *interval = INTERVAL_HOURS;
                    }
                    if ( lower_c == 'd' )
                    {
                        *interval = INTERVAL_DAYS;
                    }
                    if ( c == 'M' )
                    {
                        *interval = INTERVAL_MONTHS;
                    }
                    if ( lower_c == 'y' )
                    {
                        *interval = INTERVAL_YEARS;
                    }
                }
                else
                {
                    IO_LOWER_CHAR( buf[0] );
                    if ( io_find( ( n_string )buf, 0, ctr, "min", 3 ) > -1 )
                    {
                        *interval = INTERVAL_MINS;
                    }
                    if ( ( io_find( ( n_string )buf, 0, ctr, "hour", 4 ) > -1 ) ||
                            ( io_find( ( n_string )buf, 0, ctr, "hr", 2 ) > -1 ) )
                    {
                        *interval = INTERVAL_HOURS;
                    }
                    if ( io_find( ( n_string )buf, 0, ctr, "day", 3 ) > -1 )
                    {
                        *interval = INTERVAL_DAYS;
                    }
                    if ( io_find( ( n_string )buf, 0, ctr, "mon", 3 ) > -1 )
                    {
                        *interval = INTERVAL_MONTHS;
                    }
                }

                break;
            }
            }

            index++;
            ctr = 0;
        }
    }
    return retval;
}




/**
 * Set the time interval for simulation
 * @param ptr pointer to simulated_group object
 * @param response command parameters
 * @param output_function function used to display the output
 * @return 0
 */
n_int useful_interval( void *ptr, n_string response, n_console_output output_function )
{
    n_int number = 1, interval = INTERVAL_DAYS, interval_set = 0;

    if ( response != 0 )
    {
        if ( io_length( response, STRING_BLOCK_SIZE ) > 0 )
        {
            if ( get_time_interval( response, &number, &interval ) > -1 )
            {
                if ( number > 0 )
                {
                    n_string_block  output, number_string;
                    save_interval_steps = number * interval_steps[interval];
                    io_number_to_string( number_string, number );
                    io_three_strings( output, "Logging interval set to ", number_string, ( n_string )interval_description[interval], 0 );
                    output_function( output );
                    interval_set = 1;
                }
            }
        }
    }

    if ( interval_set == 0 )
    {
        if ( save_interval_steps < 60 )
        {
            n_string_block  output, number_string;
            io_number_to_string( number_string, save_interval_steps );
            io_three_strings( output, "Current time interval is ", number_string, " min(s)", 0 );
            output_function( output );
        }
        else
        {
            if ( save_interval_steps < 60 * 24 )
            {
                n_string_block  output, number_string;
                io_number_to_string( number_string, save_interval_steps / 60 );
                io_three_strings( output, "Current time interval is ", number_string, " hour(s)", 0 );
                output_function( output );
            }
            else
            {
                n_string_block  output, number_string;
                io_number_to_string( number_string, save_interval_steps / ( 60 * 24 ) );
                io_three_strings( output, "Current time interval is ", number_string, " day(s)", 0 );
                output_function( output );
            }
        }
    }
    return 0;
}

n_int useful_stop( void *ptr, n_string response, n_console_output output_function )
{
    simulation_running = 0;
    if ( output_function )
    {
        output_function( "Simulation stopped" );
    }
    return 0;
}

n_int useful_quit( void *ptr, n_string response, n_console_output output_function )
{
    simulation_executing = 0;
    ( void )useful_stop( ptr, response, output_function );
    return io_quit( ptr, response, output_function );
}

n_int useful_executing( void )
{
    return simulation_executing;
}

/**
 * Show details of the overall simulation
 * @param ptr pointer to simulated_group object
 * @param response parameters of the command
 * @param output_function function to be used to display the result
 * @return 0
 */
n_int useful_simulation( void *ptr, n_string response, n_console_output output_function )
{
    return cli_execute_simulation( ptr, response, output_function );
}



/**
 * Run the simulation for a single time interval
 * @param ptr pointer to simulated_group object
 * @param response command parameters
 * @param output_function function used to display the output
 * @return 0
 */
n_int useful_step( void *ptr, n_string response, n_console_output output_function )
{
    n_uint loop = 0;

    if ( response != RUN_STEP_CONST )
    {
        if ( simulation_executing == 1 )
        {
            output_function( "Simulation already running" );
            return 0;
        }

        if ( cli_file_interaction() )
        {
            output_function( "File interaction in use: step" );
            return 0;
        }

        simulation_executing = 1;
    }
    simulation_running = 1;

    while ( ( loop < save_interval_steps ) && simulation_running )
    {
        if (cli_execute_cycle(ptr) == 0)
        {
            simulation_running = 0;
        }
        loop++;
    }
    if ( response != RUN_STEP_CONST )
    {
        
    }

    if ( response != RUN_STEP_CONST )
    {
        simulation_executing = 0;
    }

    return 0;
}

/**
 * Run the simulation
 * @param ptr pointer to simulated_group object
 * @param response command parameters
 * @param output_function function used to display the output
 * @return 0
 */
n_int useful_run( void *ptr, n_string response, n_console_output output_function )
{
    n_uint run = 0;
    n_int  number = 0, interval = INTERVAL_DAYS;
    n_int  forever = 0;

    if ( simulation_executing == 1 )
    {
        output_function( "Simulation already running" );
        return 0;
    }

    if ( cli_file_interaction() )
    {
        output_function( "File interaction in use: run" );
        return 0;
    }

    simulation_executing = 1;

    simulation_running = 1;

    if ( response != 0L )
    {
        n_int length = io_length( response, STRING_BLOCK_SIZE );
        if ( length > 0 )
        {
            if ( ( io_find( response, 0, length, "forever", 7 ) > -1 ) )
            {
                forever = 1;
                number = 1;
            }
            else if ( get_time_interval( response, &number, &interval ) <= -1 )
            {
                number = -1;
            }

            if ( number > 0 )
            {
                n_uint i = 0;
                n_string_block  output;
                n_uint end_point = ( number * interval_steps[interval] );
                n_uint temp_save_interval_steps = save_interval_steps;
                n_uint count = 0;
                save_interval_steps = 1;

                if ( forever )
                {
                    io_three_strings( output, "Running forever (type \"stop\" to end)", "", "", 0 );
                }
                else
                {
                    n_string_block  number_string;
                    io_number_to_string( number_string, number );
                    io_three_strings( output, "Running for ", number_string, ( n_string )interval_description[interval], 0 );
                }

                output_function( output );

                while ( ( i < end_point ) && simulation_running )
                {
                    useful_step( ptr, RUN_STEP_CONST, output_function );

                    if ( temp_save_interval_steps )
                    {
                        if ( ( count % temp_save_interval_steps ) == 0 )
                        {
                            cli_execute_entity( ptr, output_function );
                        }
                    }
                    count++;
                    if ( !forever )
                    {
                        i++;
                    }
                }

                if ( temp_save_interval_steps )
                {
                    if ( ( count % temp_save_interval_steps ) != 1 )
                    {
                        cli_execute_entity( ptr, output_function );
                    }
                }

                save_interval_steps = temp_save_interval_steps;
                run = 1;
            }
        }
    }

    simulation_executing = 0;

    if ( run == 0 )
    {
        ( void )SHOW_ERROR( "Time not specified, examples: run 2 days, run 6 hours" );
    }

    return 0;
}


n_int useful_next( void *ptr, n_string response, n_console_output output_function )
{
    cli_execute_next(ptr, output_function);
    return 0;
}

n_int useful_previous( void *ptr, n_string response, n_console_output output_function )
{
    cli_execute_previous(ptr, output_function);
    return 0;
}
