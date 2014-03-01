#ifndef __UDTP_HANDSHAKE
#define __UDTP_HANDSHAKE
#include <stdint.h>
#include "UDTPPacket.h"

class UDTPData;
enum HandshakeType{
    /*Sent through TCP*/
    HandshakeVersion = 0x51,
    HandshakeChunkSize = 0x52,
    HandshakeSocketsCount = 0x53,
    HandshakeReady = 0x54, /*Sent once everything is up and ready!*/
    /*Sent through UDP*/
    HandshakeSendAddress = 0x61 /*Sends server or client one single flow socket address. No data, just taken from UDP sockaddr_in, Should follow ResponseNone and ResponseApproved*/

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


        bool set_return_sockets_count(unsigned int amount){ _returnedSocketsCount = amount;};
       unsigned short get_return_sockets_count() { return _returnedSocketsCount;};
    private:
        unsigned short  _chunkSizeAgreement;
        unsigned short _version;

        unsigned short _returnedSocketsCount; /*How many sockets are they wanting to use? You cannot go past the server's choices*/
        HandshakeType _handshakeType;
        ResponseCode _responseCode;
};



#endif
