#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <sys/types.h>
#include <unistd.h>

#include "utils.h"

class Log {
public:
  static Log *GetInstance();
  Log(Log &other) = delete;
  void operator=(const Log &) = delete;
  void Info(const std::string &msg);

private:
  static Log *instance_;
  Log();
};

#endif