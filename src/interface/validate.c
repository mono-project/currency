#include "interface.h"
#include "../currency.h"
#include "../Database/read.h" 
#include "../Database/write.h"
#include "../crypto/eddsa.h"

uint8_t validateSignature(uint8_t* transaction){
	uint8_t* pubKey = getPubKeyForUsername(&transaction[16]);
	uint8_t  out = verifySignature(&transaction[26], publicKey, transaction, 26);
	free(pubKey);
	return(out);
}

uint8_t processTransaction(uint8_t* transaction){
	if(!validateSignature(transaction)) return 0;
	if(*(uint64_t*)&transaction){
		uint64_t prevFunds = getFundsForUsername(&transaction[16]);
		if(!prevFunds) return 0;
		uint64_t amount = *(uint64_t*)&transaction[8];
		if(!changeFundsForUsername(&transaction[16],  amount)) return 0; // - from sender
		if(!changeFundsForUsername(&transaction[21], -amount)) return 0; // + for receiver
	}else addUsername(&transaction[8], &transaction[40]);
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
	uint8_t*  hash = (uint8_t*)malloc(64);
	if(!height){
		blakeln(block, 96, hash);
	} else {
		uint64_t txMemory = *(uint64_t*)&out[64];
		blakeln(block, 96+txMemory, hash);
	}
	return hash;
}

uint8_t validateInternalHash(uint8_t* block){
	uint8_t* hash = getLastBlockHash();
	uint64_t len  = 96;
	uint64_t size = 0;
	blakesl(block, 96, hash, 64, hash);
	blakesl(&block[96], txMemory, hash, 64, hash);
	for(uint8_t i=0;i<64;i++) if(block[i] != hash[i]) return 0;
	free(hash);
	return 1;
}

uint8_t validateBlock(uint8_t* block, uint32_t height){
	uint32_t timestamp = *(uint32_t*)&block[72];
	if(!validateTransactions(block)) return 0;
	if(!validateInternalHash(block)) return 0;

	return 1;
}
