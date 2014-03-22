#include "UDTPAcknowledge.h"
/*Implementations after class forward declaration*/

 UDTPAcknowledge::UDTPAcknowledge(UDTPPacketHeader packet){
 }

char* UDTPAcknowledge::get_raw_buffer(){
    return 0;
}

bool UDTPAcknowledge::process(UDTP* myUDTP)
{
    return true;
}
