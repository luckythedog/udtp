#include "UDTP.h"
/*Packets, Implementation after forward declaration*/
#include "UDTPPacket.h"
#include "UDTPData.h"
#include "UDTPPath.h"
#include "UDTPFile.h"
#include "UDTPHeader.h"
#include "UDTPChunk.h"
#include "UDTPWhine.h"
#include "UDTPHandshake.h"

SocketReturn UDTP::start(SocketType socketType)
{
    if(_isAlive) return ALREADY_RUNNING;
    _socketType = socketType;

    _listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    _flowSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if(_listenSocket < 0 || _flowSocket < 0) return SOCKET_NOT_INIT;

    struct sockaddr_in listenAddress; /*TCP*/
    memset(&listenAddress, 0, sizeof(listenAddress));
    listenAddress.sin_port = htons(_setup.get_port());
    listenAddress.sin_family = AF_INET;

    struct sockaddr_in flowAddress; /*UDP*/
    memset(&flowAddress, 0, sizeof(flowAddress));
    flowAddress.sin_port = htons(_setup.get_port());
    flowAddress.sin_family = AF_INET;

    /*Important part*/
    switch (_socketType)
    {
    case HOST:
        listenAddress.sin_addr.s_addr = INADDR_ANY;
        flowAddress.sin_addr.s_addr = INADDR_ANY;
        if(bind(_listenSocket, (struct sockaddr*)&listenAddress, sizeof(listenAddress)) < 0) return COULD_NOT_BIND_TCP;
        if(bind(_flowSocket, (struct sockaddr*)&flowAddress, sizeof(flowAddress)) < 0) return COULD_NOT_BIND_UDP;
        if(listen(_listenSocket,0) < 0) return COULD_NOT_LISTEN_TCP;
        break;
    case PEER:
        listenAddress.sin_addr.s_addr = inet_addr(_setup.get_ip_address());
        flowAddress.sin_addr.s_addr = inet_addr(_setup.get_ip_address());
        if(connect(_listenSocket, (struct sockaddr*)&listenAddress, sizeof(listenAddress)) < 0) return COULD_NOT_CONNECT_TCP;
        break;
    }

    if(!start_threads()) return COULD_NOT_START_THREADS;
    return SUCCESS;

}
bool UDTP::stop()
{
    _isAlive = false;
}
bool UDTP::send_data(UDTPData& data)
{
    if(data.get_from_socket() < 0) return false;
    while( (send(data.get_from_socket(), data.get_raw_buffer(), data.get_packet_size(), 0)) != data.get_packet_size()); /*Keep sending until it's all sent*/
    return true;
}
bool UDTP::process_header(UDTPHeader& readHeader)
{
    if(readHeader.get_response_code() != ResponseNone) return false; /*Get the fuck out of here if you aren't unfulfilled!*/
    UDTPFile newFile(readHeader.get_path_of_file()); /*Look for the path sent*/
   switch (readHeader.get_header_type()){
       case HeaderPull:
       if(!newFile.check_file_exist()) {
           readHeader.set_response_code(ResponseFileNotFound);
            UDTPData rejectedData(readHeader);
            send_data(rejectedData);
            return false;
       }
       newFile.retrieve_info_from_local_file(); /*Get the information to send off*/
        readHeader.set_file_identifier(UDTPSettings::get_next_file_identifier_count());
       readHeader.set_response_code(ResponseApproved);
       readHeader.set_size_of_file(newFile.get_size_of_file());
       readHeader.set_number_of_chunks(newFile.get_number_of_chunks());
        /*We fill in the header that just came in with our information.*/
       break;
       case HeaderPush:
       if(newFile.check_file_exist()) {
           readHeader.set_response_code(ResponseFileExistsAlready);
            UDTPData rejectedData(readHeader);
            send_data(rejectedData);
            return false;
       }
       readHeader.set_response_code(ResponseApproved);
       readHeader.set_file_identifier(UDTPSettings::get_next_file_identifier_count());
       newFile.set_number_of_chunks(readHeader.get_number_of_chunks());
       newFile.set_size_of_file(readHeader.get_size_of_file());
       /*We take their information.*/
       break;
       UDTPData outgoingData(readHeader); /*We pack it in a UDTPData and send it off!*/
       send_data(outgoingData);
       /*Then we add that newFile into an array of activeFiles. From there we will handle it.*/

   }
    return true;
}
bool UDTP::process_path(UDTPPath& readPath)
{
    if(readPath.get_response_code() != ResponseNone) return false;
    /*Sends back directory listing here*/
    return true;
}
bool UDTP::process_chunk(UDTPChunk& readChunk)
{
    /*SHITTT!!*/
    return true;
}
bool UDTP::process_whine(UDTPWhine& readWhine)
{
    /*Still working on the filing!*/
    return true;
}
bool UDTP::process_handshake(UDTPHandshake& readHandshake)
{
    if(readHandshake.get_response_code() != ResponseNone) return false; /*Needs to be unfulfilled*/

    switch(readHandshake.get_handshake_type())
    {
    case HandshakeVersion:
        if(readHandshake[1] == UDTPSettings::VERSION_NUMBER)
        {
            readHandshake.set_response_code(ResponseApproved);
        }
        else
        {
            readHandshake.set_response_code(ResponseRejected);
        }

        break;
    case HandshakeChunkSize:
        if((readHandshake[0] >= UDTPSettings::MIN_CHUNK_SIZE) && (readHandshake[0] <= UDTPSettings:: MAX_CHUNK_SIZE))
        {
            readHandshake.set_response_code(ResponseApproved);
        }
        else
        {
            readHandshake.set_response_code(ResponseRejected);
        }
        break;
    }
    UDTPData outgoingData(readHandshake);
    send_data(outgoingData);
    return true;
}
void* UDTP::listenThreadFunc(void* args)
{
    UDTP *accessUDTP = (UDTP*) args;
    std::vector<pollfd> activeListenSockets;
    pollfd masterPollFd;
    masterPollFd.fd = accessUDTP->_listenSocket;
    masterPollFd.events = POLLIN;
    activeListenSockets.push_back(masterPollFd);

    pollfd* activeListenSocketsPtr;
    int activeListenActivity;

    std::queue<unsigned int> removeSocketSafely;
    while(accessUDTP->_isAlive)
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
                unsigned int newPeer;
                if((newPeer= accept(activeListenSockets[0].fd, (struct sockaddr*)&newPeerAddress, (socklen_t*)sizeof(newPeerAddress))) <0)
                {
                    perror("accept");
                }
                else
                {
                    pollfd newPeerFd;
                    newPeerFd.fd = newPeer;
                    newPeerFd.events = POLLIN;
                    activeListenSockets.push_back(newPeerFd);
                }
            }
            for(unsigned int i=1; i<activeListenSockets.size(); i++)
            {
                if(activeListenSockets[i].revents & POLLIN)
                {
                    char deducePacket[3]; /*Takes Packet Size (unsigned int16 bits) at position 0 and 1 and Packet Type at position 2*/
                    if((recv(accessUDTP->_listenSocket, deducePacket, 3, 0)) != 0)
                    {
                        UDTPData incomingData(deducePacket);
                        recv(accessUDTP->_listenSocket, (char*)incomingData.write_to_buffer(), incomingData.get_packet_size(), 0);
                        incomingData.set_from_socket(activeListenSockets[i].fd); /*Takes the socket*/
                        PacketType incomingPacketType = incomingData.get_packet_type();
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
                        if(incomingPacketType == Chunk)
                        {
                            UDTPChunk readChunk(incomingData);
                            accessUDTP->process_chunk(readChunk);
                        }
                        if(incomingPacketType == Whine)
                        {
                            UDTPWhine readWhine(incomingData);
                            accessUDTP->process_whine(readWhine);
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
                    }
                }
            }
            while(!removeSocketSafely.empty())
            {
                /*Removes safely threaded*/
                activeListenSockets.erase(activeListenSockets.begin()+removeSocketSafely.front());
                removeSocketSafely.pop();
            }
        } /*End of server code*/
        /***********End of Server Code*************/



        /*********Beginning of Client Code**********/
        if(accessUDTP->_socketType == PEER)
        {
        }
        /***********End of Client Code*************/
    }

    accessUDTP = NULL;
}
void* UDTP::flowThreadsFunc(void* args)
{
    UDTP *accessUDTP = (UDTP*) args;
    while(accessUDTP->_isAlive)
    {
    }
    accessUDTP = NULL;
}

bool UDTP::start_threads()
{
#ifndef _WIN32
    pthread_create(&_listenThread, NULL, &UDTP::listenThreadFunc, (UDTP*)this);
    pthread_tryjoin_np(_listenThread, NULL);
    pthread_create(&_flowThread, NULL, &UDTP::flowThreadsFunc, (UDTP*)this);
    pthread_tryjoin_np(_flowThread, NULL);
#else

#endif
    return true;
}

