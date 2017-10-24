#ifndef RL_RANDOM_GEN
#define RL_RANDOM_GEN

#include <stdlib.h>

class RandomGenerator {
 public:
  RandomGenerator();
  ~RandomGenerator();

  void SetSeed(int seed);
  int GenerateSeed();

  int GenerateRandomIntValue(int imin, int imax);
  double GenerateRandomFloatValue(double imin, double imax);

 private:
  int mSeed;
};

#endif  // RL_RANDOM_GEN
