/****************************************************************
 
 httpd.c
 
 =============================================================
 
 Copyright 1996-2019 Tom Barbalet. All rights reserved.
 
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


#ifndef _WIN32

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>


#include <time.h>
#include <sys/time.h>

#include "../entity/entity.h"


#include "universe_internal.h"
#include "universe.h"

n_string method; // "GET" or "POST"
n_string uri;     // "/index.html" things before '?'
n_string qs;      // "a=1&b=2"     things after  '?'
n_string prot;    // "HTTP/1.1"

n_string payload; // for POST
n_int    payload_size;

typedef struct { n_string name, value; } header_t;
static header_t reqhdr[17] = {{"\0", "\0"}};

n_int   hpv_exit, hpv_dt;
n_int   hpv_x, hpv_y, hpv_f, hpv_i;
n_int   hpv_r0, hpv_r1;

n_int   exit_count = 0;

#define MAX_CONNECTIONS 1000

static n_c_int listenfd, clients[MAX_CONNECTIONS];

static n_c_int clientfd;
static n_string buf;

n_int http_get_exit(void)
{
    return hpv_exit;
}

// get request header by name
n_string request_header(const  n_string name) {
    header_t *h = reqhdr;
    while (h->name) {
        if (strcmp(h->name, name) == 0)
            return h->value;
        h++;
    }
    return NULL;
}

static n_int httpd_number(n_string number)
{
    n_int   temp = 0;
    n_int   ten_power_place = 0;
    n_byte  character;
    
    if (ASCII_NUMBER(number[ten_power_place]) == 0)
    {
        return -4;
    }
    
    do{
        character = number[ten_power_place++];
        if (ASCII_NUMBER(character))
        {
            n_int mod_ten = character - '0';
            if ((temp == 429496729) && (mod_ten > 5))
            {
                return -5;
            }
            if (temp > 429496729)
            {
                return -6;
            }
            temp = (temp * 10) + mod_ten;
        }
    }while(ASCII_NUMBER(character));
    
    return temp;
}

n_int httpd_qs(n_string compare)
{
    n_int compare_len = io_length(compare, 1024);
    n_int qs_len = io_length(qs, 1024);
    n_int count = io_find(qs, 0, qs_len, compare, compare_len);
    
    if (qs[count] != '=')
    {
        return -2;
    }
    if (count == -1)
    {
        return -1;
    }
    return httpd_number(&qs[count + 1]);
}

// get all request headers
header_t *request_headers(void) {
    return reqhdr;
}

void httpd_parameters(void)
{
    hpv_exit = httpd_qs("exit");
    hpv_dt = httpd_qs("dt");
    hpv_x = httpd_qs("x");
    hpv_y = httpd_qs("y");
    hpv_f = httpd_qs("f");
    hpv_i = httpd_qs("i");
    hpv_r0 = httpd_qs("r0");
    hpv_r1 = httpd_qs("r1");
}

typedef struct {
    n_int signature, version_number;
    n_string copyright, date;
} client_simulation_identifier;

typedef struct {
    n_vect2 location;
    n_int direction_facing, velocity, stored_energy, id;
    n_string name;
} client_being;

typedef struct {
    n_vect2 genetics;
    n_int date, time;
} client_land;

typedef struct {
    client_land land;
    client_simulation_identifier id;
    client_being * beings;
    n_int  being_count;
    n_int  being_max;
    n_uint being_selected_id;
} client_data;

void client_from_file(client_data * data, n_file * file_json)
{
    io_whitespace_json(file_json);
    {
        n_object * returned_object = object_file_to_tree(file_json);
        if (returned_object)
        {
            n_string str_information = obj_contains(returned_object, "information", OBJECT_OBJECT);
            n_string str_land = obj_contains(returned_object, "land", OBJECT_OBJECT);
            n_string str_beings = obj_contains(returned_object, "beings", OBJECT_ARRAY);
            n_int    value = 0;
            if (str_information)
            {
                n_object * information_obj = obj_get_object(str_information);
                
                n_string potential = obj_contains(information_obj, "copyright", OBJECT_STRING);
                
                if (potential)
                {
                    data->id.copyright = potential;
                }
                potential = obj_contains(information_obj, "date", OBJECT_STRING);
                
                if (potential)
                {
                    data->id.date = potential;
                }
                
                if (obj_contains_number(information_obj, "signature", &value))
                {
                    data->id.signature = value;
                }
                if (obj_contains_number(information_obj, "version_number", &value))
                {
                    data->id.version_number = value;
                }
            }
            if (str_land)
            {
                n_object * land_obj = obj_get_object(str_land);
                n_string str_genetics= obj_contains(land_obj, "genetics", OBJECT_ARRAY);
                if (str_genetics)
                {
                    n_array * array_obj = obj_get_array(str_genetics);
                    n_array * arr_second_follow = 0L;
                    n_int     count = 0;
                    while ((arr_second_follow = obj_array_next(array_obj, arr_second_follow)))
                    {
                        n_int second_follow_number = obj_get_number(arr_second_follow->data);
                        if (count >= 2)
                        {
                            break;
                        }
                        data->land.genetics.data[count++] = second_follow_number;
                    }
                }
                if (obj_contains_number(land_obj, "date", &value))
                {
                    data->land.date = value;
                }
                if (obj_contains_number(land_obj, "time", &value))
                {
                    data->land.time = value;
                }
            }
            if (str_beings)
            {
                n_array * beings_array = obj_get_array(str_beings);
                n_int     being_count = obj_array_count(beings_array);
                
                if (being_count > data->being_max)
                {
                    if (data->beings)
                    {
                        memory_free((void**)&data->beings);
                    }
                    data->beings = (client_being*) memory_new((sizeof(client_being) * being_count));
                    data->being_max = being_count;
                }
                
                
            }
        }
    }
}

static n_object * httpd_land_obj(void)
{
    n_object * noble_land = obj_number(0L, "date", land_date());
    n_byte2  * genetics_values = land_genetics();
    n_array  * land_genetics = array_number(genetics_values[0]);
    array_add(land_genetics, array_number(genetics_values[1]));
    
    obj_array(noble_land, "genetics", land_genetics);
    obj_number(noble_land, "time", land_time());
    return noble_land;
}


static n_object * httpd_being_obj(noble_being * being)
{
    n_object * noble_being = 0L;
    n_string_block simple_name;
    being_name_simple(being, simple_name);
    noble_being = obj_string(0L, "name", simple_name);
    
    {
        noble_being_delta * delta = &being->delta;
        noble_being_constant *constant =&being->constant;
        n_genetics * genetics = constant->genetics;
        n_array  * location = array_number(delta->location[0]);
        n_array  * genetics_array = array_number(genetics[0]);
        
        array_add(genetics_array, array_number(genetics[1]));
        array_add(genetics_array, array_number(genetics[2]));
        array_add(genetics_array, array_number(genetics[3]));
        
        array_add(location, array_number(delta->location[1]));
        obj_number(noble_being, "direction_facing", delta->direction_facing);
        obj_array(noble_being,  "location", location);
        obj_number(noble_being, "velocity", delta->velocity);
        obj_number(noble_being, "stored_energy", delta->stored_energy);
        obj_array(noble_being, "genetics", genetics_array);
        obj_number(noble_being, "id", genetics[0]^genetics[1]^genetics[2]^genetics[3]);
    }
    return noble_being;
}


static n_object * httpd_sim_obj(void)
{
    n_object * noble_sim_identifier = obj_number(0L, "signature", NOBLE_APE_SIGNATURE);
    obj_number(noble_sim_identifier, "version number", VERSION_NUMBER);
    obj_string(noble_sim_identifier, "copyright", FULL_VERSION_COPYRIGHT);
    obj_string(noble_sim_identifier, "date", FULL_DATE);
    return noble_sim_identifier;
}

static n_file * httpd_out_json(void)
{
    n_file *output_file = 0L;
    noble_simulation *local_sim = sim_sim();
    
    n_object * simulation_object = obj_object(0L, "information", httpd_sim_obj());
    obj_object(simulation_object, "land", httpd_land_obj());
    obj_number(simulation_object, "exit", exit_count);
    
    if (local_sim->num > 0)
    {
        n_uint        count = 1;
        noble_being *local_beings = local_sim->beings;
        n_object    *being_object = httpd_being_obj(&(local_beings[0]));
        n_array     *beings = array_object(being_object);
        while (count < local_sim->num)
        {
            being_object = httpd_being_obj(&(local_beings[count++]));
            array_add(beings, array_object(being_object));
        }
        obj_array(simulation_object, "beings", beings);
    }
    
    output_file = obj_json(simulation_object);
    
    /*obj_free(&simulation_object);*/
    
    return output_file;
}

