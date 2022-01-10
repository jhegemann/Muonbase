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

#ifndef WAL_H
#define WAL_H

#include "map.h"
#include <deque>
#include <sstream>

template <class K, class V> class LogEntry;
template <class K, class V> class WriteAheadLog;

enum StorageModification { STORAGE_INSERT = 0, STORAGE_ERASE };

template <class K, class V> class LogEntry {
  template <class, class> friend class ::WriteAheadLog;

public:
  LogEntry(StorageModification operation, K &key, V *value = nullptr);
  virtual ~LogEntry();
  StorageModification GetOperation();
  const K &GetKey();
  const V *GetValue();

private:
  StorageModification operation_;
  K key_;
  V *value_;
};

template <class K, class V>
LogEntry<K, V>::LogEntry(StorageModification operation, K &key, V *value)
    : operation_(operation), key_(key), value_(value) {}

template <class K, class V> LogEntry<K, V>::~LogEntry() {}

template <class K, class V> StorageModification LogEntry<K, V>::GetOperation() {
  return operation_;
}

template <class K, class V> const K &LogEntry<K, V>::GetKey() { return key_; }

template <class K, class V> const V *LogEntry<K, V>::GetValue() {
  return value_;
}

template <class K, class V> class WriteAheadLog {
public:
  static void Replay(const std::string &filepath, Map<K, V> &db);
  static void Append(const std::string &filepath, StorageModification operation,
                     K &key, V *value = nullptr);
};

template <class K, class V>
void WriteAheadLog<K, V>::Replay(const std::string &filepath, Map<K, V> &db) {
  std::fstream stream;
  std::deque<LogEntry<K, V>> log;
  Serializer<K> key_serializer;
  Serializer<V> value_serializer;
  size_t value_bytes = 0;
  if (FileExists(filepath)) {
    size_t size = FileSize(filepath);
    stream.open(filepath, std::fstream::in | std::fstream::binary);
    uint8_t buffer;
    StorageModification operation;
    K key;
    V *value;
    size_t bytes = 0;
    while (bytes < size) {
      stream.read((char *)&buffer, sizeof(uint8_t));
      if (!stream) {
        throw std::runtime_error("incomplete journal message");
      }
      bytes += sizeof(uint8_t);
      switch (buffer) {
      case 0:
        operation = STORAGE_INSERT;
        break;
      case 1:
        operation = STORAGE_ERASE;
        break;
      default:
        throw std::runtime_error("unknown storage operation");
      }
      bytes += key_serializer.Deserialize(key, stream);
      stream.read((char *)&buffer, sizeof(uint8_t));
      if (!stream) {
        throw std::runtime_error("incomplete journal message");
      }
      bytes += sizeof(uint8_t);
      if (buffer == 0) {
        value = nullptr;
      } else {
        value = new JsonObject();
        value_bytes = value_serializer.Deserialize((*value), stream);
        if (value_bytes == std::string::npos) {
          throw std::runtime_error("incomplete journal message");
          break;
        }
        bytes += value_bytes;
      }
      log.emplace_back(operation, key, value);
    }
    stream.close();
    if (!stream) {
      throw std::runtime_error("error when loading journal");
    }
  }
  for (auto it = log.begin(); it != log.end(); it++) {
    switch (it->GetOperation()) {
    case STORAGE_INSERT:
      db.Insert(it->GetKey(), *it->GetValue());
      break;
    case STORAGE_ERASE:
      db.Erase(it->GetKey());
      break;
    default:
      abort();
    }
  }
  for (auto it = log.begin(); it != log.end(); it++) {
    if (it->value_) {
      delete it->value_;
    }
  }
  log.clear();
}

template <class K, class V>
void WriteAheadLog<K, V>::Append(const std::string &filepath,
                                 StorageModification operation, K &key,
                                 V *value) {
  std::fstream stream;
  Serializer<K> key_serializer;
  Serializer<V> value_serializer;
  stream.open(filepath,
              std::fstream::out | std::fstream::binary | std::fstream::app);
  uint8_t buffer = static_cast<uint8_t>(operation);
  stream.write((const char *)&buffer, sizeof(uint8_t));
  if (key_serializer.Serialize(key, stream) == std::string::npos) {
    throw std::runtime_error("error appending to journal");
  }
  if (value) {
    buffer = 1;
    stream.write((const char *)&buffer, sizeof(uint8_t));
    if (value_serializer.Serialize((*value), stream) == std::string::npos) {
      throw std::runtime_error("error appending to journal");
    }
  } else {
    buffer = 0;
    stream.write((const char *)&buffer, sizeof(uint8_t));
  }
  stream.flush();
  stream.close();
  if (!stream) {
    throw std::runtime_error("error appending to journal");
  }
}

#endif