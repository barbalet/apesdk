/****************************************************************
 
 httpd.c
 
 =============================================================
 
 Copyright 1996-2020 Tom Barbalet. All rights reserved.
 
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


#ifndef _WIN32
#ifndef SIMULATED_APE_CLIENT

#include "server.h"

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

//#include "../entity/entity.h"
//#include "../universe/universe_internal.h"
//#include "../universe/universe.h"

n_string method; // "GET" or "POST"
n_string uri;     // "/index.html" things before '?'
n_string qs;      // "a=1&b=2"     things after  '?'
n_string prot;    // "HTTP/1.1"

n_string payload; // for POST
n_int    payload_size;

n_int   exit_count = 0;

typedef struct { n_string name, value; } header_t;
static header_t reqhdr[17] = {{"\0", "\0"}};

#define MAX_CONNECTIONS 1000

static n_c_int listenfd, clients[MAX_CONNECTIONS];

static n_c_int clientfd;
static n_string buf;

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

void route(httpd_parameter_read parameter_read, httpd_output_json output_json) {
    parameter_read();
    if (strcmp("/", uri) == 0 && strcmp("GET", method) == 0) {
        printf("HTTP/1.1 200 OK\r\n");
        printf("Cache-Control: no-cache, no-store, must-revalidate, max-age=0\r\n");
        printf("Content-Type: application/json; charset=utf-8\r\n\r\n");
        {
            n_file * output = output_json();

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
void respond(n_int n, httpd_parameter_read parameter_read, httpd_output_json output_json)
{
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
        route(parameter_read, output_json);
        
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

void httpd_for_now(n_string port, httpd_start start, httpd_parameter_read parameter_read, httpd_output_json output_json)
{
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
    
    //while (1 == 1)
    while (exit_count < 5)
    {
        start();
        exit_count++;
        addrlen = sizeof(clientaddr);
        clients[slot] = accept(listenfd, (struct sockaddr *)&clientaddr, &addrlen);

        if (clients[slot] < 0) {
            perror("accept() error");
        } else {
            if (fork() == 0) {
                respond(slot, parameter_read, output_json);
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

#endif

