#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <math.h>
#include <sys/time.h>

#define MAX_COMMAND_LENGTH 16
#define AUTOMATED_FILENAME 512
typedef unsigned char uchar_t;

//Largest id (in bytes) of threads (sender and receiver)
#define MAX_THREAD_ID 2
//System configuration information
struct SysConfig_t
{
    float drop_prob;
    float corrupt_prob;
    unsigned char automated;
    char automated_file[AUTOMATED_FILENAME];
};
typedef struct SysConfig_t  SysConfig;

//Command line input information
struct Cmd_t
{
    uint16_t src_id;
    uint16_t dst_id;
    char * message;
};
typedef struct Cmd_t Cmd;

//Linked list information
enum LLtype 
{
    llt_string,
    llt_frame,
    llt_integer,
    llt_head
} LLtype;

struct LLnode_t
{
    struct LLnode_t * prev;
    struct LLnode_t * next;
    enum LLtype type;

    void * value;
};
typedef struct LLnode_t LLnode;


//Receiver and sender data structures
struct Receiver_t
{
    //DO NOT CHANGE:
    // 1) buffer_mutex
    // 2) buffer_cv
    // 3) input_framelist_head
    // 4) recv_id
    pthread_mutex_t buffer_mutex;
    pthread_cond_t buffer_cv;
    LLnode * input_framelist_head;
    
    int recv_id;

    LLnode * SWP_list_head;
};

struct Sender_t
{
    //DO NOT CHANGE:
    // 1) buffer_mutex
    // 2) buffer_cv
    // 3) input_cmdlist_head
    // 4) input_framelist_head
    // 5) send_id
    pthread_mutex_t buffer_mutex;
    pthread_cond_t buffer_cv;    
    LLnode * input_cmdlist_head;
    LLnode * input_framelist_head;
    int send_id;
    LLnode * SWP_list_head;
};

//Sliding Window Protocol structs for sender and receivers

struct Sender_sw
{
    unsigned char LAR;
    unsigned char LFS;
    uint16_t receiver;
    LLnode * Frame_buffer_head;
    LLnode * exp_time;
    LLnode * ACK_rec;

    char * msg_buffer;
};
typedef struct Sender_sw Sender_SWP;
struct Receiver_sw
{
    unsigned char LFR;
    unsigned char LAF;
    uint16_t sender;
int str;
    LLnode * Frame_buffer_head;
    unsigned char ACK[8];
    uint8_t ACK_c;
};
typedef struct Receiver_sw Receiver_SWP;
enum SendFrame_DstType 
{
    ReceiverDst,
    SenderDst
} SendFrame_DstType ;

typedef struct Sender_t Sender;
typedef struct Receiver_t Receiver;


#define MAX_FRAME_SIZE 64

//TODO: You should change this!
//Remember, your frame can be AT MOST 64 bytes!
#define FRAME_PAYLOAD_SIZE 57
struct Frame_t
{
    uint16_t senderID;
    uint16_t receiverID;
    uint8_t sequence;
    char ACK;
    char data[FRAME_PAYLOAD_SIZE];
    unsigned char footer;
};
typedef struct Frame_t Frame;


//Declare global variables here
//DO NOT CHANGE: 
//   1) glb_senders_array
//   2) glb_receivers_array
//   3) glb_senders_array_length
//   4) glb_receivers_array_length
//   5) glb_sysconfig
//   6) CORRUPTION_BITS
Sender * glb_senders_array;
Receiver * glb_receivers_array;
int glb_senders_array_length;
int glb_receivers_array_length;
SysConfig glb_sysconfig;
int CORRUPTION_BITS;
#endif 
