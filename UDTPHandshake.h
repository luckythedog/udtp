#ifndef __UDTP_HANDSHAKE
#define __UDTP_HANDSHAKE
#include <stdint.h>
#include <vector>
#include "UDTPPacket.h"

class UDTPData;
enum HandshakeType{
    /*Sent through TCP*/
    HandshakeStart = 0x50, /*This is sent out from the server to the client so the client will be flagged to send required packets*/
    HandshakeVersion = 0x51,
    HandshakeChunkSize = 0x52,
    HandshakeFlowSocket = 0x53,
    HandshakeFlowLinkRequest = 0x54,
    HandshakeFlowLink = 0x55,
    HandshakeFlowThreads = 0x56,
    HandshakeComplete  = 0x57 /*Sent once everything is up and ready!*/

};
class UDTPHandshake : public UDTPPacket{
    public:
        UDTPHandshake(UDTPData& data);
        UDTPHandshake(HandshakeType handshakeType) {
            _handshakeType = handshakeType;
            _responseCode = ResponseNone; /*Unfulfilled handshake type*/
        }

        HandshakeType get_handshake_type() { return _handshakeType; } /*gets header type*/
        bool set_handshake_type(HandshakeType handshakeType) { _handshakeType = handshakeType;};

        ResponseCode get_response_code(){ return _responseCode;};
        bool set_response_code(ResponseCode responseCode) { responseCode = _responseCode;};

        bool set_chunk_size_agreement(unsigned short amount) { _chunkSizeAgreement = amount;};
        unsigned short get_chunk_size_agreement() { return _chunkSizeAgreement;};

        bool set_version(unsigned short version){ _version = version;};
        unsigned short get_version() { return _version;};

        bool set_destination_port(unsigned short destinationPort) { _destinationPort = destinationPort;};
        unsigned short get_destination_port() { return _destinationPort;};


        bool set_flow_sockets_count(unsigned int amount){ _flowSocketsCount = amount;};
       unsigned short get_flow_sockets_count() { return _flowSocketsCount;};
    private:
        /*Deals with HandshakeChunkSize*/
        unsigned short  _chunkSizeAgreement;
        /*Deals with HandshakeVersion*/
        unsigned short _version;
        /*Deals with HandshakeFlowSockets*/
        unsigned short _flowSocketsCount; /*How many sockets are they wanting to use? You cannot go past the server's choices*/
        /*Deals with HandshakeFlowLink*/
        unsigned short _destinationPort;
        /*Deals with all*/
        HandshakeType _handshakeType;
        ResponseCode _responseCode;
};



#endif
