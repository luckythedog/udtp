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
#define EMPTYPATH 0x00

SocketReturn UDTP::start(SocketType socketType)
{

    if(_isAlive) return ALREADY_RUNNING;
    _socketType = socketType;

    _listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(_listenSocket < 0) return SOCKET_NOT_INIT;

    struct sockaddr_in listenAddress; /*TCP*/
    memset(&listenAddress, 0, sizeof(listenAddress));
    listenAddress.sin_port = htons(_settings.get_port());
    listenAddress.sin_family = AF_INET;

    /*Important part*/
    switch (_socketType)
    {
    case HOST:
        listenAddress.sin_addr.s_addr = INADDR_ANY;
        if(bind(_listenSocket, (struct sockaddr*)&listenAddress, sizeof(listenAddress)) < 0) return COULD_NOT_BIND_TCP;
        if(listen(_listenSocket,0) < 0) return COULD_NOT_LISTEN_TCP;
        break;
    case PEER:
        listenAddress.sin_addr.s_addr = inet_addr(_settings.get_ip());
        if(connect(_listenSocket, (struct sockaddr*)&listenAddress, sizeof(listenAddress)) < 0) return COULD_NOT_CONNECT_TCP;
        break;
    }

    _initComplete |= (0x01 << 0); /*Set init TCP done*/

    UDTPPeer *selfPeer = new UDTPPeer(_listenSocket); /*Creates a self peer, no matter if it is client or server -- it will always be made!*/
    selfPeer->set_online();
    _listPeers.push_back(selfPeer);

    if(get_socket_type() == HOST)
    {
        _initComplete |= (0x01 << 2); /*Go ahead and show as completed, since server DOES not need to send a chunksize agreement!*/
        _initComplete |= (0x01 << 3); /* or version matching*/
        start_flow_sockets(_settings.get_number_of_flow_sockets()); /*HOST types go ahead and start their flow sockets.*/
    }

    if(!start_listen_thread()) return COULD_NOT_START_THREADS;

    _isAlive = true;

    return SUCCESS;

}
bool UDTP::start_flow_sockets(unsigned short withCount)
{
    for(unsigned int i =0; i<withCount; i++)
    {
        unsigned int flowSocket = socket(AF_INET,SOCK_DGRAM, 0);
        struct sockaddr_in flowAddress;
        flowAddress.sin_port = htons(_settings.get_port()+i); /*Default adds up one increment*/
        flowAddress.sin_family = AF_INET;
        flowAddress.sin_addr.s_addr = INADDR_ANY;
        if(bind(flowSocket, (struct sockaddr*)&flowAddress, sizeof(flowAddress))< 0)  /*Both server and client need to bind, since they will both be writing and reading*/
        {
            return false;
        }
        _listPeers[0]->add_address(flowAddress);
        _flowSockets.push_back(flowSocket);
    }

    _initComplete |= (0x01 << 4);
    start_flow_threads();
    return true;
}

