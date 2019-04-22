#include "validate.h" 
#include "../Database/write.h"

uint8_t addBlockToDBVerify(uint8_t* block, uint32_t height){
	if(!validateBlock(block, height)) return 0;	
	addHeader(block);
	return 1;
}
