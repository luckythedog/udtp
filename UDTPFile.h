#ifndef __UDTP_FILE
#define __UDTP_FILE
#include <queue>
#include <vector>
#include <fstream>
#include <ctime>
#include "UDTPPeer.h"
#include "UDTPChunk.h"
#include "UDTPPath.h"
#include "UDTPThreadFile.h"
class UDTPHeader;


class UDTPFile{
    public:
        UDTPFile(UDTPPacketHeader header);
        ~UDTPFile();
        UDTPFile(UDTPPath pathOfFile); /*Creates from a path only*/

        bool attach_header(UDTPHeader& header); /*Take information from a header and apply it to afile*/

        bool check_file_exist(); /*Checks if file exists or not. This is used by UDTP main api.*/
        bool set_info_to_zero(); /*Sets info to zero to prepare the UDTPFile to be engulfed by the UDTPHeader for pull*/
        bool retrieve_info_from_local_file();  /*Retrieves info from fstream to prepare the UDTPFile to be engulfed by the UDTPHeader for push*/

        bool start_file_processing();
        bool add_file_thread(ThreadType type);
        unsigned int get_file_id() { return _fileID;};
        bool set_file_id(unsigned int fileID) { _fileID = fileID;};

        unsigned int get_number_of_chunks() { return _numberOfChunks;};
        bool set_number_of_chunks(unsigned short numberOfChunks){ _numberOfChunks = numberOfChunks;};

        unsigned int get_size_of_file() { return _sizeOfFile;};
        bool set_size_of_file(unsigned int sizeOfFile) { _sizeOfFile = sizeOfFile;};

        bool set_peer(UDTPPeer* myPeer){ _myPeer = myPeer;};
        UDTPPeer* peer() {return _myPeer;};

        bool add_incoming_chunk(UDTPChunk* newChunk){
            incomingChunks.push(newChunk);
        }
        std::queue<UDTPChunk*> get_incoming_chunk_queue(){
            return incomingChunks;
        }

    private:
        UDTPPeer* _myPeer;
        UDTPPath _pathOfFile;
        unsigned int _fileID;
        unsigned short _numberOfChunks;
        unsigned int _sizeOfFile;
        unsigned int _chunkSize;
        int memoryMapFD;
        bool _fileExists; /*Boolean for file existance. Depending on if the user used send or push, we will cancel
                                immediately without sending a header if the file does not exist.*/

        std::queue<UDTPChunk*> incomingChunks;
        std::queue<UDTPChunk*> outgoingChunks;

        bool _hasApprovedHeader; /*Starts out with false. Waits until it receives an approved header.*/
        bool _active; /*Sets the file in activity*/
        time_t _timeStarted;
        std::queue<UDTPThreadFile*> _fileThreads;
        static void* fileThread(void*);


};

#endif
