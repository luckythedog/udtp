

class UDTPDestination{
    public:
        UDTPDestination(sockaddr_in socketAddress){ //Creates ip and port information from socket address*/
           _ip = inet_ntoa(socketAddress.sin_addr);
            _port = ntohs(socketAddress.sin_port);
        }
        char* get_ip() { return _ip;};
        unsigned int get_port() { return _port;};

        bool compare_with_destination(UDTPDestination& destination){
            if(get_ip() == destination.get_ip() && get_port() == destination.get_port())return true;else;return false;
        }
    private:
    char* _ip;
    unsigned int _port;
};
