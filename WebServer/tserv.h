//
//  tserv.h
//  Thread based web server

#ifndef _CTSERV_H
#define _CTSERV_H



class CTServ : public CServ
{
public:
    virtual void Run(int port); // start the server running

private:
    static void * ServiceConnection(void *ptr); // thread proc to service a sigle connection
};

#endif
