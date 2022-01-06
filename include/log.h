#ifndef LOG_H
#define LOG_H

#include <fstream>
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
  void SetLogfile(const std::string &filepath);
  void SetVerbose(bool verbose);  

private:
  static Log *instance_;
  std::fstream stream_;
  bool verbose_;
  Log();
  virtual ~Log();
};

#endif