#include <iostream>
#include <cstring>
#include <string>
#include "UDTP.h"
SocketType desiredSocketType;

UDTPSetup userSetup(){ /*Creates a UDTPSetup for the user*/

        UDTPSetup returnSetup;


        std::cout << "Please enter an action: " << std::endl;
        std::cout << "1 - Host a server" << std::endl;
        std::cout << "2 - Connect to a server" << std::endl;
        std::string command;
        std::cin >> command;
        if(command=="devs") {
            desiredSocketType = HOST;
        }
        if(command == "1") desiredSocketType = HOST;
        if(command == "2") desiredSocketType = PEER;

        std::string desiredAddress;
        unsigned short desiredPort;

        unsigned short desiredMinChunkSize;
        unsigned short desiredMaxChunkSize;
        unsigned short desiredChunkSizeAgreement;

        switch(desiredSocketType){
            case HOST:

            std::cout << "Please enter a port number: ";
            std::cin >> desiredPort;
            returnSetup.set_port(desiredPort);
            std::cout << std::endl;

            std::cout << "Please enter a minimum chunk size agreement (in bytes): ";
            std::cin >> desiredMinChunkSize;
            returnSetup.set_min_chunk_size(desiredMinChunkSize);
            std::cout << std::endl;

            std::cout << "Please enter a maximum chunk size agreement (in bytes): ";
            std::cin >> desiredMaxChunkSize;
            returnSetup.set_max_chunk_size(desiredMaxChunkSize);
            std::cout << std::endl;

            break;

            case PEER:
            std::cout << "Please enter an IP address: ";
            std::cin >> desiredAddress;
            returnSetup.set_ip(desiredAddress);
            std::cout << std::endl;

            std::cout << "Please enter the port number associated: ";
            std::cin >> desiredPort;
            returnSetup.set_port(desiredPort);
            std::cout << std::endl;

            std::cout << "Please enter a chunk size agreement (in bytes): " << std::endl;
            std::cin >> desiredChunkSizeAgreement;
            returnSetup.set_chunk_size_agreement(desiredChunkSizeAgreement);
            std::cout << std::endl;

            break;
        }

        return returnSetup;
}


int main()
{
    UDTPSetup desiredSetup;
    std::cout << "[User-Datagram Transfer Protocol]" << std::endl;
    desiredSetup = userSetup();


    UDTP MyUDTP(desiredSetup);
    ;
    /*SUCCESS,
    INVALID_SETUP,
    ALREADY_RUNNING,
    SOCKET_NOT_INIT,
    COULD_NOT_START_MUTEX,
    COULD_NOT_BIND_TCP,
    COULD_NOT_BIND_UDP,
    COULD_NOT_LISTEN_TCP,
    COULD_NOT_CONNECT_TCP,
    COULD_NOT_START_THREADS*/
    SocketReturn returnUDTP = MyUDTP.start(desiredSocketType);
        switch(returnUDTP){
            case INVALID_SETUP:
            std::cout << "ERROR: Invalid setup. " << std::endl;
            break;
            case ALREADY_RUNNING:
            std::cout << "ERROR: Session is already running!" << std::endl;
            break;
            case SOCKET_NOT_INIT:
            std::cout << "ERROR: Could not initialize socket!" << std::endl;
            break;
            case COULD_NOT_START_MUTEX:
            std::cout << "ERROR: Could not start mutexes!" << std::endl;
            break;
            case COULD_NOT_BIND_TCP:
            std::cout << "ERROR: Could not bind TCP to socket!" << std::endl;
            break;
            case COULD_NOT_LISTEN_TCP:
            std::cout << "ERROR: Could not listen on TCP socket!" << std::endl;
            break;
            case COULD_NOT_CONNECT_TCP:
            std::cout << "ERROR: Could not connect to host!" << std::endl;
            break;
            case COULD_NOT_START_THREADS:
            std::cout << "ERROR: Could not start listen threads!" << std::endl;
            break;
        }
        if(returnUDTP != 0) return 666;
            std::cout << "SUCCESS: Session has began successfully." << std::endl;
        while(MyUDTP.alive()){
            std::string command;
            std::cin >> command;
            /*This is where peers or hosts can send files, stop server, or anything else!*/
        }


    return 0;
}
