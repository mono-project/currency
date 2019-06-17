#include "interface.h"
#include "../currency.h"
#include "../Crypto/squash.h" 
#include "../Database/read.h" 
#include "../Database/write.h"
#include "../crypto/eddsa.h"

uint8_t validateSignature(uint8_t* block){
	uint8_t* signature = &block[64];
	uint8_t* transactions = &block[188];
	//TODO if(missmatch) return 0
}

uint8_t processTransaction(uint8_t* transaction){
	if(*(uint64_t*)&transaction){
		uint64_t prevFunds = getFundsForUsername(&transaction[16]);
		if(!prevFunds) return 0;
		uint64_t amount = *(uint64_t*)&transaction[8];
		if(!changeFundsForUsername(&transaction[16], amount, 0)) return 0; // - from sender
		if(!changeFundsForUsername(&transaction[21], amount, 1)) return 0; // + for receiver
	} else {
		addUsername(&transaction[8], &transaction[40]);
	}
	return 1;
}

uint8_t validateTransactions(uint8_t* block, uint64_t txMemory){
	uint64_t pos = 96;
	while(pos < txMemory){
		if(!processTransaction(&block[pos])) return 0;
		pos += (*(uint64_t*)&block[pos])?90:57;
	}
	uint64_t totalFundchange = getTotalFundchange();
	if(totalFundchange!=0){
		setChangeToNull();
		return 0;
	}
	if(!addChangeToFunds()){
		setChangeToNull();
		return 0;
	}
	return 1;
	
}

uint8_t* hashBlock(uint8_t* block, uint32_t height){
	uint8_t*  hash    = (uint8_t*)malloc(64);
	if(!height){
		blakeln(block, 96, hash);
	} else {
		uint64_t txMemory = *(uint64_t*)&out[64];
		squash_light_api(block, 96+txMemory, hash, height);
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

uint8_t validateInternalHash(uint8_t* block){
	uint8_t* hash      = getLastBlockHash();
	uint32_t txCount   = 0;
	uint64_t len = 188;
	uint64_t size = 0;
	for(uint8_t i=0; i<4; i++) txCount += out[i+160]<<(8*i);;
	for(uint32_t i=0; i<txCount;i++){
		size = block[pos]&0x7F;
		len += size;
	}
	len -= 188; size = len;
	len >>= 9; len <<= 9;
	blakesl(&block[64], 96, hash, 64, hash);
	//TODO: Enable adaptive Tx size
	crc512(&block[188], len, hash);
	if(len != size) blakesl(&block[188+len], size-len, hash, 64, hash);
	for(uint8_t i=0;i<64;i++) if(block[i] != hash[i]) return 0;
	free(hash);
	return 1;
}

uint8_t validateBlock(uint8_t* block, uint32_t height){
	uint32_t timestamp = 0;
	for(uint8_t i=0; i<4; i++) timestamp += block[i+164]<<(8*i);
	uint64_t difficulty = getDifficultyForTimestamp(timestamp, height);
	if(!validateBlockHash(block, height, difficulty)) return 0;
	if(!validateTransactions(block)) return 0;
	if(!validateInternalHash(block)){
		// TODO: Try adding blocks corresponding to
		// "the other" chain. If not working, return 0.
		// addBlockToDBVerify(prevBlockOnDifferentChain, height-1);
		return 0;
	}
	return 1;
}
