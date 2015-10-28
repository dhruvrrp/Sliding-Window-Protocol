#ifndef __CRC_H__
#define __CRC_H__

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
#include "common.h"
#include "util.h"
#include "communicate.h"

char get_bit(char byte, int pos);
char crc8(char * array, int char_len);
void append_crc(char * array, int array_len);
int is_corrupted(char * array, int array_len);

#endif
