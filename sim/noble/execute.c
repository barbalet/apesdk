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

#include "noble.h"

#ifdef EXECUTE_DEBUG
    #include <stdio.h>
#endif


#define MAX_EXECUTION_THREAD_SIZE 3
#define EXECUTION_QUEUE_SIZE      32

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
static int execution_cycle = 1;

static pthread_t         thread[MAX_EXECUTION_THREAD_SIZE] = {0L};
static execution_thread  execution[MAX_EXECUTION_THREAD_SIZE] = {0L};
static execute_object    queue[EXECUTION_QUEUE_SIZE] = {0L};

static execute_periodic * periodic_function = 0L;

static int               written = 0, read = 0;

#ifdef EXECUTE_DEBUG
    static int           written_actual = 0, read_actual = 0;
    static int           print_count = 0;
    static int           greatest_delta = 0;
#endif

void execute_set_periodic(execute_periodic * function)
{
    periodic_function = function;
}

void execute_add(execute_function * function, void * general_data, void * read_data, void * write_data)
{
    
    queue[written].function = function;
    queue[written].general_data = general_data;
    queue[written].read_data = read_data;
    queue[written].write_data = write_data;
    
    written = (written + 1) & (EXECUTION_QUEUE_SIZE-1);
#ifdef EXECUTE_DEBUG
    written_actual++;
    {
        int delta = (written_actual - read_actual);
        if (delta > greatest_delta)
        {
            greatest_delta = delta;
        }
    }
    if ((print_count & 255) == 0)
    {
        printf("delta %d\n", greatest_delta);
        greatest_delta = 0;
    }
    print_count++;
#endif
}

void execute_quit(void)
{
    global_cycle = 0;
}

static void * execute_thread(void * id)
{
    execution_thread * value = id;
    
    do{
        if (value->state == ES_WAITING)
        {
            execute_object * object = value->executed;
            value->state = ES_STARTED;
            if (object->function(object->general_data, object->read_data, object->write_data) == -1)
            {
                execute_quit();
            }
            value->state = ES_DONE;
        }
    }while (global_cycle);
    pthread_exit(NULL);
}

void execute_main_loop(execute_periodic * initialization, execute_periodic * regular_cycle)
{
    int loop = 0;
    
    if (initialization == 0L)
    {
        return;
    }
    
    execute_set_periodic(regular_cycle);
    
    while (loop < MAX_EXECUTION_THREAD_SIZE)
    {
        pthread_create(&thread[loop], NULL, execute_thread, &execution[loop]);
        loop++;
    }
    
    if (initialization() != -1)
    {
        do{
            int idle_count = 0;
            loop = 0;
            while (loop < MAX_EXECUTION_THREAD_SIZE)
            {
                if (execution[loop].state == ES_DONE)
                {
                    if (written == read)
                    {
                        idle_count++;
                    }
                    else
                    {
                        execution[loop].executed = &queue[read];
                        execution[loop].state = ES_WAITING;
                        read = (read + 1) & (EXECUTION_QUEUE_SIZE-1);
#ifdef EXECUTE_DEBUG
                        read_actual++;
#endif
                    }
                }
                
                loop++;
            }
            
            if (idle_count == MAX_EXECUTION_THREAD_SIZE)
            {
                if (periodic_function)
                {
                    if (periodic_function() == -1)
                    {
                        execution_cycle = 0;
                    }
                }
            }
        }while (global_cycle && execution_cycle);
    }
}
