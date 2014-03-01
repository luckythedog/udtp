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
#include "UDTPSettings.h"
#include "UDTPSetup.h"
class UDTPSetup;
class UDTPPath;
class UDTPFile;
class UDTPData;
class UDTPHeader;
class UDTPChunk;
class UDTPAcknowledge;
class UDTPHandshake;
class UDTPFlowThreadData;
class UDTPAddress;
class UDTPPeer;

class UDTP;

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
    UDTP(UDTPSetup settings)
    {
        _initComplete = 0x00;
        _settings = settings;
        _isAlive = false;
    }   /*Default constructor for both client and server*/

    SocketReturn start(SocketType socketType);
    bool start_flow_sockets(unsigned short withCount);
    bool send_listen_data(UDTPData& data);
    void send_flow_data(UDTPFlowThreadData* threadFlowData, UDTPData& data); //*Different types of sockets, Listen goes to TCP and Flow goes to UDP*/

    SocketType get_socket_type() {
        return _socketType;
    }
        bool alive(){
        return _isAlive;
    };

    TransferReturn send_file(UDTPPath addressPath);
    TransferReturn get_file(UDTPPath addressPath);

    bool start_listen_thread();
    bool start_flow_threads();
    bool send_required_packets();
    bool process_handshake(UDTPHandshake& handshake);
    bool process_header(UDTPHeader& readHeader);
    bool process_path(UDTPPath& readPath);
    bool process_chunk(UDTPChunk& readChunk);
    bool process_acknowledge(UDTPAcknowledge& readAcknowledge);
    static void* listenThreadFunc(void *args);
    static void* flowThreadsFunc(void*);
    static void* openFlowThread(void*);

    bool stop(); /*Stop server/client. This will eject anything and everything no matter what.*/
private:
    unsigned char _initComplete;

    UDTPSetup _settings;
    SocketType _socketType;
    bool _isAlive; /*Is it running?*/



    pthread_t _listenThread; /*There will be only one primary listen thread*/
    std::vector<UDTPFlowThreadData*> _flowThreads; /*Flow thread holders*/
    /*For server use only!*/
    std::vector<UDTPPeer*> _listPeers; /*Peers that are connected, including themselves*/

    /*Both server and client use this*/
    unsigned int _listenSocket; /*Holds TCP socket*/
    std::vector<unsigned int> _flowSockets; /*Holds UDP socket*/



};

#endif
