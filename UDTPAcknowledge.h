#include "UDTPPacket.h"
#include <sstream>
class UDTPData;
class UDTP;
class UDTPPeer;
enum AcknowledgeType{
    AcknowledgePacketCount = 0xA0, /*Kind of like Dota 2 when you type -ping you see packet loss. We will keep a local integer of UDP packets sent and compare it between host and peer*/
    AcknowledgeMissing = 0xA1, /*This is a missing chunk request*/
    AcknowledgeMissingSent = 0xA2, /*All missing are sent!*/
    AcknowledgeAddThread = 0xA3, /*Request to add a flow socket*/
    AcknowledgeRemoveThread = 0xA4, /*Get rid of a flow socket*/ /******THESE ARE USED TO ADD AND REMOVE BASED ON MISSING PACKETS COUNT*****/
    AcknowledgeComplete = 0xA5 /*This notifies the other person (usually the puller) that they are complete. If it is responded with a
                                                    ResponseApproved -- that means both parties can close the files without worry. If one party
                                                    is sending a ResponseRejected, the other party will send an AcknowledgeComplete after
                                                    every AcknowledgeMissing.*/
};
class UDTPAcknowledge : public UDTPPacket{
    public:
        UDTPAcknowledge(UDTPPacketHeader packet); /*Unpacks an Acknowledgement*/

        UDTPAcknowledge(AcknowledgeType acknowledgeType, unsigned short fileID) {
            _header.packetType = Acknowledge;
            _fileID = fileID;
            _acknowledgeType = acknowledgeType;
           _responseCode = ResponseNone;
        };


        unsigned short get_file_id() { return _fileID; };
        bool set_file_id(unsigned short fileID) { _fileID = fileID;};

        unsigned short get_chunk_id() { return _chunkID;};
        bool set_chunk_id(unsigned short chunkID) { _chunkID = chunkID;};

        ResponseCode get_response_code(){ return _responseCode;};
        bool set_response_code(ResponseCode responseCode) { responseCode = _responseCode;};

        AcknowledgeType get_acknowledge_type() { return _acknowledgeType;};
        bool set_packets_count(unsigned int packetsCount){
            _packetsCount = packetsCount;
        }

        bool set_acknowledge_type(AcknowledgeType newType){
            _acknowledgeType = newType;
        }

        unsigned int get_packets_count(){
            return _packetsCount;
        }
        /* must impliment pure virtual functions */
        char* get_raw_buffer();
        bool pack();
        bool unpack();
        bool respond();
    private:
        AcknowledgeType _acknowledgeType;
        ResponseCode _responseCode;
        unsigned short _fileID;
        unsigned short _chunkID;
        unsigned int _packetsCount;
};
