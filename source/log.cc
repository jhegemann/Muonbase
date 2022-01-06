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

void Log::Info(const std::string &msg) {
  if (!verbose_) {
    return;
  }
  std::string pid = std::to_string(getpid());
  std::string datetime = EpochToString(time(nullptr), "%d.%m.%Y-%H:%M:%S");
  if (stream_.is_open()) {
    stream_ << "[" << pid << "|" << datetime << "|Info] " << msg << std::endl;
  } else {
    std::cout << "[" << pid << "|" << datetime << "|Info] " << msg << std::endl;
  }
}

void Log::SetLogfile(const std::string &filepath) {
  if (!filepath.empty()) {
    stream_.open(filepath, std::fstream::out | std::fstream::app);
    return;
  }
  if (stream_.is_open()) {
    stream_.close();
  }
}

void Log::SetVerbose(bool verbose) { verbose_ = verbose; }