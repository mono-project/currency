#ifndef EDDSA_H
#define EDDSA_H

void genKeyPair(uint8_t* pub, uint8_t* priv);
void getPubFromPriv(uint8_t* pub, uint8_t* priv);
void sign(uint8_t* out, uint8_t* pub, uint8_t* priv, uint8_t* data, uint32_t len);
uint8_t verifySignature(uint8_t* signature, uint8_t* pub, uint8_t* data, uint32_t len);

#endif
