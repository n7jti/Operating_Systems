///
//
// eserv.h
//
///

// Event based server

#ifndef _ESERV_H
#define _ESERV_H

// Class CEserv is an abstraction of an event based web server. 
class CEServ : public CServ
{
public:
    virtual void Run(int port); // run the web server

private:
    void ProcessRequest(CRequest* pRequest); // process a single request
    void OnRead(CRequest* pRequest);    // Process the read state
    void OnWriteHeaders(CRequest* pRequest);    // Process the write headers state
    void OnWriteContent(CRequest* pRequest);    // Process the Write Heaaders state
    void RebuildArray( int* pcapfd, struct pollfd *apfd); // Rebulld the poll array from internal data structures

    std::map<int, std::shared_ptr<CRequest> > _mapRequests; // map of file descriptors to CRequest objects

};


#endif //_ESERV_H
