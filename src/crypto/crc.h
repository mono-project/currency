#ifndef CRC_H
#define CRC_H

#include <stdint.h>
void crc32p(uint32_t* in, uint32_t* out);
void crc32i(uint32_t* in);
void crc256(uint8_t* in, uint32_t len, uint8_t* out);
void crc512(uint8_t* in, uint32_t len, uint8_t* out);

#endif
