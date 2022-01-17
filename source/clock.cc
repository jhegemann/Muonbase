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

#include "clock.h"

Clock::Clock()
    : start_(std::chrono::high_resolution_clock::now()),
      stop_(std::chrono::high_resolution_clock::now()) {}

Clock::~Clock() {}

void Clock::Start() { start_ = std::chrono::high_resolution_clock::now(); }

void Clock::Stop() { stop_ = std::chrono::high_resolution_clock::now(); }

double Clock::Time() { return (stop_ - start_).count() * 1e-6; }