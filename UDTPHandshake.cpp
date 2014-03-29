#include "UDTPHandshake.h"
#include "UDTPSetup.h"
#include "UDTPPeer.h"
#include "UDTP.h"
#include <string.h>
/*Implementations after class forward declarations*/
    UDTPHandshake::UDTPHandshake(UDTPPacketHeader header){
        _header = header;
        _raw = new char[header.packetSize];
    }
    UDTPHandshake::UDTPHandshake(ResponseCode responseCode){
        _responseCode = responseCode;
        _header.packetSize =  sizeof(UDTPHandshake);
        _header.packetType = Handshake;
        _raw = 0;
    }
bool UDTPHandshake::pack(){
    return true;
}

bool UDTPHandshake::unpack(){
    return true;
}

bool UDTPHandshake::respond(){
    return true;
}
