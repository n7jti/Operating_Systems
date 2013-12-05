//
// event base server
//

#include <signal.h>
#include <poll.h>
#include <string.h>

#include <iostream>
#include <map>
#include <string>
#include <memory>

#include "csock.h"
#include "content.h"
#include "serv.h"
#include "request.h"
#include "eserv.h"

extern int g_continue;

using namespace std;
#define MAXREQUESTS 128

void CEServ::Run(int port)
{
    CListenSocket lsock(port, SOCK_NONBLOCK);
    lsock.Listen();
    
    struct pollfd apfd[MAXREQUESTS];
    int capfd = 1;
    memset(apfd, 0, sizeof(apfd));

    apfd[0].fd = lsock.GetFd();
    apfd[0].events = POLLIN;

    while(g_continue) {
        // Wait for I/O to be ready
        int cevents = poll(apfd,capfd,-1);

        // now cevents tells us how many events there are
        while (cevents > 0) {
            
            // check the listening socket first, but only if we've got more room in our array
            if (apfd[0].revents) {
                if (cevents < MAXREQUESTS) {
                    // There is a connection!
                    CConnectionSocket* pConSock;
                    pConSock = lsock.Accept();
                    if (pConSock != nullptr) {
                        _mapRequests[pConSock->GetFd()] = std::shared_ptr<CRequest>(new CRequest(pConSock));
                        pConSock = nullptr; 
                    }
                }
                cevents--;
            }
            
            // now check the rest
            for(int i = 1; ((i < capfd) && (cevents > 0)); i++) {
                
                // check if we have events recieved for the file descriptor
                if (apfd[i].revents) {

                    // we found one, decrement the count of events.  This allows for early termination of the loop
                    cevents--;

                    // look up the file descriptor in the map
                     std::map<int, std::shared_ptr<CRequest> >::const_iterator mIter;
                     mIter = _mapRequests.find(apfd[i].fd);

                     // if we found the reqeust for the file descriptor
                     if(mIter != _mapRequests.end()) {
                         std::shared_ptr<CRequest> spRequest(mIter->second);
                         spRequest->_pServ = this;
                         try{
                             ProcessRequest(spRequest.get());
                         }catch (const char * e){
                             //std::cout << "Error Encountered" << std::endl << e << std::endl;
                             spRequest->_state = REQUEST_ERROR;
                         }

                         // if we're done with the request, remove it from the map
                         if ((spRequest->_state == REQUEST_ERROR) ||
                             (spRequest->_state == REQUEST_COMPLETE)) {

                             // remove it from the map. 
                             _mapRequests.erase(mIter);
                         }
                     }
                }
            }
        }
        // now rebuild the array and go around again
        RebuildArray(&capfd, apfd);
    }
}

void CEServ::ProcessRequest(CRequest* pRequest)
{
    // dispatch events
    switch (pRequest->_state) {
    case REQUEST_READ:
        OnRead(pRequest);
        break;
    case REQUEST_WRITE_HEADERS:
        OnWriteHeaders(pRequest);
        break;
    case REQUEST_WRITE_CONTENT:
        OnWriteContent(pRequest);
        break;
    default:
        throw "ERROR: Bad request state";
        break;
    }
}

void CEServ::OnRead(CRequest* pRequest){
    ssize_t rc;
    char buffer[1024];

    // termination sequence. Two <cr><lf> in a row;
    char term[] = "\r\n\r\n";
    rc = pRequest->_spConSock->Read(buffer, sizeof(buffer));
    if(rc > 0) {
        if (pRequest->_spRequest == nullptr) {
            pRequest->_spRequest.reset(new std::string);
        }
        // we read something, add it to our internal buffer
        pRequest->_spRequest->append(buffer, rc);

        // if we'eve got a blank line in there, we're done!
        if (std::string::npos != pRequest->_spRequest->find(term)) {
            pRequest->ParseRequest();
            pRequest->BuildResponse();

            // updates our state
            pRequest->_state = REQUEST_WRITE_HEADERS;
        }
    }
}

void CEServ::OnWriteHeaders(CRequest* pRequest){
    ssize_t rc = pRequest->_spConSock->Write(pRequest->_reply.c_str() + pRequest->_writeHeaderPos, pRequest->_reply.size() - pRequest->_writeHeaderPos);
    pRequest->_writeHeaderPos += rc;
    if (pRequest->_writeHeaderPos >= pRequest->_reply.size()) {
        pRequest->_state = REQUEST_WRITE_CONTENT;
    }

}


void CEServ::OnWriteContent(CRequest* pRequest){
    ssize_t rc = pRequest->_spConSock->Write(pRequest->_spContent->GetData() + pRequest->_writeContentPos, pRequest->_spContent->GetSize() - pRequest->_writeContentPos);
    pRequest->_writeContentPos += rc;
    if (pRequest->_writeContentPos >= pRequest->_spContent->GetSize()) {
        pRequest->_state = REQUEST_COMPLETE;
    }

}

void CEServ::RebuildArray(int* pcapfd, struct pollfd *apfd)
{
    apfd[0].revents = 0;

    // look up the file descriptor in the map
    std::map<int, std::shared_ptr<CRequest> >::const_iterator mIter;
    mIter = _mapRequests.begin();
    int cRequests = 1;
    while (mIter != _mapRequests.end()) {
        apfd[cRequests].fd = mIter->second->_spConSock->GetFd();
        apfd[cRequests].revents = 0;
        if (mIter->second->_state == REQUEST_READ) {
            apfd[cRequests].events = POLLIN;
        }
        else {
            apfd[cRequests].events = POLLOUT;
        }
        mIter++;
        cRequests++;
    }
    *pcapfd = cRequests; 
}
