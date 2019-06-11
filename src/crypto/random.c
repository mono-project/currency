#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

uint8_t* urand(int length) {
    uint8_t* bytes = (uint8_t*) calloc(1, length);
#if defined(_WIN32) || defined(_WIN64)
    HCRYPTPROV hcp;
    CryptAcquireContext(&hcp,NULL,NULL,PROV_RSA_FULL,CRYPT_VERIFYCONTEXT or CRYPT_SILENT);
    CryptGenRandom(hcp, length, bytes);
    CryptReleaseContext(hcp, 0);
#else
    FILE* urand = fopen("/dev/urandom", "r");
    fread(bytes, 1, length, urand);
    fclose(urand);
#endif
    return bytes;
}

void urand32(uint8_t* bytes){
#if defined(_WIN32) || defined(_WIN64)
    HCRYPTPROV hcp;
    CryptAcquireContext(&hcp,NULL,NULL,PROV_RSA_FULL,CRYPT_VERIFYCONTEXT or CRYPT_SILENT);
    CryptGenRandom(hcp, 32, bytes);
    CryptReleaseContext(hcp, 0);
#else
    FILE* urand = fopen("/dev/urandom", "r");
    fread(bytes, 1, 32, urand);
    fclose(urand);
#endif
    return bytes;
}

int main(){
	uint8_t* seed = urand(64);
	for(uint8_t i=0; i<64; i++) printf("%02x",seed[i]);
	printf("\n");
}
