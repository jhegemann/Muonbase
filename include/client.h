#ifndef CLIENT_H
#define CLIENT_H

#include <map>
#include <string>

#include "http.h"
#include "json.h"
#include "log.h"
#include "tcp.h"

class Client {
public:
  Client(const std::string &ip, const std::string &port);
  virtual ~Client();
  void RandomInsert(const size_t count = 1);
  void RandomRemove(const size_t count = 1);
  void CompleteLookup();

private:
  std::string ip_;
  std::string port_;
  std::map<std::string, JsonObject> internal;
};

#endif