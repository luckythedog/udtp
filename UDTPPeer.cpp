#include "UDTPPeer.h"
#include "UDTPAddress.h"

UDTPPeer::~UDTPPeer()
{

}


    bool UDTPPeer::add_address(sockaddr_in socketAddress){ //*Don't ever worry about removing them**/
        UDTPAddress newAddress(socketAddress);
        _addresses.push_back(newAddress);
    }
    bool UDTPPeer:: get_address_exist(sockaddr_in socketAddress){
        UDTPAddress tempAddress(socketAddress);
        for(int i=0; i<addresses_count(); i++){
            if(_addresses[i].compare_with_address(tempAddress)) return true;
        }
        return false;
    }
    bool UDTPPeer:: get_address_exist(UDTPAddress& address){
        for(int i=0; i<addresses_count(); i++){
            if(_addresses[i].compare_with_address(address)) return true;
        }
        return false;
    }
UDTPAddress& UDTPPeer::operator[](const unsigned int index)
{
    if(index >= 0 && index < _addresses.size()){
        return _addresses[index];
    }
}
