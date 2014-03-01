#ifndef __UDTP_SOCKET
#define __UDTP_SOCKET
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
class UDTPAddress;

class UDTPPeer{ /*The way we can identify different UDP ports is that, we can identify that they have the same TCP socket. the TCP socket never changes!*/
   public:
    UDTPPeer();
    UDTPPeer(unsigned int listenSocket) { _listenSocket = listenSocket; _ready = false;};/*starts off TCP.*/
    ~UDTPPeer();

    bool compare_listen_socket(unsigned int listenSocket){ if(_listenSocket== listenSocket) return true;else;return false;}

    bool set_listen_socket(unsigned int listenSocket) { _listenSocket = listenSocket; return true;};
    unsigned int get_listen_socket(){ return _listenSocket; };

    bool add_address(sockaddr_in socketAddress);
    bool get_address_exist(sockaddr_in socketAddress);
    bool get_address_exist(UDTPAddress& address);

    UDTPAddress& get_address_at(unsigned int index) { return _addresses[index];};

    unsigned short addresses_count () { return _addresses.size(); };

    bool set_ready() { _ready = true;};
    bool check_ready() { return _ready;};

    bool set_online(){ _connectionStatus = true;};
    bool set_offline(){ _connectionStatus = false;};
    bool get_connection_status(){ return _connectionStatus;};

    UDTPAddress& operator[] (const unsigned int index);

    private:
    bool _ready; /*Ready for file transfer?*/
    std::vector<UDTPAddress> _addresses;
    unsigned int _listenSocket;
    bool _connectionStatus;

};

#endif
