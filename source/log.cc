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

#include "log.h"

Log *Log::instance_ = nullptr;

Log::Log() : verbose_(false) {}

Log::~Log() {
  if (stream_.is_open()) {
    stream_.close();
  }
}

Log *Log::GetInstance() {
  if (!Log::instance_) {
    Log::instance_ = new Log();
  }
  return Log::instance_;
}

void Log::Info(const std::string &msg, const std::string &file, int line) {
  std::lock_guard<std::mutex> guard(mutex_);
  if (!verbose_) {
    return;
  }
  std::string pid = std::to_string(getpid());
  std::string datetime = EpochToString(time(nullptr), kLogDatetimeFormat);
  if (stream_.is_open()) {
    stream_ << kStringSquareBracketOpen << pid << kStringPipe << datetime
            << kStringPipe << kLogInfo << kStringPipe << file << kStringColon
            << line << kStringSquareBracketClose << kStringSpace << msg
            << std::endl;
  } else {
    std::cout << kStringSquareBracketOpen << pid << kStringPipe << datetime
              << kStringPipe << kLogInfo << kStringPipe << file << kStringColon
              << line << kStringSquareBracketClose << kStringSpace << msg
              << std::endl;
  }
}

void Log::SetLogfile(const std::string &filepath) {
  if (stream_.is_open()) {
    stream_.close();
  }
  if (!filepath.empty()) {
    stream_.open(filepath, std::fstream::out | std::fstream::app);
    return;
  }
}

void Log::SetVerbose(bool verbose) { verbose_ = verbose; }