#include "UDTPAcknowledge.h"
#include "UDTP.h"
#include "UDTPPeer.h"
/*Implementations after class forward declaration*/

UDTPAcknowledge::UDTPAcknowledge(UDTPPacketHeader packet)
{
}

char* UDTPAcknowledge::get_raw_buffer()
{
    return 0;
}

bool UDTPAcknowledge::pack()
{
    return true;
}

bool UDTPAcknowledge::unpack()
{
    return true;
}

bool UDTPAcknowledge::respond()
{

    if(get_acknowledge_type() == AcknowledgePacketCount)  /*Usually sent by PEER and since HOST has a copy of that peer then he can just check local*/
    {
        unsigned int percentageOfSuccessPacket;
        switch(get_response_code())
        {
        case ResponseNone:
           percentageOfSuccessPacket = get_packets_count() / udtp()->get_peer(get_peer_id())->get_packets_count();
            if(percentageOfSuccessPacket <= udtp()->setup()->get_critical_loss_packet_percentage())  /*PEER is sending/receiving more than the HOST*/
            {
                if(peer()->get_thread_count() != 0)
                {
                    set_response_code(ResponseNone);
                    set_acknowledge_type(AcknowledgeRemoveThread); /*Informs to remove a thread on the other side*/
                    peer()->remove_thread(); /*Locally removes a thread!*/
                    udtp()->display_msg("Removed thread from PEER");
                }
                return true;
            }
            if(percentageOfSuccessPacket >= udtp()->setup()->get_satisfactory_loss_packet_percentage())  /*PEER is sending/receiving more than the HOST*/
            {
                if(peer()->get_thread_count() < udtp()->setup()->get_max_number_of_flow_sockets())
                {
                    set_response_code(ResponseNone);
                    set_acknowledge_type(AcknowledgeAddThread); /*Informs to add a thread on the other side*/
                    udtp()->display_msg("Sent a request to Add Threads to PEER");
                    return true;
                }
            }
                                udtp()->display_msg("Packet count is okay! No changes to thread count");
                set_response_code(ResponseConfirm); /*Confirms that there will be no modification!*/
                set_acknowledge_type(AcknowledgePacketCount);
                break;
            case ResponseConfirm:
                    udtp()->display_msg("Sent a request to Remove Threads to PEER");
                return false;
                break;

            }
    }

            if(get_acknowledge_type() == AcknowledgeAddThread)
            {
                switch(get_response_code())
                {
                case ResponseNone: /*PEER's turn to add thread now!*/
                    peer()->add_thread() == true ? set_response_code(ResponseApproved) : set_response_code(ResponseRejected);
                    set_response_code(ResponseRejected);
                    set_acknowledge_type(AcknowledgeAddThread);
                    return true; /*Send it off!*/
                    break;

                case ResponseApproved:
                udtp()->display_msg("PEER has approved of thread increase.");
                peer()->add_thread();
                break;

                case ResponseRejected:
                udtp()->display_msg("PEER has rejected thread increase");
                break;
                    }
            }


            if(get_acknowledge_type() == AcknowledgeRemoveThread)
            {
                switch(get_response_code())
                {
                case ResponseNone: /*PEER's turn to add thread now!*/
                    peer()->remove_thread() == true ? set_response_code(ResponseApproved) : set_response_code(ResponseRejected);
                    set_response_code(ResponseRejected);
                    set_acknowledge_type(AcknowledgeRemoveThread);
                    return true; /*Send it off!*/
                    break;

                case ResponseApproved:
                udtp()->display_msg("PEER has approved of removal of thread");
                peer()->add_thread();
                break;

                case ResponseRejected:
                udtp()->display_msg("PEER has rejected removal of threads");
                break;
                    }
            }

        return true;
    }
