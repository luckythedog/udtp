#ifndef __UDTP_HEADER
#define __UDTP_HEADER
#include "UDTPPacket.h"

class UDTPFile;
class UDTPPath;



class UDTPHeader : public UDTPPacket{
    public:
    ~UDTPHeader();
    UDTPHeader(){
    set_response_code(ResponseNone);};
        UDTPHeader(UDTPPacketHeader header); /*Unpacks a header from UDTPPacket*/
        UDTPHeader(TransferType headerType, UDTPFile* file); /*Creates a header from a file. If it is a push request, it will fill in numberOfChunks and sizeOfFile.
                                                                                        If it is a pull request, it will only take the path to the file.*/

        TransferType get_header_type() { return _transferType; } /*gets header type*/
        bool set_header_type(TransferType headerType) { _transferType = headerType;};


        unsigned short get_file_id() { return _fileID;};
        bool set_file_id(unsigned short fileID) { _fileID = fileID;}; /*sets the file identifier*/

        unsigned short get_number_of_chunks() { return _numberOfChunks;};
        bool set_number_of_chunks(unsigned short numberOfChunks) { _numberOfChunks = numberOfChunks;};

        unsigned int get_size_of_file() { return _sizeOfFile;};
        bool set_size_of_file(unsigned int sizeOfFile){ _sizeOfFile = sizeOfFile;};

        UDTPPath* get_path_of_file() { return _pathOfFile;};

        /* must impliment pure virtual functions */
        char* get_raw_buffer();
        bool pack();
        bool unpack();
        bool respond();

    private:
        UDTPPath* _pathOfFile;
        unsigned short _fileID;
        unsigned short _numberOfChunks;
        unsigned int _sizeOfFile;

        TransferType _transferType;
        ResponseCode _responseCode;
};

#endif
