//
// Min program for the event-based web server
//

#include <unistd.h>     // misc Unix functions
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include <iostream>
#include <memory>
#include <map>

#include "content.h"
#include "serv.h"
#include "csock.h"
#include "request.h"
//#include "tserv.h"
#include "eserv.h"


// default port
#define PORT 8080



// protoypes of functions for main
void usage(void);
int parseCmd(int argc, char* argv[]);

int g_continue = 1;
void Sigint(int sig)
{
    g_continue = 0;
}

// main function
int main (int argc, char *argv[])
{
    // We expect write failures to occur but we want to handle them where 
    // the error occurs rather than in a SIGPIPE handler.
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, Sigint);


    int port = parseCmd(argc,argv);
    if (port == -1) {
        return 0;
    }
    
    // Event based server
    CEServ serv;
    serv.LoadResources();
    serv.Run(port);

    return(0);
}

// parse the command line
int parseCmd(int argc, char* argv[]){
    int port;
    if(argc == 1) {
        port = PORT;
    }
    else if (argc == 2) {
        port = atol(argv[1]);
        if (port == 0) {
            port = PORT;
        }
    }
    else{
        usage();
        return -1;
    }

    return port;
}


// print out usage
void usage(void){
    std::cout << "Usage:" << std::endl;
    std::cout << "    551server_threaded [port]" << std::endl;
    std::cout << std::endl;
    std::cout << "port - if not specified, the default port is 8080." << std::endl;
}


