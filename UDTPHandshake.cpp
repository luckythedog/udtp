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
 /***************************************************************************************************/
bool UDTPHandshake::pack(){
    return true;
}
 /**********************************************/
bool UDTPHandshake::unpack(){
    return true;
}
 /**********************************************/
bool UDTPHandshake::respond(){
    if(get_response_code() == ResponseStart){ /*From server requesting us to send everything!*/
        set_response_code(ResponseNone); /*We will set it to none and send it!*/
        return true;
    }
    if(get_response_code() == ResponseNone){ /*Now in server's perspective. The instance variables are actually the PEER's!*/
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
        _myUDTP->self_peer()->set_init_process_complete(HANDSHAKE); /*Pat himself on the back!*/
        return false; /*Returns false because it does not need to resend*/
    }
    if(get_response_code() == ResponseCriticalError){ /*boo hoo!!!*/
        _myUDTP->display_msg("You were rejected by the HOST");
    }
    return false;
}
/***************************************************************************************************/


bool UDTPHandshake::compare_settings(){
        if(versionNumber != _myUDTP->get_setup()->get_version()) {
                        return false;
        }
        if(chunkSizeAgreement < _myUDTP->get_setup()->get_min_chunk_size()) {
                        return false;
        }
        if(chunkSizeAgreement > _myUDTP->get_setup()->get_max_chunk_size()) {
            return false;
        }
        return true;
}
