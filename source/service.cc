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

#include "service.h"

namespace db {

size_t Serialize(const std::string &filepath, const Database &database,
                 const std::atomic<bool> &cancel) {
  size_t bytes;
  std::ofstream stream;
  remove(filepath.c_str());
  stream.open(filepath, std::fstream::binary);
  bytes = DatabaseSerializer::Serialize(database, stream, cancel);
  stream.close();
  return bytes;
}

size_t Deserialize(const std::string &filepath, Database &database,
                   const std::atomic<bool> &cancel) {
  size_t bytes;
  std::ifstream stream;
  stream.open(filepath, std::fstream::binary);
  bytes = DatabaseSerializer::Deserialize(database, stream, cancel);
  stream.close();
  return bytes;
}

} // namespace db

ApiService::ApiService() {}

ApiService::~ApiService() {}

DocumentDatabase::DocumentDatabase(const std::string &filepath)
    : filepath_(filepath), filepath_journal_(filepath + kServiceSuffixJournal),
      filepath_closed_(filepath + kServiceSuffixJournal + kServiceSuffixClosed),
      filepath_snapshot_(filepath + kServiceSuffixSnapshot),
      filepath_corrupted_(filepath_ + kServiceSuffixCorrupted),
      rollover_in_progress_(false), rollover_cancel_(false) {}

DocumentDatabase::~DocumentDatabase() {}

void DocumentDatabase::Initialize() {
  size_t bytes;
  random_.Seed((uint64_t)time(nullptr));
  if (FileExists(filepath_)) {
    bytes = db::Deserialize(filepath_, database_);
    if (bytes == std::string::npos) {
      rename(filepath_.c_str(), filepath_corrupted_.c_str());
      throw std::runtime_error("error when deserializing database from disk");
    }
  }
  bool rollover_necessary = false;
  bool unlink_closed = false;
  bool unlink_journal = false;
  if (FileExists(filepath_closed_)) {
    DatabaseJournal::Replay(filepath_closed_, database_);
    rollover_necessary = true;
    unlink_closed = true;
  }
  if (FileExists(filepath_journal_)) {
    DatabaseJournal::Replay(filepath_journal_, database_);
    rollover_necessary = true;
    unlink_journal = true;
  }
  if (rollover_necessary) {
    LOG_INFO("database journal rollover");
    bytes = db::Serialize(filepath_snapshot_, database_);
    if (bytes == std::string::npos) {
      remove(filepath_snapshot_.c_str());
      throw std::runtime_error("error when writing snapshot to disk");
    } else {
      rename(filepath_snapshot_.c_str(), filepath_.c_str());
    }
  }
  if (unlink_closed) {
    remove(filepath_closed_.c_str());
  }
  if (unlink_journal) {
    remove(filepath_journal_.c_str());
  }
  stream_journal_.open(filepath_journal_, std::fstream::binary);
  rollover_in_progress_ = false;
  rollover_cancel_ = false;
  double usage = DatabaseMemory::Consumption(database_) / 1024.0 / 1024.0;
  LOG_INFO("memory usage: " + std::to_string(usage) + " megabytes");
}

void DocumentDatabase::Tick() { Rollover(); }

void DocumentDatabase::Shutdown() {
  if (rollover_worker_.joinable()) {
    rollover_cancel_ = true;
    rollover_worker_.join();
  }
  stream_journal_.close();
}

void DocumentDatabase::RotateJournal() {
  stream_journal_.close();
  rename(filepath_journal_.c_str(), filepath_closed_.c_str());
  stream_journal_.open(filepath_journal_, std::fstream::binary);
}

