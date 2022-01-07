/* Copyright [2022] [Jonas Hegemann]

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

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
  void RandomErase(const size_t count = 1);
  void FindAll();

private:
  std::string ip_;
  std::string port_;
  std::map<std::string, JsonObject> internal_;
};

#endif