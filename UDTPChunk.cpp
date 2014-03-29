#include "UDTPChunk.h"
/*Implementations after forward declaration*/
#include "UDTPAcknowledge.h"

UDTPChunk::UDTPChunk(UDTPPacketHeader _header){ /*Unpacking of a UDTPChunk from UDTPData*/
}

char* UDTPChunk::get_raw_buffer(){
}

bool UDTPChunk::pack(){
    return true;
}

bool UDTPChunk::unpack(){
    return true;
}

bool UDTPChunk::respond(){
    return true;
}

