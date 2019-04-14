#ifndef CONF
#define CONF

#define CURRENCY_NAME "mono"   // Lowercase, written in quotation marks
#define BASE_REWARD 0x40000000 // Maximum block reward, multiplied with REWARD_FUNCTION(difficulty)

#ifdef GET_REWARD_FUNCTION
  #define REWARD_FUNCTION(x) pow(log2(x), 2.0)
#endif

#define QUICK_VERIFY    // Comment this line if quick verification should not be used in the miner
			// Pro: Hash verification is faster. Much faster.
			// Con: Hash verification requires much more L1/L2 cache.

#endif
