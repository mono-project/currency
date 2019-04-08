#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include "crypto/crc.h"
#include "crypto/blake2.h"

#define GET_REWARD_FUNCTION

#include "config.h"

// The following code assumes that little endian is globally used.
// All BLS code is in "TODO" state, since none of the libraries appear
// to work.

uint8_t* buildTx(uint8_t* in, uint8_t* out, uint64_t amount){
	uint8_t* tx       = malloc(128); // Raw transaction data + signature
	uint8_t* amount_8 = (uint8_t*)&amount;
	uint8_t* nonce_8  = (uint8_t*)&nonce;
	uint8_t* fee_8    = (uint8_t*)&fee;
	for(uint8_t i=0;i<8;i++) tx[i   ] = in[i];
	for(uint8_t i=0;i<8;i++) tx[i+ 8] = out[i];
	for(uint8_t i=0;i<8;i++) tx[i+16] = amount_8[i];
	return tx;
}

uint8_t* buildCoinbaseTx(uint8_t* out, uint32_t amount){
	uint8_t* tx          = malloc(44);
	uint8_t* amount_8    = (uint8_t*)&amount;
	for(uint8_t i=0;i<8;i++) tx[i]   = out[i];
	for(uint8_t i=0;i<4;i++) tx[i+8] = amount_8[i];
	return tx;
}

void addNonce(uint8_t* tx, uint64_t nonce){
	uint8_t* nonce_8 = (uint8_t*)&nonce;
	for(uint8_t i=0;i<8;i++) tx[i+24] = nonce_8[i];
}

uint8_t* constructRawTx(uint8_t* in, uint8_t* out, uint64_t amount, uint64_t nonce){
	uint8_t* tx = buildTx(in, out, amount);
	addNonce(tx, nonce);
	return tx;
}

void signRawTx(uint8_t* rawTx, uint8_t* privateKey){
	// TODO: Sign Tx
}

void blockHeader(uint8_t* minerUserName, uint8_t* transactions, uint8_t* signatures, uint8_t* prevBlockHash, uint32_t txCount, uint64_t difficulty, uint32_t fee, uint32_t timestamp, uint8_t* out){
	uint8_t  root[64]      = {0};
	uint8_t  signature[96] = {0};
	uint8_t  tx_blocks     = txCount>>6;
	uint8_t* txCount_8     = (uint8_t*)&txCount;
	uint8_t* timestamp_8   = (uint8_t*)&timestamp;
	uint32_t reward        = (uint32_t)(REWARD_FUNCTION(difficulty)*BASE_REWARD) + txCount*fee;
	uint8_t* coinbase      = buildCoinbaseTx(minerUserName, reward, timestamp);
	// TODO: Aggregate signatures using BLS lib
	blakesl(signature, 96, prevBlockHash, 64, root);
	crc512(transactions, 24*(tx_blocks<<6), root);
	if( (tx_blocks<<9) != txCount ) blakesl(&transactions[24*(tx_blocks<<6)], 24*(txCount&0x3F), root, 64, root);
	for(uint8_t i=0; i<64; i++) out[i    ] = root[i];
	for(uint8_t i=0; i<96; i++) out[i+ 64] = signature[i];
	for(uint8_t i=0; i< 4; i++) out[i+160] = txCount_8[i];
	for(uint8_t i=0; i< 4; i++) out[i+164] = timestamp_8[i];
	for(uint8_t i=0; i<12; i++) out[i+168] = coinbase[i];
	// Eight Nonce Bytes
}

uint8_t* blockTemplate(uint8_t* minerUserName, uint8_t* transactions, uint8_t* signatures, uint8_t* prevBlockHash, uint32_t txCount, uint64_t difficulty, uint32_t fee, uint32_t timestamp){
	uint8_t* block         = malloc(188+24*txCount);
	blockHeader(minerUserName, transactions, signatures, prevBlockHash, txCount, difficulty, fee, timestamp);
	for(uint8_t i=0; i<txCount; i++)
		for(uint8_t j=0;j<24;j++)
			block[24*i+j+188] = transactions[128*j+i];
			// Position in block (Every tx has 24 bytes, without signature)
							// Position in Tx array (every tx has 128 bytes)
	return block;
}

uint8_t* blockTemplateFromHeader(uint8_t* header, uint8_t* transactions){
	uint8_t* block         = malloc(188+24*txCount);
	for(uint8_t i=0;i<188;i++) block[i] = header[i];
	// Four Nonce Bytes
	for(uint8_t i=0; i<txCount; i++)
		for(uint8_t j=0;j<24;j++)
			block[24*i+j+188] = transactions[128*j+i];
			// Position in block (Every tx has 24 bytes, without signature)
							// Position in Tx array (every tx has 128 bytes)
	return block;
}
