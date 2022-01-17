/* Copyright 2022 Jonas Hegemann 26 Oct 1988

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#ifndef CLOCK_H
#define CLOCK_H

#include <chrono>

class Clock {
public:
  Clock();
  virtual ~Clock();
  void Start();
  void Stop();
  double Time();

private:
  std::chrono::_V2::system_clock::time_point start_;
  std::chrono::_V2::system_clock::time_point stop_;
};

#endif