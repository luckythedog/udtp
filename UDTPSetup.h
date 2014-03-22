/*******************************************
*UDTPSetup holds ip address and port number
*for both client and server setup. It also contains
*chunk size agreements and settings on chunk
*size restrictions.
*******************************************/
#ifndef __UDTPSETUP
#define __UDTPSETUP
#include <string>
#include <queue>
#include <iostream>
#include "UDTPSettings.h"

/*These are DEFAULT settings for UDTP*/
#define UDTP_VERSION_NUMBER 0x0001

#define UDTP_DEBUG_ENABLED true /*set this for debugging messages*/

#define DEFAULT_CHUNK_SIZE_AGREEMENT 420
#define DEFAULT_MAX_CHUNK_SIZE 1024
#define DEFAULT_MIN_CHUNK_SIZE 256
#define DEFAULT_ENCRYPT_AUTOMATICALLY false

#define DEFAULT_MAX_NUMBER_OF_FLOW_SOCKETS 5 /*Number of flow sockets opened per connection*/

#define DEFAULT_MAX_CONNECTIONS 5

struct UDTPSettings{
    /*Sets all defaults! These are used for global use. They can be edited with UDTPSetup.*/
    static unsigned short VERSION_NUMBER = UDTP_VERSION_NUMBER;

    bool DEBUG_ENABLED = UDTP_DEBUG_ENABLED;

    std::string ROOT_DIRECTORY = ""; /*Blank means it's just going to be where the exe is*/

    unsigned short CHUNK_SIZE_AGREEMENT  =  DEFAULT_CHUNK_SIZE_AGREEMENT; /*So everyone can access it. It's on default setting*/
    unsigned short MAX_CHUNK_SIZE = DEFAULT_MAX_CHUNK_SIZE;
    unsigned short MIN_CHUNK_SIZE = DEFAULT_MIN_CHUNK_SIZE;
    unsigned short MAX_NUMBER_OF_FLOW_SOCKETS = DEFAULT_MAX_NUMBER_OF_FLOW_SOCKETS;
};

class UDTPSetup{
    public:
    UDTPSetup(){ };
        /*This creates a client UDTPSetup*/
        UDTPSetup(std::string ip, unsigned int port) {
            _ip = ip.c_str();
            _port = port;
            _currentPort = port;
        }
        /*This creates a server UDTPSetup, since servers really only need ports to start up*/
        UDTPSetup(unsigned int port){
            _port = port;
            _currentPort = port;
        }

        const char* get_ip(){ return _ip.c_str(); };
        unsigned int get_port() { return _port; };

        bool set_ip(std::string ip) { _ip = ip;};
        bool set_port(unsigned int port) { _port = port; _currentPort = port;};
            /*Gettors for these are provided at _settings.get_chunk_size_agreement(), etc.*/


        bool set_number_of_flow_sockets(unsigned short numOfFlowSockets) { _settings.NUMBER_OF_FLOW_SOCKETS = numOfFlowSockets;} /*Server use only!*/
        bool get_number_of_flow_sockets() { return _settings.NUMBER_OF_FLOW_SOCKETS;};

        bool set_number_of_threads(unsigned short numOfThreads) {
            if(numOfThreads >= get_number_of_flow_sockets()){ /*Number of threads MUST be more than or equal to flow sockets*/
            _settings.NUMBER_OF_THREADS  = numOfThreads;
            return true;
            }else{
                return false;
            }
            };
        unsigned short get_number_of_threads() { return _settings.NUMBER_OF_THREADS;};

        bool set_min_chunk_size(unsigned short minChunkSize){ _settings.MIN_CHUNK_SIZE = minChunkSize; return true;};
        unsigned short get_min_chunk_size() { return _settings.MIN_CHUNK_SIZE;};

        bool set_max_chunk_size(unsigned short maxChunkSize){ _settings.MAX_CHUNK_SIZE = maxChunkSize;};
        unsigned short get_max_chunk_size() { return _settings.MAX_CHUNK_SIZE;};

        bool set_chunk_size_agreement(unsigned short chunkSizeAgreement){ _settings.CHUNK_SIZE_AGREEMENT = chunkSizeAgreement; return true;};
        unsigned short get_chunk_size_agreement() { return _settings.CHUNK_SIZE_AGREEMENT;};

        bool set_root_directory(std::string rootDirectory){ _settings.ROOT_DIRECTORY = rootDirectory; return true;};
        std::string get_root_directory() { return _settings.ROOT_DIRECTORY;};

        unsigned short get_version() { return _settings.VERSION_NUMBER;};

        static bool get_debug_enabled() { return _settings.DEBUG_ENABLED;};
        static bool set_debug_enabled(bool newValue) { _settings.DEBUG_ENABLED = newValue; };

        // TODO: Should fix port code to account for conflict
        bool add_available_port(unsigned int portReuse) { _reusablePorts.push(portReuse);};
        unsigned int reuse_available_port() {
            unsigned int portReuse = _reusablePorts.front();
             _reusablePorts.pop();
             return portReuse;
             };
        unsigned int get_next_available_port() {
            if(!_reusablePorts.empty()) return reuse_available_port();
                    unsigned int nextAvailablePort = _currentPort;
                    _currentPort++;
                    return nextAvailablePort;
            };

    private:
        std::string _ip;
        unsigned int _port;
        unsigned int _currentPort;
        std::queue<unsigned int> _reusablePorts;
        UDTPSettings _settings;
};


#endif
