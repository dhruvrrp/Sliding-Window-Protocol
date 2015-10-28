#include "sender.h"

void init_sender(Sender * sender, int id)
{
    //TODO: You should fill in this function as necessary
    sender->send_id = id;
    sender->input_cmdlist_head = NULL;
    sender->input_framelist_head = NULL;
    pthread_cond_init(&sender->buffer_cv, NULL);
    pthread_mutex_init(&sender->buffer_mutex, NULL);

    sender->SWP_list_head = NULL;
}




struct timeval * sender_get_next_expiring_timeval(Sender * sender)
{
    //TODO: You should fill in this function so that it returns the next timeout that should occur
    return NULL;
}


Sender_SWP * get_SWP_instance(Sender * sender, uint16_t recvID)
{
int count = 0;
    int i = 0;
Sender_SWP * foundSWP;
    Sender_SWP *cur_SWP = (Sender_SWP *) malloc(sizeof(Sender_SWP));
    int SWP_list_length = ll_get_length(sender->SWP_list_head);
//fprintf(stderr, "SWP length %d\n",SWP_list_length);
    if(SWP_list_length == 0)
    {
//        Sender_SWP * s_SWP = (Sender_SWP *) malloc(sizeof(Sender_SWP));
        cur_SWP->Frame_buffer_head = NULL;
        cur_SWP->LAR = -1;
        cur_SWP->LFS = -1;
        cur_SWP->exp_time = NULL;
        cur_SWP->msg_buffer = "";
        cur_SWP->receiver = recvID;
        cur_SWP->ACK_rec = NULL;
        ll_append_node(&sender->SWP_list_head, cur_SWP);
    //    memcpy(cur_SWP, s_SWP, sizeof(Sender_SWP));

    }
    else
    {
        for(i = 0; i < SWP_list_length; i++)
        {
            LLnode * oldhead = ll_pop_node(&sender->SWP_list_head);
            Sender_SWP * check = (Sender_SWP *) oldhead->value;

            if(check->receiver == recvID)
            {
                count = 1;
                foundSWP = check;
//fprintf(stderr, "FOUND SWP %p\n",foundSWP);
                i = SWP_list_length + 2;
            }
            ll_append_node(&sender->SWP_list_head, check);

        }
        if(count == 0)
        {
   //         Sender_SWP * s_SWP = (Sender_SWP *) malloc(sizeof(Sender_SWP));
            cur_SWP->LAR = -1;
            cur_SWP->LFS = -1;
            cur_SWP->msg_buffer = "";
            cur_SWP->receiver = recvID;
            cur_SWP->exp_time = NULL;
            cur_SWP->ACK_rec = NULL;
            cur_SWP->Frame_buffer_head = NULL;
            ll_append_node(&sender->SWP_list_head, cur_SWP);
  //          memcpy(cur_SWP, s_SWP, sizeof(Sender_SWP));
  //          free(s_SWP);
        }
    }
if(count == 1)
return foundSWP;
SWP_list_length = ll_get_length(sender->SWP_list_head);
//fprintf(stderr, "SWP length %d\n",SWP_list_length);
//fprintf(stderr, "Frame_buffer_head ack %p\n",cur_SWP);
    return cur_SWP;
}

