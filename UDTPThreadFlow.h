
#ifndef __UDTP_THREAD_SOCKET
#define __UDTP_THREAD_FLOW
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "UDTPThread.h"



class UDTPThreadFlow : public UDTPThread{ /*Handles socket thread of a peer*/
    public:
    UDTPThreadFlow();
    UDTPThreadFlow(UDTP* myUDTP, pthread_t threadHandler, unsigned int flowSocket, ThreadType flowThreadType){
        _flowThreadType = flowThreadType;
        _flowSocket = flowSocket;
        _myUDTP = myUDTP;
        _threadHandler = threadHandler;
        _alive = true;
    }

    bool set_flow_socket(unsigned int flowSocket){
        _flowSocket = flowSocket;
    }
    bool set_destination_address(sockaddr_in destinationAddress){
        _destinationAddress = destinationAddress;
    }

    unsigned int flow_socket() { return _flowSocket;};
    sockaddr_in get_destination_address() { return _destinationAddress;};

    private:
    ThreadType _flowThreadType;
    unsigned int _flowSocket; /*One to recvfrom() and send()*/
    sockaddr_in _destinationAddress; /*holds destination address for UDP*/
};


#endif
