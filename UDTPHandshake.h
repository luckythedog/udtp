#ifndef __UDTP_HANDSHAKE
#define __UDTP_HANDSHAKE
#include <stdint.h>
#include "UDTPPacket.h"

class UDTPData;
enum HandshakeType{
    HandshakeVersion = 0x51,
    HandshakeChunkSize = 0x52
};
class UDTPHandshake : UDTPPacket{
    public:
        UDTPHandshake(UDTPData& data);
        UDTPHandshake(HandshakeType handshakeType, unsigned short tokenOne) {
            _handshakeType = handshakeType;
            _responseCode = ResponseNone; /*Unfulfilled handshake type*/
                containsTokens = 0;
            if(handshakeType == HandshakeVersion){
                containsTokens ^= 1;
                tokensValue[1] = tokenOne;
            }
            if(handshakeType == HandshakeChunkSize){
                containsTokens ^= 2;
                tokensValue[0] = tokenOne;
            }
        }

        HandshakeType get_handshake_type() { return _handshakeType; } /*gets header type*/
        bool set_handshake_type(HandshakeType handshakeType) { _handshakeType = handshakeType;};

        ResponseCode get_response_code(){ return _responseCode;};
        bool set_response_code(ResponseCode responseCode) { responseCode = _responseCode;};

        unsigned short& operator[] (const unsigned int index);



    private:
        uint8_t containsTokens; /*This is to save bandwidth*/
                                            /* 0 1    means only has version value. ^= 1
                                            /* 1 0  means only has chunk agreement. ^ = 2
                                            /* Turn them both on with ^= 3, then 1 1*/
                                            /*This is so we can pack data in optionally. There does not have to be definite space for all four of these tokens*/
        unsigned short tokensValue[2]; /*0 is Chunk size agreement, 1 is version value*/
        HandshakeType _handshakeType;
        ResponseCode _responseCode;
};



#endif
