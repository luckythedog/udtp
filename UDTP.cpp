#include "UDTP.h"
/*Packets, Implementation after forward declaration*/
#include "UDTPSetup.h"
#include "UDTPPacket.h"
#include "UDTPPath.h"
#include "UDTPFile.h"
#include "UDTPHeader.h"
#include "UDTPChunk.h"
#include "UDTPAcknowledge.h"
#include "UDTPHandshake.h"
#include "UDTPFlowThreadData.h"
#include "UDTPAddress.h"
#include "UDTPPeer.h"
#include <fstream>
#include <string.h>
#define EMPTY 0x00

SocketReturn UDTP::start(SocketType socketType)
{

    if(_isAlive) return ALREADY_RUNNING;
    if(!start_mutex()) return COULD_NOT_START_MUTEX;
    _socketType = socketType;

    _listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(_listenSocket < 0) return SOCKET_NOT_INIT;

    struct sockaddr_in listenAddress; /*TCP*/
    memset(&listenAddress, 0, sizeof(listenAddress));
    listenAddress.sin_port = htons(_myUDTP.get_port());
    listenAddress.sin_family = AF_INET;



    /*Important part*/
    switch (_socketType)
    {
    case HOST:
        listenAddress.sin_addr.s_addr = INADDR_ANY;
        if(bind(_listenSocket, (struct sockaddr*)&listenAddress, sizeof(struct sockaddr_in)) < 0) return COULD_NOT_BIND_TCP;
        if(listen(_listenSocket,0) < 0) return COULD_NOT_LISTEN_TCP;
        display_msg("HOST socket began listening");
        break;
    case PEER:
        listenAddress.sin_addr.s_addr = inet_addr(_myUDTP.get_ip());
        if(connect(_listenSocket, (struct sockaddr*)&listenAddress, sizeof(struct sockaddr_in)) < 0) return COULD_NOT_CONNECT_TCP;
        display_msg("PEER socket has connected to server");
        break;
    }

    unsigned int peerID = add_peer(_listenSocket); /*Add self as socket. Doesn't matter if you're server or client!*/
    self_peer()->set_init_process_complete(LISTEN_SOCKET);

    if(!start_listen_thread()) return COULD_NOT_START_THREADS;

    _isAlive = true;
    return SUCCESS;

}
bool UDTP::start_mutex()
{
    display_msg("Mutex have been started");
    if (pthread_mutex_init(&_mutexFlowThread, NULL) != 0) return false;
    if (pthread_mutex_init(&_mutexPeer, NULL) != 0) return false;
    return true;
}
bool UDTP::start_flow_sockets(unsigned int peerID) /*Could be self peer id too!*/
{
    // TODO: Kevin will need to re-write.  Refer to previous commits for code.
    return true;
}
bool UDTP::send_flow_links(unsigned int peerID)
{
    // TODO: Kevin will need to re-write.  Refer to previous commits for code.
    return true;
}
bool UDTP::stop()
{
    _isAlive = false;

}
bool UDTP::start_flow_threads(unsigned int peerID)
{
    // TODO: Kevin will need to re-write.  Refer to previous commits for code.
    return true;
}
bool UDTP::send_listen_data(UDTPPacket* packet)
{
    if(get_socket_type() == PEER) display_msg("PEER has sent out data using send_listen_data()");
    if(get_socket_type() == HOST) display_msg("HOST has sent out data using send_listen_data()");
    send(packet->get_socket_id(), packet->get_raw_buffer(), packet->get_packet_size(), 0); /*The UDTPPAclet's socket id is used to denote where to send the packet. The socket id is handled in the polling threads. For HOSTS, the socket id
                                                                                                                    will be the receiving socket's file descriptor. For PEER, it will always be its own _listenSocket being that that is where the central receiving and sending
                                                                                                                    happens.*/
    return true;
}
unsigned int UDTP::add_peer(unsigned int listenSocketOfPeer)
{
    unsigned int posID = 0;
    pthread_mutex_lock(&_mutexPeer); /*Adds a lock just in case*/
    UDTPPeer* newPeer = new UDTPPeer(listenSocketOfPeer);
    newPeer->set_online();
    _listPeers.push_back(newPeer);
    posID = _listPeers.size() - 1; /*Capacity minus 1!*/
    pthread_mutex_unlock(&_mutexPeer);
    return posID;
}
unsigned int UDTP::find_peer_pos(unsigned int listenSocketOfPeer)
{
    pthread_mutex_lock(&_mutexPeer);
    for(unsigned int posID=0; posID<_listPeers.size(); posID++)
    {
        if(_listPeers[posID]->get_listen_socket() == listenSocketOfPeer)
        {
            return posID;
        }
    }
    return 0; /*This return 0, since self is ZERO, this means false!*/
    pthread_mutex_unlock(&_mutexPeer);
}
UDTPPeer* UDTP::get_peer(unsigned int posID)
{
    pthread_mutex_lock(&_mutexPeer);
    unsigned int sizeOfListPeers = _listPeers.size();
    pthread_mutex_unlock(&_mutexPeer);
    if(!(posID >= 0 && posID < sizeOfListPeers)) return NULL; /*Out of bounds!*/
    return _listPeers[posID];
}

