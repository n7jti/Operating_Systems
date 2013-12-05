//
// csock.cpp
//

// impementation file for network sockets

#include <unistd.h>     // misc Unix functions
#include <errno.h>
#include <string.h>
#include <stdio.h>

#include <iostream>
#include <string>

#include "csock.h"

// Constructor for a network socket
CConnectionSocket::CConnectionSocket(int conn_s, int flags) :
    _conn_s(conn_s),
    _flags(flags)
{

}


// desructor, note it automatically closes the socket
CConnectionSocket::~CConnectionSocket(){
    if (_conn_s > 0) {
        close(_conn_s);
        _conn_s = 0;
    }
}

// read the web request from the socket. Keeps reading a character at a time
// until the first blank line is read in.  Used with blocking sockets
std::string * CConnectionSocket::ReadRequest(){
    std::string *pStr = new std::string();
    ssize_t rc;
    char ch[2];

    // termination sequence. Two <cr><lf> in a row;
    std::string term("\r\n\r\n");

    //Initialize ch to 0;
    memset(ch, 0, sizeof(ch));

    // loop infinitely until all characters are read or we error out
    while(1) {
        if((rc=read(_conn_s, ch, 1))==1) {
            pStr->append(ch);

            // if the last four characters are crlf then we're done
            if (pStr->length() >= 4) {
               if (std::string::npos != pStr->rfind(term.c_str(), pStr->length(), 4)) {
                   break;
               }
            }

        }
        else if(rc ==0) {
            break;
        }
        else{
            if(errno == EINTR) {
                continue;
            }
            throw "ERROR: read failed";
        }
    }

    // uncomment to write out the request for debugging purposes
    //std::cout << pStr->c_str() << std::endl;

    return pStr;
}

// read in up to maxline characers from the socket. Use with non-blocking sockets
ssize_t CConnectionSocket::Read(void* buffer, size_t maxlen){
    ssize_t rc = read(_conn_s, buffer, maxlen);
    if(rc < 0) {
        if(errno == EINTR) {
            rc = 0;
        }
        else {
            throw "ERROR: read failed";
        }
    }
    return rc;
}

//write out maxlen characers.  Use with blocking sockets
ssize_t CConnectionSocket::Writeline(const void *vptr, size_t maxlen){
    size_t nleft;
    ssize_t nwritten;
    const char *buffer;

    buffer = reinterpret_cast<const char*>(vptr);
    nleft = maxlen;

    while (nleft > 0) {
        if((nwritten = write(_conn_s, buffer, nleft))<=0) {
            if(errno == EINTR) {
                nwritten = 0;
            }
            else{
                throw "ERROR: write failed";
            }
        }
        nleft -= nwritten;
        buffer += nwritten;
    }

    return maxlen;

}

// write out up to maxlen characters.  Use with non-blocking sockets
ssize_t CConnectionSocket::Write(const void *vptr, size_t maxlen){
    ssize_t rc = write(_conn_s, vptr, maxlen);
    if (rc < 0) {
        if((errno == EINTR) || 
           (errno == EAGAIN)) {
            rc = 0;
        }
        else{
            throw "ERROR: write failed";
        }
    }
    return rc;
}

int CConnectionSocket::GetFd(){
    return _conn_s;
}


// Constructor for the listener socket
CListenSocket::CListenSocket(short int port, int flags) :
    _list_s(0),
    _port(port),
    _flags(flags)
{
    memset(&_servaddr, 0, sizeof(_servaddr));
    _servaddr.sin_family = AF_INET;
    _servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    _servaddr.sin_port = htons(_port);
}


// destructor for the listener socket.  Note that the destructor closes the socket
CListenSocket::~CListenSocket(){
    if (_list_s > 0) {
        close(_list_s);
        _list_s = 0;
    }
}

// Bind and listen
void CListenSocket::Listen(){
    int ret = 0;
    const int LISTENQ = 127;

    _list_s = socket(AF_INET, SOCK_STREAM | _flags, 0);
    if (_list_s == -1) {
        throw "ERROR: Could not create listening socket";
    }
    
    ret  = ::bind(_list_s, reinterpret_cast<const sockaddr*>(&_servaddr), sizeof(_servaddr));
    if (ret) {
        throw "ERROR: Failed to bind";
    }

    ret = listen(_list_s, LISTENQ);
    if (ret < 0) {
        throw "ERROR: Could not listen.";
    }

}

// accept the connection and return a connection socket
CConnectionSocket* CListenSocket::Accept(void)
{
    int conn_s;
    CConnectionSocket *pConnectionSocket = nullptr;
    conn_s = accept4(_list_s, NULL, NULL, _flags);
    if(conn_s < 0) {
        if (_flags & SOCK_NONBLOCK) {
            // no connections waitings
        }
        else
        {
            throw "ERROR: accept failed";
        }
    }
    else
    {
        pConnectionSocket = new CConnectionSocket(conn_s);
    }
    return(pConnectionSocket);
}


// get the file descriptor of the connection socket
int CListenSocket::GetFd(void){
    return _list_s;
}



