#ifndef __UDTP_SETTINGS
#define __UDTP_SETTINGS
#include <climits>
#include <string>
#include <queue>
#include <iostream>
/*These are settings for UDTP, don't confuse this with UDTPSetup!*/
#define UDTP_VERSION_NUMBER 0x0001

#define UDTP_DEBUG_ENABLED true /*set this for debugging messages*/

#define DEFAULT_CHUNK_SIZE_AGREEMENT 420
#define DEFAULT_MAX_CHUNK_SIZE 1024
#define DEFAULT_MIN_CHUNK_SIZE 256
#define DEFAULT_ENCRYPT_AUTOMATICALLY false

#define DEFAULT_NUMBER_OF_FLOW_SOCKETS 5 /*Number of flow sockets opened per connection*/
#define DEFAULT_NUMBER_OF_THREADS 5

#define DEFAULT_MAX_CONNECTIONS 5



namespace UDTPSettings{
        /*Sets all defaults! These are used for global use. They can be edited with UDTPSetup.*/
    static unsigned short VERSION_NUMBER = UDTP_VERSION_NUMBER;
    static bool DEBUG_ENABLED = UDTP_DEBUG_ENABLED;

    static std::string ROOT_DIRECTORY = "\"; /*Blank means it's just going to be where the exe is*/

    static unsigned int FILE_ID_COUNT = 0;
    static unsigned short CHUNK_SIZE_AGREEMENT  =  DEFAULT_CHUNK_SIZE_AGREEMENT; /*So everyone can access it. It's on default setting*/
    static unsigned short MAX_CHUNK_SIZE = DEFAULT_MAX_CHUNK_SIZE;
    static unsigned short MIN_CHUNK_SIZE = DEFAULT_MIN_CHUNK_SIZE;
    static unsigned short NUMBER_OF_FLOW_SOCKETS = DEFAULT_NUMBER_OF_FLOW_SOCKETS;
    static unsigned short NUMBER_OF_THREADS = DEFAULT_NUMBER_OF_THREADS;


};

#endif
