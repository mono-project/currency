#include <stdlib.h>
#include "../currency.h"
#include "../crypto/blake2.h"

#ifndef QUICK_VERIFY
#include "validate.h"
#endif


uint64_t mine(uint8_t* minerUserName, uint64_t minerFunds, uint8_t* transactions, uint8_t* signatures, uint8_t* prevBlockHash, uint32_t height, uint32_t txCount, uint64_t difficulty, uint32_t fee, uint32_t timestamp){
	uint8_t   header[188] = {0};
	uint64_t* nonce_64    = (uint64_t*)&header[180];
	uint64_t  out_64_0[4] = {0};
#ifdef QUICK_VERIFY
	uint64_t* out_64_1    = &out_64_0[1];
	uint64_t* out_64_2    = &out_64_0[2];
	uint64_t* out_64_3    = &out_64_0[3];
	uint64_t  temp_0      = 0;
	uint64_t  temp_1      = 0;
	uint64_t  temp_2      = 0;
	uint64_t  temp_3      = 0;
#endif
	uint8_t*  out_8       = (uint64_t*)out_64;
	blockHeader(minerUserName, transactions, signatures, prevBlockHash, txCount, difficulty, fee, timestamp, header);
	for(uint8_t i=0;i<8;i++) header[180+i] = r_8[7-i];
	for(uint64_t i;i<minerFunds;r++){
		*nonce_64 = i;
		blakenl(header, 188, out_8);
#ifdef QUICK_VERIFY
		temp_0 = *out_64_0 * difficulty; temp_1 = *out_64_1 * difficulty;
		temp_2 = *out_64_2 * difficulty; temp_3 = *out_64_3 * difficulty;
		if(temp_0/difficulty==hash_64_0 || temp_1/difficulty==hash_64_2
		|| temp_2/difficulty==hash_64_2 || temp_3/difficulty==hash_64_3) return i;
#else
		if(validateHash) return i;
#endif
	}
}
