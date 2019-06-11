#include <stdint.h>
#include "random.h"
#include "eddsa/eddsa.h"

void genKeyPair(uint8_t* pub, uint8_t* priv){
	urand32(priv);
	ed25519_genpub(pub, priv);
}

void getPubFromPriv(uint8_t* pub, uint8_t* priv){
	ed25519_genpub(pub, priv);
}
