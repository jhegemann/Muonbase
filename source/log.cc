#include "log.h"

Log *Log::instance_ = nullptr;

Log::Log() {}

Log *Log::GetInstance() {
  if (!Log::instance_) {
    Log::instance_ = new Log();
  }
  return Log::instance_;
}

void Log::Info(const std::string &msg) {
  std::string pid = std::to_string(getpid());
  std::string datetime = EpochToString(time(nullptr), "%d.%m.%Y %H:%M:%S");
  std::cout << "[" << pid << " " << datetime << " Info] " << msg << std::endl;
}