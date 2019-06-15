#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include "crypto/crc.h"
#include "crypto/blake2.h"

#define GET_REWARD_FUNCTION

#include "config.h"

// The following code assumes that little endian is globally used.
// All EdDSA code is in "TODO" state, since none of the libraries appear
// to work.

uint8_t* buildCoinbaseTx(uint8_t* out, uint32_t amount){
	uint8_t* tx = (uint8_t*)malloc(9);
	uint8_t* amount_8    = (uint8_t*)&amount;
	for(uint8_t i=0;i<5;i++) tx[i] = out[i];
	*(uint32_t*)&out[5] = amount;
	return tx;
}

void addNonce(uint8_t* tx, uint64_t nonce){
	uint8_t* nonce_8 = (uint8_t*)&nonce;
	for(uint8_t i=0;i<8;i++) tx[i] = nonce_8[i];
}

uint8_t* constructRawTx(uint8_t* in, uint8_t* out, uint64_t amount, uint64_t nonce){
	if(!nonce){
		uint64_t* tx_64     = (uint64_t*)malloc(53); //64bit PoW, 64bit nonce, 5 byte out, 32 byte in
		uint64_t* txNonce   = (uint64_t*)&tx[45];
		uint64_t  powHash_0 = 0;
		uint64_t  powHash_1 = 0;
		uint32_t* powH_32_0 =   (uint32_t*)&powHash_1;
		uint32_t* powH_32_1 = &((uint32_t*)&powHash_1)[1];
		for(uint8_t i=0;i<32;i++) tx[i+ 8] = in[i];
		for(uint8_t i=0;i< 5;i++) tx[i+40] = out[i];
		powHash_0 = *tx_64 + tx_64[1] + tx_64[2] + tx_64[3] + tx_64[4] + tx_64[5] + tx_64[6];
		*txNonce++;
		while(1){
			powHash_1 = powHash_0 ^ *txNonce;
			crc32i(powH_32_0);
			crc32i(powH_32_1);
			*txNonce = powHash_1;
			if(powHash_1 < 0x1000000000) break;
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
	addNonce(tx, nonce);
	return tx;
}

void signRawTx(uint8_t* rawTx, uint8_t* privateKey){
	// TODO: Sign Tx
}

void blockHeader(uint8_t* minerUserName, uint8_t* transactions, uint8_t* prevBlockHash, uint32_t txCount, uint64_t difficulty, uint32_t fee, uint32_t timestamp, uint8_t* out){
	uint8_t  root[64]      = {0};
	uint32_t reward        = (uint32_t)(REWARD_FUNCTION(difficulty)*BASE_REWARD) + txCount*fee;
	uint8_t* coinbase      = buildCoinbaseTx(minerUserName, reward);
	blakesl(transactions, txCount*90, prevBlockHash, 64, root);
	for(uint8_t i=0; i<64; i++) out[i] = root[i];
	*(uint32_t*)&out[64] = txCount;
	*(uint32_t*)&out[68] = timestamp;
	for(uint8_t i=0; i<12; i++) out[i+72] = coinbase[i];
	// Eight Nonce Bytes
}

uint8_t* blockTemplate(uint8_t* minerUserName, uint8_t* transactions, uint8_t* signatures, uint8_t* prevBlockHash, uint32_t txCount, uint64_t difficulty, uint32_t fee, uint32_t timestamp){
	uint8_t* block = (uint8_t*)malloc(88+txCount*90);
	blockHeader(minerUserName, transactions, signatures, prevBlockHash, txCount, difficulty, fee, timestamp);
	for(uint32_t i=0;i<txCount;i++) for(uint32_t j=0;j<90;j++) block[88+90*i+j] = transactions[90*i+j];
	return block;
}

uint8_t* blockTemplateFromHeader(uint8_t* header, uint8_t* transactions){
	uint8_t* block = (uint8_t*)malloc(88+txCount*90);
	for(uint8_t i=0;i<88;i++) block[i] = header[i];
	uint32_t txCount = *(uint32_t*)&block[64];
	for(uint32_t i=0;i<txCount;i++) for(uint32_t j=0;j<90;j++) block[88+90*i+j] = transactions[90*i+j];
	return block;
}
