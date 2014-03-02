#include "UDTPFlowThreadData.h"
#include "UDTPChunk.h"
#include "UDTPAddress.h"
#include "UDTPPeer.h"
        UDTPFlowThreadData::UDTPFlowThreadData(pthread_t thread, unsigned int flowSocket, sockaddr_in socketAddress){
            _alive = false;
            _linked = false;
            _approved = false;
            _flowSocket = flowSocket;
            _thread = thread;
            _socketAddress = socketAddress;
        }

