
#ifndef __UDTP_ADDRESS
#define __UDTP_ADDRESS
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
class UDTPAddress{
    public:
    UDTPAddress();
        UDTPAddress(sockaddr_in socketAddress){ //Creates ip and port information from socket address*/
           _ip = inet_ntoa(socketAddress.sin_addr);
            _port = ntohs(socketAddress.sin_port);
        }
        char* get_ip() { return _ip;};
        unsigned int get_port() { return _port;};

        bool compare_with_address(UDTPAddress& address){
            if(get_ip() == address.get_ip() && get_port() == address.get_port())return true;else;return false;
        }
    private:
    char* _ip;
    unsigned int _port;
};

#endif
