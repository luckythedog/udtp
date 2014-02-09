/*******************************************
*General UDTP Packet. Easy place to hold all the
*enums.
*******************************************/
#ifndef __UDTP_PACKET
#define __UDTP_PACKET

enum PacketType{
    Packet = 0x00,
    Handshake = 0x01,
    Header = 0x02,
    Path = 0x03,
    Chunk = 0x04,
    Whine = 0x05
};
enum ResponseCode{
    ResponseNone = 0xF0,  /*Every response code starts out with none. Tihs means the request is unfulfilled.*/
    ResponseApproved = 0xF1,
    ResponseRejected = 0xF2,
    ResponseFileNotFound = 0xF3,
    ResponseFileExistsAlready = 0xF4
};

class UDTPPacket{
    public:
        /*There is no UDTPPacket constructor, since we will never use it!*/
        ResponseCode get_response_code() { return _responseCode; };
        bool set_response_code(ResponseCode responseCode) { _responseCode = responseCode; return true;};

        PacketType get_packet_type() { return _packetType; };
        bool set_packet_type(PacketType packetType) { _packetType = packetType; return true;};

    protected:

        PacketType _packetType;
        ResponseCode _responseCode; /*Not every packet uses a response code, like Chunk or Whine -- since those do not need a response.*/
};
#endif