static n_int run_once = 0;

static void http_sim_start(void)
{
    if (run_once == 0)
    {
        srand((unsigned int) time(NULL) );
        sim_init(KIND_START_UP, rand(), MAP_AREA, 0);
    }
    else
    {
        sim_cycle();
    }
    run_once = 1;
}

void route() {
    httpd_parameters();
    if (strcmp("/", uri) == 0 && strcmp("GET", method) == 0) {
        printf("HTTP/1.1 200 OK\r\n");
        printf("Cache-Control: no-cache, no-store, must-revalidate, max-age=0\r\n");
        printf("Content-Type: application/json; charset=utf-8\r\n\r\n");
        {
            n_file * output = httpd_out_json();

            io_file_debug(output);
            /*io_file_free(&output);*/
        }
        printf("\r\n");
        
        
    } else if (strcmp("/", uri) == 0 && strcmp("POST", method) == 0) {
        printf("HTTP/1.1 200 OK\r\n\r\n");
        printf("Wow, seems that you POSTed %ld bytes. \r\n", payload_size);
        printf("Fetch the data using `payload` variable.");
    } else printf("HTTP/1.1 500 Internal Server Error\n\nThe server has no handler to the request.\n");
}


// client connection
void respond(n_int n) {
    n_int    rcvd;

    buf = memory_new(65535);
    rcvd = recv(clients[n], buf, 65535, 0);

    if (rcvd < 0) // receive error
        fprintf(stderr, ("recv() error\n"));
    else if (rcvd == 0) // receive socket closed
        fprintf(stderr, "Client disconnected upexpectedly.\n");
    else // message received
    {
        buf[rcvd] = '\0';
        
        method = strtok(buf, " \t\r\n");
        uri = strtok(NULL, " \t");
        prot = strtok(NULL, " \t\r\n");
        
        fprintf(stderr, "\x1b[32m + [%s] %s\x1b[0m\n", method, uri);
        
        qs = strchr(uri, '?');
        
        if (qs) {
            *qs++ = '\0'; // split URI
        } else {
            qs = uri - 1; // use an empty string
        }
        
        header_t *h = reqhdr;
        n_string t = NULL, t2;
        while (h < reqhdr + 16) {
            n_string  k, v;
            
            k = strtok(NULL, "\r\n: \t");
            if (!k)
                break;
            
            v = strtok(NULL, "\r\n");
            while (*v && *v == ' ')
                v++;
            
            h->name = k;
            h->value = v;
            h++;
            fprintf(stderr, "[H] %s: %s\n", k, v);
            t = v + 1 + strlen(v);
            if (t[1] == '\r' && t[2] == '\n')
                break;
        }
        t++; // now the *t shall be the beginning of user payload
        t2 = request_header("Content-Length"); // and the related header if there is
        payload = t;
        payload_size = t2 ? atol(t2) : (rcvd - (t - buf));
        
        // bind clientfd to stdout, making it easier to write
        clientfd = clients[n];
        dup2(clientfd, STDOUT_FILENO);
        close(clientfd);
        
        // call router
        route();
        
        // tidy up
        fflush(stdout);
        shutdown(STDOUT_FILENO, SHUT_WR);
        close(STDOUT_FILENO);
    }
    
    // Closing SOCKET
    shutdown(clientfd, SHUT_RDWR); // All further send and recieve operations are DISABLED...
    close(clientfd);
    clients[n] = -1;
}


