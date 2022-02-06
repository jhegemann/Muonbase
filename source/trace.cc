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

#include "trace.h"

Trace *Trace::instance_ = nullptr;

Trace::Trace() {}

Trace::~Trace() {}

Trace *Trace::GetInstance() {
  if (!Trace::instance_) {
    Trace::instance_ = new Trace();
  }
  return Trace::instance_;
}

void Trace::Push(const std::string &file, int line,
                 const std::string &function) {
  std::lock_guard<std::mutex> guard(mutex_);
  trace_.push_back(file + kStringColon + function + kStringColon +
                   std::to_string(line));
  if (trace_.size() > kTraceLimit) {
    trace_.pop_front();
  }
}

void Trace::Print() {
  std::lock_guard<std::mutex> guard(mutex_);
  std::cout << kStringTab << "** STACKTRACE **" << std::endl;
  for (auto it = trace_.begin(); it != trace_.end(); it++) {
    std::cout << kStringTab << *it << std::endl;
  }
}