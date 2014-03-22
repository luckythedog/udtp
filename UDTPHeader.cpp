#include "UDTPHeader.h"
/*Implementations after forward declaration*/
#include "UDTPFile.h"
#include "UDTPPath.h"

UDTPHeader::~UDTPHeader(){
    _pathOfFile = NULL;
}

UDTPHeader::UDTPHeader(UDTPPacketHeader header){
    _header = header;
    // TODO - need to allocate _raw buffer size based on header.packetSize
    // need to do this FOR ALL packet types;
}

UDTPHeader::UDTPHeader(HeaderType headerType, UDTPFile* file) {
    _headerType = headerType; /*Set header type*/
    _responseCode = ResponseNone; /*Set to unfulfilled response code*/
    _fileID = 0; /*File identifier is always zero when sending out a request*/
    switch(headerType){
        case HeaderPull:
        file->set_info_to_zero(); /*Set information to zero*/
        break;
        case HeaderPush:
        file->retrieve_info_from_local_file();
        break;
    }
    set_number_of_chunks(file->get_number_of_chunks());
    set_size_of_file(_sizeOfFile = file->get_size_of_file());
}
UDTPPath& UDTPHeader::get_path_of_file(){
    return *_pathOfFile;
}

char* UDTPHeader::get_raw_buffer(){
}

bool UDTPHeader::process(UDTP* myUDTP){
    return true;
}
