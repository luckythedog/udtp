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

class UDTPPath;
class UDTPFile;
class UDTPData;
class UDTPHeader;
class UDTPChunk;
class UDTPWhine;
class UDTPHandshake;


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

    UDTP(UDTPSetup setup)   /*Default constructor for both client and server*/
    {
        _setup = setup;
        _isAlive = false;
        _fileIdentifierCount = 0;
    };
    SocketReturn start(SocketType socketType);
    bool send_data(UDTPData& data);

    TransferReturn send_file(UDTPPath addressPath);
    TransferReturn get_file(UDTPPath addressPath);

    bool start_threads();
    bool process_handshake(UDTPHandshake& handshake);
    bool process_header(UDTPHeader& readHeader);
    bool process_path(UDTPPath& readPath);
    bool process_chunk(UDTPChunk& readChunk);
    bool process_whine(UDTPWhine& readWhine);


    bool stop(); /*Stop server/client. This will eject anything and everything no matter what.*/
private:
    UDTPSetup _setup;
    SocketType _socketType;
    bool _isAlive; /*Is it running?*/

    static void* listenThreadFunc(void*);
    static void* flowThreadsFunc(void*);
    pthread_t _listenThread; /*There will be only one primary listen thread*/
    pthread_t _flowThread; /*There will be numerous flow threads*/

    unsigned short _listenSocket; /*Holds TCP socket*/
    unsigned short _flowSocket; /*Holds UDP socket*/


    /*Server dedicated member variables*/
    unsigned short _fileIdentifierCount; /*Holds file identifier counts. This increments as each file goes on.*/

};

#endif
