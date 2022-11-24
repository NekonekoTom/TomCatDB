#ifndef NEKO_RANDOM_H_
#define NEKO_RANDOM_H_

#include <random>

namespace neko
{

// Generate random integer numbers subjects to U(0, n)
inline const int RandomUniform(int n) {
  return rand() % n;
}

// Return true if a randomly generated integer is 0 
// in a random number space of total n numbers
inline bool SelectedInProb1DivdN(int n) {
  return RandomUniform(n) == 0;
}

} // namespace neko


#endif