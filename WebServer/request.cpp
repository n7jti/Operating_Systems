#include <string.h>

#include <iostream>
#include <memory>
#include <string>
#include <map>

#include "content.h"
#include "serv.h"
#include "csock.h"
#include "request.h"

CRequest::CRequest() :
    _state(REQUEST_READ),
    _writeHeaderPos(0),
    _writeContentPos(0),
    _pServ(nullptr)
{
}

CRequest::CRequest(CConnectionSocket *pConSock) :
    _spConSock(pConSock),
    _state(REQUEST_READ),
    _writeHeaderPos(0),
    _writeContentPos(0),
    _pServ(nullptr)
{
}


void CRequest::ParseRequest(){
    std::string::const_iterator stri = _spRequest->begin();

    // Parse the verb
    // verb is everything up to the first space. 
    while (stri != _spRequest->end()) {
        if (*stri == ' ') {
            stri++;
            break;
        }
        _strVerb += *stri;
        stri++;
    }

    // Parse the resource
    // resource is everyring up to the next space;
    while(stri != _spRequest->end()) {
        if(*stri == ' ') {
            stri++;
            break;
        }
        _strResource += *stri;
        stri++;
    }

    // Parse the protocol
    // protocal is everyring upto the /r/n
    while(stri != _spRequest->end()) {
        if(*stri == '\r') {
            stri++;
            break;
        }
        _strProtocol += *stri;
        stri++;
    }
}

void CRequest::BuildResponse(){
    // build the response
    std::map<std::string, std::shared_ptr<CContent> > ::const_iterator mIter;
    if (_strVerb != "GET") {
        // error 501
        mIter = _pServ->_mapContent.find("/501.html");
        _spContent = mIter->second;
        Build501Header();
        std::cout << "Bad Verb: " << _strVerb << std::endl;
    }
    else
    {   
        mIter =_pServ->_mapContent.find(_strResource);
        if (mIter == _pServ->_mapContent.end()) {
            //error 404
            mIter = _pServ->_mapContent.find("/404.html");
            _spContent = mIter->second;
            Build404Header();
            std::cout << "Not Found: " << _strResource << std::endl;
        }
        else
        {
            _spContent = mIter->second;
            Build200Header();
            std::cout << _strVerb << " " << _strResource << std::endl;
        }
    }
}

void CRequest::Build200Header()
{
    char ch[32];
    memset(ch,0,sizeof(ch));
    _reply.append("HTTP/1.1 200 OK\r\n");

    // Content length header
    _reply.append("Content-Length: ");
    sprintf(ch, "%d\r\n", _spContent->GetSize());
    _reply.append(ch);

    _reply.append("Content-Type:");
    _reply.append(_spContent->GetContentType());
    _reply.append("\r\n");
    _reply.append("\r\n");
}

void CRequest::Build404Header()
{
    char ch[32];
    memset(ch,0,sizeof(ch));
    _reply.append("HTTP/1.1 404 Not Found\r\n");

    // Content length header
    _reply.append("Content-Length: ");
    sprintf(ch, "%d\r\n", _spContent->GetSize());
    _reply.append(ch);

    _reply.append("Content-Type:");
    _reply.append(_spContent->GetContentType());
    _reply.append("\r\n");
    _reply.append("\r\n");
}

void CRequest::Build501Header()
{
    char ch[32];
    memset(ch,0,sizeof(ch));
    _reply.append("HTTP/1.1 501 Method Not Implemented\r\n");

    // Content length header
    _reply.append("Content-Length: ");
    sprintf(ch, "%d\r\n", _spContent->GetSize());
    _reply.append(ch);

    _reply.append("Content-Type:");
    _reply.append(_spContent->GetContentType());
    _reply.append("\r\n");
    _reply.append("\r\n");
}
