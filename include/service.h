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

#ifndef SERVICE_H
#define SERVICE_H

#include "json.h"
#include "map.h"
#include "rand.h"
#include "utils.h"
#include "wal.h"
#include <atomic>
#include <fstream>
#include <optional>
#include <thread>

template <class K, class V> class Map;

typedef WriteAheadLog<std::string, JsonObject> Journal;

const std::string kJournalSuffix = ".journal";
const std::string kSnapshotSuffix = ".snapshot";
const std::string kClosedSuffix = ".closed";
const std::string kCorruptedSuffix = ".corrupted";

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
  std::optional<std::string> Insert(JsonObject &document);
  std::optional<std::string> Erase(std::string id);
  std::optional<JsonObject> Find(std::string id);
  std::vector<std::string> Keys();
  JsonObject Image();

private:
  std::string filepath_;
  std::string filepath_journal_;
  std::string filepath_journal_closed_;
  std::string filepath_snapshot_;
  std::string filepath_corrupted_;
  std::fstream stream_;
  Map<std::string, JsonObject> db_;
  Serializer<Map<std::string, JsonObject>> serializer_;
  RandomGenerator random_;
  std::thread rollover_worker_;
  std::atomic<bool> rollover_in_progress_;
  std::atomic<bool> wait_for_join_;
};

class UserPool : public ApiService {
public:
  UserPool(const std::string &filepath);
  virtual ~UserPool();
  virtual void Initialize();
  virtual void Tick();
  virtual void Shutdown();
  bool AccessPermitted(const std::string &user, const std::string &passwd);

private:
  std::string filepath_;
  JsonObject users_;
};

#endif