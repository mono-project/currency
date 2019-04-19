#include "../currency.h"
#include "../Crypto/squash.h" 
#include "../Database/difficulty.h" 
#include "../Database/read.h" 
#include "../Database/write.h"
//#include "../crypto/bls.h" TODO: BLS Lib

uint8_t validateBlockSignature(uint8_t* block){
	uint8_t* signature = &block[64];
	uint8_t* transactions = &block[188];
	//TODO if(missmatch) return 0
}

uint8_t processTransaction(uint8_t* transaction){
	if(*transaction != 255){
		uint64_t prevFunds = getFundsForUsername(&transaction[1]);
		if(!prevFunds) return 0 + *transaction;
		uint64_t amount = 0;
		for(uint8_t i=0;i<8;i++) amount += transaction[17+i]<<(8*i);
		if(!changeFundsForUsername(&transaction[1], amount, 0)) return 0; // - from sender
		if(!changeFundsForUsername(&transaction[9], amount, 1)) return 0; // + for receiver
		return 128 + *transaction;
	} else {
		addUsername(&transaction[49], &transaction[1]);
		return 185; // 128 + 57
	}
	
}

uint8_t validateTransactions(uint8_t* block, uint32_t txCount){
	if(!validateBlockSignature(block)) return 0;
	uint64_t pos = 188;
	for(uint32_t i=0;i<txCount;i++){
		uint8_t status = processTransaction(&block[pos]);
		if(!(status>>7)) return 0;
		pos += status&0x7F;
	}
	return 1;
	
}

uint8_t* hashBlock(uint8_t* block, uint32_t height){
	uint64_t* hash_64 = (uint64_t*)malloc(64);
	uint8_t*  hash    = (uint8_t*)hash_64;
	if(!height){
		blakeln(block, 188, hash);
	} else {
		uint32_t txCount = 0;
		for(uint8_t i=0; i<32;i+=4) txCount += block[i+160]<<(8*i);
		squash_light_api(block, 188+txCount*24, hash, height);
	}
	return hash;
}

uint8_t validateHash(uint8_t* hash, uint64_t difficulty){
	return(checkDiff(difficulty, hash));
}

uint8_t validateBlockHash(uint8_t* block, uint32_t height, uint64_t difficulty){
	uint8_t*  hash = hashBlock(block, height);
	return(validateHash(hash, difficulty));
} 

uint8_t validateBlock(uint8_t* block, uint32_t height){
	uint32_t timestamp = 0;
	for(uint8_t i=0; i<4; i++) timestamp += block[i+164]<<(8*i);
	uint64_t difficulty = getDifficultyForTimestamp(timestamp, height);
	if(!validateBlockHash(block, height, difficulty)) return 0;
	if(!validateTransactions(block)) return 0;
	return 1;
}
