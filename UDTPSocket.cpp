#include "UDTPPeer.h"
#include "UDTPAddress.h"

UDTPPeer::~UDTPPeer()
{
    delete _address;
}
bool UDTPPeer::set_address(sockaddr_in socketAddress)
{
    _address = new UDTPAddress(socketAddress);
    return true;
};