bool UDTP::remove_peer(unsigned int posID)
{
    pthread_mutex_lock(&_mutexPeer);
    if(!(posID >= 0 && posID < _listPeers.size()))
    {
        pthread_mutex_unlock(&_mutexPeer);
        return false; /*Out of bounds!*/
    }
    delete _listPeers[posID];
    _listPeers.erase(_listPeers.begin()+posID);
    pthread_mutex_unlock(&_mutexPeer);
    return true;
}

bool UDTP::send_peer_init_completed(unsigned int peerID)
{
    // TODO: Kevin will need to re-write.  Refer to previous commits for code.
    return true;
}

void* UDTP::listenThreadFunc(void* args)
{
    std::cout << "Here" << std::endl;
    bool sentRequiredPackets = false; /*For client*/

    UDTP *accessUDTP = (UDTP*) args;
    std::vector<pollfd> activeListenSockets;
    pollfd masterPollFd;
    masterPollFd.fd = accessUDTP->_listenSocket;
    masterPollFd.events = POLLIN;
    activeListenSockets.push_back(masterPollFd);

    pollfd* activeListenSocketsPtr;
    int activeListenActivity;

    std::queue<unsigned int> removePeerAndSocketSafely;
    if(accessUDTP->get_socket_type() == PEER) accessUDTP->display_msg("PEER's polling has been configured and is starting");
    if(accessUDTP->get_socket_type() == HOST) accessUDTP->display_msg("HOST's polling has been configured and is starting");
    while(accessUDTP->alive())
    {
        /*********Beginning of Server Code**********/
        if(accessUDTP->_socketType == HOST)
        {
            activeListenSocketsPtr = &activeListenSockets[0];
            activeListenActivity = poll(activeListenSocketsPtr, activeListenSockets.size(), -1);
            if(activeListenActivity < 0 ) perror("poll_host");
            if((activeListenSockets[0].revents & POLLIN))
            {
                struct sockaddr_in newPeerAddress;
                int newPeerAddressLen = sizeof(newPeerAddress);
                unsigned int newPeerListenSocket;
                if((newPeerListenSocket = accept(activeListenSockets[0].fd, (struct sockaddr*)&newPeerAddress, (socklen_t*)&newPeerAddressLen)) <0)
                {
                    perror("accept");
                }
                else
                {
                    accessUDTP->display_msg("HOST has detected a new peer and is accepting the connection");
                    pollfd newPollFd; //*Add to polling*/
                    newPollFd.fd = newPeerListenSocket;
                    newPollFd.events = POLLIN;
                    activeListenSockets.push_back(newPollFd);
                    /*Add to peers list. Both Pollfd and _listPeer are Parallel..*/
                    unsigned int newPeerID = accessUDTP->add_peer(newPeerListenSocket);
                    accessUDTP->get_peer(newPeerID)->set_listen_address(newPeerAddress);

                    UDTPHandshake handshakeInitiation(HandshakeInitiation); /*Send this handshake request to client so he will start activate the function: send_required_packets();*/
                    handshakeInitiation.set_socket_id(newPeerListenSocket); /*Take socket id! so the send_listen_data function will know where to send it to!*/
                    // TODO: Might want to refactor this code
                    handshakeInitiation.set_udtpsetup(accessUDTP->get_udtpsetup()); /*Pass UDTP instance setup info to handshake packet*/
                    accessUDTP->display_msg("HOST has sent out a HandshakeStart to notify new peer to use function");
                    accessUDTP->send_listen_data(&handshakeInitiation); /*Send out Handshake start so the client knows to start the function send_required_packets()*/


                }
            }
            for(unsigned int i=1; i<activeListenSockets.size(); i++)
            {
                if(activeListenSockets[i].revents & POLLIN)
                {
                    UDTPPacketHeader packetDeduction;
                    if((recv(accessUDTP->_listenSocket, &packetDeduction, sizeof(UDTPPacketHeader), 0)) != 0)
                    {
                        accessUDTP->display_msg("HOST has received incoming packet");
                        UDTPPacket *incomingData = 0;

                        /* Determine packet type and create new packet object */
                        accessUDTP->display_msg("HOST is processing packet deducation");

                        /*USE packet type and build correctly. here*/
                        switch(packetDeduction.packetType)
                        {
                            // TODO: abstract out the 3 calls in each case (set_socket_id, set_peer_id, recv)
                        case Header:
                            accessUDTP->display_msg("HOST has deduced the packet as a Header and is now processing");
                            incomingData = new UDTPHeader(packetDeduction);
                            incomingData->set_socket_id(activeListenSockets[i].fd); /*Applies socket to UDTPPacket*/
                            incomingData->set_peer_id(i); /*Sets index as the peer's ID location*/
                            recv(accessUDTP->_listenSocket, (char*)incomingData->write_to_buffer(), incomingData->get_packet_size(), 0);
                            incomingData->process(accessUDTP);
                            break;
                        case Path:
                            accessUDTP->display_msg("HOST has deduced the packet as a Path and is now processing");
                            incomingData = new UDTPPath(packetDeduction);
                            incomingData->set_socket_id(activeListenSockets[i].fd); /*Applies socket to UDTPPacket*/
                            incomingData->set_peer_id(i); /*Sets index as the peer's ID location*/
                            recv(accessUDTP->_listenSocket, (char*)incomingData->write_to_buffer(), incomingData->get_packet_size(), 0);
                            incomingData->process(accessUDTP);
                            break;
                        case Acknowledge:
                            accessUDTP->display_msg("HOST has deduced the packet as an Acknowledge and is now processing");
                            incomingData = new UDTPAcknowledge(packetDeduction);
                            incomingData->set_socket_id(activeListenSockets[i].fd); /*Applies socket to UDTPPacket*/
                            incomingData->set_peer_id(i); /*Sets index as the peer's ID location*/
                            recv(accessUDTP->_listenSocket, (char*)incomingData->write_to_buffer(), incomingData->get_packet_size(), 0);
                            incomingData->process(accessUDTP);
                            break;
                        case Handshake:
                            accessUDTP->display_msg("HOST has deduced the packet as a Handshake and is now processing");
                            incomingData = new UDTPHandshake(packetDeduction);
                            incomingData->set_socket_id(activeListenSockets[i].fd); /*Applies socket to UDTPPacket*/
                            incomingData->set_peer_id(i); /*Sets index as the peer's ID location*/
                            recv(accessUDTP->_listenSocket, (char*)incomingData->write_to_buffer(), incomingData->get_packet_size(), 0);
                            ((UDTPHandshake*)incomingData)->set_udtpsetup(accessUDTP->get_udtpsetup());
                            if(incomingData->process(accessUDTP))
                                accessUDTP->send_listen_data(incomingData);
                            break;
                        default:
                            accessUDTP->display_msg("HOST was unable to deduce the packet type.");
                            break;
                        }

                        accessUDTP->display_msg("HOST has completed packet processing");
                        if(incomingData != 0)
                            delete incomingData;
                    }
                    else
                    {
                        /*Client has disconnected*/
                        accessUDTP->_listPeers[i]->set_offline();
                        removePeerAndSocketSafely.push(i);
                    }
                }
            }
            while(!removePeerAndSocketSafely.empty())
            {
                unsigned int posID = removePeerAndSocketSafely.front(); /*Get pos Id, this is just a locational id in the parallel array..*/

                activeListenSockets.erase(activeListenSockets.begin()+posID); /*Remove from polling(*/
                accessUDTP->remove_peer(posID);
                removePeerAndSocketSafely.pop();
            }
        } /*End of server code*/
        /***********End of Server Code*************/

            activeListenSocketsPtr = &activeListenSockets[0]; /*This line fixes poll: Bad Address*/
        /*********Beginning of Client Code**********/
        if(accessUDTP->_socketType == PEER)
        {

            activeListenActivity = poll(activeListenSocketsPtr, activeListenSockets.size(), -1);
            if(activeListenActivity < 0 ) perror("poll_peer");

            if((activeListenSockets[0].revents & POLLIN))
            {
                UDTPPacketHeader packetDeduction;
                std::cout << sizeof(UDTPPacketHeader) << std::endl;
                if((recv(accessUDTP->_listenSocket, &packetDeduction, sizeof(UDTPPacketHeader), MSG_WAITALL)) != 0)
                {
                    packetDeduction.packetSize -= sizeof(UDTPPacketHeader); /*already recv packet header*/
                    accessUDTP->display_msg("PEER has received incoming packet");

                    UDTPPacket *incomingData = NULL;
                    char* incomingDataTemp;

                    /* Determine packet type and create new packet object */
                    accessUDTP->display_msg("PEER is processing packet deduction");

                    /*USE packet type and build correctly. here*/
                    switch(packetDeduction.packetType)
                    {
                        // TODO: Abstract out the 3 repetitive calls
                    case Header:
                        accessUDTP->display_msg("PEER has deduced the packet as a Header and is now processing");
                        incomingData = new UDTPHeader(packetDeduction);
                        incomingData->set_socket_id(accessUDTP->self_peer()->get_listen_socket()); /*Applies socket to data set again*/
                        incomingData->set_peer_id(0); /*Set to self*/
                        recv(accessUDTP->_listenSocket, (char*)incomingData->write_to_buffer(), incomingData->get_packet_size(), 0);
                        incomingData->process(accessUDTP);
                        break;
                    case Path:
                        accessUDTP->display_msg("PEER has deduced the packet as a Path and is now processing");
                        incomingData = new UDTPPath(packetDeduction);
                        incomingData->set_socket_id(accessUDTP->self_peer()->get_listen_socket()); /*Applies socket to data set again*/
                        incomingData->set_peer_id(0); /*Set to self*/
                        recv(accessUDTP->_listenSocket, (char*)incomingData->write_to_buffer(), incomingData->get_packet_size(), 0);
                        incomingData->process(accessUDTP);
                        break;
                    case Acknowledge:
                        accessUDTP->display_msg("PEER has deduced the packet as an Acknowledge and is now processing");
                        incomingData = new UDTPAcknowledge(packetDeduction);
                        incomingData->set_socket_id(accessUDTP->self_peer()->get_listen_socket()); /*Applies socket to data set again*/
                        incomingData->set_peer_id(0); /*Set to self*/
                        recv(accessUDTP->_listenSocket, (char*)incomingData->write_to_buffer(), incomingData->get_packet_size(), 0);
                        incomingData->process(accessUDTP);
                        break;
                    case Handshake:
                        accessUDTP->display_msg("PEER has deduced the packet as a Handshake and is now processing");
                        incomingData = new UDTPHandshake(packetDeduction);
                        incomingData->set_socket_id(accessUDTP->self_peer()->get_listen_socket()); /*Applies socket to data set again*/
                        incomingData->set_peer_id(0); /*Set to self*/
                        // TODO: Maybe make this more elegant or revise write_to_buffer!
                        /* FIXED: Found problem, it dealt with write_to_buffer(), I just created a temp and allocated the size then read into that. I guess the bad address (pointer) was referring to char* address (pointer)*/
                        incomingDataTemp = new char [ incomingData->get_packet_size()];
                        if(recv(accessUDTP->_listenSocket, incomingDataTemp, incomingData->get_packet_size(), MSG_WAITALL) == -1)
                        {
                            perror("recv error");
                        }
                        incomingData->set_raw_buffer(incomingDataTemp);

                        ((UDTPHandshake*)incomingData)->set_udtpsetup(accessUDTP->get_udtpsetup());
                        if(incomingData->process(accessUDTP))
                            accessUDTP->send_listen_data(incomingData);
                        break;
                    default:
                        accessUDTP->display_msg("PEER was unable to deduce the packet type.");
                        break;
                    }

                    accessUDTP->display_msg("PEER has completed packet processing");
                    if(incomingData != 0)
                        delete incomingData;
                }
                else
                {
                    accessUDTP->stop(); /*Client has disconnected or server has disconnected client.*/
                }
            }
        }
        /***********End of Client Code*************/
    }

    accessUDTP = NULL;
}
/*void UDTP::send_flow_data(UDTPData& data){ /*Only chunks really!
    while( (send(_flowSocket, data.get_raw_buffer(), data.get_packet_size(), 0)) != data.get_packet_size()); /*Keep sending until it's all sent
}*/
void UDTP::send_flow_data(UDTPFlowThreadData* threadFlowData, UDTPPacket& packet)
{
    if(!threadFlowData->check_approved() && get_socket_type() == PEER)
    {
        struct sockaddr_in serverMainUDPAddress;
        serverMainUDPAddress.sin_port = htons(_myUDTP.get_port()); /*Main port, no additive*/
        serverMainUDPAddress.sin_addr.s_addr = inet_addr(_myUDTP.get_ip());
        serverMainUDPAddress.sin_family = AF_INET;
        sendto(threadFlowData->get_flow_socket(), packet.get_raw_buffer(), packet.get_packet_size(), 0, (struct sockaddr*)&serverMainUDPAddress, sizeof(serverMainUDPAddress));
    }


    /*switch (data.get_packet_type()){
        case Handshake:
        break;
        case Chunk: //*Chunk will be sent as is!

        break;
    }*/
    threadFlowData = NULL;
    /*Have to find out the server's different addresses*/
}
void UDTP::display_msg(std::string message)
{

    if(_myUDTP.get_debug_enabled()) std::cout << message << std::endl;
}
void* UDTP::flowThreadsFunc(void* args)
{
//0x01<<6 here
    UDTPFlowThreadData *myFlowData = (UDTPFlowThreadData*) args;
    while(myFlowData->alive())  /*Thread is good and running!*/
    {
        char packetDeduction[3];
        recv(myFlowData->get_flow_socket(), packetDeduction, 3, MSG_WAITALL);
    }

    myFlowData  = NULL;
}

bool UDTP::start_listen_thread()
{
#ifndef _WIN32
    pthread_create(&_listenThread, NULL, &UDTP::listenThreadFunc, (UDTP*)this);
    pthread_tryjoin_np(_listenThread, NULL);
    self_peer()->set_init_process_complete(LISTEN_THREAD);



#else

#endif
    return true;
}

SocketType UDTP::get_socket_type()
{
    return _socketType;
}
bool UDTP::alive()
{

    return _isAlive;
}
