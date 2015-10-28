#include "receiver.h"

void init_receiver(Receiver * receiver,
                   int id)
{
    receiver->recv_id = id;
    receiver->input_framelist_head = NULL;
    pthread_cond_init(&receiver->buffer_cv, NULL);
    pthread_mutex_init(&receiver->buffer_mutex, NULL);
    receiver->SWP_list_head = NULL;
}



Receiver_SWP * get_SWP_instanceR(Receiver * receiver, uint16_t sendID)
{
int count = 0;
    int i = 0;
    Receiver_SWP *FOUND_SWP;
    Receiver_SWP *cur_SWP;
    int SWP_list_length = ll_get_length(receiver->SWP_list_head);
    if(SWP_list_length == 0)
    {
        cur_SWP = (Receiver_SWP *) malloc(sizeof(Receiver_SWP));
        cur_SWP->Frame_buffer_head = NULL;
        cur_SWP->LFR = -1;
        cur_SWP->LAF = 7;
        cur_SWP->str = 0;
        int k =0;
        for(k = 0; i< 8; i++)
        cur_SWP->ACK[k] = 0;
        cur_SWP->ACK_c = 0;
        cur_SWP->sender = sendID;

        ll_append_node(&receiver->SWP_list_head, cur_SWP);
      //  memcpy(cur_SWP, r_SWP, sizeof(Receiver_SWP));
      //  free(r_SWP);
    }
    else
    {
        count = 0;
        for(i = 0; i < SWP_list_length; i++)
        {
            LLnode * oldhead = ll_pop_node(&receiver->SWP_list_head);
            Receiver_SWP * check = (Receiver_SWP *) oldhead->value;
            if(check->sender == sendID)
            {
                count = 1;
                FOUND_SWP = check;
             //   fprintf(stderr, "Found_SWP %p \n", check);
           //     memcpy(cur_SWP, check, sizeof(Sender_SWP));
                i = SWP_list_length + 2;
            }
            ll_append_node(&receiver->SWP_list_head, check);
  //          free(check);
        }
        if(count == 0)
        {
            cur_SWP = (Receiver_SWP *) malloc(sizeof(Receiver_SWP));
            cur_SWP->Frame_buffer_head = NULL;
            cur_SWP->LFR = -1;
            cur_SWP->LAF = 7;
            int k =0;
            for(k = 0; i< 8; i++)
                cur_SWP->ACK[k] = 0;
            cur_SWP->ACK_c = 0;
            cur_SWP->str = 0;
            cur_SWP->sender = sendID;
            ll_append_node(&receiver->SWP_list_head, cur_SWP);
    //        memcpy(cur_SWP, r_SWP, sizeof(Sender_SWP));
      //      free(r_SWP);
        }
    }
if(count == 1)
return FOUND_SWP;
    return cur_SWP;
}


void handle_incoming_msgs(Receiver * receiver,
                          LLnode ** outgoing_frames_head_ptr)
{
    //TODO: Suggested steps for handling incoming frames
    //    1) Dequeue the Frame from the sender->input_framelist_head
    //    2) Convert the char * buffer to a Frame data type
    //    3) Check whether the frame is corrupted
    //    4) Check whether the frame is for this receiver
    //    5) Do sliding window protocol for sender/receiver pair

    int incoming_msgs_length = ll_get_length(receiver->input_framelist_head);
    while (incoming_msgs_length > 0)
    {
        //Pop a node off the front of the link list and update the count
        LLnode * ll_inmsg_node = ll_pop_node(&receiver->input_framelist_head);
        incoming_msgs_length = ll_get_length(receiver->input_framelist_head);
        int cor = 0;
        //DUMMY CODE: Print the raw_char_buf
        //NOTE: You should not blindly print messages!
        //      Ask yourself: Is this message really for me?
        //                    Is this message corrupted?
        //                    Is this an old, retransmitted message?           
        char * raw_char_buf = (char *) ll_inmsg_node->value;
        if(is_corrupted(raw_char_buf, strlen(raw_char_buf)))
        {
            fprintf(stderr, "xxxCORRUPT-PACKETxxx\n");
            cor = 1; 
        }
        Frame * inframe = convert_char_to_frame(raw_char_buf);
        Receiver_SWP * cur_SWP;
   //     cur_SWP = get_SWP_instanceR(receiver, inframe->senderID);
        //Free raw_char_buf
        free(raw_char_buf);
        //Check if data belongs to this receiver and send ACK
        if(inframe->receiverID == receiver->recv_id && cor == 0) 
        {
fprintf(stderr, "NO CORRUPTION\n");
            cur_SWP = get_SWP_instanceR(receiver, inframe->senderID);
 //ADD CASE FOR DUPLICATES
            if((inframe->sequence >= cur_SWP->LFR && inframe->sequence < cur_SWP->LAF)||cur_SWP->str == 0)
            {
                ll_append_node(&cur_SWP->Frame_buffer_head, inframe);
            }
            int i = 0;
            for(i = 0; i < 8; i++)
            {
                if(inframe->sequence == cur_SWP->ACK[i])
                {
                    Frame * rep_c = (Frame *) malloc(sizeof(Frame));
                    uint16_t rtemp = inframe->senderID;
                    rep_c->senderID = inframe->receiverID;
                    rep_c->receiverID = rtemp;
                    rep_c->ACK = 1;
                    rep_c->sequence = inframe->sequence;
                    char * r_char_buf = (char *) convert_frame_to_char(rep_c);
                    append_crc(r_char_buf, strlen(r_char_buf));
                    ll_append_node(outgoing_frames_head_ptr,
                                   r_char_buf);
                }
            }
            int Frame_buffer_length = ll_get_length(cur_SWP->Frame_buffer_head);
            int count = Frame_buffer_length;
            while(count > 0)
            {
                LLnode * cur = ll_pop_node(&cur_SWP->Frame_buffer_head);
                count --;
                Frame * curr = (Frame *) malloc(sizeof(Frame));
                memcpy(curr, cur->value, sizeof(Frame));
               // curr = (Frame *) cur->value;
                unsigned char aa = curr->sequence;
                unsigned char cLFR = cur_SWP->LFR +1;
                if(aa == (cLFR))
                {
                    printf("<RECV_%d>:[%s]\n", receiver->recv_id, 
                           inframe->data);

                        
                    curr->ACK = 1;
                    uint16_t temp = inframe->senderID;
                    curr->senderID = inframe->receiverID;
                    curr->sequence = inframe->sequence;
                    curr->receiverID = temp;
                    raw_char_buf = (char *) convert_frame_to_char(curr);
                    append_crc(raw_char_buf, strlen(raw_char_buf));
                    ll_append_node(outgoing_frames_head_ptr,
                          raw_char_buf);
                    cur_SWP->LFR = cur_SWP->LFR+1;
                    cur_SWP->LAF = cur_SWP->LAF+1;
                    cur_SWP->ACK[cur_SWP->ACK_c%8] = cur_SWP->LFR;
                    count = Frame_buffer_length-1;
                    if(cur_SWP->str == 0){//cur_SWP->LFR = 0;
                        cur_SWP->str =1;}
                }
                else
                {
                    ll_append_node(&cur_SWP->Frame_buffer_head, curr); 
                }
                free(curr);
            }
        }
        free(inframe);
        free(ll_inmsg_node);
    }
}