void DocumentDatabase::Rollover() {
  if (rollover_in_progress_) {
    return;
  }
  if (rollover_worker_.joinable()) {
    rollover_worker_.join();
    LOG_INFO("deferred journal rollover completed");
  }
  if (FileExists(filepath_)) {
    if (FileExists(filepath_journal_)) {
      if (FileSize(filepath_journal_) > FileSize(filepath_)) {
        RotateJournal();
      }
    }
  } else {
    if (FileExists(filepath_journal_)) {
      if (FileSize(filepath_journal_) > 16 * 1024 * 1024) {
        RotateJournal();
      }
    }
  }
  if (FileExists(filepath_closed_)) {
    rollover_in_progress_ = true;
    LOG_INFO("defer journal rollover");
    rollover_worker_ = std::thread([this] {
      size_t bytes;
      Database database;
      if (FileExists(filepath_)) {
        LOG_INFO("journal rollover: load snapshot");
        bytes = db::Deserialize(filepath_, database, rollover_cancel_);
        if (bytes == std::string::npos) {
          if (rollover_cancel_) {
            LOG_INFO("rollover cancel");
          } else {
            LOG_INFO("rollover failed: snapshot corrupted");
            rename(filepath_.c_str(), filepath_corrupted_.c_str());
          }
          return;
        }
      }
      LOG_INFO("journal rollover: replay closed journal");
      DatabaseJournal::Replay(filepath_closed_, database);
      LOG_INFO("journal rollover: write snapshot");
      bytes = db::Serialize(filepath_snapshot_, database, rollover_cancel_);
      if (bytes == std::string::npos) {
        if (rollover_cancel_) {
          LOG_INFO("rollover cancel");
        } else {
          LOG_INFO("rollover failed: remove snapshot");
        }
        remove(filepath_snapshot_.c_str());
        return;
      } else {
        rename(filepath_snapshot_.c_str(), filepath_.c_str());
        remove(filepath_closed_.c_str());
      }
      rollover_in_progress_ = false;
    });
  }
}

JsonArray DocumentDatabase::Insert(const JsonArray &values) {
  JsonArray result;
  std::string key;
  bool unique;
  JsonObject value;
  for (size_t i = 0; i < values.Size(); i++) {
    if (!values.IsObject(i)) {
      result.PutNull();
      continue;
    }
    unique = false;
    while (!unique) {
      key = random_.Uuid();
      if (!database_.Contains(key)) {
        unique = true;
      }
    }
    result.PutString(key);
    value = values.GetObject(i);
    DatabaseJournal::Append(stream_journal_, kStorageInsert, key, value);
    try {
      database_.Insert(key, value);
    } catch (std::exception &e) {
      Trace::GetInstance()->Print();
      LOG_INFO(std::string(e.what()));
      abort();
    }
  }
  return result;
}

JsonObject DocumentDatabase::Update(const JsonObject &values) {
  JsonObject result;
  JsonObject value;
  for (std::string &key : values.Keys()) {
    if (!values.IsObject(key)) {
      result.PutNull(key);
      continue;
    }
    auto iterator = database_.Find(key);
    if (iterator == database_.End()) {
      result.PutNull(key);
      continue;
    }
    result.PutObject(key, iterator.GetValue());
    value = values.GetObject(key);
    DatabaseJournal::Append(stream_journal_, kStorageUpdate, key, value);
    try {
      database_.Update(iterator, value);
    } catch (std::exception &e) {
      Trace::GetInstance()->Print();
      LOG_INFO(std::string(e.what()));
      abort();
    }
  }
  return result;
}

JsonArray DocumentDatabase::Erase(const JsonArray &keys) {
  JsonArray result;
  std::string key;
  JsonObject value;
  for (size_t i = 0; i < keys.Size(); i++) {
    if (!keys.IsString(i)) {
      result.PutNull();
      continue;
    }
    key = keys.GetString(i);
    auto iterator = database_.Find(key);
    if (iterator == database_.End()) {
      result.PutNull();
      continue;
    }
    result.PutString(key);
    value = iterator.GetValue();
    DatabaseJournal::Append(stream_journal_, kStorageErase, key, value);
    try {
      database_.Erase(iterator);
    } catch (std::exception &e) {
      Trace::GetInstance()->Print();
      LOG_INFO(std::string(e.what()));
      abort();
    }
  }
  return result;
}

JsonArray DocumentDatabase::Find(const JsonArray &keys) const {
  JsonArray result;
  for (size_t i = 0; i < keys.Size(); i++) {
    if (!keys.IsString(i)) {
      result.PutNull();
      continue;
    }
    auto iterator = database_.Find(keys.GetString(i));
    if (iterator == database_.End()) {
      result.PutNull();
      continue;
    }
    result.PutObject(iterator.GetValue());
  }
  return result;
}

UserPool::UserPool(const std::string &filepath) : filepath_(filepath) {}

UserPool::~UserPool() {}

void UserPool::Initialize() { users_.Parse(FileToString(filepath_)); }

void UserPool::Tick() {}

void UserPool::Shutdown() {}

bool UserPool::AccessPermitted(const std::string &user,
                               const std::string &password) const {
  if (users_.Has(user) && users_.IsString(user)) {
    if (Sha256Hash(password).compare(users_.GetString(user)) == 0) {
      return true;
    }
  }
  return false;
}