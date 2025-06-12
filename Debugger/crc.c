#include <stdio.h>
#include "crc.h"


static int is_CRC32_inited = 0;
static unsigned int crc32_table[256];


/*****************************************************************************
  * @brief  crc16校验
  * @param 
  * @retval 
******************************************************************************/
unsigned short getCRC16(unsigned char *ptr,unsigned char len) {
    unsigned char i;
    unsigned short crc=0xFFFF;

    if(len==0)
    {
        len=1;
    }
    while(len--)
    {
        crc ^= *ptr;
        for(i=0; i<8; i++)
        {
            if(crc&1)
            {
                crc>>=1;
                crc^=0xA001;
            }
            else
            {
                crc>>=1;
            }
        }
        ptr++;
    }
    return crc;
}

/*
* crc32校验
*/
static unsigned int Reflect(unsigned int ref, char ch) {
    // Used only by Init_CRC32_Table()
    //unsigned long value(0);
    unsigned int value = 0;
    // Swap bit 0 for bit 7 , bit 1 for bit 6, etc.
    for(int i = 1; i < (ch + 1); i++) {
        if(ref & 1)
            value |= 1 << (ch - i);
        ref >>= 1;
    }
    return value;
}

//初始化crc32表
static void initCRC32Table() {
    // This is the official polynomial used by CRC-32 in PKZip, WinZip and Ethernet. 
    //unsigned long ulPolynomial = 0x04c11db7;
    unsigned int ulPolynomial = 0x04c11db7;

    // 256 values representing ASCII character codes.
    for(int i = 0; i <= 0xFF; i++)
    {
        crc32_table[i]=Reflect(i, 8) << 24;
        //if (i == 1)printf("old1--i=%d,crc32_table[%d] = %lu\r\n",i,i,crc32_table[i]);
        
        for (int j = 0; j < 8; j++)
        {
            //if(i == 1)
            //{
            //    unsigned int tmp1 = (crc32_table[i] << 1);
            //    unsigned int tmp2 = (crc32_table[i] & (1 << 31) ? ulPolynomial : 0);
            //    unsigned int tmp3 = tmp1 ^ tmp2;
            //    tmp3 += 1;
            //    tmp3 -= 1;
            //
            //}
            
            crc32_table[i] = (crc32_table[i] << 1) ^ (crc32_table[i] & (1 << 31) ? ulPolynomial : 0);
            //if (i == 1)printf("old3--i=%d,crc32_table[%d] = %lu\r\n",i,i,crc32_table[i]);
        }
        //if (i == 1)printf("old2--i=%d,crc32_table[%d] = %lu\r\n",i,i,crc32_table[i]);
        crc32_table[i] = Reflect(crc32_table[i], 32);
    }
}

unsigned int getCRC32(unsigned char* buffer, unsigned int dwSize) {
    // Be sure to use unsigned variables,
    // because negative values introduce high bits
    // where zero bits are required.
    //unsigned long  crc(0xffffffff);
    unsigned int crc = 0xffffffff;
    int len;

    if (0 == is_CRC32_inited) {
        initCRC32Table();
        is_CRC32_inited = 1;
    }
    
    len = dwSize;    
    // Perform the algorithm on each character
    // in the string, using the lookup table values.
    while(len--)
        crc = (crc >> 8) ^ crc32_table[(crc & 0xFF) ^ *buffer++];
    // Exclusive OR the result with the beginning value.
    return crc^0xffffffff;
}
