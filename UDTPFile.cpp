#include "UDTPFile.h"
/*Implementations after forward class declaration*/
#include "UDTPHeader.h"
#include <pthread.h>
/*Setup*/
#include "UDTPSetup.h"

UDTPFile::UDTPFile(UDTPPacketHeader header)
{
    // TODO
}

UDTPFile::UDTPFile(UDTPPath pathOfFile)
{
    _pathOfFile = pathOfFile;
    // TODO: pass the chunk size agreement
    _chunkSize = DEFAULT_CHUNK_SIZE_AGREEMENT;
    _active = false;
}

UDTPFile::~UDTPFile()
{
}

bool UDTPFile::check_file_exist()  /*Just checks if the file length is more than 0*/
{
    std::ifstream tempFileStream(_pathOfFile.get_address_path());
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
bool UDTPFile::attach_header(UDTPHeader& header)
{
    set_number_of_chunks(header.get_number_of_chunks());
    set_size_of_file(header.get_size_of_file());
    set_file_id(header.get_file_id());
    return true;
}
bool UDTPFile::add_file_thread(ThreadType type){
    pthread_t newFileThread;
    UDTPThreadFile* fileThread = new UDTPThreadFile(newFileThread, type);
    pthread_create(&newFileThread, NULL, UDTPFile::fileThread, (UDTPThreadFile*) fileThread);
    pthread_tryjoin_np(newFileThread, NULL);

}
bool UDTPFile::start_file_processing(){
    _active = true;
}

void* UDTPFile::fileThread(void* args){
    UDTPThreadFile* fileThread = (UDTPThreadFile*) args;
    while(fileThread->is_alive() && fileThread->get_thread_type() == INCOMING){
        while(!fileThread->file()->get_incoming_chunk_queue().empty()){
                fileThread->file()->get_incoming_chunk_queue().pop();
        }
    }
    while(fileThread->is_alive() && fileThread->get_thread_type() == OUTGOING){
    }

    fileThread = NULL;

}
bool UDTPFile::retrieve_info_from_local_file()  /*Retrieves critical information from files. This is usually used for pushing files*/
{
    std::ifstream tempFileStream(_pathOfFile.get_address_path());
    tempFileStream.seekg(0, std::ios::end);

    unsigned short retrieveSizeOfFile = tempFileStream.tellg();

    unsigned short retrieveNumberOfChunks = retrieveSizeOfFile / _chunkSize;
    if((retrieveNumberOfChunks % _chunkSize) > 0)
    {
        retrieveNumberOfChunks++;
    }

    set_file_id(0);
    set_size_of_file(retrieveSizeOfFile);
    set_number_of_chunks(retrieveNumberOfChunks);

    tempFileStream.close();

    return true;
}
