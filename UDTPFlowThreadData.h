#ifndef __UDTP_FLOW_THREAD_DATA
#define __UDTP_FLOW_THREAD_DATA

#include <queue>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
class UDTPChunk;
class UDTPAddress;

class UDTP;
class UDTPFlowThreadData{
    public:
        UDTPFlowThreadData( pthread_t thread, unsigned int flowSocket, sockaddr_in socketAddress);
        ~UDTPFlowThreadData(){
            _alive = false;
            pthread_cancel(_thread);
        }
        bool set_approved(){ _approved = true;};
        bool check_approved(){ return _approved;};
        bool set_alive() { _alive = true; return true;};
        bool alive() { return _alive;};
        pthread_t get_thread();

        bool set_socket_address(sockaddr_in newAddress) { _socketAddress = newAddress;};
        sockaddr_in get_socket_address() { return _socketAddress;};

        bool set_destination_address(sockaddr_in newAddress) { _destinationAddress = newAddress;};
        sockaddr_in get_destination_address() { return _destinationAddress;};

        bool is_linked(){ return _linked; };
        bool set_linked() { _linked = true;};
        unsigned int get_flow_socket() { return _flowSocket;};
    private:
    bool _alive;
    bool _approved;
    bool _linked;
    pthread_t _thread;
    unsigned int _flowSocket;
    sockaddr_in _socketAddress;
    sockaddr_in _destinationAddress;
};


#endif
