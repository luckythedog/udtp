#include "UDTPPath.h"

UDTPPath::UDTPPath(UDTPPacketHeader header){
    // TODO
}

UDTPPath::UDTPPath()
{

}

char* UDTPPath::get_filename()
{
    return NULL;
}
char* UDTPPath::get_directory(){
    return NULL;
}

char* UDTPPath::get_raw_buffer(){
}

bool UDTPPath::process(UDTP* myUDTP)
{
    return true;
}
