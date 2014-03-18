#ifndef __UDTP_HEADER
#define __UDTP_HEADER
#include "UDTPPacket.h"

class UDTPFile;
class UDTPPath;

enum HeaderType{
    HeaderPush = 0x21,
    HeaderPull = 0x22
};

class UDTPHeader : public UDTPPacket{
    public:
    ~UDTPHeader();
        UDTPHeader(UDTPPacketHeader header); /*Unpacks a header from UDTPPacket*/
        UDTPHeader(HeaderType headerType, UDTPFile* file); /*Creates a header from a file. If it is a push request, it will fill in numberOfChunks and sizeOfFile.
                                                                                        If it is a pull request, it will only take the path to the file.*/

        HeaderType get_header_type() { return _headerType; } /*gets header type*/
        bool set_header_type(HeaderType headerType) { _headerType = headerType;};

        ResponseCode get_response_code(){ return _responseCode;};
        bool set_response_code(ResponseCode responseCode) { responseCode = _responseCode;};

        unsigned short get_file_id() { return _fileID;};
        bool set_file_id(unsigned short fileID) { _fileID = fileID;}; /*sets the file identifier*/

        unsigned short get_number_of_chunks() { return _numberOfChunks;};
        bool set_number_of_chunks(unsigned short numberOfChunks) { _numberOfChunks = numberOfChunks;};

        unsigned int get_size_of_file() { return _sizeOfFile;};
        bool set_size_of_file(unsigned int sizeOfFile){ _sizeOfFile = sizeOfFile;};

        UDTPPath& get_path_of_file();

        /* must impliment pure virtual functions */
        char* get_raw_buffer();
        bool unpack();

    private:
        UDTPPath* _pathOfFile;
        unsigned short _fileID;
        unsigned short _numberOfChunks;
        unsigned int _sizeOfFile;

        HeaderType _headerType;
        ResponseCode _responseCode;
};

#endif
