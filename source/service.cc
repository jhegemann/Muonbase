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

#include "service.h"

ApiService::ApiService() {}

ApiService::~ApiService() {}

DocumentDatabase::DocumentDatabase(const std::string &filepath)
    : filepath_(filepath), filepath_journal_(filepath + kJournalSuffix),
      filepath_snapshot_(filepath + kSnapshotSuffix) {}

DocumentDatabase::~DocumentDatabase() {}

void DocumentDatabase::Initialize() {
  random_.Seed(time(nullptr));
  if (FileExists(filepath_)) {
    std::fstream stream;
    stream.open(filepath_, std::fstream::in | std::fstream::binary);
    serializer_.Deserialize(db_, stream);
    stream.close();
  }
  log_.Load(filepath_journal_);
  Log::GetInstance()->Info("replay write ahead log");
  for (auto it = log_.Log().begin(); it != log_.Log().end(); it++) {
    switch (it->GetOperation()) {
    case STORAGE_INSERT:
      db_.Insert(it->GetKey(), *it->GetValue());
      break;
    case STORAGE_ERASE:
      db_.Erase(it->GetKey());
      break;
    default:
      throw std::runtime_error("unknown storage operation");
    }
  }
  log_.Unload();
  Rollover();
}

void DocumentDatabase::Tick() { Rollover(); }

void DocumentDatabase::Shutdown() {}

void DocumentDatabase::Rollover() {
  if (db_.Size() == 0) {
    return;
  }
  if (!FileExists(filepath_journal_)) {
    return;
  }
  if (FileSize(filepath_journal_) < FileSize(filepath_)) {
    return;
  }
  Log::GetInstance()->Info("rollover database journal");
  std::fstream stream;
  stream.open(filepath_snapshot_, std::fstream::out | std::fstream::binary);
  serializer_.Serialize(db_, stream);
  stream.close();
  rename(filepath_snapshot_.c_str(), filepath_.c_str());
  unlink(filepath_journal_.c_str());
}

std::optional<std::string> DocumentDatabase::Insert(JsonObject &document) {
  std::string id;
  bool unique = false;
  while (!unique) {
    id = random_.Uuid();
    if (db_.Find(id) == db_.End()) {
      unique = true;
    }
  }
  log_.Append(filepath_journal_, STORAGE_INSERT, id, &document);
  db_.Insert(id, document);
  return id;
}

std::optional<std::string> DocumentDatabase::Erase(std::string id) {
  MapIterator<std::string, JsonObject> it = db_.Find(id);
  if (it == db_.End()) {
    return {};
  }
  log_.Append(filepath_journal_, STORAGE_ERASE, id, nullptr);
  db_.Erase(it);
  return id;
}

std::optional<JsonObject> DocumentDatabase::Find(std::string id) {
  MapIterator<std::string, JsonObject> it = db_.Find(id);
  if (it == db_.End()) {
    return {};
  }
  return it.GetValue();
}

std::vector<std::string> DocumentDatabase::Keys() {
  MapIterator<std::string, JsonObject> it = db_.Begin();
  std::vector<std::string> keys;
  while (it != db_.End()) {
    keys.emplace_back(it.GetKey());
    it++;
  }
  return keys;
}

JsonObject DocumentDatabase::Image() {
  JsonObject image;
  MapIterator<std::string, JsonObject> it = db_.Begin();
  while (it != db_.End()) {
    image.PutObject(it.GetKey(), it.GetValue());
    it++;
  }
  return image;
}

UserPool::UserPool(const std::string &filepath) : filepath_(filepath) {}

UserPool::~UserPool() {}

void UserPool::Initialize() { users_.Parse(FileToString(filepath_)); }

void UserPool::Tick() {}

void UserPool::Shutdown() { }

bool UserPool::AccessPermitted(const std::string &user,
                               const std::string &passwd) {
  if (users_.Has(user) && users_.IsString(user)) {
    if (Sha256Hash(passwd).compare(users_.GetAsString(user)) == 0) {
      return true;
    }
  }
  return false;
}