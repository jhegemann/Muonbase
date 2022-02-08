/* Copyright 2022 Jonas Hegemann <jonas.hegemann@hotmail.de>

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

Random::Random() {}

Random::Random(uint64_t seed) { Seed(seed); }

Random::~Random() {}

void Random::Seed(uint64_t seed) { generator_.seed(seed); }

unsigned long Random::UniformInteger() {
  return distribution_integer_(generator_);
}
double Random::UniformDouble() {
  return distribution_float_(generator_);
}

std::string Random::Uuid(size_t length) {
  std::string uuid;
  uuid.resize(length);
  for (size_t i = 0; i < length; i++) {
    uuid[i] = kUuidCharset[UniformInteger() % kUuidCharset.length()];
  }
  return uuid;
}