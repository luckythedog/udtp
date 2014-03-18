#ifndef __UDTP_CHUNK
#define __UDTP_CHUNK

#include "UDTPPacket.h"
class UDTPData;
class UDTPAcknowledge;
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



        /* must impliment pure virtual functions */
        char* get_raw_buffer();
        bool unpack();


    private:
        char* _chunkBuffer; /*This is the raw chunk*/
        unsigned short _fileID;
        unsigned short _chunkID;
        unsigned short _sizeOfChunk;
        bool _isVerified;
};

#endif
