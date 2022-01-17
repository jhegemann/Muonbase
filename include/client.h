/* Copyright 2022 Jonas Hegemann 26 Oct 1988

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

#include "api.h"
#include "http.h"
#include "json.h"
#include "log.h"
#include "tcp.h"

class Client {
public:
  Client(const std::string &ip, const std::string &port,
         const std::string &user, const std::string &password);
  virtual ~Client();
  JsonArray Insert(const JsonArray &values);
  JsonArray Erase(const JsonArray &keys);
  JsonArray Find(const JsonArray &keys);
  JsonArray Keys();
  JsonArray Values();
  JsonObject Image();

private:
  std::string ip_;
  std::string port_;
  std::string user_;
  std::string password_;
};

#endif