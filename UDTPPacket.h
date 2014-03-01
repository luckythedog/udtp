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
    Acknowledge = 0x05
};
enum ResponseCode{
    ResponseNone = 0xF0,  /*Every response code starts out with none. Tihs means the request is unfulfilled.*/
    ResponseApproved = 0xF1,
    ResponseRejected = 0xF2,
    ResponseFileNotFound = 0xF3,
    ResponseFileExistsAlready = 0xF4,
    ResponseListingFound= 0xF5,
    ResponseInvalidPath = 0xF6,

    ResponseRetry = 0xF7, /*Just in case server is not ready to provide something, he will ask client to retry the packet again at a different time*/
    ResponseNotReady = 0xF8, /*Client has not completed steps in getting ready or there was an error. Either way Path, Header, Chunks, or Acknowledge requests will not be handled. Only handshake.*/

    ResponseNoneAndFlowLeader = 0xF9 /*It's like ResponseNone but you proclaim yourself as the Flow leader in threading in it*/
};

class UDTPPacket{
    public:
        /*There is no UDTPPacket constructor, since we will never use it!*/
        ResponseCode get_response_code() { return _responseCode; };
        bool set_response_code(ResponseCode responseCode) { _responseCode = responseCode; return true;};

        PacketType get_packet_type() { return _packetType; };
        bool set_packet_type(PacketType packetType) { _packetType = packetType; return true;};

        unsigned int get_socket_id() { return _socketID;};
        bool set_socket_id(unsigned int socketID) { _socketID = socketID;};

        unsigned int get_peer_id() { return _peerID;};
        bool set_peer_id(unsigned int peerID) { _peerID = peerID;};

    protected:
        unsigned int _peerID; /*It's numerical location in the peer's list*/
       unsigned int _socketID; /*Optional holder*/ /*Since indexes and sockets are different entirely on each system! These are local!*/
        PacketType _packetType;
        ResponseCode _responseCode; /*Not every packet uses a response code, like Chunk or Whine -- since those do not need a response.*/
};
#endif


