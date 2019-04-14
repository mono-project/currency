#ifdef VALIDATE_H
#define VALIDATE_H

void validateBlockSignature(uint8_t* transactions, uint8_t* signature);
uint8_t* hashBlock(uint8_t* block, uint32_t height);
uint8_t validateHash(uint8_t* hash, uint64_t difficulty);
uint8_t validateBlockHash(uint8_t* block, uint32_t height, uint64_t difficulty);

#endif
