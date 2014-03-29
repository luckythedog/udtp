/*******************************************
*UDTP main API. This is the only thing that needs
*to be included for a developer.
*******************************************/
#ifndef __UDTP
#define __UDTP
/*C++ libraries*/
#include <vector>
#include <cstring>
#include <queue>
/*Sockets library and threading*/
#ifndef _WIN32
#include <poll.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <semaphore.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <asm/socket.h>
#include <unistd.h>
#else
#include <winsocks.h>
#include <windows.h>
#endif
/*Include necessary libraries of UDTP*/
#include "UDTPSetup.h"
#include "UDTPPacket.h"
#include "UDTPThread.h"
#define SO_REUSEPORT 15

class UDTPSetup;
class UDTPPath;
class UDTPFile;
/*PAckets!*/;
class UDTPHeader;
class UDTPAcknowledge;
class UDTPHandshake;
class UDTPAddress;
class UDTPPeer;
class UDTPChunk;
/*Threads!*/
class UDTPThreadFlow;

enum SocketType
{
    HOST,
    PEER
};
enum SocketReturn
{
    SUCCESS,
    INVALID_SETUP,
    ALREADY_RUNNING,
    SOCKET_NOT_INIT,
    COULD_NOT_START_MUTEX,
    COULD_NOT_BIND_TCP,
    COULD_NOT_BIND_UDP,
    COULD_NOT_LISTEN_TCP,
    COULD_NOT_CONNECT_TCP,
    COULD_NOT_START_THREADS
};
enum TransferReturn
{
    TRANSFER_APPROVED,
    TRANSFER_REJECTED,
    FILE_ALREADY_EXIST_LOCAL,
    FILE_ALREADY_EXIST_REMOTE,
    FILE_NOT_EXIST_LOCAL,
    FILE_NOT_EXIST_REMOTE
};
class UDTP
{
public:
    UDTP(UDTPSetup setup)
    {
        _myUDTP = setup;
        _isAlive = false;
        _fileIDCount = 0;
    }   /*Default constructor for both client and server*/

    SocketReturn start(SocketType socketType); /*Starts Listen TCP socket*/
    bool start_mutex(); /*Starts mutexes*/
    bool start_listen_socket(SocketType socketType);
    bool start_queue_threads();
    bool add_flow_thread(ThreadType type, unsigned int peerID = 0);
    bool remove_flow_thread(unsigned int peerID = 0);/*This remove one by one*/
    bool start_listen_thread(); /*Starts listen thread*/

    unsigned int add_peer(unsigned int listenSocketOfPeer); /*Adds peer and returns posID*/
    int find_peer_pos(unsigned int listenSocketOfPeer); /*Returns the positional ID of the peer*/
    bool remove_peer(unsigned int posID); /*Removes peer*/
    UDTPPeer* get_peer(unsigned int posID); /*Gets a peer from a specific position ID*/
    UDTPPeer* self_peer() { return _listPeers[0];}; /*Gets self peer which is stored at zero.*/
    int find_peer_using_address(sockaddr_in searchAddress);

    bool send_listen_data(UDTPPacket* packet); /*Starts listen*/

    SocketType get_socket_type();
    bool alive();

    TransferReturn send_file(UDTPPath addressPath);
    TransferReturn get_file(UDTPPath addressPath);
    UDTPFile* get_file_by_id(unsigned int fileID);

    static void* listenThread(void *args);
    static void* flowThread(void*);
    static void* processThread(void*);
    static void* listenQueueThread(void* args); /*Handles request and allows polling thread to continue*/
    static void* flowQueueThread(void* args); /*Processes Recv()'d chunks to identify them!*/

    void add_queue_listen(UDTPPacket* packet);
    void add_queue_flow(UDTPChunk* chunk);

    void display_msg(std::string message); /*Displays message on console*/
    bool stop(); /*Stop server/client. This will eject anything and everything no matter what.*/

    bool reset_file_id(){
        _fileIDCount = 0;
        return true;
    }

    unsigned short get_next_file_id(){
        _fileIDCount++;
        return _fileIDCount;
    }

    UDTPSetup* get_udtpsetup() {
        return &_myUDTP;
    }

private:


    UDTPSetup _myUDTP;
    SocketType _socketType;
    bool _isAlive; /*Is it running?*/

    /*Mutexes*/
    pthread_mutex_t _mutexFlowThread;
    pthread_mutex_t _mutexPeer;
    pthread_mutex_t _mutexFile;
    sem_t _semListenPacketQueue;
    sem_t _semFlowPacketQueue;


    pthread_t _listenThreadHandler; /*There will be only one primary listen thread*/
    pthread_t _listenPacketQueueThreadHandler;
    pthread_t _flowPacketQueueThreadHandler;
    /*For server use only!*/
    std::vector<UDTPPeer*> _listPeers; /*Peers that are connected, including themselves*/

    /*Both server and client use this*/
    unsigned int _listenSocket; /*Holds TCP socket*/
    UDTPThread* _listenThread;
    unsigned int _flowSocket; /*Holds UDP socket!*/
    std::queue<UDTPPacket*> _listenPacketQueue;
    std::queue<UDTPChunk*> _flowPacketQueue;
    std::vector<UDTPFile*> _activeFiles;
    /*Current file count*/
    unsigned int _flowThreadCount;
    unsigned int _fileIDCount;


};

#endif
