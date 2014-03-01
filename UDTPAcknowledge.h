#include "UDTPPacket.h"
#include <sstream>
class UDTPData;
enum AcknowledgeType{
    AcknowledgeMissing = 0xA0, /*This is a missing chunk request*/
    AcknowledgeComplete = 0xA1 /*This notifies the other person (usually the puller) that they are complete. If it is responded with a
                                                    ResponseApproved -- that means both parties can close the files without worry. If one party
                                                    is sending a ResponseRejected, the other party will send an AcknowledgeComplete after
                                                    every AcknowledgeMissing.*/
};
class UDTPAcknowledge : public UDTPPacket{
    public:
        UDTPAcknowledge(UDTPData& _data); /*Unpacks an Acknowledgement*/

        UDTPAcknowledge(AcknowledgeType acknowledgeType, unsigned short fileID) {
            _packetType = Acknowledge;
            _missingChunksCount  = 0;
            _fileID = fileID;
            _acknowledgeType = acknowledgeType;
           _responseCode = ResponseNone;
        };

    bool add_missing_chunk(unsigned short chunkID) {
        std::stringstream tempMissingChunkStream;
        tempMissingChunkStream << _missingChunks << std::hex << chunkID; /*Each chunk is seperated by 2 bytes. So we know exactly where to cut!*/
        _missingChunks = tempMissingChunkStream.str();
        _missingChunksCount++;
    };
    const char* get_missing_chunks() { return _missingChunks.c_str();};
    unsigned short get_missing_chunks_count() { return _missingChunksCount;};

    unsigned short get_file_id() { return _fileID; };
    bool set_file_id(unsigned short fileID) { _fileID = fileID;};

    unsigned short get_chunk_id() { return _chunkID;};
    bool set_chunk_id(unsigned short chunkID) { _chunkID = chunkID;};

    ResponseCode get_response_code(){ return _responseCode;};
    bool set_response_code(ResponseCode responseCode) { responseCode = _responseCode;};

    AcknowledgeType get_acknowledge_type() { return _acknowledgeType;};
    private:
        unsigned short _missingChunksCount;
        std::string _missingChunks;
        AcknowledgeType _acknowledgeType;
        ResponseCode _responseCode;
        unsigned short _fileID;
        unsigned short _chunkID;
};
