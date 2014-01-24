/****************************************************************
 
 execute.c
 
 =============================================================
 
 Copyright 1996-2014 Tom Barbalet. All rights reserved.
 
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

#include "noble.h"
#undef EXECUTE_THREADED
#ifdef EXECUTE_THREADED

#ifdef _WIN32

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>

#define MAX_EXECUTION_THREAD_SIZE 4

#else

#include <pthread.h>
#include <time.h>

#define MAX_EXECUTION_THREAD_SIZE 2

#endif


typedef enum
{
    ES_DONE = 0,
    ES_WAITING,
    ES_STARTED
}execute_state;

typedef struct
{
    execute_function * function;
    void             * general_data;
    void             * read_data;
    void             * write_data;
} execute_object;

typedef struct
{
    execute_object * executed;
    execute_state  state;
} execution_thread;

static n_int global_cycle = 1;
static n_int execution_cycle = 0;


#ifdef _WIN32

static HANDLE            thread[MAX_EXECUTION_THREAD_SIZE];
static DWORD             threadId[MAX_EXECUTION_THREAD_SIZE];
#else

static pthread_t         thread[MAX_EXECUTION_THREAD_SIZE] = {0L};

#endif

static execution_thread  execution[MAX_EXECUTION_THREAD_SIZE] = {0L};

static void execute_wait_ms(void)
{
    /* In time cycle testing this may be seen to be wasted time
     the problem however is the execution is not available
     */
#ifndef _WIN32
    struct timespec tim, tim2;
    tim.tv_sec = 0;
    tim.tv_nsec = 1;
    (void)nanosleep(&tim , &tim2);
#endif
}


#endif

void execute_add(execute_function * function, void * general_data, void * read_data, void * write_data)
{
#ifndef EXECUTE_THREADED
    function(general_data,read_data,write_data);
#else
    n_byte2     random[2];
    n_byte2    *value = (n_byte2*)function;
    random[0] = value[0];
    random[1] = value[1];
    
    execution_cycle = 1;
    do{
        n_int   loop = 0;
        n_byte2 loop_order = math_random(random);
        
        while (loop < MAX_EXECUTION_THREAD_SIZE)
        {
            n_byte2 hit_location = (loop ^ loop_order) & (MAX_EXECUTION_THREAD_SIZE - 1);
            if (execution[hit_location].state == ES_DONE)
            {
                execute_object * new_object = io_new(sizeof(execute_object));
                new_object->function = function;
                new_object->general_data = general_data;
                new_object->read_data = read_data;
                new_object->write_data = write_data;
                    
                execution[hit_location].executed = new_object;
                execution[hit_location].state = ES_WAITING;
                
                return;
            }
            
            loop++;
        }
        execute_wait_ms();
    }while (global_cycle);
#endif
}

void execute_complete_added(void)
{
#ifdef EXECUTE_THREADED
    do
    {
    }while (execution_cycle);
#endif
}

void execute_close(void)
{
#ifdef EXECUTE_THREADED
    global_cycle = 0;
#ifdef _WIN32
	{
		n_int loop = 0;
		while (loop < MAX_EXECUTION_THREAD_SIZE)
		{
			CloseHandle(thread[loop]);
			loop++;
		}
	}
#endif
#endif
}

#ifdef EXECUTE_THREADED

static void execute_thread_generic(void * id)
{
    execution_thread * value = id;
    do{
        n_int            loop = 0;
        n_int            all_idle = 1;
        if (value->state != ES_WAITING)
        {
            execute_wait_ms();
        }
        if (value->state == ES_WAITING)
        {
            execute_object * object = value->executed;
            value->state = ES_STARTED;
            
            if (object->function(object->general_data, object->read_data, object->write_data) == -1)
            {
                execute_close();
            }
            value->state = ES_DONE;
            io_free((void **)&object);
            while ((loop < MAX_EXECUTION_THREAD_SIZE) && all_idle)
            {
                if (execution[loop].state != ES_DONE)
                {
                    all_idle = 0;
                }
                loop++;
            }
            if (all_idle)
            {
                execution_cycle = 0;
            }
        }
    }while (global_cycle);
}

#ifdef _WIN32

static DWORD WINAPI execute_thread_win( LPVOID lpParam )
{
    HANDLE hStdout;
    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    if( hStdout == INVALID_HANDLE_VALUE )
        return 1;
    execute_thread_generic((void *)lpParam);
    return 0;
}

#else
static void * execute_thread_posix(void * id)
{
    execute_thread_generic(id);
    pthread_exit(0L);
}
#endif

#endif

void execute_init(void)
{
#ifdef    EXECUTE_THREADED
    n_int loop = 0;
    while (loop < MAX_EXECUTION_THREAD_SIZE)
    {
#ifdef _WIN32
        threadId[loop] = loop;
        thread[loop] = CreateThread(
                                       NULL,                   // default security attributes
                                       0,                      // use default stack size
                                       execute_thread_win,       // thread function name
                                       &execution[loop],          // argument to thread function
                                       0,                      // use default creation flags
                                       &threadId[loop]);   // returns the thread identifier
#else
        pthread_create(&thread[loop], 0L, execute_thread_posix, &execution[loop]);
#endif
        loop++;
    }
#endif
}
