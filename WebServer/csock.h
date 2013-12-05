///
//
// csock -- RAII sockets class
//
///
/// 

// RAII wrapper for sockets.  

#include <sys/socket.h> // socket definitions
#include <sys/types.h>  // socket types
#include <arpa/inet.h>  // inet (3) funcitons

#ifndef __CSOCK_H
#define __CSOCK_H


//
// class Connection Socket
// wraps a network socket connection

class CConnectionSocket{
public:
    CConnectionSocket(int conn_s, int flags = 0); // constructor for a connection socket wrapper.  
    ~CConnectionSocket();

    ssize_t Writeline(const void *vptr, size_t maxlen); // Write a line to the socket. Blocks until line is written
    std::string * ReadRequest(); // Reads from the socket until an entire request is read. Blocks

    ssize_t Read(void* vptr, size_t maxlen); //Does a single read up to maxlen chars.
    ssize_t Write(const void* vptr, size_t maxlen); // Does a single write up to maxlen chars

    int GetFd(); // get the wraped file descriptor

private:
    int _conn_s; // the wrapped file descriptor
    int _flags; // the construction flags. Used to make it non-blocking
};


//
// class CListenSocket
// a network listner socket

class CListenSocket{
public:
    CListenSocket(short int port, int flags = 0); // constructor.  Passes flags to the create sockets functions
    ~CListenSocket();  
    void Listen(void);  // make the listen call on the underlying socket. Also does the bind
    CConnectionSocket* Accept(void); // Call accept and return a wrapped network socket
    int GetFd(); // get the file descriptor for the socket

private:
    int _list_s; // listening socket
    short int _port; // port number
    int _flags; // flags passed to socket creation functions
    struct sockaddr_in _servaddr; // socket address structure
};

#endif
