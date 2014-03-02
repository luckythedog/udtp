#ifndef __UDTP_SOCKET
#define __UDTP_SOCKET
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
class UDTPAddress;
class UDTPFlowThreadData;

enum initProcessEvent{
    LISTEN_SOCKET = 0x00, /*Has Listen socket started*/
    LISTEN_THREAD = 0x01, /*Has listen thread started*/
    VERSION_AGREE = 0x02, /*Had version agreed*/
    CHUNKSIZE_AGREE = 0x03, /*Had chunksized agreed*/
    FLOW_SOCKETS = 0x04, /*Has completed flow sockets startup with server*/
    FLOW_ADDRESSES = 0x05,
    FLOW_THREADS = 0x06, /*Has completed thread startup*/
    COMPLETE  = 0x07, /*Completion only verified by themselves*/


};

class UDTPPeer{ /*The way we can identify different UDP ports is that, we can identify that they have the same TCP socket. the TCP socket never changes!*/
   public:
    UDTPPeer();
    UDTPPeer(unsigned int listenSocket) {
        _listenSocket = listenSocket;
        _initProcess = 0x00;
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

    bool add_flow_thread(pthread_t thread, unsigned int flowSocket, sockaddr_in socketAddress);
    UDTPFlowThreadData* get_next_thread_link_needed();
    UDTPFlowThreadData* get_thread(unsigned int posID){ return _flowThreads[posID];};
    UDTPFlowThreadData* find_thread_with_port(unsigned short port);
    bool check_all_flow_threads_approved();
     bool check_all_flow_threads_alive();
        bool remove_all_flow_threads();
    bool set_online(){ _connectionStatus = true;};
    bool set_offline(){ _connectionStatus = false;};
    bool get_connection_status(){ return _connectionStatus;};
    bool set_listen_address(sockaddr_in newAddress){ _listenAddress = newAddress; return true;};
    sockaddr_in get_listen_address() { return _listenAddress;};
    private:
    std::vector<UDTPFlowThreadData*> _flowThreads; /*Holds all threads*/
    unsigned char _initProcess; /*Holds 8 booleans for the startup process*/
    unsigned int _listenSocket;
    bool _connectionStatus;
    sockaddr_in _listenAddress;
};

#endif
