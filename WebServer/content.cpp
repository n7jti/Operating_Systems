//
// content.cpp
// implementation file to wrap the in-memory cache for the content files
//

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <memory>
#include <string>

#include "content.h"

CContent::CContent() :
    _size(0),
    _spData(nullptr)
{
   
}

// Read the file from disk and place in memory
void CContent::SetFile(const char *spFileName)
{
    int ret = 0;
    struct stat sb; 
    // get the size of the file

    ret = stat(spFileName, &sb);
    if (ret == -1) {
        throw "ERROR: failed to open file";
    }

    _size = sb.st_size;

    // allocate the memory
    _spData.reset(new unsigned char[_size]);

    // read it in
    int f = open(spFileName, O_RDONLY);
    if (f == -1) {
        throw "ERROR: Could Not Open File";
    }

    ReadData(f, _spData.get(), _size);

    // close the file
    close(f);
    f = -1;
    return;
}

// Set the content type for the file
void CContent::SetContentType(const std::string& strContentType){
    _strContentType = strContentType;
}

// Get the content type of the file
const std::string& CContent::GetContentType(void) const {
    return _strContentType;
}

// Read the data from an open file descripor to the provided pointer
void CContent::ReadData(int f, void *vptr, size_t maxlen){
    ssize_t n, rc;
    char c, *buffer;

    buffer = reinterpret_cast<char*>(vptr);

    n = 0;
    do{
        // read data from the file
        rc = read(f, buffer + n, maxlen - n);
        if (rc > 0) {
            n += rc;
        }
        else if (rc = 0) {
            // EOF!
            break;
        }
        else{
            //ERROR
             if(errno == EINTR) {
                continue;
            }
            throw "ERROR: read failed";
        }

    } while(n < maxlen);
   
    return;
}

// Get the size of the content in bytes
long CContent::GetSize() const{
    return _size;
}

// Get a pointer to the contents
const unsigned char* CContent::GetData() const{
    return _spData.get();
}


