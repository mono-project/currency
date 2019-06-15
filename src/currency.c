#include <stdint.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include "crypto/crc.h"
#include "crypto/blake2.h"

#define GET_REWARD_FUNCTION

#include "config.h"

uint8_t* buildCoinbaseTx(uint8_t* out, uint32_t amount){
	uint8_t* tx = (uint8_t*)malloc(9);
	uint8_t* amount_8    = (uint8_t*)&amount;
	for(uint8_t i=0;i<5;i++) tx[i] = out[i];
	*(uint32_t*)&out[5] = amount;
	return tx;
}

uint8_t* constructRawTx(uint8_t* in, uint8_t* out, uint64_t amount, uint64_t nonce){
	if(!nonce){
		uint64_t* tx_64     = (uint64_t*)malloc(57); //64bit nonce, 64bit PoW, 32bit timestamp, 5 byte out, 32 byte in
		uint64_t* txNonce   = (uint64_t*)&tx[49];
		uint64_t  powHash_0 = 0;
		uint64_t  powHash_1 = 0;
		uint32_t* powH_32_0 =   (uint32_t*)&powHash_1;
		uint32_t* powH_32_1 = &((uint32_t*)&powHash_1)[1];
		for(uint8_t i=0;i<32;i++) tx[i+ 8] = in[i];
		for(uint8_t i=0;i< 5;i++) tx[i+40] = out[i];
		*(uint32_t*)&tx[45] = 3 + (uint32_t)time(NULL); // A tx may not be included one minute after generation, 3 second offset added for PoW
		powHash_0 = *tx_64 + tx_64[1] + tx_64[2] + tx_64[3] + tx_64[4] + tx_64[5] + tx_64[6] + tx_64[7];
		*txNonce++;
		while(1){
			powHash_1 = powHash_0 ^ *txNonce;
			crc32i(powH_32_0);
			crc32i(powH_32_1);
			*txNonce = powHash_1;
			if(powHash_1 < 0x1000000000) break; // Approximately 2.5s on Xeon E2-1225v2
		}
	}
	else{
		uint64_t* tx_64    = (uint64_t*)malloc(90); // Raw transaction data (26) + signature (64)
		uint8_t*  tx       = (uint8_t*)tx_64; 
		uint8_t*  amount_8 = (uint8_t*)&amount;
		*tx_64 = nonce;
		tx_64[1] = amount;
		for(uint8_t i=0;i<5;i++) tx[i+16] = in[i];		
		for(uint8_t i=0;i<5;i++) tx[i+21] = out[i];
	}
	return tx;
}

void signRawTx(uint8_t* rawTx, uint8_t* publicKey, uint8_t* privateKey){
	sign(&rawTx[26], publicKey, privateKey, rawTx, 26);
}

void blockHeader(uint8_t* minerUserName, uint8_t* transactions, uint8_t* prevStateHash, uint64_t txMemory, uint64_t difficulty, uint32_t fee, uint32_t timestamp, uint8_t* out){
	uint8_t  root[64]      = {0};
	uint32_t reward        = (uint32_t)(REWARD_FUNCTION(difficulty)*BASE_REWARD) + txMemory*fee;
	uint8_t* coinbase      = buildCoinbaseTx(minerUserName, reward);
	blakesl(transactions, txMemory, prevStateHash, 64, root);
	for(uint8_t i=0; i<64; i++) out[i] = root[i];
	*(uint32_t*)&out[64] = txCount;
	*(uint32_t*)&out[72] = timestamp;
	for(uint8_t i=0; i<12; i++) out[i+76] = coinbase[i];
	// Eight Nonce Bytes
}

uint8_t* blockTemplate(uint8_t* minerUserName, uint8_t* transactions, uint8_t* signatures, uint8_t* prevStateHash, uint64_t txMemory, uint64_t difficulty, uint32_t fee, uint32_t timestamp){
	uint8_t* block = (uint8_t*)malloc(96+txMemory);
	blockHeader(minerUserName, transactions, signatures, prevStateHash, txMemory, difficulty, fee, timestamp);
	for(uint64_t i=0;i<txMemory;i++) block[i] = transactions[i];
	return block;
}

uint8_t* blockTemplateFromHeader(uint8_t* header, uint8_t* transactions){
	uint8_t* block = (uint8_t*)malloc(96+txCount*90);
	for(uint8_t i=0;i<96;i++) block[i] = header[i];
	uint64_t txCount = *(uint64_t*)&block[64];
	for(uint64_t i=0;i<txMemory;i++) block[i] = transactions[i];
	return block;
}
