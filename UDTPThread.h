#ifndef __UDTP_THREAD
#define __UDTP_THREAD

#include <pthread.h>

class UDTP;
enum ThreadType{
    INCOMING,
    OUTGOING
};

class UDTPThread{
    public:
    bool is_alive(){
        return _alive;
    }
    bool kill(){
        _alive = false;
        return true;
    }
        bool get_thread_type() { return  _threadType;};
    pthread_t get_thread_handler() { return _threadHandler;};
    bool set_udtp(UDTP* myUDTP) { _myUDTP = myUDTP;};
    UDTP* udtp() { return _myUDTP;};
    protected:
    ThreadType _threadType;
    UDTP* _myUDTP;
    bool _alive;
    pthread_t _threadHandler;
};
#endif
