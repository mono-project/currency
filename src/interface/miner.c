#include <time.h>
#include <stdlib.h>
#include "../currency.h"
#include "../crypto/squash.h"

#ifndef QUICK_VERIFY
#include "validate.h"
#endif


uint64_t mine(uint8_t* minerUserName, uint8_t* transactions, uint8_t* signatures, uint8_t* prevBlockHash, uint32_t height, uint32_t txCount, uint64_t difficulty, uint32_t fee, uint32_t timestamp){
	srand(time(NULL));
	uint64_t  r           = (rand()<<32)|(rand());  
	uint8_t   r_8         = (uint8_t*)&r;
	uint8_t   header[188] = {0};
	uint64_t* dataset_64  = (uint64_t*)calloc(536870912,8);
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
	if(!dataset_64) exit(1);
	dataset_from_height(height, dataset_64);
	blockHeader(minerUserName, transactions, signatures, prevBlockHash, txCount, difficulty, fee, timestamp, header);
	for(uint8_t i=0;i<8;i++) header[180+i] = r_8[7-i];
	while(true){
		squash_full_api(header, 188, out_8, dataset);
#ifdef QUICK_VERIFY
		temp_0 = *out_64_0 * difficulty; temp_1 = *out_64_1 * difficulty;
		temp_2 = *out_64_2 * difficulty; temp_3 = *out_64_3 * difficulty;
		if(temp_0/difficulty==hash_64_0 || temp_1/difficulty==hash_64_2
		|| temp_2/difficulty==hash_64_2 || temp_3/difficulty==hash_64_3) return r;
#else
		if(validateHash) return r;
#endif
		r = out_64_0;
	}
}
