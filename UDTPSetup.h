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

/*These are DEFAULT settings for UDTP*/
static const  unsigned short UDTP_VERSION_NUMBER = 0x0001;
static const bool UDTP_DEBUG_ENABLED = true;
static const unsigned short DEFAULT_CHUNK_SIZE_AGREEMENT = 420;
static const unsigned short DEFAULT_MAX_CHUNK_SIZE = 1024;
static const unsigned short DEFAULT_MIN_CHUNK_SIZE = 256;
static const unsigned short DEFAULT_MAX_CONNECTIONS = 5; /*For HOST!*/

static const unsigned short DEFAULT_STARTING_NUMBER_OF_FLOW_SOCKETS = 1; /*Starting number!*/
static const unsigned short DEFAULT_STARTING_NUMBER_OF_FILE_THREADS = 1;

static const unsigned short DEFAULT_MAX_NUMBER_OF_FLOW_SOCKETS = 5; /*For HOST. This is max number of sockets per PEER*/
static const unsigned short DEFAULT_MAX_NUMBER_OF_FILE_THREADS = 5; /*For both. Number of file threads per file*/
static const unsigned int DEFAULT_MAX_MEMORY_USAGE_PER_FILE = 5000; /*Zero means unlimited!*/

static const unsigned int DEFAULT_CRITICAL_LOSS_PACKET_PERCENTAGE = 80;
static const unsigned int DEFAULT_SATISFACTORY_LOSS_PACKET_PERCENTAGE = 95; /*This*/



struct UDTPSettings{
    /*Sets all defaults! These are used for global use. They can be edited with UDTPSetup.*/
    // TODO: We need to find a way to make this const and allow peerSetup to still compare versions
    unsigned short VERSION_NUMBER;

    bool DEBUG_ENABLED;

    std::string ROOT_DIRECTORY; /*Blank means it's just going to be where the exe is*/

    unsigned short CHUNK_SIZE_AGREEMENT; /*So everyone can access it. It's on default setting*/
    unsigned short MAX_CHUNK_SIZE;
    unsigned short MIN_CHUNK_SIZE;
    unsigned short STARTING_NUMBER_OF_FLOW_SOCKETS;
    unsigned short STARTING_NUMBER_OF_FILE_THREADS;
    unsigned short MAX_NUMBER_OF_FLOW_SOCKETS;
    unsigned short MAX_NUMBER_OF_FILE_THREADS;
    unsigned int MAX_MEMORY_USAGE_PER_FILE ;
    unsigned int CRITICAL_LOSS_PACKET_PERCENTAGE;
    unsigned int SATISFACTORY_LOSS_PACKET_PERCENTAGE;
};

class UDTPSetup{
    public:
    UDTPSetup(){ set_defaults(); };
        /*This creates a client UDTPSetup*/
        UDTPSetup(std::string ip, unsigned int port) {
            _ip = ip.c_str();
            _port = port;
            set_defaults();
        }
        /*This creates a server UDTPSetup, since servers really only need ports to start up*/
        UDTPSetup(unsigned int port){
            _port = port;
            set_defaults();
        }

        const char* get_ip(){ return _ip.c_str(); };
        unsigned int get_port() { return _port; };

        bool set_ip(std::string ip) { _ip = ip;};
        bool set_port(unsigned int port) { _port = port; };
            /*Gettors for these are provided at _settings.get_chunk_size_agreement(), etc.*/

        bool set_min_chunk_size(unsigned short minChunkSize){ _settings.MIN_CHUNK_SIZE = minChunkSize; return true;};
        unsigned short get_min_chunk_size() { return _settings.MIN_CHUNK_SIZE;};

        bool set_max_chunk_size(unsigned short maxChunkSize){ _settings.MAX_CHUNK_SIZE = maxChunkSize;};
        unsigned short get_max_chunk_size() { return _settings.MAX_CHUNK_SIZE;};

        bool set_chunk_size_agreement(unsigned short chunkSizeAgreement){ _settings.CHUNK_SIZE_AGREEMENT = chunkSizeAgreement; return true;};
        unsigned short get_chunk_size_agreement() { return _settings.CHUNK_SIZE_AGREEMENT;};

        bool set_root_directory(std::string rootDirectory){ _settings.ROOT_DIRECTORY = rootDirectory; return true;};
        std::string get_root_directory() { return _settings.ROOT_DIRECTORY;};

