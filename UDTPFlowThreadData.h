#ifndef __UDTP_FLOW_THREAD_DATA
#define __UDTP_FLOW_THREAD_DATA

#include <queue>
#include <pthread.h>
class UDTPChunk;
class UDTPAddress;

class UDTP;
class UDTPFlowThreadData{
    public:
        UDTPFlowThreadData(unsigned int flowSocket, pthread_t thread, unsigned short id, UDTP* accessUDTP);
        ~UDTPFlowThreadData(){
        _accessUDTP = NULL;
        }
        UDTP* udtp();

        bool add_recv_address(UDTPAddress& newAddress) { _addressesRecv.push_back(newAddress);};
        std::vector<UDTPAddress> get_recv_addresses() { return _addressesRecv;};
        unsigned int get_count_of_recv_addresses() { return _addressesRecv.size();};

                bool add_send_address(UDTPAddress& newAddress) { _addressesSend.push_back(newAddress);};
        std::vector<UDTPAddress> get_send_addresses() { return _addressesSend;};
        unsigned int get_count_of_send_addresses() { return _addressesSend.size();};
        bool set_approved(){ _approved = true;};
        bool check_approved(){ return _approved;};
        pthread_t get_thread();
        unsigned short get_id();

        unsigned int get_socket() { return _flowSocket; };
        std::queue<UDTPChunk> read_queue() { return _chunksRead;};
        std::queue<UDTPChunk> write_queue() { return _chunksWrite;};
        unsigned int get_flow_socket() { return _flowSocket;};
    private:
    bool _approved;
    pthread_t _thread;
    unsigned short _id;
    unsigned int _flowSocket;
    std::vector<UDTPAddress> _addressesRecv;
    std::vector<UDTPAddress> _addressesSend;
        std::queue<UDTPChunk> _chunksRead;
        std::queue<UDTPChunk> _chunksWrite;
    UDTP* _accessUDTP;
};


#endif
