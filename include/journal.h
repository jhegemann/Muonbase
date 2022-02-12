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

#ifndef JOURNAL_H
#define JOURNAL_H

#include <deque>
#include <sstream>

#include "json.h"
#include "map.h"

const uint8_t kStorageInsert = 0;
const uint8_t kStorageUpdate = 1;
const uint8_t kStorageErase = 2;

template <class K, class V>
class Journal {
 public:
  static void Replay(const std::string &filepath, Map<K, V> &db,
                     const std::atomic<bool> &cancel = false);
  static void Append(std::ofstream &stream, uint8_t operation, const K &key,
                     const V &value);
};

template <class K, class V>
void Journal<K, V>::Replay(const std::string &filepath, Map<K, V> &db,
                           const std::atomic<bool> &cancel) {
  if (!FileExists(filepath)) {
    return;
  }
  std::ifstream stream;
  size_t key_bytes = 0;
  size_t value_bytes = 0;
  size_t size = FileSize(filepath);
  stream.open(filepath, std::fstream::binary);
  uint8_t operation;
  K key;
  V value;
  size_t bytes = 0;
  while (bytes < size) {
    if (cancel) {
      return;
    }
    stream.read((char *)&operation, sizeof(uint8_t));
    bytes += sizeof(uint8_t);
    if (!stream) {
      throw std::runtime_error("journal: could not read storage modification");
    }
    key_bytes = Serializer<K>::Deserialize(key, stream);
    if (key_bytes == std::string::npos) {
      throw std::runtime_error("journal: could not read key");
    }
    bytes += key_bytes;
    value_bytes = Serializer<V>::Deserialize(value, stream);
    if (value_bytes == std::string::npos) {
      throw std::runtime_error("journal: could not read value");
    }
    bytes += value_bytes;
    MapIterator<K, V> iterator;
    switch (operation) {
      case kStorageInsert:
        db.Insert(key, value);
        break;
      case kStorageUpdate:
        iterator = db.Find(key);
        if (iterator != db.End()) {
          db.Update(iterator, value);
        } else {
          throw std::runtime_error("journal: update non-existent key " + key);
        }
        break;
      case kStorageErase:
        db.Erase(key);
        break;
      default:
        throw std::runtime_error("journal: unknown storage modification");
    }
  }
  stream.close();
  if (!stream) {
    throw std::runtime_error("journal: invalidated stream");
  }
}

template <class K, class V>
void Journal<K, V>::Append(std::ofstream &stream, uint8_t operation,
                           const K &key, const V &value) {
  stream.write((const char *)&operation, sizeof(uint8_t));
  if (!stream) {
    throw std::runtime_error("journal: could write operation");
  }
  if (Serializer<K>::Serialize(key, stream) == std::string::npos) {
    throw std::runtime_error("journal: could append key");
  }
  if (Serializer<V>::Serialize(value, stream) == std::string::npos) {
    throw std::runtime_error("journal: could not append value");
  }
  stream.flush();
  if (!stream) {
    throw std::runtime_error("journal: invalidated stream");
  }
}

#endif