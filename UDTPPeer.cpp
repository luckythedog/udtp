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
        UDTPFlowThreadData* UDTPPeer::find_thread_with_port(unsigned short port){
            for(unsigned int i=0; i<_flowThreads.size(); i++){
                if(port = ntohs(_flowThreads[i]->get_socket_address().sin_port)){
                    return _flowThreads[i];
                }
            }
            return NULL;

        }
        bool UDTPPeer::check_all_flow_threads_approved(){
            for(unsigned int i=0; i<_flowThreads.size(); i++){
                if(!_flowThreads[i]->check_approved()) return false;
            }
            return true;
        }

                bool UDTPPeer::check_all_flow_threads_alive(){
            for(unsigned int i=0; i<_flowThreads.size(); i++){
                if(!_flowThreads[i]->alive()) return false;
            }
            return true;
        }
bool UDTPPeer::remove_all_flow_threads(){
    for(unsigned int i=0; i<_flowThreads.size(); i++){

        delete _flowThreads[i];
        _flowThreads[i] = NULL;
    }
    _flowThreads.clear();
    return true;
}
