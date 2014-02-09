#include "UDTPPacket.h"
class UDTPData;

class UDTPWhine : UDTPPacket{
    public:
        UDTPWhine(UDTPData& _data); /*Unpacks a whine*/
        UDTPWhine(unsigned short fileIdentifier, unsigned short chunkIdentifier) {
           _fileIdentifier = fileIdentifier;
            _chunkIdentifier = chunkIdentifier;
        };

    unsigned short get_file_identifier() { return _fileIdentifier; };
    bool set_file_identifier(unsigned short fileIdentifier) { _fileIdentifier = fileIdentifier;};

    unsigned short get_chunk_identifier() { return _chunkIdentifier;};
    bool set_chunk_identifier(unsigned short chunkIdentifier) { _chunkIdentifier = chunkIdentifier;};

    private:
        unsigned short _fileIdentifier;
        unsigned short _chunkIdentifier;
};
