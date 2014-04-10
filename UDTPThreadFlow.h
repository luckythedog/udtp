
#ifndef __UDTP_THREAD_SOCKET
#define __UDTP_THREAD_FLOW
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "UDTPThread.h"



class UDTPThreadFlow : public UDTPThread{ /*Handles socket thread of a peer*/
    public:
    UDTPThreadFlow();
    UDTPThreadFlow(UDTP* myUDTP, pthread_t threadHandler, unsigned int flowSocket){
        _flowSocket = flowSocket;
        _myUDTP = myUDTP;
        _threadHandler = threadHandler;
        _alive = true;
    }

    bool set_flow_socket(unsigned int flowSocket){
        _flowSocket = flowSocket;
    }

    unsigned int flow_socket() { return _flowSocket;};
    bool set_peer(UDTPPeer* peer){
        _peer = peer;
    }
    UDTPPeer* peer(){
        return _peer;
    }
    private:
    UDTPPeer* _peer;
    ThreadType _flowThreadType;
    unsigned int _flowSocket; /*One to recvfrom() and send()*/

};


#endif
