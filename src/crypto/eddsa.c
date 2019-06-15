#include <stdint.h>
#include <stdbool.h>
#include "random.h"
#include "eddsa/eddsa.h"
#include "blake2.h"

void genKeyPair(uint8_t* pub, uint8_t* priv){
	urand32(priv);
	ed25519_genpub(pub, priv);
}

void getPubFromPriv(uint8_t* pub, uint8_t* priv){
	ed25519_genpub(pub, priv);
}

void sign(uint8_t* out, uint8_t* pub, uint8_t* priv, uint8_t* data, uint32_t len){
	uint8_t hash[32] = {0};
	blakens(data, len, hash);
	ed25519_sign(out, priv, pub, hash, 32);
}

uint8_t verifySignature(uint8_t* signature, uint8_t* pub, uint8_t* data, uint32_t len){
	uint8_t hash[32] = {0};
	blakens(data, len, hash);
	return((uint8_t)ed25519_verify(signature, pub, hash, 32);
}
