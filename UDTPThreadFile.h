
#ifndef __UDTP_THREAD_FILE
#define __UDTP_THREAD_FILE
#include <fstream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "UDTPThread.h"
class UDTPFile;
class UDTPThreadFile : public UDTPThread{
    public:
    UDTPThreadFile(pthread_t threadHandler, ThreadType threadType){
        _threadType = threadType;

        _threadHandler = threadHandler;
        _alive = true;
    }
        UDTPFile* file() { return _file;};

    private:
    UDTPFile* _file;

};

#endif
