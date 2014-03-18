/*******************************************
*UDTPPath can hold a directory or filename, or
*both. It depends entirely on what the
*constructor's argument points to. There are
*calls like get_filename() which dissects the
*string path for "filename" or "directory."
*UDTPPath is also part of the UDTPPacket family
*meaning it can be networked and also means
*that it can also serve as a request.
*******************************************/
#ifndef __UDTP_PATH
#define __UDTP_PATH
#include <string>
#include <iostream>
#include "UDTPPacket.h"
/*Forward declarations*/
class UDTPData;

class UDTPPath : public UDTPPacket{
    public:
        UDTPPath(UDTPPacketHeader header);
        UDTPPath();

        UDTPPath(std::string addressPath) { /*Creates a new path from a string.*/
            _addressPath = addressPath;
            set_packet_type(Path);
            set_response_code(ResponseNone); /*Set unfulfilled request*/
        };
        const char* get_address_path() { return _addressPath.c_str();};
        const char* get_return_path() { return _returnPath.c_str();};

        char* get_filename();/*Dissects filename from addressPath then returns it here, if none it will return empty*/
        char* get_directory(); /*Dissects directory from addressPath then returns it here, if none it will return empty*/

        /* must impliment pure virtual functions */
        char* get_raw_buffer();
        bool unpack();

    private:
        std::string _addressPath;
        std::string _returnPath; /*If there is a return from the destination, it will be stored here and the response code should be changed to ResponseApproved*/

};

#endif

