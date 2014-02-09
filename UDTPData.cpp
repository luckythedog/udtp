#include "UDTPData.h"
/*Implementations in the cpp file after forward declaration*/
#include "UDTPPath.h"
#include "UDTPHeader.h"
#include "UDTPChunk.h"
#include "UDTPWhine.h"
#include "UDTPHandshake.h"



/*For deduction*/
UDTPData::UDTPData(char* deducePacket){
    memcpy(&_packetSize, &deducePacket, 2); /*Take unsigned short for size*/
    set_packet_type((PacketType)deducePacket[2]); /*Packet type is located at location 2*/
}

/*Serialization*/
UDTPData::UDTPData(UDTPPath& path){
}
UDTPData::UDTPData(UDTPHeader& header){
}
UDTPData::UDTPData(UDTPChunk& chunk){
}
UDTPData::UDTPData(UDTPWhine& whine){
}
UDTPData::UDTPData(UDTPHandshake& handshake){
}
/*Getters and setters*/
char* UDTPData::get_raw_buffer(){
    return _raw;
}
