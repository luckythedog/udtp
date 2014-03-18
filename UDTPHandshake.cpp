#include "UDTPHandshake.h"
#include <string.h>
/*Implementations after class forward declarations*/

UDTPHandshake::UDTPHandshake(UDTPPacketHeader packet){

}

char* UDTPHandshake::get_raw_buffer()
{
    if(_raw == 0)
        _raw = new char[_header.packetSize];

    /* serialization order:
    /  UDTPPacketHeader _header;
    /  ResponseCode _responseCode;
    /  unsigned short  _chunkSizeAgreement;
    /  unsigned short _version;
    /  unsigned short _flowSocketsCount;
    /  unsigned short _destinationPort;
    /  HandshakeType _handshakeType;
    */
    char *_buf = _raw;
    memcpy(_buf, &_header, sizeof(_header));
    _buf += sizeof(_header);
    memcpy(_buf, &_responseCode, sizeof(_responseCode));
    _buf += sizeof(_responseCode);
    memcpy(_buf, &_chunkSizeAgreement, sizeof(_chunkSizeAgreement));
    _buf += sizeof(_chunkSizeAgreement);
    memcpy(_buf, &_version, sizeof(_version));
    _buf += sizeof(_version);
    memcpy(_buf, &_flowSocketsCount, sizeof(_flowSocketsCount));
    _buf += sizeof(_flowSocketsCount);
    memcpy(_buf, &_destinationPort, sizeof(_destinationPort));
    _buf += sizeof(_destinationPort);
    memcpy(_buf, &_handshakeType, sizeof(_handshakeType));
    _buf += sizeof(_handshakeType);


    return _raw;
}

bool UDTPHandshake::unpack()
{
    return true;
}
