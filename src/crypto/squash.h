#ifndef SQUASH_H
#define SQUASH_H

#include <stdint.h>

void dataset_from_height(uint32_t height, uint64_t* dataset);
void squash_light_api(const void* data, uint32_t length, uint8_t* hash_out, uint32_t height);
void squash_full_api(const void* data, uint32_t length, uint8_t* hash_out, uint64_t* dataset);

#endif
