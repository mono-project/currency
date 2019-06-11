// Copyright (c) 2019, The CCLib Developers
//
// Please see the included LICENSE file for more information.

void aesSingleRound(uint8_t* state, uint8_t* key);
void ccc(uint8_t* in, uint32_t len, uint8_t* key, uint8_t* iv, uint8_t* out);
