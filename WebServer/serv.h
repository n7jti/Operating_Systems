//
// serv.h  
// base class for both servers 
//

#ifndef _SERV_H
#define _SERV_H



class CServ
{
public:
    void LoadResources(); // load the content served up by the web servers
    virtual void Run(int port) = 0; // pure virtual function that runs the servers
    std::map<std::string, std::shared_ptr<CContent> > _mapContent; // map from resource string to the CContent
};

#endif
