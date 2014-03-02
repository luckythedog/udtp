#include "UDTP.h"
/*Packets, Implementation after forward declaration*/
#include "UDTPSetup.h"
#include "UDTPPacket.h"
#include "UDTPData.h"
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
        if(bind(_listenSocket, (struct sockaddr*)&listenAddress, (socklen_t)sizeof(listenAddress)) < 0) return COULD_NOT_BIND_TCP;
        if(listen(_listenSocket,0) < 0) return COULD_NOT_LISTEN_TCP;
        display_msg("HOST socket began listening");
        break;
    case PEER:
        listenAddress.sin_addr.s_addr = inet_addr(_myUDTP.get_ip());
        if(connect(_listenSocket, (struct sockaddr*)&listenAddress, (socklen_t)sizeof(listenAddress)) < 0) return COULD_NOT_CONNECT_TCP;
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
    if (pthread_mutex_init(&_mutexFlowThread, NULL) != 0) return false;
    if (pthread_mutex_init(&_mutexPeer, NULL) != 0) return false;
    return true;
}
bool UDTP::start_flow_sockets(unsigned int peerID) /*Could be self peer id too!*/
{
    if(get_socket_type() == PEER) peerID = 0; //Just making sure!*/
    for(unsigned int flowCount = 0; flowCount<_myUDTP.get_number_of_flow_sockets(); flowCount++)
    {
        pthread_t flowThread; /*Creates thread*/

        unsigned int flowSocket = socket(AF_INET, SOCK_DGRAM, 0); /*Creates socket*/

        struct sockaddr_in flowAddress; /*Creates struct*/
        memset(&flowAddress, 0, sizeof(flowAddress));
        flowAddress.sin_port = htons(_myUDTP.get_next_available_port()); /*Get next available port!*/
        flowAddress.sin_addr.s_addr = INADDR_ANY;
        flowAddress.sin_family = AF_INET;

        if(bind(flowSocket, (struct sockaddr*)&flowAddress, (socklen_t)sizeof(flowAddress)) < 0)
        {
            return false;
        }

        get_peer(peerID)->add_flow_thread(flowThread, flowSocket, flowAddress); /*Adds to peer list!*/
    }
    get_peer(peerID)->set_init_process_complete(FLOW_SOCKETS); /*Set to complete*/

    UDTPHandshake handshakeFlowLinkRequest(HandshakeFlowLinkRequest); /*Send out a flow link request when done*/
    UDTPData handshakeFlowLinkRequestData(handshakeFlowLinkRequest);
    handshakeFlowLinkRequestData.set_socket_id(get_peer(peerID)->get_listen_socket());
    send_listen_data(handshakeFlowLinkRequestData);

    return true;
}
bool UDTP::send_flow_links(unsigned int peerID)
{
    for(unsigned int i=0; i<_myUDTP.get_number_of_flow_sockets(); i++)
    {
        UDTPHandshake handshakeFlowLink(HandshakeFlowLink);
        handshakeFlowLink.set_destination_port(ntohs(get_peer(peerID)->get_thread(i)->get_socket_address().sin_port)); /*Server holds their own version of peers. This will be their port number or client's port*/
        UDTPData handshakeFlowLinkData(handshakeFlowLink);
        handshakeFlowLinkData.set_socket_id(get_peer(peerID)->get_listen_socket());
        send_listen_data(handshakeFlowLinkData);
    }
    return true;
}
bool UDTP::stop()
{
    _isAlive = false;

}
bool UDTP::start_flow_threads(unsigned int peerID)
{
    for(unsigned int threadIndex=0; threadIndex<_myUDTP.get_number_of_flow_sockets(); threadIndex++)
    {
        pthread_create((pthread_t*)get_peer(peerID)->get_thread(threadIndex)->get_pthread(), NULL, UDTP::flowThreadsFunc, (UDTPFlowThreadData*)get_peer(peerID)->get_thread(threadIndex));
        pthread_tryjoin_np(get_peer(peerID)->get_thread(threadIndex)->get_pthread(), NULL);
        get_peer(peerID)->get_thread(threadIndex)->set_alive();
    }

    get_peer(peerID)->set_init_process_complete(FLOW_THREADS);

    UDTPHandshake handshakeFlowThreads(HandshakeFlowThreads);
    UDTPData handshakeFlowThreadsData(handshakeFlowThreads);
    handshakeFlowThreadsData.set_socket_id(get_peer(peerID)->get_listen_socket());
    send_listen_data(handshakeFlowThreadsData);

    return true;
}
bool UDTP::send_listen_data(UDTPData& data)
{
    send(data.get_socket_id(), data.get_raw_buffer(), data.get_packet_size(), 0);
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

bool UDTP::process_header(UDTPHeader& readHeader)
{
    unsigned int posID = readHeader.get_peer_id(); /*Takes the peer ID from here*/
    if(get_socket_type() == PEER) posID  = 0;
    UDTPFile newFile(readHeader.get_path_of_file()); /*Look for the path sent*/
    switch (readHeader.get_header_type())
    {
    case HeaderPull: /*This guy wants to take a file from the server!*/
        if(readHeader.get_response_code() == ResponseApproved)
        {
            /*Prepare the file to be read() in!*/
        }
        if(readHeader.get_response_code() == ResponseFileNotFound)
        {
            /*Boo hoo message here*/
        }
        if(readHeader.get_response_code() == ResponseNone)
        {
            if(!newFile.check_file_exist())
            {
                readHeader.set_response_code(ResponseFileNotFound);
                UDTPData rejectedData(readHeader);
                send_listen_data(rejectedData);
                return false;
            }
            newFile.retrieve_info_from_local_file(); /*Get the information to send off*/
            readHeader.set_file_id(_myUDTP.get_next_file_id()); //*Get that unique file ID!*/
            readHeader.set_response_code(ResponseApproved); /*Add the approving response code!*/
            readHeader.set_size_of_file(newFile.get_size_of_file());
            readHeader.set_number_of_chunks(newFile.get_number_of_chunks());
            /*We fill in the header that just came in with our information.*/
        }
        break;
    case HeaderPush: /*This guy wants to give a file to the server*/
        if(readHeader.get_response_code() == ResponseApproved)
        {
            /*File has been approved! Prepare it here, cut the chunks and send them!*/
        }
        if(readHeader.get_response_code() == ResponseFileExistsAlready)
        {
            /*Boo hoo message here*/
        }
        if(readHeader.get_response_code() == ResponseNone)
        {
            if(newFile.check_file_exist())
            {
                readHeader.set_response_code(ResponseFileExistsAlready);
                UDTPData rejectedData(readHeader);
                send_listen_data(rejectedData);
                return false;
            }
            readHeader.set_response_code(ResponseApproved);
            readHeader.set_file_id(_myUDTP.get_next_file_id());
            newFile.set_number_of_chunks(readHeader.get_number_of_chunks());
            newFile.set_size_of_file(readHeader.get_size_of_file());
            /*We take their information.*/
            break;

            UDTPData outgoingData(readHeader); /*We pack it in a UDTPData and send it off!*/
            send_listen_data(outgoingData);
            /*Then we add that newFile into an array of activeFiles. From there we will handle it.*/
        }
    }
    return true;
}
bool UDTP::process_path(UDTPPath& readPath)
{
    unsigned int posID = readPath.get_peer_id(); /*Takes the peer ID from here*/
    if(get_socket_type() == PEER) posID  = 0;
    UDTPData outgoingData(readPath);
    send_listen_data(outgoingData);
    return true;
}
bool UDTP::process_chunk(UDTPChunk& readChunk)
{
    /*SHITTT!!*/
    return true;
}
bool UDTP::process_acknowledge(UDTPAcknowledge& readAcknowledge)
{
    unsigned int posID = readAcknowledge.get_peer_id(); /*Takes the peer ID from here*/
    if(get_socket_type() == PEER) posID  = 0;
    /*Still working on the filing!*/

    UDTPData outgoingData(readAcknowledge);
    send_listen_data(outgoingData);
    return true;
}
bool UDTP::process_handshake(UDTPHandshake& readHandshake)
{
    unsigned int posID = readHandshake.get_peer_id(); /*Takes the peer ID from here*/
    if(get_socket_type() == PEER) posID  = 0;
    switch(readHandshake.get_handshake_type())
    {
    case HandshakeVersion:
        if(readHandshake.get_response_code() == ResponseApproved)   /*Client has it approved*/
        {
            self_peer()->set_init_process_complete(VERSION_AGREE);
            return true;
        }
        if(readHandshake.get_response_code() == ResponseCriticalError)
        {
            display_msg("Your version number was not approved!");
            stop(); /*Disconnect entirely!*/
            return true;
        }
        if(readHandshake.get_response_code() == ResponseNone)  //*Client has pushed a request*/
        {
            if(readHandshake.get_version() != _myUDTP.get_version())
            {
                readHandshake.set_response_code(ResponseCriticalError);
            }
            else
            {
                get_peer(posID)->set_init_process_complete(VERSION_AGREE);
                readHandshake.set_response_code(ResponseApproved);
            }
        }
        break;
    case HandshakeChunkSize:
        if(readHandshake.get_response_code() == ResponseApproved)  /*Has it approved!*/
        {
            _myUDTP.set_chunk_size_agreement(readHandshake.get_chunk_size_agreement());
            self_peer()->set_init_process_complete(CHUNKSIZE_AGREE);
            return true;
        }
        if(readHandshake.get_response_code() == ResponseCriticalError)
        {
            display_msg("Your chunk size agreement was not approved!");
            stop(); /*Disconnect entirely!*/
            return true;
        }
        if(readHandshake.get_response_code() == ResponseNone)  /*Client has pushed a request*/
        {
            if(readHandshake.get_chunk_size_agreement() <= _myUDTP.get_max_chunk_size() && readHandshake.get_chunk_size_agreement() >= _myUDTP.get_min_chunk_size())
            {
                get_peer(posID)->set_init_process_complete(CHUNKSIZE_AGREE);
                readHandshake.set_response_code(ResponseApproved);
            }
            else
            {
                readHandshake.set_response_code(ResponseCriticalError);
            }
        }

        break;
    case HandshakeFlowSocket: //*Client requests this upon connecting!*/
        if(readHandshake.get_response_code() == ResponseApproved)
        {
            start_flow_sockets(posID);
            return true;
        }
        if(readHandshake.get_response_code() == ResponseNone)
        {
            start_flow_sockets(posID);
            readHandshake.set_response_code(ResponseApproved);
        }

        break;
    case HandshakeFlowLinkRequest:
        if(readHandshake.get_response_code() == ResponseRetry)
        {
            readHandshake.set_response_code(ResponseNone); /*Send once again!*/
        }
        if(readHandshake.get_response_code() == ResponseApproved)
        {
            send_flow_links(posID);
        }
        if(readHandshake.get_response_code() == ResponseNone)
        {
            if(get_peer(posID)->check_init_process(FLOW_SOCKETS))  /*Check if ready then go for it!*/
            {
                readHandshake.set_response_code(ResponseApproved);
            }
            else
            {
                readHandshake.set_response_code(ResponseRetry);
            }
        }
        break;
    case HandshakeFlowLink:
        if(readHandshake.get_response_code() == ResponseApproved)
        {
            get_peer(posID)->find_thread_with_port(readHandshake.get_destination_port())->set_approved();

            if(get_peer(posID)->check_all_flow_threads_approved())  /*Exit strategy!*/
            {
                get_peer(posID)->set_init_process_complete(FLOW_ADDRESSES);
                start_flow_threads(posID);
            }
            return true;
        }
        if(readHandshake.get_response_code() == ResponseNone)
        {
            struct sockaddr_in destinationAddress;
            destinationAddress.sin_port = ntohs(readHandshake.get_destination_port()); /*Only needs the port from FLOW socket!*/
            destinationAddress.sin_family = AF_INET;
            destinationAddress.sin_addr.s_addr = get_peer(posID)->get_listen_address().sin_addr.s_addr; /*Takes IP Address from LISTEN socket.*/

            get_peer(posID)->get_next_thread_link_needed()->set_destination_address(destinationAddress);
            get_peer(posID)->get_next_thread_link_needed()->set_linked(); /*Sets destination and sets it as linked*/
            readHandshake.set_response_code(ResponseApproved);
        }
        break;
    case HandshakeFlowThreads:
        if(readHandshake.get_response_code() == ResponseApproved)
        {
            get_peer(posID)->set_init_process_complete(FLOW_THREADS);
            send_peer_init_completed(posID);
            return true;
        }
        if(readHandshake.get_response_code() == ResponseRetry)
        {
            readHandshake.set_response_code(ResponseNone); /*Resend again!*/
        }
        if(readHandshake.get_response_code() == ResponseNone)
        {
            if(get_peer(posID)->check_all_flow_threads_alive())  /*Both checks out to be working just good!*/
            {
                readHandshake.set_response_code(ResponseApproved);
            }
            else
            {
                readHandshake.set_response_code(ResponseRetry);
            }
        }
        break;
    case HandshakeComplete:
        if(readHandshake.get_response_code() == ResponseApproved)
        {
            get_peer(posID)->set_init_process_complete(COMPLETE);
            return true;
        }
        if(readHandshake.get_response_code() == ResponseCriticalError)
        {
        }
        if(readHandshake.get_response_code() == ResponseRetry)
        {
            readHandshake.set_response_code(ResponseNone);
        }
        if(readHandshake.get_response_code() == ResponseNone)
        {
            if(!get_peer(posID)->check_init_process(LISTEN_SOCKET)) readHandshake.set_response_code(ResponseCriticalError);
            if(!get_peer(posID)->check_init_process(LISTEN_THREAD)) readHandshake.set_response_code(ResponseCriticalError);
            if(!get_peer(posID)->check_init_process(VERSION_AGREE)) readHandshake.set_response_code(ResponseCriticalError);
            if(!get_peer(posID)->check_init_process(CHUNKSIZE_AGREE)) readHandshake.set_response_code(ResponseCriticalError);
            if(!get_peer(posID)->check_init_process(FLOW_SOCKETS)) readHandshake.set_response_code(ResponseCriticalError);
            if(!get_peer(posID)->check_init_process(FLOW_ADDRESSES)) readHandshake.set_response_code(ResponseCriticalError);
            if(readHandshake.get_response_code() == ResponseCriticalError) get_peer(posID)->set_offline(); /*Set to offline!*/

            if(get_peer(posID)->check_init_process(FLOW_THREADS))   /*Last stop! Either approve or retry.*/
            {
                get_peer(posID)->set_init_process_complete(COMPLETE);
                readHandshake.set_response_code(ResponseApproved);
            }
            else
            {
                readHandshake.set_response_code(ResponseRetry);
            }

        }
        break;
    }

    UDTPData outgoingData(readHandshake);
    send_listen_data(outgoingData);
    return true;
}
bool UDTP::send_peer_init_completed(unsigned int peerID)
{
    UDTPHandshake handshakeComplete(HandshakeComplete);
    UDTPData dataComplete(handshakeComplete);
    dataComplete.set_peer_id(peerID);
    /*THIS MIGHT HAVE CAUSED THE SEG FAULT*/
    dataComplete.set_socket_id(get_peer(peerID)->get_listen_socket());
    send_listen_data(dataComplete);
    return true;
}
bool UDTP::send_required_packets()
{
    if(get_socket_type() == HOST) return false; /*Hosts don't need to send this. Peers need to request it!*/
    if(!self_peer()->check_init_process(VERSION_AGREE))
    {
        UDTPHandshake versionToApprove(HandshakeVersion); /*Send handshake of version first!*/
        versionToApprove.set_version(_myUDTP.get_version());

        UDTPData dataOfHandshakeVersion(versionToApprove);
        send_listen_data(dataOfHandshakeVersion);
    }
    if(!self_peer()->check_init_process(CHUNKSIZE_AGREE))
    {
        UDTPHandshake chunkSizeToApprove(HandshakeChunkSize); /*Send chunksize agreements*/
        chunkSizeToApprove.set_chunk_size_agreement(_myUDTP.get_chunk_size_agreement());

        UDTPData dataOfHandshakeChunkSize(chunkSizeToApprove);
        send_listen_data(dataOfHandshakeChunkSize);
    }
    if(!self_peer()->check_init_process(FLOW_SOCKETS))  /*Gets address and port of the flow sockets*/
    {
        UDTPHandshake socketsCountRequest(HandshakeFlowSocket); /*Send a request to find out flow socket count on server-end*/
        UDTPData dataOfRequestSocketsCount(socketsCountRequest);
        send_listen_data(dataOfRequestSocketsCount);
    }
    return true;
}
void* UDTP::listenThreadFunc(void* args)
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
    while(accessUDTP->alive())
    {
        /*********Beginning of Server Code**********/
        if(accessUDTP->_socketType == HOST)
        {
            activeListenSocketsPtr = &activeListenSockets[0];
            activeListenActivity = poll(activeListenSocketsPtr, activeListenSockets.size(), -1);
            if(activeListenActivity < 0 ) perror("poll");
            if((activeListenSockets[0].revents & POLLIN))
            {
                struct sockaddr_in newPeerAddress;
                unsigned int newPeerListenSocket;
                if((newPeerListenSocket = accept(activeListenSockets[0].fd, (struct sockaddr*)&newPeerAddress, (socklen_t*)sizeof(newPeerAddress))) <0)
                {
                    perror("accept");
                }
                else
                {
                    pollfd newPollFd; //*Add to polling*/
                    newPollFd.fd = newPeerListenSocket;
                    newPollFd.events = POLLIN;
                    activeListenSockets.push_back(newPollFd);
                    /*Add to peers list. Both Pollfd and _listPeer are Parallel..*/
                    unsigned int newPeerID = accessUDTP->add_peer(newPeerListenSocket);
                    accessUDTP->get_peer(newPeerID)->set_listen_address(newPeerAddress);
                }
            }
            for(unsigned int i=1; i<activeListenSockets.size(); i++)
            {
                if(activeListenSockets[i].revents & POLLIN)
                {
                    char packetDeduction[3]; /*Takes Packet Size (unsigned int16 bits) at position 0 and 1 and Packet Type at position 2*/
                    if((recv(accessUDTP->_listenSocket, packetDeduction, 3, 0)) != 0)
                    {
                        UDTPData incomingData(packetDeduction); //*Use UDTPData class to get packet type more easily, and also get unsigned short of file size to read.
                        recv(accessUDTP->_listenSocket, (char*)incomingData.write_to_buffer(), incomingData.get_packet_size(), 0);
                        incomingData.set_socket_id(activeListenSockets[i].fd); /*Applies socket to UDTPData*/
                        incomingData.set_peer_id(i); /*Sets index as the peer's ID location*/
                        PacketType incomingPacketType = incomingData.get_packet_type();

                        /*USE packet type and build correctly. here*/
                        if(incomingPacketType == Header)
                        {
                            UDTPHeader readHeader(incomingData);
                            accessUDTP->process_header(readHeader);
                        }
                        if(incomingPacketType == Path)
                        {
                            UDTPPath readPath(incomingData);
                            accessUDTP->process_path(readPath);
                        }
                        if(incomingPacketType == Acknowledge)
                        {
                            UDTPAcknowledge readAcknowledge(incomingData);
                            accessUDTP->process_acknowledge(readAcknowledge);
                        }
                        if(incomingPacketType == Handshake)
                        {
                            UDTPHandshake readHandshake(incomingData);
                            accessUDTP->process_handshake(readHandshake);
                        }
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



        /*********Beginning of Client Code**********/
        if(accessUDTP->_socketType == PEER)
        {
            if(!sentRequiredPackets) sentRequiredPackets = accessUDTP->send_required_packets();

            activeListenActivity = poll(activeListenSocketsPtr, activeListenSockets.size(), -1);
            if(activeListenActivity < 0 ) perror("poll");

            if((activeListenSockets[0].revents & POLLIN))
            {
                char packetDeduction[3]; /*Takes Packet Size (unsigned int16 bits) at position 0 and 1 and Packet Type at position 2*/
                if((recv(accessUDTP->_listenSocket, packetDeduction, 3, 0)) != 0)
                {
                    UDTPData incomingData(packetDeduction); //*Use UDTPData class to get packet type more easily, and also get unsigned short of file size to read.
                    recv(accessUDTP->_listenSocket, (char*)incomingData.write_to_buffer(), incomingData.get_packet_size(), 0);
                    incomingData.set_socket_id(accessUDTP->self_peer()->get_listen_socket()); /*Applies socket to data set again*/
                    incomingData.set_peer_id(0); /*Set to self*/
                    PacketType incomingPacketType = incomingData.get_packet_type();

                    /*USE packet type and build correctly. here*/
                    if(incomingPacketType == Header)
                    {
                        UDTPHeader readHeader(incomingData);
                        accessUDTP->process_header(readHeader);
                    }
                    if(incomingPacketType == Path)
                    {
                        UDTPPath readPath(incomingData);
                        accessUDTP->process_path(readPath);
                    }
                    if(incomingPacketType == Acknowledge)
                    {
                        UDTPAcknowledge readAcknowledge(incomingData);
                        accessUDTP->process_acknowledge(readAcknowledge);
                    }
                    if(incomingPacketType == Handshake)
                    {
                        UDTPHandshake readHandshake(incomingData);
                        accessUDTP->process_handshake(readHandshake);
                    }
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
void UDTP::send_flow_data(UDTPFlowThreadData* threadFlowData, UDTPData& data)
{
    if(!threadFlowData->check_approved() && get_socket_type() == PEER)
    {
        struct sockaddr_in serverMainUDPAddress;
        serverMainUDPAddress.sin_port = htons(_myUDTP.get_port()); /*Main port, no additive*/
        serverMainUDPAddress.sin_addr.s_addr = inet_addr(_myUDTP.get_ip());
        serverMainUDPAddress.sin_family = AF_INET;
        sendto(threadFlowData->get_flow_socket(), data.get_raw_buffer(), data.get_packet_size(), 0, (struct sockaddr*)&serverMainUDPAddress, sizeof(serverMainUDPAddress));
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
    while(myFlowData->alive()){ /*Thread is good and running!*/
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