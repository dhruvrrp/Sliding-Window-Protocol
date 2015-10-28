#include "crc.h"
char get_bit(char byte, int pos)
{
    byte = byte >> (pos);
    char result = byte & 0x01;
    return result;
}
char crc8(char* array, int array_len)
{
    char poly = 0x07;
    char va = 0x80;
    char crc = array[0];
    int i, j;
    for(i = 1; i < array_len; i++)
    {
        char next_byte = array[i];
        for(j = 7; j >=0; j--)
        {
            char aw = crc;
            aw = aw >> 7;
            if((va & crc)  == 0)
            {
                crc = crc << 1;
                crc = crc | get_bit(next_byte, j);
            } 
            else
            {
                crc = crc << 1;
                crc = crc | get_bit(next_byte, j);
                crc = crc ^ poly;
            }
        }
    }
    return crc;
}
void append_crc(char * array, int array_len)
{
    char crc = crc8(array, array_len-1);
    array[array_len] = crc;
    array[array_len+1] = '\0';
}
int is_corrupted(char *array, int array_len)
{
    char crc = crc8(array, array_len);
    if(crc == 0)
        return 0;
    else
        return 1;
}

