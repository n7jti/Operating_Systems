///
//
// Content.h
//
///

// CContent is a wrapper class to hold the 
// individual web pages or files that are 
// to be served up by the web server. 

#ifndef _CONTENT_H
#define _CONTENT_H

class CContent{
public:
    CContent(); // constructor
    void SetFile(const char *spFileName); // provide the file name for the content
    void SetContentType(const std::string& strContentType);  // Set the content type for the file
    const std::string& GetContentType() const; // Get the content type for the file
    long GetSize() const; // get the size of the file in bytes
    const unsigned char* GetData() const; // get a pointer to the data

    
private:
    void ReadData(int f, void *vptr, size_t maxlen); // read the data in from the file

    off_t _size;                                // size of the content in bytes
    std::unique_ptr<unsigned char[]> _spData;   // pointer to the content
    std::string _strContentType;                // the media type of the content.  For example text/html
};



#endif
