#ifndef __UDTP_SETTINGS
#define __UDTP_SETTINGS
#include <climits>
/*These are settings for UDTP, don't confuse this with UDTPSetup!*/
#define DEFAULT_CHUNK_SIZE_AGREEMENT 420
#define DEFAULT_MAX_CHUNK_SIZE 1024
#define DEFAULT_MIN_CHUNK_SIZE 256
#define DEFAULT_ENCRYPT_AUTOMATICALLY false

namespace UDTPSettings{
    static unsigned short VERSION_NUMBER = 0x0001;
    static bool ENCRYPT_AUTOMATICALLY = DEFAULT_ENCRYPT_AUTOMATICALLY;
    static unsigned short CHUNK_SIZE_AGREEMENT  =  DEFAULT_CHUNK_SIZE_AGREEMENT; /*So everyone can access it. It's on default setting*/
    static unsigned short MAX_CHUNK_SIZE = DEFAULT_MAX_CHUNK_SIZE;
    static unsigned short MIN_CHUNK_SIZE = DEFAULT_MIN_CHUNK_SIZE;

    static bool set_chunk_size_agreement(unsigned short newChunkSize){
        CHUNK_SIZE_AGREEMENT = newChunkSize;
    }
    static unsigned short get_chunk_size_agreement(){
        return CHUNK_SIZE_AGREEMENT;
    }


    static bool set_max_chunk_size(unsigned short newMaxChunkSize){
        MAX_CHUNK_SIZE = newMaxChunkSize;
    }
    static unsigned short get_max_chunk_size(){
        return MAX_CHUNK_SIZE;
    }

    static bool set_min_chunk_size(unsigned short newMinChunkSize){
        MIN_CHUNK_SIZE = newMinChunkSize;
    }
    static unsigned short get_min_chunk_size(){
        return MIN_CHUNK_SIZE;
    }

    static unsigned short get_next_file_identifier_count(){ /*Increments fileIdentifierCount and returns it*/
    }

    static bool set_encrypt_automatically(bool decideToEncrypt){
        ENCRYPT_AUTOMATICALLY = decideToEncrypt;
    }
    static bool get_encrypt_automatically(){
        return ENCRYPT_AUTOMATICALLY;
    }
};


#endif
