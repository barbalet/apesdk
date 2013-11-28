//
//  main.c
//  thread_maximization
//
//  Created by Thomas Barbalet on 11/27/13.
//  Copyright (c) 2013 Thomas Barbalet. All rights reserved.
//

#include <stdio.h>
#include <pthread.h>

#define MAX_EXECUTION_THREAD_SIZE 7
#define EXECUTION_QUEUE_SIZE 512

typedef int (execution_function)(void * read_data, void * write_data);

typedef enum
{
    ES_DONE = 0,        // internally written
    ES_WAITING,         // externally written
    ES_STARTED,         // internally written
}execute_state;

typedef struct
{
    execution_function * function;
    void               * read_data;
    void               * write_data;
}execute_object;


typedef struct
{
    execute_object * executed;
    execute_state  state;
    
}execution_thread;

static int global_cycle = 1;

static pthread_t         thread[MAX_EXECUTION_THREAD_SIZE] = {0L};
static execution_thread  execution[MAX_EXECUTION_THREAD_SIZE] = {0L};
static execute_object    queue[EXECUTION_QUEUE_SIZE] = {0L};

static int               written = 0, read = 0;
static int               written_actual = 0, read_actual = 0;

static int               conclude_count = 0;
static int               print_count = 0;

static int               greatest_delta = 0;

static void execute_add(execution_function * function, void * read_data, void * write_data)
{
    int delta = (written_actual - read_actual);
    
    queue[written].function = function;
    queue[written].read_data = read_data;
    queue[written].write_data = write_data;
    
    written = (written + 1) & (EXECUTION_QUEUE_SIZE-1);
    written_actual++;
    
    if (delta > greatest_delta)
    {
        greatest_delta = delta;
    }
    
    if ((print_count & 1023) == 0)
    {
        printf("delta %d\n", greatest_delta);
        greatest_delta = 0;
    }
    print_count++;
}

static void * execute_thread(void * id)
{
    execution_thread * value = id;
    
    do{
        if (value->state == ES_WAITING)
        {
            execute_object * object = value->executed;
            value->state = ES_STARTED;
            if (object->function(object->read_data, object->write_data) == -1)
            {
                global_cycle = 0;
            }
            value->state = ES_DONE;
        }
    }while (global_cycle);
    pthread_exit(NULL);
}


int calculate_c(void * read_data, void * write_data)
{
    int * read_int = read_data;
    int * write_int = write_data;
    
    write_int[0] ^= read_int[0];
    return 0;
}


int calculate_b(void * read_data, void * write_data)
{
    int * read_int = read_data;
    int * write_int = write_data;
    int loop = 0;
    
    while (loop < 20500)
    {
        int location = loop % 5;
        write_int[location] += read_int[location];
        loop++;
    }
    
    execute_add(calculate_c, &read_int[1], &write_int[0]);
    execute_add(calculate_c, &read_int[2], &write_int[1]);
    execute_add(calculate_c, &read_int[3], &write_int[2]);
    execute_add(calculate_c, &read_int[4], &write_int[3]);
    return 0;
}


int calculate_a(void * read_data, void * write_data)
{
    int * read_int = read_data;
    int * write_int = write_data;
    int loop = 0;
    
    while (loop < 20500)
    {
        int location = loop % 20;
        write_int[location] += read_int[location];
        loop++;
    }
    
    execute_add(calculate_b, &read_int[5], &write_int[0]);
    execute_add(calculate_b, &read_int[10], &write_int[5]);
    execute_add(calculate_b, &read_int[15], &write_int[10]);
    execute_add(calculate_b, &read_int[0], &write_int[15]);

    
    conclude_count++;
    
    if (conclude_count == 4000)
        return -1;
        
    return 0;
}

static void start_cycle_again()
{
    int     read_data[20] = {1,2,3,4,5,6,7,8,9,10,11,12}, write_data[20] = {0};
    
    //printf("cycle again\n");
    
    execute_add(calculate_a, read_data, write_data);
}



int main(int argc, const char * argv[])
{
    int loop = 0;
    
    while (loop < MAX_EXECUTION_THREAD_SIZE)
    {
        pthread_create(&thread[loop], NULL, execute_thread, &execution[loop]);
        loop++;
    }

    start_cycle_again();

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
                    
                    read_actual++;
                }
            }
            
            loop++;
        }
        
        if (idle_count == MAX_EXECUTION_THREAD_SIZE)
        {
            start_cycle_again();
        }
        
    }while (global_cycle);
    
    return 0;
}

