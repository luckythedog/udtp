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
    UDTPSetup();
        /*This creates a client UDTPSetup*/
        UDTPSetup(std::string ipAddress, unsigned int port) {
            _ipAddress = ipAddress;
            _port = port;
            _hasPassword = false;
        }
        /*This creates a server UDTPSetup, since servers really only need ports to start up*/
        UDTPSetup(unsigned int port){
            _port = port;
             _hasPassword = false;
        }

        const char* get_ip_address(){ return _ipAddress.c_str(); };
        unsigned int get_port() { return _port; };

        bool set_ip_address(std::string ipAddress) { _ipAddress = ipAddress;};
        bool set_port(unsigned int port) { _port = port; };
            /*Gettors for these are provided at UDTPSettings::get_chunk_size_agreement(), etc.*/
        bool set_password(std::string password){_password = password;};
        std::string get_password() { return _password;};

    private:
        std::string _ipAddress;
        unsigned int _port;
        std::string _password; /*Optional password for encrypting*/
        bool _hasPassword;
        /*Client related options*/
        unsigned int _chunkSizeAgreement;
        /*Server related options*/
        unsigned int _maxChunkSize;
        unsigned int _minChunkSize;
};


#endif
