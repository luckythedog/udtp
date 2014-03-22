#ifndef __UDTP_HANDSHAKE
#define __UDTP_HANDSHAKE
#include <stdint.h>
#include <vector>
#include "UDTPPacket.h"

class UDTPSetup;
class UDTP;

class UDTPHandshake : public UDTPPacket{
    public:

        UDTPHandshake(UDTPPacketHeader packet);

        UDTPHandshake(ResponseCode handshakeStatus) {
            _responseCode = handshakeStatus;
            _header.packetSize = sizeof(UDTPHandshake);
            _header.packetType = Handshake;
            _raw = 0;
            _mySetup = 0;
             _peerSetup = 0;
        }

        ~UDTPHandshake() {
            if(_raw != 0) delete _raw;
            //if(_peerSetup != 0) delete _peerSetup;
        }

        bool set_udtpsetup(UDTPSetup* setup) {
            _mySetup = setup;
        }

        bool confirmHandshake();

        /* must impliment pure virtual functions */
        char* get_raw_buffer();
        bool process(UDTP* myUDTP);
    private:
        /*Deals with all*/
        UDTPSetup* _mySetup;
        UDTPSetup* _peerSetup;
        bool compareSetups();
};



#endif
