#ifndef __UDTP_HANDSHAKE
#define __UDTP_HANDSHAKE
#include <stdint.h>
#include <vector>
#include "UDTPPacket.h"

class UDTPSetup;
class UDTP;
class UDTPPacketHeader;
class UDTPHandshake : public UDTPPacket{
    public:
    UDTPHandshake();
            UDTPHandshake(ResponseCode responseCode);
        UDTPHandshake(UDTPPacketHeader header);

        ~UDTPHandshake() {
            if(_raw != 0) delete _raw;
            //if(_peerSetup != 0) delete _peerSetup;
        }

        bool confirmHandshake();

        /* must impliment pure virtual functions */
        char* get_raw_buffer();
        bool pack();
        bool unpack();
        bool respond();
    private:
        unsigned short versionNumber;
        unsigned short chunkSizeAgreement;
};



#endif
