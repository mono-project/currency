#include <stdint.h>
#include <math.h>
#include "../config.h"

// Zawy LWMA difficulty algorithm
// The mono variant enables a change of
// the number of difficulties used over time
// to allow a higher security.

uint64_t diff(uint64_t* difficulties, uint32_t* timestamps, uint8_t inDiffCount){
	uint8_t x = log2(log2(states[inDiffCount-1].bits));
	uint8_t N = pow(pow(x,x),0.5);
	if(diffCount<N) return 1;

	uint32_t k = (N+1)/2*BLOCKTIME;
	uint64_t sumTarget = 0;
	uint64_t t = 0;
	for(uint8_t i=0; i<N;i++){
		t += (states[inDiffCount-(N-i)].timestamp - states[inDiffCount-(N-i+1)].timestamp)*(i+1)/N;
		sumTarget += bits_to_target(states[inDiffCount-(N-i)].bits)/N;
	}
	if (t < k / 3) t = k / 3;
	return (t * sumTarget / k);
}

uint8_t checkDiff(uint64_t difficulty, uint8_t* hash){
	uint64_t* hash_64 = (uint64_t*)hash;
	uint64_t  temp    = 0;
	for(uint8_t i=0;i<4;i++){
		temp = hash_64[i] * difficulty;
		if(temp/difficulty != hash_64[i])
			return 0;
	}
	return 1;
}

