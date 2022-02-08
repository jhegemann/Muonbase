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

#ifndef RAND_H
#define RAND_H

#include <limits>
#include <mutex>
#include <random>
#include <string>

const std::string kUuidCharset =
    "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

class Random {
public:
  Random();
  Random(uint64_t seed);
  virtual ~Random();
  void Seed(uint64_t seed);
  unsigned long UniformInteger();
  double UniformDouble();
  std::string Uuid(size_t length = 8);

private:
  std::mt19937_64 generator_;
  std::uniform_int_distribution<unsigned long> distribution_integer_;
  std::uniform_real_distribution<double> distribution_float_;
};

#endif