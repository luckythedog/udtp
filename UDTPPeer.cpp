#include "UDTPPeer.h"
#include "UDTPAddress.h"
#include "UDTPThreadFlow.h"
#include "UDTPPacket.h"
#include "UDTPChunk.h"
#include "UDTPThreadProcess.h"
#include "UDTPAcknowledge.h"
#include "UDTP.h"
#include "UDTPFile.h"
UDTPPeer::~UDTPPeer()
{
    while(remove_thread()); /*Removes all flow threads!*/
    _udtp = NULL;
}

bool UDTPPeer::start_self_mutex(){

    if(check_init_process(MUTEX_SELF_INIT)) return false;
        if(sem_init(&_semProcessChunkThread,0,0)  != 0) return false;
        if(pthread_mutex_init(&_mutexActiveFiles, NULL) != 0) return false;
        set_init_process_complete(MUTEX_SELF_INIT);
}

bool UDTPPeer::add_thread()
{
    if(!is_online())  return false; /*Not online? Can't add!*/
    unsigned int flowSocket;
    flowSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if(flowSocket <0) return false;
    int optval = 1;
    if((setsockopt(flowSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))) < 0) return false;
    struct sockaddr_in flowAddress;
    if(host_local()){
        /*HOSTs have several destinations but luckily we stored the address on connect() here!*/
        /*Only downside is that when we receive messages from PEER on UDPs that we will not know where it is coming from!*/
        flowAddress = get_address();
    }else{
        /*PEER's only have one destination and that is the HOST!*/
        flowAddress.sin_addr.s_addr = inet_addr(udtp()->setup()->get_ip());
        flowAddress.sin_port = htons(udtp()->setup()->get_port());
        flowAddress.sin_family = AF_INET;
    }
    if( (bind(flowSocket, (struct sockaddr*)&flowAddress, sizeof(flowAddress))) < 0 )
    {
        perror("udp-bind");
        return false;
    }

    /*Start a flow thread!*/
    pthread_t flowThreadHandler;
    UDTPThreadFlow *newFlowThread = new UDTPThreadFlow((UDTP*)this, flowThreadHandler, flowSocket);
    newFlowThread->set_peer(this);
    pthread_create(&flowThreadHandler, NULL, UDTPPeer::flowThread, (UDTPThreadFlow*)newFlowThread);
    pthread_tryjoin_np(flowThreadHandler, NULL);
    /*Start a process thread!*/
    pthread_t processThreadHandler;
    UDTPThreadProcess *newProcessThread = new UDTPThreadProcess();
    newProcessThread->set_peer(this);
    pthread_create(&processThreadHandler, NULL, UDTPPeer::flowThread, (UDTPThreadProcess*)newProcessThread);
    pthread_tryjoin_np(processThreadHandler, NULL);



    std::cout << "PEER " << get_unique_id() << " has added a flow and process thread!"  << std::endl;
    increment_thread_count();
    return true;
}
bool UDTPPeer::start_file(UDTPFile* file){
        _activeFiles.push_back(file);
        _selfOverallFiles.push_back(file);
        file->begin_file_processing();

    }
bool UDTPPeer::remove_thread()
{
    if(get_thread_count()  <= 0) return false;

    UDTPThreadFlow *removeFlowThread = front_flow_thread();
    UDTPThreadProcess *removeProcessThread = front_process_thread();

    removeFlowThread->kill();

    removeProcessThread->kill();
    sem_post(&_semProcessChunkThread); /*Have to post one!*/

    close(removeFlowThread->flow_socket()); /****FIND A WAY TO CLOSE BUT KEEP WHATEVER IS REMAINING INSIDE!*/

    removeFlowThread = NULL;
    removeProcessThread = NULL;


    std::cout << "PEER " << get_unique_id() << " has removed a flow and process thread!"  << std::endl;
    decrement_thread_count();
    pop_thread();
    return true;
}
void* UDTPPeer::flowThread(void* args)
{

    UDTPThreadFlow* myFlowThread = (UDTPThreadFlow*) args;
    bool hostLocal = myFlowThread->peer()->host_local();
    while(myFlowThread->is_alive() && myFlowThread->get_thread_type() == INCOMING)
    {
        struct sockaddr_in incomingAddress;
        UDTPPacketHeader packetDeduction;
        if((recvfrom(myFlowThread->flow_socket(), &packetDeduction, sizeof(UDTPPacketHeader), 0, (struct sockaddr*)&incomingAddress, (socklen_t*)sizeof incomingAddress)))
        {
        std::cout << "PEER's file thread has received a packet. " << std::endl;
        UDTPPacket* incomingData = 0;
        bool deductionVerified = true;
            switch(packetDeduction.packetType)
                {
                    case Chunk:
                                std::cout << "PEER's file thread has deduced it as a Chunk!" << std::endl;
                                incomingData = new UDTPChunk(packetDeduction);
                                recv(myFlowThread->flow_socket(), (char*)incomingData->write_to_buffer(), incomingData->get_packet_size(), 0);

                    break;
                    default:
                            deductionVerified = false;
                              std::cout << "PEER's file thread could NOT identify packet!" << std::endl;
                    break;
                }
                if(deductionVerified){
                     std::cout << "PEER's file thread has deduced it as a Chunk!" << std::endl;
                    std::cout <<"Chunk has been sent to chunk queue for identification." << std::endl;
                   ((UDTPChunk*)incomingData)->set_receive_address(incomingAddress); /*Take address received from!*/
                    myFlowThread->peer()->add_chunk_to_process((UDTPChunk*)incomingData);
                    myFlowThread->peer()->increment_packet_count();
                     sem_post(&myFlowThread->peer()->_semProcessChunkThread);

                }
                         std::cout <<"Flow Thread has finished processing." << std::endl;
        }
        poll(0,0,50);
    }


   delete myFlowThread;
}
    void UDTPPeer::add_chunk_to_process(UDTPChunk* chunk){
            _processChunks.push(chunk);
    }

