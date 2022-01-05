#ifndef RAND_H
#define RAND_H

#include <limits>
#include <string>

class RandomGenerator {
public:
  RandomGenerator();
  RandomGenerator(uint64_t seed);
  virtual ~RandomGenerator();
  void Seed(uint64_t seed);
  uint64_t Uint64();
  double Double();
  double Uniform();
  std::string Uuid(size_t length = 16);

private:
  uint64_t state_;
};

#endif