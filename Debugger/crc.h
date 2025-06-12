#ifndef _CRC_H
#define _CRC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>


//crc16校验
unsigned short getCRC16(unsigned char *ptr,unsigned char len);
//crc32校验
unsigned int getCRC32(unsigned char* buffer, unsigned int dwSize);


#ifdef __cplusplus
}
#endif

#endif /* _CRC_H */
