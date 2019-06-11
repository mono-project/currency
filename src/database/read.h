#ifndef READ_H
#define READ_H

#include <stdint.h>

char* getFolder();
uint64_t calcDifficultyForHeight(uint32_t height);
uint64_t getDifficultyForTimestamp(uint32_t prevTimestamp, uint32_t height);
uint8_t* getTx(uint64_t txNumber);
uint8_t* getBlockTx(uint32_t height);
uint32_t* getPeerList();
uint32_t* getUsernames();
uint64_t getFundsForUsername(uint8_t* username);
uint32_t* getPubKeys();

#endif
