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

#ifndef LOG_H
#define LOG_H

#include <sys/types.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <mutex>

#include "utils.h"

#define LOG_INFO(message) \
  Log::GetInstance()->Info(message, __FILE__, __LINE__, __FUNCTION__)

const std::string kLogInfo = "info";
const std::string kLogDatetimeFormat = "%Y%m%d%H%M%S";

class Log {
 public:
  static Log *GetInstance();
  Log(Log &other) = delete;
  void operator=(const Log &) = delete;
  void Info(const std::string &msg, const std::string &file, int line,
            const std::string &function);
  void SetLogfile(const std::string &filepath);
  void SetVerbose(bool verbose);

 private:
  static Log *instance_;
  std::fstream stream_;
  bool verbose_;
  std::mutex mutex_;
  Log();
  virtual ~Log();
};

#endif