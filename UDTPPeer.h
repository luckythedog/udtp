#ifndef __UDTP_SOCKET
#define __UDTP_SOCKET
#include <vector>
#include <queue>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <poll.h>
#include <unistd.h>
#include "UDTPThread.h"

class UDTPPacket;
class UDTPChunk;
class UDTPThread;
class UDTPThreadFlow;
class UDTPThreadProcess;
class UDTPAddress;
class UDTPFile;
class UDTPAcknowledge;
class UDTP;
struct UDTPFileAddress;



enum initProcessEvent{
    LISTEN_SOCKET = 0x00, /*Has Listen socket started*/
    LISTEN_THREAD = 0x01, /*Has listen thread started*/
    MUTEX_FRAMEWORK_INIT = 0x02,
    MUTEX_SELF_INIT = 0x03,
    QUEUE_THREADS = 0x04,
    HANDSHAKE = 0x05,
    COMPLETE  = 0x06 /*Completion only verified by themselves*/
};


class UDTPPeer{ /*The way we can identify different UDP ports is that, we can identify that they have the same TCP socket. the TCP socket never changes!*/
   public:
    UDTPPeer();
    UDTPPeer(UDTP* udtp, unsigned int listenSocket) {
        _udtp = udtp;
        _listenSocket = listenSocket;
        _initProcess = 0x00;
        start_self_mutex();
       reset_thread_count();
        reset_packet_count();
        reset_processed_count();
        };/*starts off TCP.*/
    ~UDTPPeer();

    bool compare_listen_socket(unsigned int listenSocket){ if(_listenSocket== listenSocket) return true;else;return false;}

    bool set_listen_socket(unsigned int listenSocket) { _listenSocket = listenSocket; return true;};
    unsigned int get_listen_socket(){ return _listenSocket; };
    bool send_from(UDTPPacket* sendPacket);
    bool send_to(UDTPPacket* sendPacket);

    bool set_init_process_complete(initProcessEvent eventCompleted) {
            _initProcess |=  (0x01<<eventCompleted);
            return true;
    };
    bool check_init_process(initProcessEvent eventCompleted) {
        if(!(_initProcess & (0x01<<eventCompleted))) return false; /*Has not been completed*/
        return true;
    };
    bool set_unique_id(unsigned int newID){
        _uniqueID = newID;
    }
    unsigned int get_unique_id(){
        return _uniqueID;
    }
    unsigned int get_packets_count(){
        return _packetsCount;
    }
    void increment_packet_count(){
        _packetsCount++;
    }
    bool reset_packet_count() {
        _packetsCount = 0;
    }


    bool reset_thread_count(){
        _threadCount  = 0;
    }
    bool increment_thread_count(){
        _threadCount++;

    }
    bool decrement_thread_count(){
        _threadCount--;
    }

    unsigned short get_thread_count(){
        return _threadCount;
    }

    bool set_online(){ _connectionStatus = true;};
    bool set_offline(){ _connectionStatus = false;};
    bool is_online(){ return _connectionStatus;};
    bool set_address(sockaddr_in newAddress){ _address = newAddress; return true;};
    sockaddr_in get_address() { return _address;};
    unsigned short get_chunk_size() { return _chunkSize;};
    bool set_chunk_size(unsigned short chunkSize) { _chunkSize = chunkSize; return true;}

    UDTPFile* get_file_with_id(unsigned int idToLook){
    }
    UDTPThreadFlow* front_flow_thread(){
        return _flowThreads.front();
    }

    UDTPThreadProcess* front_process_thread(){
        return _processThreads.front();
    }
    bool pop_thread(){
        _flowThreads.pop();
        _processThreads.pop();
    }
    bool start_file(UDTPFile* newFile);
    bool add_thread();
    bool remove_thread();

    bool find_file_by_id(unsigned int fileID){
    }


    bool host_local(){ /*Denotes if this peer is host local*/
        return _hostLocal;
    }
    bool set_host_local(bool hostLocal){
        _hostLocal = hostLocal;
    }

    UDTP* udtp(){
        return _udtp;
    }
    static void* flowThread(void*);
    void add_chunk_to_process(UDTPChunk* chunk);

    UDTPChunk* get_next_chunk(){
        if(_processChunks.empty()) return NULL;
        UDTPChunk* nextChunk = _processChunks.front();
        _processChunks.pop();

        return nextChunk;
    }
    bool increment_processed_count(){
        _processedCount++;
    }
    unsigned int get_processed_count(){
        return _processedCount;
    }
    bool reset_processed_count(){
        _processedCount = 0;
    }
    UDTPFile* search_file_by_id(unsigned int id);
    static void* processThread(void* args);
    bool start_self_mutex();
    private:
    UDTP* _udtp;

    bool _hostLocal;
    unsigned char _initProcess; /*Holds 8 booleans for the startup process*/
    unsigned int _listenSocket;
    unsigned short _chunkSize;
    unsigned int _uniqueID; /*To detect disconnects*/
    bool _connectionStatus;
    sockaddr_in _address;
    pthread_mutex_t _mutexFlowThreads;
    pthread_mutex_t _mutexActiveFiles;
    sem_t _semProcessChunkThread;

    std::queue<UDTPThreadFlow*> _flowThreads;
    std::queue<UDTPThreadProcess*> _processThreads;
    std::queue<UDTPChunk*> _processChunks;
    std::vector<UDTPFile*> _selfOverallFiles; /*Copy of own file ID*/

    unsigned int _packetsCount;
    unsigned int _processedCount;
    unsigned short _threadCount;

    std::vector<UDTPFile*> _activeFiles;
};

#endif
