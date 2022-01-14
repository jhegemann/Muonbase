/* Copyright [2022] [Jonas Hegemann, 26 Oct 1988]

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
    size_t bytes = serializer_.Deserialize(db_, stream_);
    stream_.close();
    if (bytes == std::string::npos) {
      rename(filepath_.c_str(), filepath_corrupted_.c_str());
      throw std::runtime_error("error when deserializing database from disk");
    }
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
    Log::GetInstance()->Info("database journal rollover");
    stream_.open(filepath_snapshot_, std::fstream::out | std::fstream::binary |
                                         std::fstream::trunc);
    size_t bytes = serializer_.Serialize(db_, stream_);
    stream_.close();
    if (bytes == std::string::npos) {
      remove(filepath_snapshot_.c_str());
      throw std::runtime_error("error when writing snapshot to disk");
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
          rename(filepath_.c_str(), filepath_corrupted_.c_str());
          throw std::runtime_error(
              "error when deserializing database from disk");
        }
      }
      Journal::Replay(filepath_journal_closed_, db);
      stream_.open(filepath_snapshot_, std::fstream::out |
                                           std::fstream::binary |
                                           std::fstream::trunc);
      size_t bytes = serializer_.Serialize(db, stream_);
      stream_.close();
      if (bytes == std::string::npos) {
        remove(filepath_snapshot_.c_str());
        throw std::runtime_error("error when writing snapshot to disk");
      } else {
        rename(filepath_snapshot_.c_str(), filepath_.c_str());
        unlink(filepath_journal_closed_.c_str());
      }
      rollover_in_progress_ = false;
    });
  }
}

void DocumentDatabase::Shutdown() {}

JsonArray DocumentDatabase::Insert(const JsonArray &values) {
  JsonArray result;
  std::string key;
  bool unique = false;
  JsonObject value;
  for (size_t i = 0; i < values.Size(); i++) {
    value = values.GetObject(i);
    unique = false;
    while (!unique) {
      key = random_.Uuid();
      if (db_.Find(key) == db_.End()) {
        unique = true;
      }
    }
    result.PutString(key);
    Journal::Append(filepath_journal_, STORAGE_INSERT, key, &value);
    db_.Insert(key, value);
  }
  return result;
}

JsonArray DocumentDatabase::Erase(const JsonArray &keys) {
  JsonArray result;
  std::string key;
  for (size_t i = 0; i < keys.Size(); i++) {
    key = keys.GetString(i);
    auto it = db_.Find(key);
    if (it == db_.End()) {
      result.PutNull();
      continue;
    }
    result.PutString(key);
    Journal::Append(filepath_journal_, STORAGE_ERASE, key, nullptr);
    db_.Erase(it);
  }
  return result;
}

JsonArray DocumentDatabase::Find(const JsonArray &keys) {
  JsonArray result;
  for (size_t i = 0; i < keys.Size(); i++) {
    auto it = db_.Find(keys.GetString(i));
    if (it == db_.End()) {
      result.PutNull();
      continue;
    }
    result.PutObject(it.GetValue());
  }
  return result;
}

JsonArray DocumentDatabase::Keys() {
  MapIterator<std::string, JsonObject> it = db_.Begin();
  JsonArray keys;
  while (it != db_.End()) {
    keys.PutString(it.GetKey());
    it++;
  }
  return keys;
}

JsonArray DocumentDatabase::Values() {
  MapIterator<std::string, JsonObject> it = db_.Begin();
  JsonArray values;
  while (it != db_.End()) {
    values.PutObject(it.GetValue());
    it++;
  }
  return values;
}

JsonObject DocumentDatabase::Image() {
  MapIterator<std::string, JsonObject> it = db_.Begin();
  JsonObject image;
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
                               const std::string &password) {
  if (users_.Has(user) && users_.IsString(user)) {
    if (Sha256Hash(password).compare(users_.GetString(user)) == 0) {
      return true;
    }
  }
  return false;
}