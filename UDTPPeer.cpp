#include "UDTPPeer.h"
#include "UDTPAddress.h"
#include "UDTPFlowThreadData.h"

UDTPPeer::~UDTPPeer()
{
    remove_all_flow_threads();
}



bool UDTPPeer::add_flow_thread(pthread_t thread, unsigned int flowSocket, sockaddr_in socketAddress){
        UDTPFlowThreadData* newFlowThread = new UDTPFlowThreadData(thread, flowSocket, socketAddress);
        _flowThreads.push_back(newFlowThread);
        return true;
}
    UDTPFlowThreadData* UDTPPeer::get_next_thread_link_needed() { /*Returns the next thread that needs a link, if all done.. then it is returned NULL!*/
        for(unsigned int i=0; i<_flowThreads.size(); i++){
            if(!_flowThreads[i]->is_linked()) return _flowThreads[i];
        }
            return NULL;
        };
bool UDTPPeer::remove_all_flow_threads(){
    for(unsigned int i=0; i<_flowThreads.size(); i++){

        delete _flowThreads[i];
        _flowThreads[i] = NULL;
    }
    _flowThreads.clear();
    return true;
}
