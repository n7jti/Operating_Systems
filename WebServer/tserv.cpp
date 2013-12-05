//
// tserv.cpp
//

// implementation file for thread-based server
#include <poll.h>
#include <pthread.h>
#include <map>
#include <iostream>
#include <string>
#include <memory>

#include "csock.h"
#include "content.h"
#include "serv.h"
#include "tserv.h"
#include "request.h"

extern int g_continue;

void CTServ::Run(int port)
{
    CListenSocket lsock(port);
    lsock.Listen();

    while(g_continue) {
        pthread_t thread;
        CRequest* pRequest;
        CConnectionSocket* pConSock;

        struct pollfd pfd;
        pfd.events = POLLIN;
        pfd.fd = lsock.GetFd();
        pfd.revents = 0;
       
        int rc;
        rc = poll(&pfd, 1, 250);
        if (rc > 0) {
            pConSock = lsock.Accept();

            pRequest = new CRequest();
            pRequest->_spConSock.reset(pConSock);
            pRequest->_pServ = this;

            pthread_create(&thread, NULL, CTServ::ServiceConnection, pRequest);
            pthread_detach(thread);
        }
    }

}


void * CTServ::ServiceConnection(void *ptr)
{
    std::unique_ptr<CRequest> spRequest(reinterpret_cast<CRequest*>(ptr));

    try{
        
        spRequest->_spRequest.reset(spRequest->_spConSock->ReadRequest());
        spRequest->ParseRequest();
        spRequest->BuildResponse();

        spRequest->_spConSock->Writeline(spRequest->_reply.c_str(), spRequest->_reply.size());
        //std::cout << spRequest->_reply << std::endl;
        spRequest->_spConSock->Writeline(spRequest->_spContent->GetData() , spRequest->_spContent->GetSize());

        //std::cout << spRequest->_reply << std::endl;
    }catch (const char * e){
        //catch the exception and rock on!
        std::cout << e << std::endl;
    }

    spRequest = nullptr;
    pthread_exit(nullptr);
}


