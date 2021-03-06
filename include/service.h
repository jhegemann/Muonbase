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

#ifndef SERVICE_H
#define SERVICE_H

#include <atomic>
#include <fstream>
#include <optional>
#include <thread>

#include "journal.h"
#include "json.h"
#include "map.h"
#include "rand.h"
#include "trace.h"
#include "utils.h"

class ApiService;
class DocumentDatabase;
class UserPool;

const std::string kServiceSuffixJournal = ".journal";
const std::string kServiceSuffixSnapshot = ".snapshot";
const std::string kServiceSuffixClosed = ".closed";
const std::string kServiceSuffixCorrupted = ".corrupted";

typedef Map<std::string, JsonObject> Database;
typedef Serializer<Database> DatabaseSerializer;
typedef Memory<Database> DatabaseMemory;
typedef Journal<std::string, JsonObject> DatabaseJournal;

namespace db {

size_t Serialize(const std::string &filepath, const Database &database,
                 const std::atomic<bool> &cancel = false);
size_t Deserialize(const std::string &filepath, Database &database,
                   const std::atomic<bool> &cancel = false);

}  // namespace db

class ApiService {
 public:
  ApiService();
  virtual ~ApiService();
  virtual void Initialize() = 0;
  virtual void Tick() = 0;
  virtual void Shutdown() = 0;
};

class DocumentDatabase : public ApiService {
 public:
  DocumentDatabase(const std::string &filepath);
  virtual ~DocumentDatabase();
  virtual void Initialize();
  virtual void Tick();
  virtual void Shutdown();
  JsonArray Insert(const JsonArray &values);
  JsonObject Update(const JsonObject &values);
  JsonArray Erase(const JsonArray &keys);
  JsonArray Find(const JsonArray &keys) const;

 private:
  void RotateJournal();
  void Rollover();
  std::string filepath_;
  std::string filepath_journal_;
  std::string filepath_closed_;
  std::string filepath_snapshot_;
  std::string filepath_corrupted_;
  std::ofstream stream_journal_;
  Database database_;
  Random random_;
  std::thread rollover_worker_;
  std::atomic<bool> rollover_in_progress_;
  std::atomic<bool> rollover_cancel_;
};

class UserPool : public ApiService {
 public:
  UserPool(const std::string &filepath);
  virtual ~UserPool();
  virtual void Initialize();
  virtual void Tick();
  virtual void Shutdown();
  bool AccessPermitted(const std::string &user,
                       const std::string &password) const;

 private:
  std::string filepath_;
  JsonObject users_;
};

#endif