bool UDTP::stop()
{
    _isAlive = false;
    for(unsigned int i=0; i<_flowThreads.size(); i++)
    {
        delete _flowThreads[i];
    }
}
bool UDTP::send_listen_data(UDTPData& data)
{
    if(data.get_socket_id() < 0) return false;

    if(get_socket_type() == HOST)
    {
        send(data.get_socket_id(), data.get_raw_buffer(), data.get_packet_size(), 0);
    }
    if(get_socket_type() == PEER)
    {
        send(_listenSocket, data.get_raw_buffer(), data.get_packet_size(), 0); /*Same thing, except the socket is _listenSocket which is the connection to the server!*/
    }
    return true;
}
bool UDTP::process_header(UDTPHeader& readHeader)
{
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
            readHeader.set_file_id(_settings.get_next_file_id()); //*Get that unique file ID!*/
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
            readHeader.set_file_id(_settings.get_next_file_id());
            newFile.set_number_of_chunks(readHeader.get_number_of_chunks());
            newFile.set_size_of_file(readHeader.get_size_of_file());
            /*We take their information.*/
            break;
            if(_listPeers[readHeader.get_peer_id()]->check_ready())
            {
            }

            UDTPData outgoingData(readHeader); /*We pack it in a UDTPData and send it off!*/
            send_listen_data(outgoingData);
            /*Then we add that newFile into an array of activeFiles. From there we will handle it.*/
        }
    }
    return true;
}
bool UDTP::process_path(UDTPPath& readPath)
{
    switch(readPath.get_response_code())
    {
    case ResponseNotReady:
        break;
    case ResponseListingFound:
        break;
    case ResponseFileExistsAlready:
        break;
    case ResponseFileNotFound:
        break;
    case ResponseNone: /*You're getting a request!*/
        if(readPath.get_filename() == EMPTYPATH )
        {
            if(readPath.get_directory() == EMPTYPATH)  /*Invalid path*/
            {
                readPath.set_response_code(ResponseInvalidPath);
            }
            if(readPath.get_directory() != EMPTYPATH)  /*Directory request*/
            {
                readPath.set_response_code(ResponseListingFound);
                /*Do some directory listing in this shit and put it in readPath's returnPath member variable*/
            }
        }
        if(readPath.get_filename() != EMPTYPATH)  /*Means it is an individual file!*/
        {
            /*Open a filestream using _settings.get_root_directory() + readPath.get_directory() + readPath.get_filename();
            Then respond with a ResponseFileNotFound, ResponseFileAlreadyExists or something*/
        }
        break;

    }


    if(!_listPeers[readPath.get_peer_id()]->check_ready())   /*Check if he is ready!!*/
    {
        readPath.set_response_code(ResponseNotReady);
    }/*Put this at the end so it takes sequential importance over existing case and switches*/
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
    switch(readAcknowledge.get_acknowledge_type())
    {
    case AcknowledgeMissing:
        if(readAcknowledge.get_response_code() == ResponseNotReady)
        {
            send_required_packets(); /*Try to ready up again!*/
        }
        if(readAcknowledge.get_response_code() == ResponseApproved)
        {
        }
        if(readAcknowledge.get_response_code() == ResponseRejected)
        {
        }
        if(readAcknowledge.get_response_code() == ResponseNone)
        {
        }
        break;
    case AcknowledgeComplete:
        if(readAcknowledge.get_response_code() == ResponseNotReady)
        {
            send_required_packets();  /*Try to ready up again!*/
        }
        if(readAcknowledge.get_response_code() == ResponseApproved)
        {
        }
        if(readAcknowledge.get_response_code() == ResponseRejected)
        {
        }
        if(readAcknowledge.get_response_code() == ResponseNone)
        {
        }
        break;
    }
    /*Still working on the filing!*/
    if(!_listPeers[readAcknowledge.get_peer_id()]->check_ready())   /*Check if he is ready!!*/
    {
        readAcknowledge.set_response_code(ResponseNotReady);
    }/*Put this at the end so it takes sequential importance over existing case and switches*/
    UDTPData outgoingData(readAcknowledge);
    send_listen_data(outgoingData);
    return true;
}
bool UDTP::process_handshake(UDTPHandshake& readHandshake)
{
    switch(readHandshake.get_handshake_type())
    {
    case HandshakeVersion:
        if(readHandshake.get_response_code() == ResponseApproved)   /*Client has it approved*/
        {
            _initComplete |= (0x01 << 2);
            return true;
        }
        if(readHandshake.get_response_code() == ResponseRejected)
        {
            return true;
        }
        if(readHandshake.get_response_code() == ResponseNone)  //*Client has pushed a request*/
        {
            if(readHandshake.get_version() != _settings.get_version())
            {
                readHandshake.set_response_code(ResponseRejected);
            }
            else
            {
                readHandshake.set_response_code(ResponseApproved);
            }
        }
        break;
    case HandshakeChunkSize:
        if(readHandshake.get_response_code() == ResponseApproved)  /*Has it approved!*/
        {
            _initComplete |= (0x01 << 3); /*Complete handshake chunksize*/
            return true;
        }
        if(readHandshake.get_response_code() == ResponseRejected)
        {
            return true;
        }
        if(readHandshake.get_response_code() == ResponseNone)  /*Client has pushed a request*/
        {
            if(readHandshake.get_chunk_size_agreement() <= _settings.get_max_chunk_size() && readHandshake.get_chunk_size_agreement() >= _settings.get_min_chunk_size())
            {
                readHandshake.set_response_code(ResponseApproved);
            }
            else
            {
                readHandshake.set_response_code(ResponseRejected);
            }
        }

        break;
    case HandshakeSocketsCount: //*Client requests this upon connecting!*/
        if(readHandshake.get_response_code() == ResponseRetry)
        {
            send_required_packets();
            return true;
        }
        if(readHandshake.get_response_code() == ResponseApproved) //They are approved with an amount.*/
        {
            _settings.set_number_of_flow_sockets(readHandshake.get_return_sockets_count());
            start_flow_sockets(_settings.get_number_of_flow_sockets()); /*Now we can start making UDP sockets!*/
            return true;
        }
        if(readHandshake.get_response_code() == ResponseNone)  //*They are requesting socket count, this is server doing the job*/
        {
            if(!(_initComplete & (0x01 << 4)))  /*Check if server is done with Flow sockets setup*/
            {
                readHandshake.set_response_code(ResponseRetry); /*Try again later when server has UDP sockets up!*/
            }
            else
            {
                readHandshake.set_return_sockets_count(UDTPSettings::NUMBER_OF_FLOW_SOCKETS);
                readHandshake.set_response_code(ResponseApproved);
            }
        }

        break;
    case HandshakeReady:
        if(readHandshake.get_response_code() == ResponseApproved)
        {
            _initComplete |= (0x01<<7); /*Set final complete here!*/
        }
        if(readHandshake.get_response_code() == ResponseNone)  /*Came from a client now server's setting back the work.*/
        {
            _listPeers[readHandshake.get_peer_id()]->set_ready(); /*Set ready in the list so they are allowed to request/send headers, paths, etc.*/
            readHandshake.set_response_code(ResponseApproved);
        }


        break;
    }

    UDTPData outgoingData(readHandshake);
    send_listen_data(outgoingData);
    return true;
}
bool UDTP::send_required_packets()
{
    if(!(_initComplete & (0x01 << 2)))  /*Not completed, then send, if it is completed don't waste bandwidth!*/
    {
        UDTPHandshake versionToApprove(HandshakeVersion); /*Send handshake of version first!*/
        UDTPData dataOfHandshakeVersion(versionToApprove);
        send_listen_data(dataOfHandshakeVersion);
    }
    if(!(_initComplete & (0x01 << 3)))
    {
        UDTPHandshake chunkSizeToApprove(HandshakeChunkSize); /*Send chunksize agreements*/
        UDTPData dataOfHandshakeChunkSize(chunkSizeToApprove);
        send_listen_data(dataOfHandshakeChunkSize);
    }
    if(!(_initComplete & (0x01 << 4)))
    {
        UDTPHandshake socketsCountRequest(HandshakeSocketsCount); /*Send a request to find out flow socket count on server-end*/
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

    std::queue<unsigned int> removeSocketSafely;
    std::queue<unsigned int> removePeerSafely;
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
                    UDTPPeer* newPeer = new UDTPPeer(newPeerListenSocket);
                    newPeer->set_online();
                    accessUDTP->_listPeers.push_back(newPeer);

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
                        removeSocketSafely.push(activeListenSockets[i].fd);
                        removePeerSafely.push(i); /*Since index or i is the same as the position of the peers on the list. We can just add that.*/
                    }
                }
            }
            while(!removeSocketSafely.empty())
            {
                /*Removes safely threaded*/
                activeListenSockets.erase(activeListenSockets.begin()+removeSocketSafely.front());
                accessUDTP->_listPeers.erase(accessUDTP->_listPeers.begin()+removePeerSafely.front());
                removeSocketSafely.pop();
                removePeerSafely.pop();
            }
        } /*End of server code*/
        /***********End of Server Code*************/



        /*********Beginning of Client Code**********/
        if(accessUDTP->_socketType == PEER && !sentRequiredPackets)
        {
            sentRequiredPackets = accessUDTP->send_required_packets();
        }
        if(accessUDTP->_socketType == PEER)
        {
            activeListenActivity = poll(activeListenSocketsPtr, activeListenSockets.size(), -1);
            if(activeListenActivity < 0 ) perror("poll");

            if((activeListenSockets[0].revents & POLLIN))
            {
                char packetDeduction[3]; /*Takes Packet Size (unsigned int16 bits) at position 0 and 1 and Packet Type at position 2*/
                if((recv(accessUDTP->_listenSocket, packetDeduction, 3, 0)) != 0)
                {
                    UDTPData incomingData(packetDeduction); //*Use UDTPData class to get packet type more easily, and also get unsigned short of file size to read.
                    recv(accessUDTP->_listenSocket, (char*)incomingData.write_to_buffer(), incomingData.get_packet_size(), 0);
                    incomingData.set_socket_id(accessUDTP->_listenSocket); /*Applies socket to data set again*/
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
        serverMainUDPAddress.sin_port = htons(_settings.get_port()); /*Main port, no additive*/
        serverMainUDPAddress.sin_addr.s_addr = inet_addr(_settings.get_ip());
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

void* UDTP::flowThreadsFunc(void* args)
{
//0x01<<6 here
    UDTPFlowThreadData *myFlowData = (UDTPFlowThreadData*) args;
    if(myFlowData->udtp()->get_socket_type() == PEER)
    {
        UDTPHandshake SendAddress(HandshakeSendAddress);
        if(myFlowData->get_id() == 0) SendAddress.set_response_code(ResponseNoneAndFlowLeader);else;SendAddress.set_response_code(ResponseNone);
        UDTPData dataOfSendAddress(SendAddress);
        myFlowData->udtp()->send_flow_data(myFlowData, dataOfSendAddress);
    }
    while(myFlowData->get_id() == 0){ /*Stops number 0*/

    }
    while(!myFlowData->check_approved()); /*Stop people who aren't number 0*/

    while(myFlowData->udtp()->alive())
    {



    }
    myFlowData  = NULL;
}

bool UDTP::start_flow_threads()
{
    unsigned int flowSocketsCounter = 0; /*Ghetto way but w/e*/
    unsigned int numOfFlowSockets  = _settings.get_number_of_flow_sockets(); /*Let's say we have 5 sockets, and 10 threads. We will try to make every socket encompass two threads.*/
    for(unsigned int i=0; i<_settings.get_number_of_threads(); i++) /*Thread count (works with the output of the system) and socket count are different!*/
    {
        pthread_t flowThread;
        UDTPFlowThreadData *flowThreadData = new UDTPFlowThreadData(_flowSockets[flowSocketsCounter], flowThread, i, (UDTP*)this);
        flowThreadData->add_recv_address(_listPeers[0]->get_address_at(flowSocketsCounter)); /*Take the addresses in of the Flow Sockets*/
        pthread_create(&flowThread, NULL, flowThreadsFunc, (UDTPFlowThreadData*)flowThreadData);
        pthread_tryjoin_np(flowThread, NULL);
        _flowThreads.push_back(flowThreadData);
        if(flowSocketsCounter < numOfFlowSockets)
        {
            flowSocketsCounter++;
        }
        else
        {
            flowSocketsCounter = 0;
        }
    }
    _initComplete |= (0x01 << 5); /*Completed Flow Threads*/
    return true;
}
bool UDTP::start_listen_thread()
{
#ifndef _WIN32
    pthread_create(&_listenThread, NULL, &UDTP::listenThreadFunc, (UDTP*)this);
    pthread_tryjoin_np(_listenThread, NULL);
    _initComplete |= (0x01 << 1); /*Set thread init done*/



#else

#endif
    return true;
}

