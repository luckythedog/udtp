#include "UDTPAddress.h"
class UDTPListenSocket{ /*Is held in an array for incoming TCP clients*/
    UDTPListenSocket(unsigned int listenSocket, UDTPDestination destination){
        destination = _destination;
        _listenSocket = listenSocket;
    }

    unsigned int get_socket(){ return _listenSocket; };
    private:
    UDTPDestination _destination;
    unsigned int _listenSocket;
};
