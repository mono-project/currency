#include "../currency.h"
#include "../crypto/squash.h" 
//#include "../crypto/bls.h" TODO: BLS Lib

void validateBlockSignature(uint8_t* transactions, uint8_t* signature){
	//TODO
}

uint8_t* hashBlock(uint8_t* block, uint32_t height){
	uint64_t* hash_64 = (uint64_t*)malloc(64);
	uint8_t*  hash    = (uint8_t*)hash_64;
	if(!height){
		blakeln(block, 180, hash);
	} else {
		uint32_t txCount = 0;
		for(uint8_t i=0; i<32;i+=8) txCount += block[i+160]<<i;
		squash_light_api(block, 180+txCount*24, hash, height);
	}
	return hash;
}

uint8_t validateHash(uint8_t* hash, uint64_t difficulty){
	uint64_t* hash_64 = (uint64_t*)hash;
	uint64_t  temp    = 0;
	
	for(uint8_t i=0;i<4;i++){
		temp = hash_64[0] * difficulty;
		if(temp/difficulty != hash_64[0]) return(0);
	} 
	return(1);

}

uint8_t validateBlockHash(uint8_t* block, uint32_t height, uint64_t difficulty){
	uint8_t*  hash    = hashBlock(block, height);
	return(validateHash(hash, difficulty));
} 

