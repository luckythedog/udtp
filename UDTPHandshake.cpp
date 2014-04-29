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
        _header.packetSize =  sizeof(UDTPHandShakeData);
        _header.packetType = Handshake;
        _raw = new char[sizeof(_header)+_header.packetSize];
    }
 /***************************************************************************************************/
bool UDTPHandshake::pack(){
    /*Packing into raw! Getting ready to send! get_raw_buffer() will be used by send_listen_data()*/
        switch(get_response_code()){
        case ResponseStart:
        _data._responseCode = ResponseStart;
        memcpy(_raw, &_header, sizeof(_header));
        memcpy(_raw+sizeof(_header), &_data, sizeof(_data));
        return true;
        break;
        case ResponseNone:
        _data._chunkSizeAgreement = udtp()->setup()->get_chunk_size_agreement();
        _data._versionNumber = udtp()->setup()->get_version();
        memcpy(_raw, &_header, sizeof(_header));
        memcpy(_raw+sizeof(_header), &_data, sizeof(_data));
        return true;
        break;
        case ResponseApproved:
        case ResponseCriticalError:
        memcpy(_raw, &_header, sizeof(_header));
        return true;
        break;
    }
    return false;
}
 /**********************************************/
bool UDTPHandshake::unpack(){
    /*I would've added the serialization code from previous heavy commit but I didn't want to mess things up!*/
    /*Justin serialization goes here!*/
    _myUDTP->display_msg("Unpacking...");

    memcpy(&_data, _raw, _header.packetSize);
    _responseCode = _data._responseCode;
    switch(get_response_code()){
        case ResponseStart:
        _myUDTP->display_msg("Received ResponseStart.");
        return true;
        break;
        case ResponseNone:
        versionNumber = _data._versionNumber;
        chunkSizeAgreement = _data._chunkSizeAgreement;
        _myUDTP->display_msg("Received ResponseNone.");
        return true;
        break;
        case ResponseApproved:
        case ResponseCriticalError:
        _myUDTP->display_msg("Received ResponseApproved/Criticalerror.");
        return false;
        default:
        _myUDTP->display_msg("Received unknown handshake.");
        break;
    }

    return false;
}
 /**********************************************/
bool UDTPHandshake::respond(){
    if(get_response_code() == ResponseStart){ /*From server requesting us to send everything!*/
        _myUDTP->display_msg("Received ResponseStart.");
        set_response_code(ResponseNone); /*We will set it to none and send it!*/
        return true;
    }
    if(get_response_code() == ResponseNone){ /*Now in server's perspective. The instance variables are actually the PEER's!*/
        _myUDTP->display_msg("Received ResponseNone.");
        if(!compare_settings()) {
            set_response_code(ResponseCriticalError); /*Disgraceful PEER*/
            _myUDTP->get_peer(get_peer_id()) ->set_offline(); /*Kick him off the server*/
            return true;
        }
        set_response_code(ResponseApproved);
        _myUDTP->get_peer(get_peer_id())->set_init_process_complete(HANDSHAKE);
        return true;
    }
    if(get_response_code() == ResponseApproved){ /*Back to the peer talking!*/
        _myUDTP->display_msg("Received ResponseApproved.");
        _myUDTP->self_peer()->set_init_process_complete(HANDSHAKE); /*Pat himself on the back!*/
        return false; /*Returns false because it does not need to resend*/
    }
    if(get_response_code() == ResponseCriticalError){ /*boo hoo!!!*/
        _myUDTP->display_msg("Received ResponseCriticalError.");
        _myUDTP->display_msg("You were rejected by the HOST");
    }
    return false;
}
/***************************************************************************************************/


bool UDTPHandshake::compare_settings(){
        if(versionNumber != _myUDTP->setup()->get_version()) {
                        return false;
        }
        if(chunkSizeAgreement < _myUDTP->setup()->get_min_chunk_size()) {
                        return false;
        }
        if(chunkSizeAgreement > _myUDTP->setup()->get_max_chunk_size()) {
            return false;
        }
        return true;
}



