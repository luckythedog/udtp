#include "UDTPFile.h"
/*Implementations after forward class declaration*/
#include "UDTPHeader.h"
/*Settings*/
#include "UDTPSettings.h"

UDTPFile::UDTPFile(UDTPPacketHeader header)
{
    // TODO
}

UDTPFile::UDTPFile(UDTPPath pathOfFile)
{
    _pathOfFile = pathOfFile;
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

bool UDTPFile::set_chunk_completed_true(unsigned short num)
{
    if(num < 0 || num > _numberOfChunks) return false;
    _chunksCompleted[num] = true;
    return true;
}
bool UDTPFile::empty_fill_chunks_completed()  /*pushes numberOfChunks false bools into chunksCompleted vector. This should only RUN one time.*/
{
    for(int i=0; i<_numberOfChunks; i++)
    {
        _chunksCompleted.push_back(false);
    }
    return true;
}
bool UDTPFile::retrieve_info_from_local_file()  /*Retrieves critical information from files. This is usually used for pushing files*/
{
    std::ifstream tempFileStream(_pathOfFile.get_address_path());
    tempFileStream.seekg(0, std::ios::end);

    unsigned short retrieveSizeOfFile = tempFileStream.tellg();

    unsigned short retrieveNumberOfChunks = retrieveSizeOfFile / UDTPSettings::CHUNK_SIZE_AGREEMENT;
    if((retrieveNumberOfChunks % UDTPSettings::CHUNK_SIZE_AGREEMENT) > 0)
    {
        retrieveNumberOfChunks++;
    }

    set_file_id(0);
    set_size_of_file(retrieveSizeOfFile);
    set_number_of_chunks(retrieveNumberOfChunks);

    tempFileStream.close();

    return true;
}
