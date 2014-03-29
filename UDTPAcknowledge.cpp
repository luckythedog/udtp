#include "UDTPAcknowledge.h"
/*Implementations after class forward declaration*/

 UDTPAcknowledge::UDTPAcknowledge(UDTPPacketHeader packet){
 }

char* UDTPAcknowledge::get_raw_buffer(){
    return 0;
}

bool UDTPAcknowledge::pack()
{
    return true;
}

bool UDTPAcknowledge::unpack()
{
    return true;
}

bool UDTPAcknowledge::respond()
{
    switch(get_acknowledge_type()){
        case AcknowledgeMissing:
            if(get_response_code() == ResponseNone){ /*Someone remotely is complaining about a chunk*/

            }
        break;
        case AcknowledgeComplete:
        break;
    }
    return true;
}
