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
#include "UDTPAddress.h"
#include "UDTPPeer.h"
#include "UDTPThreadFlow.h"
#include "UDTPThreadFile.h"
#include "UDTPThreadProcess.h"
#include <fstream>
#include <string.h>
#define EMPTY 0x00

SocketReturn UDTP::start(SocketType socketType)
{
    _flowThreadCount = 0;
    if(_isAlive) return ALREADY_RUNNING;
    _socketType = socketType;

    /*TCP set up!*/
    if (!start_listen_socket(socketType)) return SOCKET_NOT_INIT; /*starts a listen socket on bind type*/
    /*Flow socket setup!*/
    /*Important part*/
    socketType == HOST ? display_msg("HOST has successfully completed socket setup.") : display_msg("HOST has successfully completed socket setup.");
    unsigned int peerID = add_peer(_listenSocket); /*Add self as socket. Doesn't matter if you're server or client!*/
    self_peer()->set_init_process_complete(LISTEN_SOCKET);

    if(!start_mutex()) return COULD_NOT_START_MUTEX;
    if(!start_queue_threads()) return COULD_NOT_START_THREADS;
    if(!start_listen_thread()) return COULD_NOT_START_THREADS;

    _isAlive = true;
    return SUCCESS;

}
bool UDTP::start_listen_socket(SocketType bindType)
{
    _listenSocket = socket(AF_INET, SOCK_STREAM, 0); /*TCP Socket!*/
    if(_listenSocket < 0) return false;
    struct sockaddr_in listenAddress; /*TCP Struct address!*/
    memset(&listenAddress, 0, sizeof(listenAddress));
    listenAddress.sin_port = htons(get_setup()->get_port());
    listenAddress.sin_family = AF_INET;

    switch (bindType)
    {
    case HOST:
        listenAddress.sin_addr.s_addr = INADDR_ANY;
        if(bind(_listenSocket, (struct sockaddr*)&listenAddress, sizeof(struct sockaddr_in)) < 0) return false;
        if(listen(_listenSocket,0) < 0) return false;
        break;
    case PEER:
        listenAddress.sin_addr.s_addr = inet_addr(get_setup()->get_ip());
        if(connect(_listenSocket, (struct sockaddr*)&listenAddress, sizeof(struct sockaddr_in)) < 0) return false;
        break;
    }

    return true;
}
bool UDTP::add_flow_thread(ThreadType threadType, unsigned int peerID)
{
    UDTPPeer* accessPeer = get_peer(peerID); /*So we only have to run get peer one time!*/
    if(accessPeer->is_online())  return false;
    unsigned int flowSocket;
    flowSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if(flowSocket <0) return false;
    int optval = 1;
    if((setsockopt(flowSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))) < 0) return false;
    struct sockaddr_in flowAddress = get_peer(peerID)->get_address();

    if( (bind(flowSocket, (struct sockaddr*)&flowAddress, sizeof(flowAddress))) < 0 )
    {
        perror("udp-bind");
        return false;
    }
    pthread_t flowThreadHandler;
    UDTPThreadFlow *newFlowThread = new UDTPThreadFlow((UDTP*)this, flowThreadHandler, flowSocket, threadType);
    pthread_create(&flowThreadHandler, NULL, UDTP::flowThread, (UDTPThreadFlow*)newFlowThread);
    pthread_tryjoin_np(flowThreadHandler, NULL);

    get_peer(peerID)->assign_flow_thread(newFlowThread);
    _flowThreadCount++;
    return true;
}
bool UDTP::remove_flow_thread(unsigned int peerID)
{
    UDTPThreadFlow *removeThread = get_peer(peerID)->front_flow_thread();
    removeThread->kill();
    close(removeThread->flow_socket());
    removeThread = NULL;
    _flowThreadCount--;
    return true;
}
bool UDTP::start_mutex()
{
    display_msg("Mutex have been started");
    if (pthread_mutex_init(&_mutexFlowThread, NULL) != 0) return false;
    if (pthread_mutex_init(&_mutexPeer, NULL) != 0) return false;
    if(pthread_mutex_init(&_mutexActiveFiles, NULL) != 0) return false;
        if(pthread_mutex_init(&_mutexPendingFiles, NULL) != 0) return false;
    if(sem_init(&_semListenPacketQueue,0,0) != 0) return false;
    if(sem_init(&_semFlowPacketQueue,0,0)  != 0) return false;
    self_peer()->set_init_process_complete(MUTEX_FRAMEWORK_INIT);

    return true;
}

