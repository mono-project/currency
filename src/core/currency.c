#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include "../crypto/blake2.h"

uint8_t* buildTx(uint8_t* in, uint8_t* out, uint64_t amount, uint32_t nonce, uint16_t fee){
	uint8_t* tx       = malloc(76);
	uint8_t* amount_8 = (uint8_t*)&amount;
	uint8_t* nonce_8  = (uint8_t*)&nonce;
	uint8_t* fee_8    = (uint8_t*)&fee;
	for(uint8_t i=0;i<32;i++) tx[i   ] = in[i];
	for(uint8_t i=0;i<32;i++) tx[i+32] = out[i];
	for(uint8_t i=0;i< 8;i++) tx[i+64] = amount_8[i];
	for(uint8_t i=0;i< 4;i++) tx[i+72] = nonce_8[i];
	for(uint8_t i=0;i< 2;i++) tx[i+76] = fee_8[i];
	return tx;
}

uint8_t* buildCoinbaseTx(uint8_t* out, uint32_t amount, uint32_t timestamp){
	uint8_t* tx          = malloc(44);
	uint8_t* amount_8    = (uint8_t*)&amount;
	uint8_t* timestamp_8 = (uint8_t*)&timestamp;
	for(uint8_t i=0;i<32;i++) tx[i]    = out[i];
	for(uint8_t i=0;i< 8;i++) tx[i+32] = amount_8[i];
	for(uint8_t i=0;i< 4;i++) tx[i+40] = timestamp_8[i];
	return tx;
}

uint8_t* blockTemplate(uint8_t* minerPublicKey, uint8_t* transactions, uint8_t* signatures, uint8_t* prevBlockHash, uint32_t txCount, uint32_t difficulty, uint32_t timestamp){
	uint8_t* block         = malloc(153+78*txCount);
	uint8_t  root[64]      = {0};
	uint8_t  signature[33] = {0};
	uint8_t  tx_blocks     = txCount/(1<<25);
	uint8_t* txCount_8     = (uint8_t*)&txCount;
	uint8_t* timestamp_8   = (uint8_t*)&timestamp;
	uint32_t reward        = (uint32_t)(pow(log2(difficulty), 2.0)*0x40000000);
	uint8_t* coinbase;
	for(uint32_t i=0; i<txCount;i++){
		reward += transactions[74 + i*76];
		reward += transactions[75 + i*76] << 8;
	}
	coinbase = buildCoinbaseTx(minerPublicKey, reward, timestamp);
	// TODO: Aggregate signatures using BLS lib
	blakesl(signature, 33, prevBlockHash, 64, root);
	for(uint8_t i=0; i<tx_blocks; i++) blakesl(&transactions[i<<25], 1<<25, root, 64, root);
	if( (tx_blocks<<25) != txCount ) blakesl(&transactions[tx_blocks<<25], txCount&0x1ffffff, root, 64, root);
	for(uint8_t i=0; i<64; i++) block[i    ] = root[i];
	for(uint8_t i=0; i<33; i++) block[i+ 64] = signature[i];
	for(uint8_t i=0; i< 4; i++) block[i+ 97] = txCount_8[i];
	for(uint8_t i=0; i< 4; i++) block[i+101] = timestamp_8[i];
	// Four Nonce Bytes
	for(uint8_t i=0; i<txCount*78; i++) block[i+105] = transactions[i];
	return block;
}