void* UDTPPeer::processThread(void* args){ /*Open as many chunk queue threads as INCOMING flow threads*/ /*HOST VS PEER sending! HOST will receive from different addresses! While PEER receives from one!*/
    UDTPThreadProcess* myProcessThread = (UDTPThreadProcess*) args;
    bool hostLocal = myProcessThread->peer()->host_local();
  /*HOST chunk processing! Reminder to delete chunk!*/
        while(myProcessThread->is_alive()){
            sem_wait(&myProcessThread->peer()->_semProcessChunkThread);
                UDTPChunk* processChunk = myProcessThread->peer()->get_next_chunk();
                if(processChunk != NULL){
                    UDTPFile* processFile = myProcessThread->peer()->search_file_by_id(processChunk->get_file_id());
                    if(processFile != NULL){
                        //*Write to file here!*/
                        processFile->add_incoming_chunk(processChunk);
                        myProcessThread->udtp()->display_msg("Chunk processing complete.");
                    }else{
                        delete processFile;
                    }
                }else{
                    delete processChunk;
                }
        }


   delete myProcessThread;
}
    UDTPFile* UDTPPeer::search_file_by_id(unsigned int fileID){ /*One search!*/
        for(unsigned int i=0; i<_selfOverallFiles.size(); i++){
            if(_selfOverallFiles[i]->get_file_id() == fileID){ /*Looks through PEER's own list of file addresses!*/
                return _selfOverallFiles[i];
            }
        }

        /*********This asks main framework UDTP to search a file in its overall listing and then afterwards PEER keeps a copy of it!***********/
        UDTPFile* searchFile = udtp()->get_file_with_id(fileID); /*Search for it in the main framework!*/

        if(searchFile != NULL){
            return searchFile;
            _selfOverallFiles.push_back(searchFile); /*Now adds it to the local list in order for faster searching*/
        }
        return NULL;
    }
 bool UDTPPeer::send_from(UDTPPacket* sendPacket){ /*PEER from the peer itself!*/
     std::cout << "PEER has sent out a packet" << std::endl;
    send(get_listen_socket(), sendPacket->get_raw_buffer(), sendPacket->get_packet_size(), 0); /*The UDTPPAclet's socket id is used to denote where to send the packet. The socket id is handled in the polling threads. For HOSTS, the socket id
                                                                                                                    will be the receiving socket's file descriptor. For PEER, it will always be its own _listenSocket being that that is where the central receiving and sending
                                                                                                                    happens.*/
 }

bool UDTPPeer::send_to(UDTPPacket* sendPacket){ /*Send from the server usually*/
    if(get_unique_id() != sendPacket->get_unique_id() ) {
        std::cout << "PACKET WILL NOT SEND: Packet's unique ID does not match with PEER's unique ID. This PEER may have disconnected (and removed from polling and peer list) and this packet was assigned that PEER's last position." << std::endl;
        return false;
    }
    std::cout << "HOST has sent out a packet" << std::endl;
    send(get_listen_socket(), sendPacket->get_raw_buffer(), sendPacket->get_packet_size(), 0); /*The UDTPPAclet's socket id is used to denote where to send the packet. The socket id is handled in the polling threads. For HOSTS, the socket id
                                                                                                                    will be the receiving socket's file descriptor. For PEER, it will always be its own _listenSocket being that that is where the central receiving and sending
                                                                                                                    happens.*/
}
