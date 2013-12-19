/****************************************************************
 
 execute.c
 
 =============================================================
 
 Copyright 1996-2013 Tom Barbalet. All rights reserved.
 
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

#include <pthread.h>
#include <time.h>

#include "noble.h"

#ifdef EXECUTE_THREADED

#define MAX_EXECUTION_THREAD_SIZE 8

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

static int global_cycle = 1;
static int execution_cycle = 0;
static int threads_started = 0;
static pthread_t         thread[MAX_EXECUTION_THREAD_SIZE] = {0L};
static execution_thread  execution[MAX_EXECUTION_THREAD_SIZE] = {0L};

#endif

void execute_add(execute_function * function, void * general_data, void * read_data, void * write_data)
{
#ifndef EXECUTE_THREADED
    function(general_data,read_data,write_data);
#else
    if (threads_started == 0)
    {
        execute_init();
    }
    
    execution_cycle = 1;
    do{
        n_int loop = 0;
        while (loop < MAX_EXECUTION_THREAD_SIZE)
        {
            if (execution[loop].state == ES_DONE)
            {
                execute_object * new_object = io_new(sizeof(execute_object));
                new_object->function = function;
                new_object->general_data = general_data;
                new_object->read_data = read_data;
                new_object->write_data = write_data;
                    
                execution[loop].executed = new_object;
                execution[loop].state = ES_WAITING;
                
                return;
            }
            
            loop++;
        }
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
#endif
}

#ifdef EXECUTE_THREADED

static void * execute_thread(void * id)
{
    execution_thread * value = id;
    do{
        n_int            loop = 0;
        n_int            all_idle = 1;
        if (value->state != ES_WAITING)
        {
            /* In time cycle testing this may be seen to be wasted time
               the problem however is the execution is not available
             */
            struct timespec tim, tim2;
            tim.tv_sec = 0;
            tim.tv_nsec = 6;
            (void)nanosleep(&tim , &tim2);
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
    pthread_exit(NULL);
}
#endif


void execute_init(void)
{
#ifdef    EXECUTE_THREADED
    int loop = 0;
    while (loop < MAX_EXECUTION_THREAD_SIZE)
    {
        pthread_create(&thread[loop], NULL, execute_thread, &execution[loop]);
        loop++;
    }
    threads_started = 1;
#endif
}
