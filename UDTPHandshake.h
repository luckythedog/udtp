#ifndef __UDTP_HANDSHAKE
#define __UDTP_HANDSHAKE
#include <stdint.h>
#include <vector>
#include "UDTPPacket.h"

class UDTPSetup;
class UDTP;
class UDTPPacketHeader;

struct UDTPHandShakeData
{
    ResponseCode _responseCode;
    unsigned short _versionNumber;
    unsigned short _chunkSizeRequest;
    unsigned short _chunkSizeAgreement;

};

class UDTPHandshake : public UDTPPacket{
    public:
    UDTPHandshake();
            UDTPHandshake(ResponseCode responseCode);
        UDTPHandshake(UDTPPacketHeader header);

        ~UDTPHandshake() {
            if(_raw != 0) delete _raw;
            //if(_peerSetup != 0) delete _peerSetup;
        }

        bool compare_settings();

        /* must impliment pure virtual functions */
        char* get_raw_buffer();
        bool pack();
        bool unpack();
        bool respond();
    private:
        UDTPHandShakeData _data;
        unsigned short versionNumber;
        unsigned short chunkSizeAgreement;
};



#endif