bool UDTP::stop()
{
    _isAlive = false;

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
int UDTP::find_peer_pos(unsigned int listenSocketOfPeer)
{
    pthread_mutex_lock(&_mutexPeer);
    for(unsigned int posID=0; posID<_listPeers.size(); posID++)
    {
        if(_listPeers[posID]->get_listen_socket() == listenSocketOfPeer)
        {
            pthread_mutex_unlock(&_mutexPeer);
            return posID;
        }
    }
    return 0; /*This return 0, since self is ZERO, this means false!*/
    pthread_mutex_unlock(&_mutexPeer);
}
int UDTP::find_peer_using_address(sockaddr_in searchAddress)
{
    pthread_mutex_lock(&_mutexPeer);
    for(int posID=0; posID<_listPeers.size(); posID++)
    {
        if((inet_ntoa(searchAddress.sin_addr)) ==inet_ntoa(_listPeers[posID]->get_address().sin_addr))
        {
            if((ntohs(searchAddress.sin_port)) ==ntohs(_listPeers[posID]->get_address().sin_port))
            {
                pthread_mutex_unlock(&_mutexPeer);
                return posID;
            }
        }
    }
    pthread_mutex_unlock(&_mutexPeer);
    return -1; /*Nothing was found!*/

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
UDTPFile* UDTP::get_file_by_id(unsigned int fileID)
{
    pthread_mutex_lock(&_mutexActiveFiles);
    for(unsigned int i=0; i<_activeFiles.size(); i++)
    {
        if(_activeFiles[i]->get_file_id() == fileID)
        {
            pthread_mutex_unlock(&_mutexActiveFiles);
            return _activeFiles[i];
        }
    }
    pthread_mutex_unlock(&_mutexActiveFiles);
    return NULL;

}
    bool UDTP::add_file_to_active(UDTPFile* file){
        _activeFiles.push_back(file);
    }
    bool UDTP::add_file_to_pending(UDTPFile* file){
        _pendingFiles.push_back(file);
    }
  bool UDTP::approve_pending_file(UDTPHeader* compare){
  }
void UDTP::add_queue_listen(UDTPPacket* packet){
    display_msg("Packet was added to request queue!");
    _listenPacketQueue.push(packet);
}
void UDTP::add_queue_flow(UDTPChunk* chunk){
    display_msg ("Chunk was sent to request queue!");
    _flowPacketQueue.push(chunk);
}
void* UDTP::listenQueueThread(void* args){ /*This will handle all listen packets*/
    UDTP *accessUDTP = (UDTP*) args;
    while(accessUDTP->alive()){
        sem_wait(&accessUDTP->_semListenPacketQueue);
        while(!accessUDTP->_listenPacketQueue.empty()){

        }

    }
    accessUDTP = NULL;
}
void* UDTP::listenThread(void* args)
{

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
                    accessUDTP->get_peer(newPeerID)->set_address(newPeerAddress);

                    UDTPHandshake handshakeStart(ResponseStart); /*Send this handshake request to client so he will start activate the function: send_required_packets();*/
                    handshakeStart.set_socket_id(newPeerListenSocket); /*Take socket id! so the send_listen_data function will know where to send it to!*/
                    // TODO: Might want to refactor this code
                    accessUDTP->display_msg("HOST has sent out a HandshakeStart to notify new peer to use function");
                    accessUDTP->send_listen_data(&handshakeStart); /*Send out Handshake start so the client knows to start the function send_required_packets()*/
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
                        accessUDTP->display_msg("HOST is processing packet deduction");
                        bool deductionValid = true;
                        /*USE packet type and build correctly. here*/
                        switch(packetDeduction.packetType)
                        {
                            // TODO: abstract out the 3 calls in each case (set_socket_id, set_peer_id, recv)
                        case Header:
                            accessUDTP->display_msg("HOST has deduced the packet as a Header and is now processing");
                            incomingData = new UDTPHeader(packetDeduction);
                            break;
                        case Path:
                            accessUDTP->display_msg("HOST has deduced the packet as a Path and is now processing");
                            incomingData = new UDTPPath(packetDeduction);
                            break;
                        case Acknowledge:
                            accessUDTP->display_msg("HOST has deduced the packet as an Acknowledge and is now processing");
                            incomingData = new UDTPAcknowledge(packetDeduction);
                            break;
                        case Handshake:
                            accessUDTP->display_msg("HOST has deduced the packet as a Handshake and is now processing");
                            incomingData = new UDTPHandshake(packetDeduction);
                            break;
                        default:
                            accessUDTP->display_msg("HOST was unable to deduce the packet type.");
                            deductionValid = false;
                            break;
                        }
                        if(deductionValid){
                            incomingData->set_socket_id(activeListenSockets[i].fd);
                            incomingData->set_peer_id(i);
                            incomingData->set_udtp(accessUDTP); /*Pass on UDTP pointer*/
                            recv(accessUDTP->_listenSocket, (char*)incomingData->write_to_buffer(), incomingData->get_packet_size(), 0);
                            /*Verifying sending check!*/
                            unsigned int resendCheck = 0x00;
                            if(incomingData->unpack()) resendCheck |= (0x01 << 0x00); /*Unpack to member variables*/
                            if(incomingData->respond()) resendCheck |= (0x01 << 0x01); /*Respond to it and apply some changes or whatever*/
                            if(incomingData->pack()) resendCheck |= (0x01 << 0x02); /*Pack it up again*/

                            if(resendCheck &  0x07){
                                accessUDTP->send_listen_data(incomingData);
                            }

                            //sem_post(&accessUDTP->_semListenPacketQueue);
                        }
                        accessUDTP->display_msg("HOST has completed packet processing");

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
                    if((recv(accessUDTP->_listenSocket, &packetDeduction, sizeof(UDTPPacketHeader), 0)) != 0)
                    {
                        accessUDTP->display_msg("PEER has received incoming packet");
                        UDTPPacket *incomingData = 0;

                        /* Determine packet type and create new packet object */
                        accessUDTP->display_msg("PEER is processing packet deduction");
                        bool deductionValid = true;
                        /*USE packet type and build correctly. here*/
                        switch(packetDeduction.packetType)
                        {
                            // TODO: abstract out the 3 calls in each case (set_socket_id, set_peer_id, recv)
                        case Header:
                            accessUDTP->display_msg("PEER has deduced the packet as a Header and is now processing");
                            incomingData = new UDTPHeader(packetDeduction);
                            break;
                        case Path:
                            accessUDTP->display_msg("PEER has deduced the packet as a Path and is now processing");
                            incomingData = new UDTPPath(packetDeduction);
                            break;
                        case Acknowledge:
                            accessUDTP->display_msg("PEER has deduced the packet as an Acknowledge and is now processing");
                            incomingData = new UDTPAcknowledge(packetDeduction);
                            break;
                        case Handshake:
                            accessUDTP->display_msg("PEER has deduced the packet as a Handshake and is now processing");
                            incomingData = new UDTPHandshake(packetDeduction);
                            break;
                        default:
                            accessUDTP->display_msg("PEER was unable to deduce the packet type.");
                            deductionValid = false;
                            break;
                        }
                        if(deductionValid){
                            incomingData->set_socket_id(activeListenSockets[0].fd);
                            incomingData->set_peer_id(0);
                            incomingData->set_udtp(accessUDTP); /*Pass on UDTP pointer*/
                            recv(accessUDTP->_listenSocket, (char*)incomingData->write_to_buffer(), incomingData->get_packet_size(), 0);
                            /*Verifying sending check!*/
                            unsigned int resendCheck = 0x00;
                            if(incomingData->unpack()) resendCheck |= (0x01 << 0x00); /*Unpack to member variables*/
                            if(incomingData->respond()) resendCheck |= (0x01 << 0x01); /*Respond to it and apply some changes or whatever*/
                            if(incomingData->pack()) resendCheck |= (0x01 << 0x02); /*Pack it up again*/

                            if(resendCheck &  0x07){
                                accessUDTP->send_listen_data(incomingData);
                            }
                        }
                        accessUDTP->display_msg("PEER has completed packet processing");
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




void UDTP::display_msg(std::string message)
{

    if(get_setup()->get_debug_enabled()) std::cout << message << std::endl;
}

void* UDTP::flowQueueThread(void* args){ /*Open as many chunk queue threads as INCOMING flow threads*/
    UDTP *accessUDTP = (UDTP*) args;
    while(accessUDTP->alive()){
        sem_wait(&accessUDTP->_semFlowPacketQueue);
        UDTPChunk* processChunk;
        UDTPFile* processFile;
        while(!accessUDTP->_flowPacketQueue.empty()){
            processChunk = accessUDTP->_flowPacketQueue.front();
            accessUDTP->_flowPacketQueue.pop();

           processFile =  accessUDTP->get_file_by_id(processChunk->get_file_id());
           if(processFile == NULL){
                accessUDTP->display_msg("Could not find file with chunk's file id!");
           }else{
                accessUDTP->display_msg("Chunk has been applied to file's queue!");
                processFile->add_incoming_chunk(processChunk);
           }
        }
    }
    accessUDTP = NULL;

}
void* UDTP::flowThread(void* args)
{

    UDTPThreadFlow* myFlowThread = (UDTPThreadFlow*) args;
    while(myFlowThread->is_alive() && myFlowThread->get_thread_type() == INCOMING)
    {
        struct sockaddr_in incomingAddress;
        UDTPPacketHeader packetDeduction;
        if((recvfrom(myFlowThread->flow_socket(), &packetDeduction, sizeof(UDTPPacketHeader), 0, (struct sockaddr*)&incomingAddress, (socklen_t*)sizeof incomingAddress)))
        {
            myFlowThread->udtp()->display_msg("Received Packet Header from Flow Thread");
        UDTPPacket* incomingData = 0;
        bool deductionVerified = true;
            switch(packetDeduction.packetType)
                {
                    case Chunk:
                                myFlowThread->udtp()->display_msg("Chunk received from Flow Thread");
                                incomingData = new UDTPChunk(packetDeduction);
                                recv(myFlowThread->flow_socket(), (char*)incomingData->write_to_buffer(), incomingData->get_packet_size(), 0);

                    break;
                    default:
                            deductionVerified = false;
                             myFlowThread->udtp()->display_msg("Could not identify packet within Flow Thread!");
                    break;
                }
                if(deductionVerified){
                    myFlowThread->udtp()->add_queue_flow((UDTPChunk*)incomingData);
                    sem_post(&myFlowThread->udtp()->_semFlowPacketQueue);
                    myFlowThread->udtp()->display_msg("Chunk has been sent to chunk queue for identification.");
                }
                    myFlowThread->udtp()->display_msg("Flow Thread has finished processing.");

        }
        poll(0,0,50);
    }

    while(myFlowThread->is_alive() && myFlowThread->get_thread_type() == OUTGOING){
    }
   delete myFlowThread;
}
bool UDTP::start_queue_threads(){
    pthread_create(&_listenPacketQueueThreadHandler, NULL, &UDTP::listenQueueThread, (UDTP*) this);
    pthread_tryjoin_np(_listenPacketQueueThreadHandler, NULL);

    pthread_create(&_flowPacketQueueThreadHandler, NULL, &UDTP::flowQueueThread, (UDTP*)this);
    pthread_tryjoin_np(_flowPacketQueueThreadHandler, NULL);

    self_peer()->set_init_process_complete(QUEUE_THREADS);

    return true;
}
bool UDTP::start_listen_thread()
{

    pthread_create(&_listenThreadHandler, NULL, &UDTP::listenThread, (UDTP*)this);
    pthread_tryjoin_np(_listenThreadHandler, NULL);
    self_peer()->set_init_process_complete(LISTEN_THREAD);

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

TransferReturn UDTP::send_file(std::string path){
    if(!self_peer()->check_init_process(COMPLETE)) return SELF_NOT_READY;
    UDTPFile* sendFile = new UDTPFile(path);
    if(!sendFile->check_file_exist()) return FILE_NOT_EXIST;
    sendFile->retrieve_info_from_local_file();

    if(get_socket_type() == HOST) sendFile->set_file_id(get_next_file_id());
    if(get_socket_type() == PEER) sendFile->set_file_id(0); /*PEERS always send a zero!*/

    UDTPHeader* requestFileSend = new UDTPHeader;
    sendFile->pack_to_header(*requestFileSend); /*Packs it in!*/


    requestFileSend->pack();
    send_listen_data(requestFileSend);
    delete requestFileSend;
}

TransferReturn UDTP::get_file(std::string path){
    if(!self_peer()->check_init_process(COMPLETE)) return SELF_NOT_READY;
    UDTPFile* getFile  = new UDTPFile(path);
    if(getFile->check_file_exist()) return FILE_ALREADY_EXIST;
    getFile->set_info_to_zero();

    if(get_socket_type() == HOST) getFile->set_file_id(get_next_file_id());
    if(get_socket_type() == PEER) getFile->set_file_id(0); /*PEERS always send a zero!*/

    UDTPHeader* requestFileGet = new UDTPHeader;
    getFile->pack_to_header(*requestFileGet); /*Packs it in!*/


    requestFileGet->pack();
    send_listen_data(requestFileGet);
    delete requestFileGet;
}
