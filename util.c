#include "util.h"

//Linked list functions
int ll_get_length(LLnode * head)
{
    LLnode * tmp;
    int count = 1;
    if (head == NULL)
        return 0;
    else
    {
        tmp = head->next;
        while (tmp != head)
        {
            count++;
            tmp = tmp->next;
        }
        return count;
    }
}

void ll_append_node(LLnode ** head_ptr, 
                    void * value)
{
    LLnode * prev_last_node;
    LLnode * new_node;
    LLnode * head;

    if (head_ptr == NULL)
    {
        return;
    }
    
    //Init the value pntr
    head = (*head_ptr);
    new_node = (LLnode *) malloc(sizeof(LLnode));
    new_node->value = value;

    //The list is empty, no node is currently present
    if (head == NULL)
    {
        (*head_ptr) = new_node;
        new_node->prev = new_node;
        new_node->next = new_node;
    }
    else
    {
        //Node exists by itself
        prev_last_node = head->prev;
        head->prev = new_node;
        prev_last_node->next = new_node;
        new_node->next = head;
        new_node->prev = prev_last_node;
    }
}


LLnode * ll_pop_node(LLnode ** head_ptr)
{
    LLnode * last_node;
    LLnode * new_head;
    LLnode * prev_head;

    prev_head = (*head_ptr);
    if (prev_head == NULL)
    {
        return NULL;
    }
    last_node = prev_head->prev;
    new_head = prev_head->next;

    //We are about to set the head ptr to nothing because there is only one thing in list
    if (last_node == prev_head)
    {
        (*head_ptr) = NULL;

        prev_head->next = NULL;
        prev_head->prev = NULL;
        return prev_head;
    }
    else
    {
        (*head_ptr) = new_head;
        last_node->next = new_head;
        new_head->prev = last_node;

        prev_head->next = NULL;
        prev_head->prev = NULL;
        return prev_head;
    }
}

void ll_destroy_node(LLnode * node)
{
    if (node->type == llt_string)
    {
        free((char *) node->value);
    }
    free(node);
}


//Get function for LLnode. Returns null if n is larger than length
LLnode * ll_get(uint16_t n, LLnode ** head_ptr)
{
    if(ll_get_length(*head_ptr) < n)
        return NULL;
    LLnode * cur = (*head_ptr);
    int i = 0;
    for(i = 0; i < n; i++)
    {
       cur = cur->next;
    }
    return cur;
}
//Compute the difference in usec for two timeval objects
long timeval_usecdiff(struct timeval *start_time, 
                      struct timeval *finish_time)
{
  long usec;
  usec=(finish_time->tv_sec - start_time->tv_sec)*1000000;
  usec+=(finish_time->tv_usec- start_time->tv_usec);
  return usec;
}


//Print out messages entered by the user
void print_cmd(Cmd * cmd)
{
    fprintf(stderr, "src=%d, dst=%d, message=%s\n", 
           cmd->src_id,
           cmd->dst_id,
           cmd->message);
}


char * convert_frame_to_char(Frame * frame)
{
    char * char_buffer = (char *) malloc(MAX_FRAME_SIZE);
    memset(char_buffer,
           0,
           MAX_FRAME_SIZE);

//   snprintf(char_buffer, "%d", 
//             frame->senderID, sizeof(frame->senderID));
 /*   char * temp = (char *) malloc(MAX_FRAME_SIZE-FRAME_PAYLOAD_SIZE);
    memset(temp, 0, MAX_FRAME_SIZE-FRAME_PAYLOAD_SIZE);
    sprintf(temp, "%d", frame->senderID);
    strcat(char_buffer, temp);
    memset(temp, 0, MAX_FRAME_SIZE-FRAME_PAYLOAD_SIZE);
    sprintf(temp, "%d", frame->receiverID);
    strcat(char_buffer, temp);
    char * SeqACK = (char *) malloc(2*sizeof(char));
    SeqACK[0] = frame->sequence;
    SeqACK[1] = frame->ACK;
    strcat(char_buffer, SeqACK);
    strcat(char_buffer, frame->data);
*/
    memcpy(char_buffer,
           &frame->senderID,
           sizeof(frame->senderID));
    memmove(char_buffer+sizeof(uint16_t),
           &frame->receiverID,
           sizeof(frame->receiverID));
    memmove(char_buffer+2*sizeof(uint16_t),
           &frame->sequence,
           sizeof(char));
    memmove(char_buffer+2*sizeof(uint16_t)+sizeof(char),
           &frame->ACK,
           sizeof(char));
    memmove(char_buffer+2*(sizeof(uint16_t)+sizeof(char)), 
           frame->data,
           FRAME_PAYLOAD_SIZE);
    return char_buffer;
}


Frame * convert_char_to_frame(char * char_buf)
{
    Frame * frame = (Frame *) malloc(sizeof(Frame));
    fprintf(stderr, char_buf);
    memset(frame->data,
           0,
           FRAME_PAYLOAD_SIZE);
    memcpy(&frame->senderID, 
           char_buf,
           2);
    memcpy(&frame->receiverID,
           char_buf+2,
           2);
    frame->sequence = char_buf[4];
    frame->ACK = char_buf[5];




    memcpy(frame->data,
           char_buf+6,
           FRAME_PAYLOAD_SIZE);
    return frame;
}
