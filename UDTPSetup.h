/*******************************************
*UDTPSetup holds ip address and port number
*for both client and server setup. It also contains
*chunk size agreements and settings on chunk
*size restrictions.
*******************************************/
#ifndef __UDTPSETUP
#define __UDTPSETUP
#include <string>
#include "UDTPSettings.h"

class UDTPSetup{
    public:
    UDTPSetup(){ };
        /*This creates a client UDTPSetup*/
        UDTPSetup(std::string ip, unsigned int port) {
            _ip = ip.c_str();
            _port = port;

        }
        /*This creates a server UDTPSetup, since servers really only need ports to start up*/
        UDTPSetup(unsigned int port){
            _port = port;
        }

        const char* get_ip(){ return _ip.c_str(); };
        unsigned int get_port() { return _port; };

        bool set_ip(std::string ip) { _ip = ip;};
        bool set_port(unsigned int port) { _port = port; };
            /*Gettors for these are provided at UDTPSettings::get_chunk_size_agreement(), etc.*/


        bool set_number_of_flow_sockets(unsigned short numOfFlowSockets) { UDTPSettings::NUMBER_OF_FLOW_SOCKETS = numOfFlowSockets;} /*Server use only!*/
        bool get_number_of_flow_sockets() { return UDTPSettings::NUMBER_OF_FLOW_SOCKETS;};

        bool set_number_of_threads(unsigned short numOfThreads) {
            if(numOfThreads >= get_number_of_flow_sockets()){ /*Number of threads MUST be more than or equal to flow sockets*/
            UDTPSettings::NUMBER_OF_THREADS  = numOfThreads;
            return true;
            }else{
                return false;
            }
            };
        unsigned short get_number_of_threads() { return UDTPSettings::NUMBER_OF_THREADS;};

        bool set_min_chunk_size(unsigned short minChunkSize){ UDTPSettings::MIN_CHUNK_SIZE = minChunkSize; return true;};
        unsigned short get_min_chunk_size() { return UDTPSettings::MIN_CHUNK_SIZE;};

        bool set_max_chunk_size(unsigned short maxChunkSize){ UDTPSettings::MAX_CHUNK_SIZE = maxChunkSize;};
        unsigned short get_max_chunk_size() { return UDTPSettings::MAX_CHUNK_SIZE;};

        bool set_chunk_size_agreement(unsigned short chunkSizeAgreement){ UDTPSettings::CHUNK_SIZE_AGREEMENT = chunkSizeAgreement; return true;};
        unsigned short get_chunk_size_agreement() { return UDTPSettings::CHUNK_SIZE_AGREEMENT;};

        bool set_root_directory(std::string rootDirectory){ UDTPSettings::ROOT_DIRECTORY = rootDirectory; return true;};
        std::string get_root_directory() { return UDTPSettings::ROOT_DIRECTORY;};

        unsigned short get_version() { return UDTPSettings::VERSION_NUMBER;};

        bool reset_file_id_count(){ UDTPSettings::FILE_ID_COUNT = 0; return true;};
        unsigned short get_next_file_id(){
            UDTPSettings::FILE_ID_COUNT++;
            return UDTPSettings::FILE_ID_COUNT;
        }

    private:
        std::string _ip;
        unsigned int _port;
};


#endif
