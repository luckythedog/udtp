#ifndef __UDTP_SOCKET
#define __UDTP_SOCKET
#include <vector>
#include <queue>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
class UDTPThreadFlow;
class UDTPAddress;
class UDTPFile;

enum initProcessEvent{
    LISTEN_SOCKET = 0x00, /*Has Listen socket started*/
    LISTEN_THREAD = 0x01, /*Has listen thread started*/
    MUTEX_FRAMEWORK_INIT = 0x02,
    MUTEX_SELF_INIT = 0x03,
    QUEUE_THREADS = 0x04,
    COMPLETE  = 0x05 /*Completion only verified by themselves*/
};

class UDTPPeer{ /*The way we can identify different UDP ports is that, we can identify that they have the same TCP socket. the TCP socket never changes!*/
   public:
    UDTPPeer();
    UDTPPeer(unsigned int listenSocket) {
        _listenSocket = listenSocket;
        _initProcess = 0x00;
        init_mutex();
        };/*starts off TCP.*/
    ~UDTPPeer();

    bool compare_listen_socket(unsigned int listenSocket){ if(_listenSocket== listenSocket) return true;else;return false;}

    bool set_listen_socket(unsigned int listenSocket) { _listenSocket = listenSocket; return true;};
    unsigned int get_listen_socket(){ return _listenSocket; };


    bool set_init_process_complete(initProcessEvent eventCompleted) {
            _initProcess |=  (0x01<<eventCompleted);
            return true;
    };
    bool check_init_process(initProcessEvent eventCompleted) {
        if(!(_initProcess & (0x01<<eventCompleted))) return false; /*Has not been completed*/
        return true;
    };
    bool set_online(){ _connectionStatus = true;};
    bool set_offline(){ _connectionStatus = false;};
    bool is_online(){ return _connectionStatus;};
    bool set_address(sockaddr_in newAddress){ _address = newAddress; return true;};
    sockaddr_in get_address() { return _address;};
    unsigned short get_chunk_size() { return _chunkSize;};
    bool set_chunk_size(unsigned short chunkSize) { _chunkSize = chunkSize; return true;}
    bool init_mutex(){
        if(check_init_process(MUTEX_SELF_INIT)) return false;
        if(pthread_mutex_init(&_mutexFlowThreads, NULL) != 0) return false;
        set_init_process_complete(MUTEX_SELF_INIT);
        return true;

    }
    bool assign_flow_thread(UDTPThreadFlow* flowThread){
        _flowThreads.push(flowThread);
    }
    UDTPThreadFlow* front_flow_thread(){
        return _flowThreads.front();
    }
    bool remove_flow_thread(){
        _flowThreads.pop();
        return true;
    }
    private:

    unsigned char _initProcess; /*Holds 8 booleans for the startup process*/
    unsigned int _listenSocket;
    unsigned short _chunkSize;
    bool _connectionStatus;
    sockaddr_in _address;
    pthread_mutex_t _mutexFlowThreads;
    std::queue<UDTPThreadFlow*> _flowThreads;
};

#endif
