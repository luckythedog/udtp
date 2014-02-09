#include "UDTPPacket.h"
class UDTPData;
class UDTPWhine;
class UDTPChunk : UDTPPacket{
    public:
    UDTPChunk();
    UDTPChunk(UDTPData& data); /*Unpacks a UDTPChunk*/

    unsigned short get_chunk_identifier() { return _chunkIdentifier;};
    bool set_chunk_identifier(unsigned short chunkIdentifier) { _chunkIdentifier = chunkIdentifier;};

    unsigned short get_file_identifier() { return _fileIdentifier; };
    bool set_file_identifier(unsigned short fileIdentifier) { _fileIdentifier = fileIdentifier;};
    unsigned short get_size_of_chunk() { return _sizeOfChunk;};
    bool set_size_of_chunk (unsigned short sizeOfChunk) { _sizeOfChunk = sizeOfChunk;};

    char* get_chunk_buffer() { return _chunkBuffer; };
    bool set_chunk_buffer(char* chunkBuffer) { _chunkBuffer = chunkBuffer;};

    bool get_verified() { return _isVerified;};
    bool set_verified(bool isVerified) { _isVerified = isVerified;};


    UDTPWhine& create_whine_from_chunk();

    private:
        char* _chunkBuffer; /*This is the raw chunk*/
        unsigned short _fileIdentifier;
        unsigned short _chunkIdentifier;
        unsigned short _sizeOfChunk;
        bool _isVerified;
};
