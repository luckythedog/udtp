#ifndef __UDTP_FILE
#define __UDTP_FILE

#include <queue>
#include <vector>
#include <fstream>
#include <ctime>

class UDTPHeader;
class UDTPChunk;
class UDTPPath;


class UDTPFile{
    public:
        UDTPFile();
        ~UDTPFile();
        UDTPFile(UDTPPath& pathOfFile); /*Creates from a path only*/

        bool process_header(UDTPHeader& header);

        bool check_file_exist(); /*Checks if file exists or not. This is used by UDTP main api.*/
        bool set_info_to_zero(); /*Sets info to zero to prepare the UDTPFile to be engulfed by the UDTPHeader for pull*/
        bool retrieve_info_from_local_file();  /*Retrieves info from fstream to prepare the UDTPFile to be engulfed by the UDTPHeader for push*/

        bool set_chunk_completed_true(unsigned short num){
            if(num < 0 || num > _numberOfChunks) return false;
            _chunksCompleted[num] = true;
            return true;
        }
        bool empty_fill_chunks_completed(){ /*pushes numberOfChunks false bools into chunksCompleted vector. This should only RUN one time.*/
            for(int i=0; i<_numberOfChunks; i++){
                _chunksCompleted.push_back(false);
            }
            return true;
        }
        unsigned short amount_of_chunks_not_completed(); /*gets  amount of ones that are not completed*/
        unsigned short get_highest_chunk_completed(); /*gets the highest chunk completed. by highest it means in heirarchial order.*/
        std::vector<unsigned short> get_chunks_not_completed_up_to(unsigned short value); /* We would use get_highest_chunk_completed() then use it in this function.
                                                                                                                Certainly, if Chunk #150 was completed, then all chunks
                                                                                                                before #150 should also be completed. If not we'll whine them. Returns vector
                                                                                                                of unsigned ints of the indexes that were false.*/

        unsigned short get_file_identifier() { return _fileIdentifier;};
        bool set_file_identifier(unsigned short fileIdentifier) {_fileIdentifier = fileIdentifier;};


        unsigned short get_number_of_chunks() { return _numberOfChunks;};
        bool set_number_of_chunks(unsigned short numberOfChunks) { _numberOfChunks = numberOfChunks;};

        unsigned int get_size_of_file() { return _sizeOfFile;};
        bool set_size_of_file(unsigned int sizeOfFile){ _sizeOfFile = sizeOfFile;};

        /*Pull commands*/
        bool add_pull_chunk(UDTPChunk& _chunk);
        bool write_chunk();
        bool create_push_chunks(unsigned short amount);
        std::vector<UDTPChunk> get_push_chunks(unsigned short amount);
        UDTPChunk& get_single_push_chunk(unsigned short index);
        /*Push commands*/

    private:
        UDTPPath* _pathOfFile;
        unsigned short _fileIdentifier;
        unsigned short _numberOfChunks;
        unsigned int _sizeOfFile;

        bool _fileExists; /*Boolean for file existance. Depending on if the user used send or push, we will cancel
                                immediately without sending a header if the file does not exist.*/

        std::vector<bool> _chunksCompleted; /*Upon instancing of class, this will store 'False' numberOfChunks bools into the vector.
                                                        Usage for both push and pull can apply to this vector.*/
        std::queue<UDTPChunk> _chunksPull; /*Chunks received from pull (recv())*/
        std::vector<UDTPChunk> _chunksPush; /*Chunks created from fstream and stored in here. Vectors will allow to shotgun send()*/

        bool _hasApprovedHeader; /*Starts out with false. Waits until it receives an approved header.*/
        bool _active; /*Sets the file in activity*/
        time_t _timeStarted;

        std::fstream _inputFileStream;
        std::fstream _outputFileStream;
};

#endif
