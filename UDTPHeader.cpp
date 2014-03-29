#include "UDTPHeader.h"
/*Implementations after forward declaration*/
#include "UDTP.h"
#include "UDTPFile.h"
#include "UDTPPath.h"

UDTPHeader::~UDTPHeader(){
    _pathOfFile = NULL;
}

UDTPHeader::UDTPHeader(UDTPPacketHeader header){
    _header = header;
    // TODO - need to allocate _raw buffer size based on header.packetSize
    // need to do this FOR ALL packet types;
}

bool UDTPHeader::pack(){
    return true;
}
bool UDTPHeader::unpack(){
}
bool UDTPHeader::respond(){

    if(_transferType == Pull){  /**************************************************************************************************************************/

        if(get_response_code() == ResponseNone) { /*New file came remotely!*/
                UDTPFile* processingFile = new UDTPFile(get_path_of_file()->get_address_path());
                if(!processingFile->check_file_exist()){ /*File doesn't exist over here!*/
                     _myUDTP->display_msg("File system (local) does not contain the file.");
                    set_response_code(ResponseFileNotFound);
                    delete processingFile;
                    return true;
                }else{

                    processingFile->retrieve_info_from_local_file();
                    if(get_file_id() == 0) set_file_id(_myUDTP->get_next_file_id()); /*Set the ID code. PEER's will send it as 0. Hosts will start at 1+*/
                    processingFile->pack_to_header(*this); /*Pack the information to this header now!*/
                    processingFile->set_approved_header(true);
                    processingFile->set_approver(true); /*Sets that you were the approver.  That means the PEER was trying to PULL while you are going to PUSH for him*/
                    _myUDTP->add_file_to_active(processingFile);

                    set_response_code(ResponseApproved);
                    return true;
                }
            }
            if(get_response_code() == ResponseFileNotFound){
                _myUDTP->display_msg("File system (remote) does not contain the file.");
                return false;
            }
            if(get_response_code() == ResponseApproved){ /*Now it's approved! And back to the original person*/
                UDTPFile* processingFile = new UDTPFile(get_path_of_file()->get_address_path());
                processingFile->unpack_from_header(*this); /*Take the information from the packet! Start getting ready to recv()!*/  /*New everything, even id number*/
                processingFile->set_approved_header(true);
                processingFile->set_approver(false);
                _myUDTP->add_file_to_active(processingFile);

                processingFile->start_file_processing(); /*Start recv()ing!*/

                set_response_code(ResponseConfirm); /*needs to receive a confirm*/
                return true;
            }
            if(get_response_code() == ResponseConfirm){ /*The reason why Confirm is here is because we need the other person to be ready with recv(), otherwise the packets will be lost!*/
                _myUDTP->get_file_by_id(get_file_id())->start_file_processing(); /*We won't use processingFile* anymore, we aren't creating anything new! Now we are send()ing since PEER has confirmed he is ready.*/
                return false; /*No more response!*/
            }
    }

    if(_transferType == Push){ /**************************************************************************************************************************/
        if(get_response_code() == ResponseNone){ /*Someone wants to push!*/
            UDTPFile* processingFile = new UDTPFile(get_path_of_file()->get_address_path());
            if(processingFile->check_file_exist()){
                 _myUDTP->display_msg("File system (local) already has a file with the same name.");
                set_response_code(ResponseFileExistsAlready); /*I don't want you to overwrite my files by pushing!!*/
                delete processingFile;
                return true;
            }else{
                processingFile->unpack_from_header(*this);
                if(get_file_id() == 0) set_file_id(_myUDTP->get_next_file_id()); /*Set the ID code. PEER's will begin off at 0, and usually Hosts will start at 1.*/
                processingFile->set_approver(true);
                processingFile->set_approved_header(true);
                processingFile->start_file_processing(); /*Start recv()ing!*/

                _myUDTP->add_file_to_active(processingFile);
                set_response_code(ResponseApproved);
                return true;
            }
        }
        if(get_response_code() == ResponseFileExistsAlready){
            _myUDTP->display_msg("File system (remote) already has a file with the same name.");
            return false;
        }
        if(get_response_code() == ResponseApproved){
            UDTPFile* processingFile = new UDTPFile(get_path_of_file()->get_address_path());
            processingFile->unpack_from_header(*this); /*New everything, even id number*/
            processingFile->set_approver(false);
            processingFile->start_file_processing();
            _myUDTP->add_file_to_active(processingFile);
            return false;
        }
    }


}