        bool set_version(unsigned short version) { _settings.VERSION_NUMBER = version; return true;};
        unsigned short get_version() { return _settings.VERSION_NUMBER;};

        bool get_debug_enabled() { return _settings.DEBUG_ENABLED;};
        bool set_debug_enabled(bool newValue) { _settings.DEBUG_ENABLED = newValue; };

        bool set_max_number_of_flow_sockets(unsigned short numOfFlowSockets) { _settings.MAX_NUMBER_OF_FLOW_SOCKETS = numOfFlowSockets;} /*Server use only!*/
        unsigned short get_max_number_of_flow_sockets() { return _settings.MAX_NUMBER_OF_FLOW_SOCKETS;};

        bool set_starting_number_of_flow_sockets(unsigned short amount) { _settings.STARTING_NUMBER_OF_FLOW_SOCKETS = amount;};
        unsigned short get_starting_number_of_flow_sockets(){ return _settings.STARTING_NUMBER_OF_FLOW_SOCKETS;};

        bool set_starting_number_of_file_threads(unsigned short amount){ _settings.STARTING_NUMBER_OF_FILE_THREADS = amount;};
        unsigned short get_starting_number_of_file_threads(){ return _settings.STARTING_NUMBER_OF_FILE_THREADS;};

        bool set_max_number_of_file_threads(unsigned short amount) { _settings.MAX_NUMBER_OF_FILE_THREADS = amount;};
        unsigned short get_max_number_of_file_threads() { return _settings.MAX_NUMBER_OF_FILE_THREADS;};



        bool set_max_memory_usage(unsigned short amount){
            _settings.MAX_MEMORY_USAGE_PER_FILE = amount;
        }
        int get_max_memory_usage() { return _settings.MAX_MEMORY_USAGE_PER_FILE;};

        unsigned int get_critical_loss_packet_percentage(){
            return _settings.CRITICAL_LOSS_PACKET_PERCENTAGE;
        }
        bool set_critical_loss_packet_percentage(unsigned int amount){
            _settings.CRITICAL_LOSS_PACKET_PERCENTAGE = amount;
        }

                unsigned int get_satisfactory_loss_packet_percentage(){
            return _settings.SATISFACTORY_LOSS_PACKET_PERCENTAGE;
        }
        bool set_satisfactory_loss_packet_percentage(unsigned int amount){
            _settings.SATISFACTORY_LOSS_PACKET_PERCENTAGE = amount;
        }
        // TODO: Should fix port code to account for conflict

        void set_defaults(){
            _settings.VERSION_NUMBER = UDTP_VERSION_NUMBER;
            _settings.DEBUG_ENABLED = UDTP_DEBUG_ENABLED;
            _settings.ROOT_DIRECTORY = ""; /*Blank means it's just going to be where the exe is*/

            _settings.CHUNK_SIZE_AGREEMENT = DEFAULT_CHUNK_SIZE_AGREEMENT; /*So everyone can access it. It's on default setting*/
            _settings.MAX_CHUNK_SIZE = DEFAULT_MAX_CHUNK_SIZE;
            _settings.MIN_CHUNK_SIZE = DEFAULT_MIN_CHUNK_SIZE;
            _settings.STARTING_NUMBER_OF_FLOW_SOCKETS = DEFAULT_STARTING_NUMBER_OF_FLOW_SOCKETS;
            _settings.STARTING_NUMBER_OF_FILE_THREADS = DEFAULT_STARTING_NUMBER_OF_FILE_THREADS;
            _settings.MAX_NUMBER_OF_FLOW_SOCKETS = DEFAULT_MAX_NUMBER_OF_FLOW_SOCKETS;
            _settings.MAX_NUMBER_OF_FILE_THREADS  = DEFAULT_MAX_NUMBER_OF_FILE_THREADS;
            _settings.MAX_MEMORY_USAGE_PER_FILE = DEFAULT_MAX_MEMORY_USAGE_PER_FILE;
            _settings.CRITICAL_LOSS_PACKET_PERCENTAGE = DEFAULT_CRITICAL_LOSS_PACKET_PERCENTAGE;
            _settings.SATISFACTORY_LOSS_PACKET_PERCENTAGE = DEFAULT_SATISFACTORY_LOSS_PACKET_PERCENTAGE;
        }

    private:
        std::string _ip;
        unsigned int _port;
        UDTPSettings _settings;
};


#endif