// start server
void startServer(n_string port) {
    struct addrinfo hints, *res, *p;
    
    // getaddrinfo for host
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if (getaddrinfo(NULL, port, &hints, &res) != 0) {
        perror("getaddrinfo() error");
        exit(1);
    }
    // socket and bind
    for (p = res; p != NULL; p = p->ai_next) {
        n_int option = 1;
        listenfd = socket(p->ai_family, p->ai_socktype, 0);
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
        if (listenfd == -1)
            continue;
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
            break;
    }
    if (p == NULL) {
        perror("socket() or bind()");
        exit(1);
    }
    
    freeaddrinfo(res);
    
    // listen for incoming connections
    if (listen(listenfd, 1000000) != 0) {
        perror("listen() error");
        exit(1);
    }
}

void httpd_for_now(n_string port) {
    struct sockaddr_in clientaddr;
    socklen_t addrlen;
    n_int slot = 0;
    
    printf("Server started http://127.0.0.1:%s\n", port);

    // Setting all elements to -1: signifies there is no client connected
    n_int i;
    for (i = 0; i < MAX_CONNECTIONS; i++)
        clients[i] = -1;
    startServer(port);

    // Ignore SIGCHLD to avoid zombie threads
    signal(SIGCHLD, SIG_IGN);

    // ACCEPT connections
    while (exit_count < 10) {
        http_sim_start();

        exit_count++;
        addrlen = sizeof(clientaddr);
        clients[slot] = accept(listenfd, (struct sockaddr *)&clientaddr, &addrlen);

        if (clients[slot] < 0) {
            perror("accept() error");
        } else {
            if (fork() == 0) {
                respond(slot);
                exit(0);
            }
        }
        while (clients[slot] != -1)
        slot = (slot + 1) % MAX_CONNECTIONS;
    }
}

#if 0
int main(int c, char **v) {
    httpd_for_now("8139");
    return 0;
}
#endif

#endif