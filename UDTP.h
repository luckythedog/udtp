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
#else
#include <winsocks.h>
#include <windows.h>
#endif
/*Include necessary libraries of UDTP*/
#include "UDTPSetup.h"
#include "UDTPPacket.h"
class UDTPSetup;
class UDTPPath;
class UDTPFile;
class UDTPHeader;
class UDTPChunk;
class UDTPAcknowledge;
class UDTPHandshake;
class UDTPFlowThreadData;
class UDTPAddress;
class UDTPPeer;

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
    bool start_listen_thread(); /*Starts listen thread*/
    bool start_flow_sockets(unsigned int peerID); /*Starts flow sockets for a certain peer ID*/

    void send_flow_data(UDTPFlowThreadData* threadFlowData, UDTPPacket& packet); //*Different types of sockets, Listen goes to TCP and Flow goes to UDP*/
    bool send_flow_links(unsigned int peerID);
    bool start_flow_threads(unsigned int peerID);

    unsigned int add_peer(unsigned int listenSocketOfPeer); /*Adds peer and returns posID*/
    unsigned int find_peer_pos(unsigned int listenSocketOfPeer); /*Returns the positional ID of the peer*/
    bool remove_peer(unsigned int posID); /*Removes peer*/
    UDTPPeer* get_peer(unsigned int posID); /*Gets a peer from a specific position ID*/
    UDTPPeer* self_peer() { return _listPeers[0];}; /*Gets self peer which is stored at zero.*/
    bool send_peer_init_completed(unsigned int peerID);

    bool send_listen_data(UDTPPacket* packet); /*Starts listen*/

    SocketType get_socket_type();
    bool alive();

    TransferReturn send_file(UDTPPath addressPath);
    TransferReturn get_file(UDTPPath addressPath);

    static void* listenThreadFunc(void *args);
    static void* flowThreadsFunc(void*);
    static void* openFlowThread(void*);

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


    pthread_t _listenThread; /*There will be only one primary listen thread*/
    /*For server use only!*/
    std::vector<UDTPPeer*> _listPeers; /*Peers that are connected, including themselves*/

    /*Both server and client use this*/
    unsigned int _listenSocket; /*Holds TCP socket*/

    /*Current file count*/
    unsigned int _fileIDCount;


};

#endif
