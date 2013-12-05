///
//
//  Request.h
//
///

// Request is an abstraction of a web request. 

#ifndef _REQUEST_H
#define _REQUEST_H


// States for the Web request.  The request is used to keep the current state of the request
// while it is processed
typedef enum{   
    REQUEST_READ,           // Reading the request from the network
    REQUEST_WRITE_HEADERS,  // Writing out the response headers
    REQUEST_WRITE_CONTENT,  // Writing out the conent
    REQUEST_COMPLETE,       // The request is completed
    REQUEST_ERROR           // An error has occured
} REQUEST_STATE;

// A class that encapsulates the state for recieving and processing a request to a web server
class CRequest
{
public:
    CRequest();
    CRequest(CConnectionSocket* spConSock); // constructor that takes the socket associated with the request

    REQUEST_STATE _state;   // current state of the request
    std::unique_ptr<std::string> _spRequest;    // the string of the complete request from the web client
    std::unique_ptr<CConnectionSocket> _spConSock;  // the connection socket associated with the request
    CServ* _pServ;  // The server (base class anyway) associated with the request. 

    void ParseRequest(); // code to parse the interesting bits out of the request
    void BuildResponse(); // code to build the response to the request

    std::string _strVerb;   // The verb (for example GET, or POST)
    std::string _strResource;   // the requested resource
    std::string _strProtocol;   // the protocol of the request
    
    std::shared_ptr<CContent> _spContent; // a pointer to the content to be returned
    std::string _reply; // string containing the headers for the reply

    ssize_t _writeHeaderPos; // current position writing out the response headers
    ssize_t _writeContentPos; // current position writing out the content

    void Build200Header(); // build the 200 OK response headers
    void Build404Header(); // build the 404 Not Found headers
    void Build501Header(); // build the 501 headers
};

#endif //_REQUEST_H
