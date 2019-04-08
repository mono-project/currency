#ifndef CURRENCY_H
#define CURRENCY_H

#include <stdint.h>

uint8_t* buildTx(uint8_t* in, uint8_t* out, uint64_t amount);
uint8_t* buildCoinbaseTx(uint8_t* out, uint32_t amount);
void addNonce(uint8_t* tx, uint64_t nonce);
uint8_t* constructRawTx(uint8_t* in, uint8_t* out, uint64_t amount, uint64_t nonce);
void signRawTx(uint8_t* rawTx, uint8_t* privateKey);
uint8_t* blockTemplate(uint8_t* minerUserName, uint8_t* transactions, uint8_t* signatures, uint8_t* prevBlockHash, uint32_t txCount, uint32_t difficulty, uint32_t fee, uint32_t timestamp);

#endif
