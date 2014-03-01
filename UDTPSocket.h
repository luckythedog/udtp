#ifndef __UDTP_SOCKET
#define __UDTP_SOCKET

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
class UDTPAddress;

class UDTPPeer{ /*Is held in an array for incoming TCP and UDP clients. This is just to store information so we can track them later when they are connected.*/
    UDTPPeer();
    UDTPPeer(unsigned int socket) { _socket = socket; _readyListen = true; _readyFlow = false;};/*starts off TCP.*/
    ~UDTPPeer();

    bool is_socket(unsigned int socket){ if(_socket == socket) return true;else;return false;}

    bool set_socket(unsigned int socket) { _socket = socket; return true;};
    unsigned int get_socket(){ return _socket; };

    bool set_address(sockaddr_in socketAddress);
    UDTPAddress* address(){ return _address;};

    bool disconnected() { keepAlive = false;};
    bool connected() { return keepAlive; };
    bool ready_listen() { return _readyListen;};
    bool ready_flow() { return _readyFlow; };

    bool change_ready_listen( bool newBool) { _readyListen = newBool;};
      bool change_flow_listen( bool newBool) { _readyListen = newBool;};
    private:
    UDTPAddress* _address;
    unsigned int _socket;
    bool keepAlive;
    bool readyListen;
    bool readyFlow;
};

#endif
