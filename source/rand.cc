/* Copyright [2022] [Jonas Hegemann]

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

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