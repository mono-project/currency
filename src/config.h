#define BASE_REWARD 0x40000000 // Maximum block reward, multiplied with REWARD_FUNCTION(difficulty)

#ifdef GET_REWARD_FUNCTION
  #define REWARD_FUNCTION(x) pow(log2(x), 2.0)
#endif
