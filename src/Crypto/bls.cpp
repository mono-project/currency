#define MCLBN_FP_UNIT_SIZE 6

#include "BLS/bls.hpp"
#include <string>
#include <cstring>


class curve{
	private:
		bls::SecretKey sk;
		bls::PublicKey pk;
	public:
		void init(uint8_t* seed){
			size_t a = 64;
			sk.setLittleEndian(seed, a);
			//sk.getPublicKey(pk); // TODO: Get PublicKey (Herumi/Bls assertions get triggered)
		}
		void sign(uint8_t* message, uint32_t size, uint8_t* out){
			bls::Signature s;
			sk.signHash(s, (void*)message, (size_t)size);
			for(uint8_t i=0;i<255;i++){
				printf("%x",((uint8_t*)&s)[i]);
				fflush(stdout);
			}
		}
};

int main(){
	uint64_t seed[4] = {0xaf47b3b803db7d5f, 0xdef900ad32ecdf82,
			    0x9e9be56eb0b6ef4f, 0x73cf978a261a516d};
	uint8_t* seed_8  = reinterpret_cast<uint8_t*>(seed);
	curve c;
	c.init(seed_8);
	c.sign(seed_8, 64, seed_8);
}
