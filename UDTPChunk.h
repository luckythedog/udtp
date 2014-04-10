#ifndef __UDTP_CHUNK
#define __UDTP_CHUNK
#include <netinet/in.h>
#include <arpa/inet.h>
#include "UDTPPacket.h"
class UDTPData;
class UDTPAcknowledge;
enum ChunkCode{
    EMPTY = 0x00,
    WHINE = 0x01,
    DONE = 0x02
};



class UDTPChunk : public UDTPPacket{
    public:
        UDTPChunk(UDTPPacketHeader header);
        UDTPChunk(UDTPData& data); /*Unpacks a UDTPChunk*/

        unsigned short get_chunk_id() { return _chunkID;};
        bool set_chunk_id(unsigned short chunkID) { _chunkID = chunkID;};

        unsigned short get_file_id() { return _fileID; };
        bool set_file_id(unsigned short fileID) { _fileID = fileID;};
        unsigned short get_size_of_chunk() { return _sizeOfChunk;};
        bool set_size_of_chunk (unsigned short sizeOfChunk) { _sizeOfChunk = sizeOfChunk;};

        char* get_chunk_buffer() { return _chunkBuffer; };
        bool set_chunk_buffer(char* chunkBuffer) { _chunkBuffer = chunkBuffer;};

        bool get_verified() { return _isVerified;};
        bool set_verified(bool isVerified) { _isVerified = isVerified;};

        bool set_receive_address(sockaddr_in receiveAddress){
            _receiveAddress = receiveAddress;
        }
        sockaddr_in get_receive_address(){
            return (_receiveAddress);
        }



        /* must impliment pure virtual functions */
        char* get_raw_buffer();
        bool pack();
        bool unpack();
        bool respond();


    private:
        sockaddr_in _receiveAddress;
        char* _chunkBuffer; /*This is the raw chunk*/
        unsigned short _fileID;
        unsigned short _chunkID;
        unsigned short _sizeOfChunk;
        bool _isVerified;
};

#endif
