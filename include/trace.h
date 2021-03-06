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

#ifndef TRACE_H
#define TRACE_H

#include <sys/types.h>
#include <unistd.h>

#include <deque>
#include <fstream>
#include <iostream>
#include <mutex>

#include "utils.h"

#define STACKTRACE Trace::GetInstance()->Push(__FILE__, __LINE__, __FUNCTION__)

const size_t kTraceLimit = 1000;

class Trace {
 public:
  static Trace *GetInstance();
  Trace(Trace &other) = delete;
  void operator=(const Trace &) = delete;
  void Push(const std::string &file, int line, const std::string &function);
  void Print();

 private:
  static Trace *instance_;
  std::mutex mutex_;
  std::deque<std::string> trace_;
  Trace();
  virtual ~Trace();
};

#endif