/* Copyright [2022] [Jonas Hegemann, 26 Oct 1988]

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

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