#include "UDTPFile.h"
#include "UDTPPacket.h"
/*Implementations after forward class declaration*/
#include "UDTPHeader.h"
#include "UDTP.h"
#include "UDTPAcknowledge.h"
#include <pthread.h>
/*Setup*/
#include "UDTPSetup.h"

UDTPFile::UDTPFile() /*Coming from far away! It */
{
    set_defaults();
    // TODO
}
bool UDTPFile::set_defaults()
{
    _currentChunk = 0;
    _errorMsg = ""; /*Blank!*/
    _hasApprovedHeader= false;
    _critError = false;
    _active = false;
    return true;
}
UDTPFile::UDTPFile(std::string path) /*Coming locally!*/
{
    set_defaults();
    _path = path;
    _active = false;

}
UDTPFile::~UDTPFile()
{
}
bool UDTPFile::check_file_exist()  /*Just checks if the file length is more than 0*/
{
    std::ifstream tempFileStream(_path.c_str());
    tempFileStream.seekg(0, std::ios::end);
    unsigned int tempFileLength = tempFileStream.tellg();
    tempFileStream.close();

    if(tempFileLength == 0) return false;
    return true;
}


bool UDTPFile::set_info_to_zero()  /*Sets all critical information to zero. This is usually used for pulling files*/
{
    set_file_id(0);
    set_number_of_chunks(0);
    set_size_of_file(0);
    return true;
}

bool UDTPFile::unpack_from_header(UDTPHeader& header) /*Takes all information from a header*/
{

    set_number_of_chunks(header.get_number_of_chunks());
    set_size_of_file(header.get_size_of_file());
    set_file_id(header.get_file_id());
    _transferType = header.get_header_type();
    return true;
}
bool UDTPFile::pack_to_header(UDTPHeader& header)
{
    header.set_number_of_chunks(get_number_of_chunks());
    header.set_size_of_file(get_size_of_file());
    header.set_file_id(get_file_id());
    header.set_header_type(_transferType);
}
bool UDTPFile::set_chunk_size_from_setup()
{
    _chunkSize = _myUDTP->setup()->get_chunk_size_agreement();
    return true;
}
bool UDTPFile::set_max_queue_length_from_setup()
{
    _maxQueueLength = _myUDTP->setup()->get_max_memory_usage() / _myUDTP->setup()->get_chunk_size_agreement();
    return true;
}
bool UDTPFile::add_file_thread(ThreadType type)
{
    if(active() && _fileThreads.size() < _myUDTP->setup()->get_max_number_of_file_threads())
    {
        pthread_t newFileThread;
        UDTPThreadFile* fileThread = new UDTPThreadFile(newFileThread, type);
        pthread_create(&newFileThread, NULL, UDTPFile::fileThread, (UDTPThreadFile*) fileThread);
        pthread_tryjoin_np(newFileThread, NULL);
        _fileThreads.push(fileThread);
    }

}
bool UDTPFile::remove_file_thread()
{
    if(active() && _fileThreads.size() == _myUDTP->setup()->get_starting_number_of_file_threads()) return false;
    if(!_fileThreads.empty())
    {
        UDTPThreadFile* removeThread = _fileThreads.front();
        removeThread->kill();
        pthread_cancel(removeThread->get_thread_handler());
        return true;
    }
    return false;
}
bool UDTPFile::empty_all_chunk_status()
{
    _chunksCompletedCount = 0;
    if(_chunksCompleted.size() !=0) _chunksCompleted.clear();
    for(unsigned int i=0; i<_numberOfChunks; i++)
    {
        unsigned char emptyCode = 0x00;
        emptyCode |= (0x01 << EMPTY);
        _chunksCompleted.push_back(false);
    }
}

