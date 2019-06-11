#ifndef WRITE_H
#define WRITE_H

#include <stdint.h>

uint8_t setFundsForUsername(uint8_t* username, uint64_t funds);
uint8_t changeFundsForUsername(uint8_t* username, uint64_t change, uint8_t mode);
uint8_t addUsername(uint8_t* username, uint8_t* pubKey);

#endif