void calculate_timeout(struct timeval * timeout)
{
    gettimeofday(timeout, NULL);
    timeout->tv_usec+=100000;
    if(timeout->tv_usec>=1000000)
    {
        timeout->tv_usec-=1000000;
        timeout->tv_sec+=1;
    }
}
void handle_incoming_acks(Sender * sender,
                          LLnode ** outgoing_frames_head_ptr)
{
    int incoming_msgs_length = ll_get_length(sender->input_framelist_head);
    while (incoming_msgs_length > 0)
    {
        LLnode * ll_inmsg_node = ll_pop_node(&sender->input_framelist_head);
        incoming_msgs_length = ll_get_length(sender->input_framelist_head);
        char * raw_char_buf = (char *) ll_inmsg_node->value;
        Frame * inframe = convert_char_to_frame(raw_char_buf);
        Sender_SWP * cur_SWP = (Sender_SWP *) malloc(sizeof(Sender_SWP));
       
//        cur_SWP = get_SWP_instance(sender, inframe->senderID);
        if(inframe->receiverID == sender->send_id)
        {
            if(inframe->ACK == 1)
            {
cur_SWP = get_SWP_instance(sender, inframe->senderID);
                LLnode * curr_fra;
        //        LLnode * curr_ACK;
                Frame * curr_frame;
                int frame_buffer_length = ll_get_length(cur_SWP->Frame_buffer_head);
                int count = frame_buffer_length;
                while(count > 0)
                {
                    curr_fra = ll_get(count, &cur_SWP->Frame_buffer_head);
                    curr_frame = (Frame *) curr_fra->value;
                    LLnode * curr_ACK = ll_pop_node(&cur_SWP->ACK_rec);
                    if(curr_frame->sequence == inframe->sequence)
                    {
                        ll_append_node(&cur_SWP->ACK_rec, (void*)1);
                    }
                    else
                    {
                        ll_append_node(&cur_SWP->ACK_rec, curr_ACK->value);
                    }
                    count--;
                }
                count = 1;
                LLnode * curr_ACK;
                while(count <= frame_buffer_length)
                {
                    curr_ACK = ll_get(count, &cur_SWP->ACK_rec);
                    if((int)curr_ACK->value == 1)
                    {
                        ll_pop_node(&cur_SWP->ACK_rec);
                        ll_pop_node(&cur_SWP->Frame_buffer_head);
//fprintf(stderr, "cur_SWP ack %p\n",cur_SWP);
  //                      fprintf(stderr, "Frame_buffer_head ack %p\n",&cur_SWP->Frame_buffer_head);
                        ll_pop_node(&cur_SWP->exp_time);
//fprintf(stderr, "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");

   cur_SWP->LAR = (cur_SWP->LAR+1)%255;
                    }
                    count++;
                }
                int char_buffer_size = strlen(cur_SWP->msg_buffer);
                cur_SWP->LAR = inframe->sequence;
                count = 0;
                while((cur_SWP->LFS - cur_SWP->LAR < 8)%255 &&
                      char_buffer_size > 0)
                {
                    cur_SWP->LFS = (cur_SWP->LFS++)%255;
                    Frame * outgoing_frame = (Frame *) malloc (sizeof(Frame));
                    char * temp = (char *) malloc(FRAME_PAYLOAD_SIZE);
                    strncpy(temp, cur_SWP->msg_buffer+(count * FRAME_PAYLOAD_SIZE), FRAME_PAYLOAD_SIZE);
                    strcpy(outgoing_frame->data, temp);
                    outgoing_frame->senderID = sender->send_id;
                    outgoing_frame->receiverID = inframe->senderID;
                    outgoing_frame->ACK = 0x0;
                    outgoing_frame->sequence = cur_SWP->LFS;
                    struct timeval * timeout = (struct timeval *) malloc(sizeof(struct timeval));
                    calculate_timeout(timeout);
                    ll_append_node(&cur_SWP->exp_time, timeout);
                    char * outgoing_charbuf = convert_frame_to_char(outgoing_frame);
                    append_crc(outgoing_charbuf, strlen(outgoing_charbuf));
                    ll_append_node(outgoing_frames_head_ptr, outgoing_charbuf);
                    free(outgoing_frame);
                    free(temp);                             
                    count++;
                }
            } 
        }
    }          
             


   
    //TODO: Suggested steps for handling incoming ACKs
    //    1) Dequeue the ACK from the sender->input_framelist_head
    //    2) Convert the char * buffer to a Frame data type
    //    3) Check whether the frame is corrupted
    //    4) Check whether the frame is for this sender
    //    5) Do sliding window protocol for sender/receiver pair   

}
void handle_input_cmds(Sender * sender,
                       LLnode ** outgoing_frames_head_ptr)
{
    //TODO: Suggested steps for handling input cmd
    //    1) Dequeue the Cmd from sender->input_cmdlist_head
    //    2) Convert to Frame
    //    3) Set up the frame according to the sliding window protocol
    //    4) Compute CRC and add CRC to Frame

    int input_cmd_length = ll_get_length(sender->input_cmdlist_head);
    
    int i = 0;    
    //Recheck the command queue length to see if stdin_thread dumped a command on us
    input_cmd_length = ll_get_length(sender->input_cmdlist_head);
    while (input_cmd_length > 0)
    {
        //Pop a node off and update the input_cmd_length
        LLnode * ll_input_cmd_node = ll_pop_node(&sender->input_cmdlist_head);
        input_cmd_length = ll_get_length(sender->input_cmdlist_head);

        //Cast to Cmd type and free up the memory for the node
        Cmd * outgoing_cmd = (Cmd *) ll_input_cmd_node->value;
        free(ll_input_cmd_node);
        Sender_SWP *cur_SWP = (Sender_SWP *) malloc(sizeof(Sender_SWP));
        
        cur_SWP = get_SWP_instance(sender, outgoing_cmd->dst_id); 
        //DUMMY CODE: Add the raw char buf to the outgoing_frames list
        //NOTE: You should not blindly send this message out!
        //      Ask yourself: Is this message actually going to the right receiver (recall that default behavior of send is to broadcast to all receivers)?
        //                    Does the receiver have enough space in in it's input queue to handle this message?
        //                    Were the previous messages sent to this receiver ACTUALLY delivered to the receiver?
        int msg_length = strlen(outgoing_cmd->message);
        if (msg_length > FRAME_PAYLOAD_SIZE)
        {
            uint16_t num_frames = msg_length / FRAME_PAYLOAD_SIZE;
            num_frames += msg_length % FRAME_PAYLOAD_SIZE;
            for(i = 0; i < num_frames; i++)
            {
                if((cur_SWP->LFS - cur_SWP->LAR) < 8)
                {
                    cur_SWP->LFS = (cur_SWP->LFS++)%255;
                    Frame * outgoing_frame = (Frame *) malloc (sizeof(Frame));
                    char * temp = (char *) malloc(FRAME_PAYLOAD_SIZE);
                    strncpy(temp, outgoing_cmd->message+(i * FRAME_PAYLOAD_SIZE), FRAME_PAYLOAD_SIZE);
                    strcpy(outgoing_frame->data, temp);
                    outgoing_frame->senderID = outgoing_cmd->src_id;
                    outgoing_frame->receiverID = outgoing_cmd->dst_id;
                    outgoing_frame->ACK = 0x0;
                    outgoing_frame->sequence = cur_SWP->LFS;
                    ll_append_node(&cur_SWP->Frame_buffer_head, outgoing_frame);
                    struct timeval * timeout = (struct timeval *) 
                                               malloc(sizeof(struct timeval));
                    calculate_timeout(timeout);
                    ll_append_node(&cur_SWP->exp_time, timeout);
                    ll_append_node(&cur_SWP->ACK_rec, 0);
                    char * outgoing_charbuf = convert_frame_to_char(outgoing_frame);
                    append_crc(outgoing_charbuf, strlen(outgoing_charbuf));
                    ll_append_node(outgoing_frames_head_ptr, outgoing_charbuf);
                    free(outgoing_frame);
                    free(temp);
                }
                else
                {
                    if(cur_SWP->msg_buffer == NULL)
                        strncpy(cur_SWP->msg_buffer, 
                                outgoing_cmd->message+(i*FRAME_PAYLOAD_SIZE), FRAME_PAYLOAD_SIZE);
                    else
                        strncat(cur_SWP->msg_buffer, 
                                outgoing_cmd->message+(i*FRAME_PAYLOAD_SIZE), FRAME_PAYLOAD_SIZE);
                }
            }
        }
        else
        {
            if((cur_SWP->LFS - cur_SWP->LAR) < 8)
            {
                cur_SWP->LFS++;
            //This is probably ONLY one step you want
                Frame * outgoing_frame = (Frame *) malloc (sizeof(Frame));
                strcpy(outgoing_frame->data, outgoing_cmd->message);
                outgoing_frame->senderID = outgoing_cmd->src_id;
                outgoing_frame->receiverID = outgoing_cmd->dst_id;
                outgoing_frame->ACK = 0x0;
                outgoing_frame->sequence = cur_SWP->LFS;
//fprintf(stderr, "+++++++++++++++++++++++++\n");
//fprintf(stderr, "cur_SWP cmd %p\n",cur_SWP);
//fprintf(stderr, "Frame_buffer_head cmd %p\n",&cur_SWP->Frame_buffer_head);
//fprintf(stderr, "Frame_buffer_head size %d\n",ll_get_length(cur_SWP->Frame_buffer_head));
                ll_append_node(&cur_SWP->Frame_buffer_head, outgoing_frame);
                struct timeval * timeout = (struct timeval *) malloc(sizeof(struct timeval));
                calculate_timeout(timeout);
                ll_append_node(&cur_SWP->ACK_rec, 0);
                ll_append_node(&cur_SWP->exp_time, timeout);
            //At this point, we don't need the outgoing_cmd
  //              free(outgoing_cmd->message);
 //               free(outgoing_cmd);

            //Convert the message to the outgoing_charbuf
                char * outgoing_charbuf = convert_frame_to_char(outgoing_frame);
                append_crc(outgoing_charbuf, strlen(outgoing_charbuf));
                ll_append_node(outgoing_frames_head_ptr,
                           outgoing_charbuf);
              //  free(outgoing_frame);
            }
            else
            {
                if(cur_SWP->msg_buffer == NULL)
                    strcpy(cur_SWP->msg_buffer, outgoing_cmd->message);
                else
                    strcat(cur_SWP->msg_buffer, outgoing_cmd->message);
            }
        }
        free(outgoing_cmd->message);
        free(outgoing_cmd);
    }   
}


