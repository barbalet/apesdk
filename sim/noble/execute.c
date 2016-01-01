/****************************************************************

 execute.c

 =============================================================

 Copyright 1996-2016 Tom Barbalet. All rights reserved.

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


#ifdef EXECUTE_THREADED

#ifdef _WIN32

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>

#else

#include <pthread.h>
#include <time.h>


#endif

typedef enum
{
    ES_DONE = 0,
    ES_WAITING,
    ES_STARTED
} execute_state;

typedef struct
{
    execute_function * function;
    void             * general_data;
    void             * read_data;
    void             * write_data;
    n_int              count;
    n_int              size;
} execute_object;

typedef struct
{
    execute_object * executed;
    execute_state  state;
    n_byte2        random[2];
    n_int          counter;
} execution_thread;

static n_int global_cycle = 1;
static n_int execution_cycle = 0;


#ifdef _WIN32

static HANDLE            *thread = 0L;
static DWORD             *threadId = 0L;

#else

static pthread_t         *thread = 0L;

#endif

static execution_thread  *execution = 0L;

static n_int execution_thread_size = 4;

static void execute_wait_ns(void)
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

static void execute_add_generic(execute_function * function, void * general_data, void * read_data, void * write_data, n_int count, n_int size)
{
#ifndef EXECUTE_THREADED
    if (size)
    {
        n_byte *location = (n_byte *)read_data;
        n_int   loop = 0;
        while (loop < count)
        {
            if (function(general_data, (void *)&location[loop * size], 0L) == -1)
            {
                break;
            }
            loop++;
        }
    }
    else
    {
        function(general_data,read_data,write_data);
    }
#else
    execution_cycle = 1;
    do
    {
        n_int   loop = 0;
        while (loop < execution_thread_size)
        {
            if (execution[loop].state == ES_DONE)
            {
                execute_object * new_object = io_new(sizeof(execute_object));
                new_object->function = function;
                new_object->general_data = general_data;
                new_object->read_data  = read_data;
                new_object->write_data = write_data;
                new_object->count      = count;
                new_object->size       = size;
                execution[loop].executed = new_object;
                execution[loop].state = ES_WAITING;
                return;
            }

            loop++;
        }
    }
    while (global_cycle);
#endif
}

void execute_complete_added(void)
{
#ifdef EXECUTE_THREADED
    while (1 == 1)
    {
        if (execution_cycle == 0)
            break;
        execute_wait_ns();
    }
#endif
}

void execute_add(execute_function * function, void * general_data, void * read_data, void * write_data)
{
    /* This is short-hand to allow for generic execution paths */
    if (function)
    {
        execute_add_generic(function, general_data, read_data, write_data, 1, 0);
    }
    else
    {
        execute_complete_added();
    }
}

void execute_group(execute_function * function, void * general_data, void * read_data, n_int count, n_int size)
{
    execute_add_generic(function, general_data, read_data, 0L, count, size);
}

void execute_close(void)
{
#ifdef EXECUTE_THREADED
    global_cycle = 0;
#ifdef _WIN32
    {
        n_int loop = 0;
        while (loop < execution_thread_size)
        {
            CloseHandle(thread[loop]);
            loop++;
        }
    }
#endif
#endif
}

void execute_threads(n_int value)
{
#ifdef EXECUTE_THREADED
    execution_thread_size = value;
#else
    (void)value;
#endif
}

n_int execute_threads_value(void)
{
#ifdef EXECUTE_THREADED
    return execution_thread_size;
#else
    return 1;
#endif
}

#ifdef EXECUTE_THREADED

static void execute_thread_generic(void * id)
{
    execution_thread * value = id;
    do
    {
        n_int            loop = 0;
        n_int            all_idle = 1;
        if (value->state != ES_WAITING)
        {
            value->counter++;
            
            if (value->counter == 15)
            {
                if ((math_random(value->random) & 255) == 1)
                {
                    execute_wait_ns();
                }
                value->counter = 0;
            }
        }
        if (value->state == ES_WAITING)
        {
            execute_object * object = value->executed;
            value->state = ES_STARTED;

            if (object->size)
            {
                n_byte *location = (n_byte *)object->read_data;
                n_int   loop = 0;
                while (loop < object->count)
                {
                    if (object->function(object->general_data, (void *)&location[loop * object->size], 0L) == -1)
                    {
                        execute_close();
                        break;
                    }
                    loop++;
                }
            }
            else
            {
                if (object->function(object->general_data, object->read_data, object->write_data) == -1)
                {
                    execute_close();
                }
            }
            value->state = ES_DONE;
            io_free((void **)&object);
        }
        loop = 0;
        while (loop < execution_thread_size)
        {
            if (execution[loop].state != ES_DONE)
            {
                all_idle = 0;
                break;
            }
            loop++;
        }
        if (all_idle)
        {
            execution_cycle = 0;
        }
    }
    while (global_cycle);

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
    n_int   loop = 0;
    n_byte2 master_random[2] = {0xf672, 0x3e71};


#ifdef _WIN32
    thread = (HANDLE*)io_new(execution_thread_size * sizeof(HANDLE));
#else
    thread = (pthread_t*)io_new(execution_thread_size * sizeof(pthread_t));
#endif

    if (thread == 0L)
    {
        (void)SHOW_ERROR("Threads failed to allocate");
        return;
    }

#ifdef _WIN32
    threadId = (DWORD*)io_new(execution_thread_size * sizeof(DWORD));
    if (threadId == 0L)
    {
        io_free((void **)&thread);
        (void)SHOW_ERROR("ThreadIds failed to allocate");
        return;
    }
#endif

    execution = (execution_thread *)io_new(execution_thread_size * sizeof(execution_thread));

    if (execution == 0L)
    {
        io_free((void **)&thread);
#ifdef _WIN32
        io_free((void **)&threadId);
#endif
        (void)SHOW_ERROR("Execution thread failed to allocate");
        return;
    }
#ifdef _WIN32
    io_erase((n_byte*)thread, execution_thread_size * sizeof(HANDLE));
    io_erase((n_byte*)threadId, execution_thread_size * sizeof(DWORD));
#else
    io_erase((n_byte*)thread, execution_thread_size * sizeof(pthread_t));
#endif
    io_erase((n_byte*)execution, execution_thread_size * sizeof(execution_thread));

    while (loop < execution_thread_size)
    {
        execution[loop].random[0] = master_random[1];
        execution[loop].random[1] = master_random[0];
        execution[loop].counter = 0;

        math_random3(master_random);

#ifdef _WIN32
        threadId[loop] = loop;
        thread[loop] = CreateThread(NULL, 0, execute_thread_win, &execution[loop], 0, &threadId[loop]);
#else
        pthread_create(&thread[loop], 0L, execute_thread_posix, &execution[loop]);
#endif
        loop++;
    }
#endif
}
