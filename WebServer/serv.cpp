#include <map>
#include <string>
#include <memory>

#include "content.h"
#include "serv.h"

void CServ::LoadResources(void)
{
    CContent* pContent;

    pContent = new CContent();
    pContent->SetFile("a2.html");
    pContent->SetContentType("text/html");
    _mapContent["/a2.html"] = std::shared_ptr<CContent>(pContent);

    pContent = new CContent();
    pContent->SetFile("mountain.jpg");
    pContent->SetContentType("image/jpg");
    _mapContent["/a2_files/mountain.jpg"] = std::shared_ptr<CContent>(pContent);

    pContent = new CContent();
    pContent->SetFile("dracula.html");
    pContent->SetContentType("text/html");
    _mapContent["/book/dracula.html"] = std::shared_ptr<CContent>(pContent);

    pContent = new CContent();
    pContent->SetFile("mountain.jpg");
    pContent->SetContentType("image/jpg");
    _mapContent["/a2/mountain.jpg"] = std::shared_ptr<CContent>(pContent);

    pContent = new CContent();
    pContent->SetFile("404.html");
    pContent->SetContentType("text/html");
    _mapContent["/404.html"] = std::shared_ptr<CContent>(pContent);
    
    pContent = new CContent();
    pContent->SetFile("501.html");
    pContent->SetContentType("text/html");
    _mapContent["/501.html"] = std::shared_ptr<CContent>(pContent);  

}

