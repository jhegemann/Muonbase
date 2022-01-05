#include "rand.h"

RandomGenerator::RandomGenerator() : state_(123456789) {}

RandomGenerator::RandomGenerator(uint64_t seed) : state_(seed) {}

RandomGenerator::~RandomGenerator() {}

void RandomGenerator::Seed(uint64_t seed) { state_ = seed; }

uint64_t RandomGenerator::Uint64() {
  state_ ^= state_ << 13;
  state_ ^= state_ >> 7;
  state_ ^= state_ << 17;
  return state_;
}

double RandomGenerator::Double() { return static_cast<double>(Uint64()); }

double RandomGenerator::Uniform() {
  return Double() / static_cast<double>(std::numeric_limits<uint64_t>::max());
}

std::string RandomGenerator::Uuid(size_t length) {
  static std::string charset =
      "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  std::string uuid;
  uuid.resize(length);
  for (size_t i = 0; i < length; i++) {
    uuid[i] = charset[Uint64() % charset.length()];
  }
  return uuid;
}