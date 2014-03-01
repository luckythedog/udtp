#include "UDTPFlowThreadData.h"
#include "UDTPChunk.h"
#include "UDTPAddress.h"
        UDTPFlowThreadData::UDTPFlowThreadData(unsigned int flowSocket, pthread_t thread, unsigned short id, UDTP* accessUDTP){
            _approved = false;
            _flowSocket = flowSocket;
            _thread = thread;
            _id = id; /*Pork Additive*/
            _accessUDTP = accessUDTP;
        }
    unsigned short UDTPFlowThreadData::get_id() { return _id;};
    pthread_t UDTPFlowThreadData::get_thread(){
    return _thread;
    }
    UDTP* UDTPFlowThreadData::udtp(){
        return _accessUDTP;
    }
