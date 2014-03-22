#include "UDTPHandshake.h"
#include "UDTPSetup.h"
#include "UDTPPeer.h"
#include "UDTP.h"
#include <string.h>
/*Implementations after class forward declarations*/

UDTPHandshake::UDTPHandshake(UDTPPacketHeader packet){
    _header = packet;
    _raw = new char[_header.packetSize];
    _mySetup = 0;
    _peerSetup = 0;
}

char* UDTPHandshake::get_raw_buffer()
{
    if(_raw == 0)
        _raw = new char[_header.packetSize];

    /* the following will always be transmitted in the first 4 bytes */
    char *_buf = _raw;
    std::cout << "Packing _header" << std::endl;
    memcpy(_buf, &_header, sizeof(_header));
    _buf += sizeof(_header);

    std::cout << "Packing ResponseCode of: " << _responseCode << "." << std::endl;
    memcpy(_buf, &_responseCode, sizeof(_responseCode));
    _buf += sizeof(_responseCode);

    /* the remaining bytes will be dependent on handshake type */
    switch(_responseCode)
    {
    unsigned short temp;
    case HandshakeInitiation:
        // TODO: find out how to use memcpy without using temp variables! (UGH!)
        std::cout << "HOST is packing HandshakeInitialization.\n";
        temp = _mySetup->get_version();
        memcpy(_buf, &temp, sizeof(temp));
        _buf += sizeof(temp);
        temp = _mySetup->get_min_chunk_size();
        memcpy(_buf, &temp, sizeof(temp));
        _buf += sizeof(temp);
        temp = _mySetup->get_max_chunk_size();
        memcpy(_buf, &temp, sizeof(temp));
        _buf += sizeof(temp);
        std::cout << "HOST finished packing HandshakeInitialization.\n";
        break;
    case HandshakeResponse:
        std::cout << "CLIENT is packing HandshakeResponse.\n";
        temp = _mySetup->get_chunk_size_agreement();
        memcpy(_buf, &temp, sizeof(temp));
        _buf += sizeof(temp);
        break;
        std::cout << "CLIENT finished packing HandshakeResponse.\n";
    case HandshakeConfirmation:
        std::cout << "HOST is packing HandshakeConfirmation.\n";
        std::cout << "HOST finished packing HandshakeConfirmation.\n";
        break;
    }


    // shrink packet size to actual data size
    _header.packetSize = (_buf - _raw);
    // update _raw to reflect new header & packet size
    memcpy(_raw, &_header, sizeof(_header));

    return _raw;
}

bool UDTPHandshake::process(UDTP* myUDTP)
{
    _peerSetup = new UDTPSetup();

    /* header was already receiving in UDTP::listenThreadFunc packet deduction */
    /* the following will always be transmitted in the first byte */
    char *_buf = _raw;
    memcpy(&_responseCode, _buf, sizeof(_responseCode));
    _buf += sizeof(_responseCode);
    std::cout << "Unpacking ResponseCode of: " << _responseCode << "." << std::endl;

    /* the remaining bytes will be dependent on handshake type */
    switch(_responseCode)
    {
    case HandshakeInitiation:
        myUDTP->display_msg("CLIENT processing HandshakeInitiation from HOST.\n");
        _peerSetup->set_version(*_buf);
        _buf += sizeof(_peerSetup->get_version());
        _peerSetup->set_min_chunk_size(*_buf);
        _buf += sizeof(_peerSetup->get_min_chunk_size());
        _peerSetup->set_max_chunk_size(*_buf);
        _buf += sizeof(_peerSetup->get_max_chunk_size());
        _responseCode = HandshakeResponse;
        return(compareSetups());
        break;
    case HandshakeResponse:
        myUDTP->display_msg("HOST processing HandshakeResponse from CLIENT.\n");
        _peerSetup->set_chunk_size_agreement(*_buf);
        _buf += sizeof(_peerSetup->get_chunk_size_agreement());
        myUDTP->get_peer(get_peer_id())->set_chunk_size(_peerSetup->get_chunk_size_agreement());
        myUDTP->get_peer(get_peer_id())->set_init_process_complete(COMPLETE);
        _responseCode = HandshakeConfirmation;
        return true;
        break;
    case HandshakeConfirmation:
        myUDTP->display_msg("CLIENT received HandshakeConfirmation from HOST. Ready to transmit.\n");
        myUDTP->self_peer()->set_init_process_complete(COMPLETE);
        return false;
        break;
    default:
        myUDTP->display_msg("Unidentified Handhshake message received.\n");
        return false;
        break;
    }

}

bool UDTPHandshake::compareSetups()
{
    // this is only called by client
    // _mySetup = client setup
    // _peerSetup = host setup

    if((_peerSetup == 0) || (_mySetup == 0))
        return false;

    // TODO: Need to impliment bitmask
    /* compare each attribute */
    if(_peerSetup->get_version() != _mySetup->get_version()){
        // TODO: bitmask version fail
        return false;
    }

    if((_mySetup->get_chunk_size_agreement() >= _peerSetup->get_max_chunk_size()) ||
       (_mySetup->get_chunk_size_agreement() <= _peerSetup->get_min_chunk_size())) {
        // TODO: bitmask chunk_size_agreement fail
        return false;
    }

    // TOOD: bitmask success on all attributes
    return true;
}
