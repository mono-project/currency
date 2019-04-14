#ifndef BLAKE2_H
#define BLAKE2_H

#include "Blake2/neon/blake2.h"
#include "Blake2/sse/blake2.h"
#include <string.h>
#include <stdint.h>

void blakens(uint8_t* in, uint32_t in_len, uint8_t* out){
	// Blake2b, no seed, short (32 byte)
	blake2b(out, 32, in, (size_t)in_len, NULL, 0);
}
void blakenl(uint8_t* in, uint32_t in_len, uint8_t* out){
	// Blake2b, no seed, long (64 byte)
	blake2b(out, 64, in, (size_t)in_len, NULL, 0);
}
void blakess(uint8_t* in, uint32_t in_len, uint8_t* seed, uint8_t seedlen, uint8_t* out){
	// Blake2b, seed, short (32 byte)
	blake2b(out, 32, in, (size_t)in_len, seed, seedlen);
}
void blakesl(uint8_t* in, uint32_t in_len, uint8_t* seed, uint8_t seedlen, uint8_t* out){
	// Blake2b, seed, long (64 byte)
	blake2b(out, 32, in, (size_t)in_len, seed, seedlen);
}


#endif

