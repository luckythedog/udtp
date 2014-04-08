#ifndef __UDTP_FILE
#define __UDTP_FILE
#include <queue>
#include <vector>
#include <fstream>
#include <string>
#include <semaphore.h>
#include <ctime>
#include "UDTPPeer.h"
#include "UDTPChunk.h"
#include "UDTPPath.h"
#include "UDTPThreadFile.h"
class UDTPHeader;
class UDTPPacket;
class UDTPSetup;
class UDTP;

class UDTPFile{
    public:
        UDTPFile();
        ~UDTPFile();
        UDTPFile(std::string path); /*Creates from a path only*/
        bool set_defaults();
        bool unpack_from_header(UDTPHeader& header); /*Take information from a header and apply it to afile*/
        bool pack_to_header(UDTPHeader& header);
        bool new_header();
        bool check_file_exist(); /*Checks if file exists or not. This is used by UDTP main api.*/
        bool set_info_to_zero(); /*Sets info to zero to prepare the UDTPFile to be engulfed by the UDTPHeader for pull*/
        bool retrieve_info_from_local_file();  /*Retrieves info from fstream to prepare the UDTPFile to be engulfed by the UDTPHeader for push*/
    /********************************/
        bool start_file_processing();
        bool end_file_processing();
    /********************************/
        bool add_file_thread(ThreadType type);
        bool remove_file_thread();
        unsigned int get_file_id() { return _fileID;};
        bool set_file_id(unsigned int fileID) { _fileID = fileID;};

        unsigned int get_number_of_chunks() { return _numberOfChunks;};
        bool set_number_of_chunks(unsigned short numberOfChunks){ _numberOfChunks = numberOfChunks;};

        unsigned int get_size_of_file() { return _sizeOfFile;};
        bool set_size_of_file(unsigned int sizeOfFile) { _sizeOfFile = sizeOfFile;};
        bool set_error(std::string errorMsg) { _errorMsg = errorMsg ; _critError = true;};
        bool check_error() { return _critError;};
        void error_msg();
        bool set_udtp(UDTP* myUDTP){
            _myUDTP = myUDTP;
        }
        bool set_chunk_size_from_setup();
        bool active() { return _active;};



        bool set_peer(UDTPPeer* myPeer){ _myPeer = myPeer;};
        UDTPPeer* peer() {return _myPeer;};

       bool write_mmap(){
       }
       bool read_mmap(){
       }

        bool add_outgoing_chunk(UDTPChunk* newChunk){ /*Outgoing chunks are added from the UDTPFile themselves*/
            _outgoingChunks.push(newChunk);
        }
        bool add_incoming_chunk(UDTPChunk* newChunk){ /*Incoming comes from some other thread outside*/
            if(active()){
            _incomingChunks.push(newChunk);
            sem_post(&_semFileThread);
            return true;
            }
            return false;
        }
        std::queue<UDTPChunk*> get_incoming_chunk_queue(){
            return _incomingChunks;
        }
        std::queue<UDTPChunk*> get_outgoing_chunk_queue(){
            return _incomingChunks;
        }
        bool add_missing_outgoing_chunks(unsigned int chunkID){
            _missingOutgoingChunks.push(chunkID);
            sem_post(&_semFileThread);
            return true;
        }

        std::queue<unsigned int> get_missing_outgoing_chunk_queue(){
            return _missingOutgoingChunks;
        }
        bool set_approved_header(bool newbool) { _hasApprovedHeader = newbool;};
        bool is_approved_header() { return _hasApprovedHeader;};
        std::vector<bool> get_chunks_completed_queue(){
            return _chunksCompleted;
        }
bool set_complete_to(unsigned int chunkID);
bool set_complete_to_none();
        bool set_approver(bool newbool){ _approver = newbool;};
        bool is_approver() { return _approver;};
bool set_max_queue_length_from_setup();
unsigned short get_max_queue_length() { return _maxQueueLength;};
bool get_current_chunk() { return _currentChunk;};
void increment_current_chunk() { _currentChunk++;};
ThreadType get_thread_type();
    private:
    UDTP* _myUDTP;
        UDTPPeer* _myPeer;
        std::string _path;
        unsigned int _fileID;
        unsigned short _numberOfChunks;
        unsigned int _sizeOfFile;
        unsigned int _chunkSize;
        int memoryMapFD;
        bool _approver; /*Were you the approver?*/
        bool _fileExists; /*Boolean for file existance. Depending on if the user used send or push, we will cancel
                                immediately without sending a header if the file does not exist.*/
        TransferType _transferType;
        std::queue<UDTPChunk*> _incomingChunks;
        std::queue<UDTPChunk*> _outgoingChunks;
        std::queue<unsigned int> _missingIncomingChunks;
        std::queue<unsigned int> _missingOutgoingChunks;
        std::vector<bool> _chunksCompleted;
        unsigned short _maxQueueLength;
        bool _hasApprovedHeader; /*Starts out with false. Waits until it receives an approved header.*/
        bool _active; /*Sets the file in activity*/
        bool _critError;
        std::string _errorMsg;
        time_t _timeStarted;

        unsigned short _currentChunk;

        std::queue<UDTPThreadFile*> _fileThreads;
        sem_t _semFileThread;
        static void* fileThread(void*);


};

#endif