void handle_timedout_frames(Sender * sender,
                            LLnode ** outgoing_frames_head_ptr)
{
 
    //TODO: Suggested steps for handling timed out datagrams
    //    1) Iterate through the sliding window protocol information you maintain for each receiver
    //    2) Locate frames that are timed out and add them to the outgoing frames
    //    3) Update the next timeout field on the outgoing frames
}


void * run_sender(void * input_sender)
{    
    struct timespec   time_spec;
    struct timeval    curr_timeval;
    const int WAIT_SEC_TIME = 0;
    const long WAIT_USEC_TIME = 100000;
    Sender * sender = (Sender *) input_sender;    
    LLnode * outgoing_frames_head;
    struct timeval * expiring_timeval;
    long sleep_usec_time, sleep_sec_time;
    
    //This incomplete sender thread, at a high level, loops as follows:
    //1. Determine the next time the thread should wake up
    //2. Grab the mutex protecting the input_cmd/inframe queues
    //3. Dequeues messages from the input queue and adds them to the outgoing_frames list
    //4. Releases the lock
    //5. Sends out the messages


    while(1)
    {    
        outgoing_frames_head = NULL;

        //Get the current time
        gettimeofday(&curr_timeval, 
                     NULL);

        //time_spec is a data structure used to specify when the thread should wake up
        //The time is specified as an ABSOLUTE (meaning, conceptually, you specify 9/23/2010 @ 1pm, wakeup)
        time_spec.tv_sec  = curr_timeval.tv_sec;
        time_spec.tv_nsec = curr_timeval.tv_usec * 1000;

        //Check for the next event we should handle
        expiring_timeval = sender_get_next_expiring_timeval(sender);

        //Perform full on timeout
        if (expiring_timeval == NULL)
        {
            time_spec.tv_sec += WAIT_SEC_TIME;
            time_spec.tv_nsec += WAIT_USEC_TIME * 1000;
        }
        else
        {
            //Take the difference between the next event and the current time
            sleep_usec_time = timeval_usecdiff(&curr_timeval,
                                               expiring_timeval);

            //Sleep if the difference is positive
            if (sleep_usec_time > 0)
            {
                sleep_sec_time = sleep_usec_time/1000000;
                sleep_usec_time = sleep_usec_time % 1000000;   
                time_spec.tv_sec += sleep_sec_time;
                time_spec.tv_nsec += sleep_usec_time*1000;
            }   
        }

        //Check to make sure we didn't "overflow" the nanosecond field
        if (time_spec.tv_nsec >= 1000000000)
        {
            time_spec.tv_sec++;
            time_spec.tv_nsec -= 1000000000;
        }

        
        //*****************************************************************************************
        //NOTE: Anything that involves dequeing from the input frames or input commands should go 
        //      between the mutex lock and unlock, because other threads CAN/WILL access these structures
        //*****************************************************************************************
        pthread_mutex_lock(&sender->buffer_mutex);

        //Check whether anything has arrived
        int input_cmd_length = ll_get_length(sender->input_cmdlist_head);
        int inframe_queue_length = ll_get_length(sender->input_framelist_head);
        
        //Nothing (cmd nor incoming frame) has arrived, so do a timed wait on the sender's condition variable (releases lock)
        //A signal on the condition variable will wakeup the thread and reaquire the lock
        if (input_cmd_length == 0 &&
            inframe_queue_length == 0)
        {
            
            pthread_cond_timedwait(&sender->buffer_cv, 
                                   &sender->buffer_mutex,
                                   &time_spec);
        }
        //Implement this
        handle_incoming_acks(sender,
                             &outgoing_frames_head);

        //Implement this
        handle_input_cmds(sender,
                          &outgoing_frames_head);

        pthread_mutex_unlock(&sender->buffer_mutex);


        //Implement this
        handle_timedout_frames(sender,
                               &outgoing_frames_head);

        //CHANGE THIS AT YOUR OWN RISK!
        //Send out all the frames
        int ll_outgoing_frame_length = ll_get_length(outgoing_frames_head);
        
        while(ll_outgoing_frame_length > 0)
        {
            LLnode * ll_outframe_node = ll_pop_node(&outgoing_frames_head);
            char * char_buf = (char *)  ll_outframe_node->value;

            //Don't worry about freeing the char_buf, the following function does that
            send_msg_to_receivers(char_buf);

            //Free up the ll_outframe_node
            free(ll_outframe_node);

            ll_outgoing_frame_length = ll_get_length(outgoing_frames_head);
        }
    }
    pthread_exit(NULL);
    return 0;
}