ThreadType UDTPFile::get_thread_type()
{
    if(is_approver() && _transferType == Pull) return OUTGOING; /*You will be pushing since you approved a pull!*/
    if(!is_approver() && _transferType == Pull) return INCOMING; /*Get ready to read in!*/

    if(is_approver() && _transferType == Push) return INCOMING; /*Some PEER is asking to send you a file! better get ready to recv!()*/
    if(!is_approver() && _transferType == Push) return OUTGOING;  /*You are PEER and you're going to send a file!*/
}
bool UDTPFile::begin_file_processing()
{
    if(!is_approved_header()) return false;
    if(check_error()) return false;
    ThreadType threadType = get_thread_type();
    if(threadType == OUTGOING)
    {
        if(sem_init(&_semFileThread,0,get_number_of_chunks()) != 0) return false; /*Load enough semaphore tickets for outgoing!*/
    }
    if(threadType == INCOMING)
    {
        if(sem_init(&_semFileThread,0,0) !=0) return false;
    }
    empty_all_chunk_status();
    for(unsigned int i=0; i<_myUDTP->setup()->get_starting_number_of_file_threads(); i++)
    {
        add_file_thread(threadType);
    }

    _active = true;
    return true;
}

bool UDTPFile::end_file_processing()
{
    while(remove_file_thread()); /*Remove all file threads!*/
    _active = false;
}

int UDTPFile::set_all_empty_chunks_to_whine(){ /*Usually activated by acknowledgement!*/
    for(unsigned int i=0; i<_numberOfChunks; i++){
        if(!get_chunk_status(i,  DONE) && !get_chunk_status(i,  WHINE)) {
            set_chunk_status(i, WHINE); /*Set it to whine!*/
        }
    }
    return -1; /*-1 means all empty chunks are set to whine!!*/
    /*Check chunks first!*/
}

int UDTPFile::get_chunks_whine(){ /*Gets all the whine chunks that are not done!*/
    for(unsigned int i=0; i<_numberOfChunks; i++){
        if(get_chunk_status(i,  WHINE) && !get_chunk_status(i,  DONE)) {
            return i;
        }
    }
    return -1;
}
int UDTPFile::verify_completion(){
        for(unsigned int i=0; i<_numberOfChunks; i++){
        if(!get_chunk_status(i,  DONE)) {
            return i;
        }
    }

    end_file_processing(); /*File is completed!*/
    return -1;
}
void* UDTPFile::fileThread(void* args)
{
    UDTPThreadFile* fileThread = (UDTPThreadFile*) args;
    UDTPFile* processFile = fileThread->file();
    while(fileThread->is_alive() && fileThread->get_thread_type() == INCOMING)
    {
        sem_wait(&processFile->_semFileThread);
        while(!processFile->get_incoming_chunk_queue().empty())
        {
            UDTPChunk* processChunk = processFile->get_incoming_chunk_queue().front();
            unsigned int processChunkID = processChunk->get_chunk_id();
            if(!processFile->get_chunk_status(processChunkID, DONE)) /*Make sure it isn't completed already!*/
            {
                if(processFile->write_mmap(processChunk) == true)  /*Write it!*/
                {
                    processFile->get_chunk_status(processChunkID, DONE);
                }
                else
                {
                    /*Request missing chunks here!*/
                }
                processFile->get_incoming_chunk_queue().pop();
            }
        }
    }



    while(fileThread->is_alive() && fileThread->get_thread_type() == OUTGOING)
    {
        /***********Handles missing chunk*********************/
        //It should only be on the outgoing file threads since it is not recv()'s fault!
        sem_wait(&fileThread->file()->_semFileThread);


    }

    fileThread = NULL;

}
bool UDTPFile::retrieve_info_from_local_file()  /*Retrieves critical information from files. This is usually used for pushing files*/
{
    std::ifstream tempFileStream(_path.c_str());
    tempFileStream.seekg(0, std::ios::end);

    unsigned short retrieveSizeOfFile = tempFileStream.tellg();

    unsigned short retrieveNumberOfChunks = retrieveSizeOfFile / _chunkSize;
    if((retrieveNumberOfChunks % _chunkSize) > 0)
    {
        retrieveNumberOfChunks++;
    }

    set_size_of_file(retrieveSizeOfFile);
    set_number_of_chunks(retrieveNumberOfChunks);

    tempFileStream.close();

    return true;
}
void UDTPFile::error_msg()
{
    if(_errorMsg != "")
    {
        _myUDTP->display_msg(_errorMsg);
        _errorMsg = "";
    }
}
bool UDTPFile::write_mmap(UDTPChunk* chunk)
{
    /*Writing goes here!*/

    delete chunk;
    return true;
}
UDTPChunk* UDTPFile::read_mmap(unsigned int chunkID)
{
    /*Reading goes here!*/
    return NULL;
}
