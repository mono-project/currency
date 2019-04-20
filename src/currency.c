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

uint8_t* buildTx(uint8_t* in, uint8_t* out, uint64_t amount, uint8_t mode){
	uint8_t* tx       = malloc(121); // Raw transaction data (25) + signature (96)
	uint8_t* amount_8 = (uint8_t*)&amount;
	uint8_t* nonce_8  = (uint8_t*)&nonce;
	uint8_t* fee_8    = (uint8_t*)&fee;
	if(mode == 255){
		*tx = mode;
		for(uint8_t i=0;i<48;i++) tx[i+ 1] = in[i];
		for(uint8_t i=0;i< 8;i++) tx[i+49] = out[i];
	}
	else{
		for(uint8_t i=7;i>=0;i--) *tx += (amount_8[i] == 0);
		for(uint8_t i=0;i<  8;i++) tx[i+ 1] = in[i];		
		for(uint8_t i=0;i<  8;i++) tx[i+ 9] = out[i];
		for(uint8_t i=0;i<*tx;i++) tx[i+17] = amount_8[i];
	}
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

uint8_t* constructRawTx(uint8_t* in, uint8_t* out, uint64_t amount, uint64_t nonce, uint8_t mode){
	uint8_t* tx = buildTx(in, out, amount, mode);
	addNonce(tx, nonce);
	return tx;
}

void signRawTx(uint8_t* rawTx, uint8_t* privateKey){
	// TODO: Sign Tx
}

void blockHeader(uint8_t* minerUserName, uint8_t* transactions, uint8_t* signatures, uint8_t* prevBlockHash, uint64_t txCount, uint64_t difficulty, uint32_t fee, uint32_t timestamp, uint8_t* out){
	uint64_t txMemory = transactions[0]&0x7f;
	for(uint32_t i=1; i<txCount;i++)
		txMemory += transactions[txMemory]&0x7F;
	uint8_t  root[64]      = {0};
	uint8_t  signature[96] = {0};
	uint8_t  tx_blocks     = txMemory>>9;
	uint8_t* txMemory_8    = (uint8_t*)&txMemory;
	uint8_t* timestamp_8   = (uint8_t*)&timestamp;
	uint32_t reward        = (uint32_t)(REWARD_FUNCTION(difficulty)*BASE_REWARD) + txMemory*fee;
	uint8_t* coinbase      = buildCoinbaseTx(minerUserName, reward, timestamp);
	// TODO: Aggregate signatures using BLS lib
	blakesl(signature, 96, prevBlockHash, 64, root);
	crc512(transactions, tx_blocks<<9, root);
	if( (tx_blocks<<9) != txMemory ) blakesl(&transactions[tx_blocks<<9], txMemory&0x1ff, root, 64, root);
	for(uint8_t i=0; i<64; i++) out[i    ] = root[i];
	for(uint8_t i=0; i<96; i++) out[i+ 64] = signature[i];
	for(uint8_t i=0; i< 4; i++) out[i+160] = txCount_8[i];
	for(uint8_t i=0; i< 4; i++) out[i+164] = timestamp_8[i];
	for(uint8_t i=0; i<12; i++) out[i+168] = coinbase[i];
	// Eight Nonce Bytes
}

uint8_t* blockTemplate(uint8_t* minerUserName, uint8_t* transactions, uint8_t* signatures, uint8_t* prevBlockHash, uint32_t txCount, uint64_t difficulty, uint32_t fee, uint32_t timestamp){
	uint8_t* block = (uint8_t*)malloc(188);
	blockHeader(minerUserName, transactions, signatures, prevBlockHash, txCount, difficulty, fee, timestamp);
	uint64_t pos = 188;
	uint8_t size = 0;
	for(uint32_t i=0; i<txCount;i++){
		size = transactions[121*i]&0x7F;
		block = (uint8_t*)realloc(pos+size);
		for(uint8_t j=0;j<size;j++)
			block[pos+1] = transactions[121*i+j];
		pos += size;
	}
	// Position in block (Every tx has ~24 bytes, without signature)
	// Position in Tx array (every tx has 121 bytes)
	return block;
}

uint8_t* blockTemplateFromHeader(uint8_t* header, uint8_t* transactions){
	uint8_t* block = (uint8_t*)malloc(188);
	for(uint8_t i=0;i<188;i++) block[i] = header[i];
	uint32_t txCount = 0;
	for(uint8_t i=0;i<4;i++) txCount += 
	uint64_t pos = 188;
	uint8_t size = 0;
	for(uint32_t i=0; i<txCount;i++){
		size = transactions[121*i]&0x7F;
		block = (uint8_t*)realloc(pos+size);
		for(uint8_t j=0;j<size;j++)
			block[pos+1] = transactions[121*i+j];
		pos += size;
	}
	// Position in block (Every tx has ~24 bytes, without signature)
	// Position in Tx array (every tx has 128 bytes)
	return block;
}