void * run_receiver(void * input_receiver)
{    
    struct timespec   time_spec;
    struct timeval    curr_timeval;
    const int WAIT_SEC_TIME = 0;
    const long WAIT_USEC_TIME = 100000;
    Receiver * receiver = (Receiver *) input_receiver;
    LLnode * outgoing_frames_head;


    //This incomplete receiver thread, at a high level, loops as follows:
    //1. Determine the next time the thread should wake up if there is nothing in the incoming queue(s)
    //2. Grab the mutex protecting the input_msg queue
    //3. Dequeues messages from the input_msg queue and prints them
    //4. Releases the lock
    //5. Sends out any outgoing messages

    
    while(1)
    {    
        //NOTE: Add outgoing messages to the outgoing_frames_head pointer
        outgoing_frames_head = NULL;
        gettimeofday(&curr_timeval, 
                     NULL);

        //Either timeout or get woken up because you've received a datagram
        //NOTE: You don't really need to do anything here, but it might be useful for debugging purposes to have the receivers periodically wakeup and print info
        time_spec.tv_sec  = curr_timeval.tv_sec;
        time_spec.tv_nsec = curr_timeval.tv_usec * 1000;
        time_spec.tv_sec += WAIT_SEC_TIME;
        time_spec.tv_nsec += WAIT_USEC_TIME * 1000;
        if (time_spec.tv_nsec >= 1000000000)
        {
            time_spec.tv_sec++;
            time_spec.tv_nsec -= 1000000000;
        }

        //*****************************************************************************************
        //NOTE: Anything that involves dequeing from the input frames should go 
        //      between the mutex lock and unlock, because other threads CAN/WILL access these structures
        //*****************************************************************************************
        pthread_mutex_lock(&receiver->buffer_mutex);

        //Check whether anything arrived
        int incoming_msgs_length = ll_get_length(receiver->input_framelist_head);
        if (incoming_msgs_length == 0)
        {
            //Nothing has arrived, do a timed wait on the condition variable (which releases the mutex). Again, you don't really need to do the timed wait.
            //A signal on the condition variable will wake up the thread and reacquire the lock
            pthread_cond_timedwait(&receiver->buffer_cv, 
                                   &receiver->buffer_mutex,
                                   &time_spec);
        }

        handle_incoming_msgs(receiver,
                             &outgoing_frames_head);

        pthread_mutex_unlock(&receiver->buffer_mutex);
        
        //CHANGE THIS AT YOUR OWN RISK!
        //Send out all the frames user has appended to the outgoing_frames list
        int ll_outgoing_frame_length = ll_get_length(outgoing_frames_head);
        while(ll_outgoing_frame_length > 0)
        {
            LLnode * ll_outframe_node = ll_pop_node(&outgoing_frames_head);
            char * char_buf = (char *) ll_outframe_node->value;
            
            //The following function frees the memory for the char_buf object
            send_msg_to_senders(char_buf);

            //Free up the ll_outframe_node
            free(ll_outframe_node);

            ll_outgoing_frame_length = ll_get_length(outgoing_frames_head);
        }
    }
    pthread_exit(NULL);

}
