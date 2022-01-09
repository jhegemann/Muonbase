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
      filepath_journal_closed_(filepath + kJournalSuffix + kClosedSuffix),
      filepath_snapshot_(filepath + kSnapshotSuffix),
      filepath_corrupted_(filepath_ + kCorruptedSuffix),
      rollover_in_progress_(false), wait_for_join_(false) {}

DocumentDatabase::~DocumentDatabase() {
  if (wait_for_join_) {
    rollover_worker_.join();
  }
}

void DocumentDatabase::Initialize() {
  random_.Seed(time(nullptr));
  if (FileExists(filepath_)) {
    stream_.open(filepath_, std::fstream::in | std::fstream::binary);
    if (serializer_.Deserialize(db_, stream_) == std::string::npos) {
      rename(filepath_.c_str(), filepath_corrupted_.c_str());
      throw std::runtime_error("error when deserializing database from disk");
    }
    stream_.close();
  }
  bool rollover_necessary = false;
  bool unlink_journal_closed = false;
  bool unlink_journal = false;
  if (FileExists(filepath_journal_closed_)) {
    Journal::Replay(filepath_journal_closed_, db_);
    rollover_necessary = true;
    unlink_journal_closed = true;
  }
  if (FileExists(filepath_journal_)) {
    Journal::Replay(filepath_journal_, db_);
    rollover_necessary = true;
    unlink_journal = true;
  }
  if (rollover_necessary) {
    Log::GetInstance()->Info("complete database journal rollover");
    stream_.open(filepath_snapshot_, std::fstream::out | std::fstream::binary |
                                         std::fstream::trunc);
    size_t bytes = serializer_.Serialize(db_, stream_);
    stream_.close();
    if (bytes == std::string::npos) {
      Log::GetInstance()->Info("error when writing snapshot to disk");
      remove(filepath_snapshot_.c_str());
      unlink_journal = false;
      unlink_journal_closed = false;
    } else {
      rename(filepath_snapshot_.c_str(), filepath_.c_str());
    }
  }
  if (unlink_journal_closed) {
    unlink(filepath_journal_closed_.c_str());
  }
  if (unlink_journal) {
    unlink(filepath_journal_.c_str());
  }
}

void DocumentDatabase::Tick() {
  if (rollover_in_progress_) {
    return;
  }
  if (wait_for_join_) {
    rollover_worker_.join();
    wait_for_join_ = false;
    Log::GetInstance()->Info("deferred journal rollover completed");
  }
  if (FileExists(filepath_)) {
    if (FileExists(filepath_journal_)) {
      if (FileSize(filepath_journal_) > FileSize(filepath_)) {
        rename(filepath_journal_.c_str(), filepath_journal_closed_.c_str());
      }
    }
  } else {
    if (FileExists(filepath_journal_)) {
      if (FileSize(filepath_journal_) > 4194304) {
        rename(filepath_journal_.c_str(), filepath_journal_closed_.c_str());
      }
    }
  }
  if (FileExists(filepath_journal_closed_)) {
    rollover_in_progress_ = true;
    wait_for_join_ = true;
    Log::GetInstance()->Info("defer journal rollover");
    rollover_worker_ = std::thread([this] {
      Map<std::string, JsonObject> db;
      if (FileExists(filepath_)) {
        stream_.open(filepath_, std::fstream::in | std::fstream::binary);
        size_t bytes = serializer_.Deserialize(db, stream_);
        stream_.close();
        if (bytes == std::string::npos) {
          Log::GetInstance()->Info(
              "rollover worker: error when deserializing from disk");
          return;
        }
      }
      Journal::Replay(filepath_journal_closed_, db);
      stream_.open(filepath_snapshot_, std::fstream::out |
                                           std::fstream::binary |
                                           std::fstream::trunc);
      size_t bytes = serializer_.Serialize(db, stream_);
      stream_.close();
      if (bytes == std::string::npos) {
        Log::GetInstance()->Info(
            "rollover worker: error when serializing to disk");
        remove(filepath_snapshot_.c_str());
      } else {
        rename(filepath_snapshot_.c_str(), filepath_.c_str());
        unlink(filepath_journal_closed_.c_str());
      }
      rollover_in_progress_ = false;
    });
  }
}

void DocumentDatabase::Shutdown() {}

std::optional<std::string> DocumentDatabase::Insert(JsonObject &document) {
  std::string id;
  bool unique = false;
  while (!unique) {
    id = random_.Uuid();
    if (db_.Find(id) == db_.End()) {
      unique = true;
    }
  }
  Journal::Append(filepath_journal_, STORAGE_INSERT, id, &document);
  db_.Insert(id, document);
  return id;
}

std::optional<std::string> DocumentDatabase::Erase(std::string id) {
  MapIterator<std::string, JsonObject> it = db_.Find(id);
  if (it == db_.End()) {
    return {};
  }
  Journal::Append(filepath_journal_, STORAGE_ERASE, id, nullptr);
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

void UserPool::Shutdown() {}

bool UserPool::AccessPermitted(const std::string &user,
                               const std::string &passwd) {
  if (users_.Has(user) && users_.IsString(user)) {
    if (Sha256Hash(passwd).compare(users_.GetAsString(user)) == 0) {
      return true;
    }
  }
  return false;
}