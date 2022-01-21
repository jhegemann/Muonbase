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

#include "json.h"
#include "map.h"
#include <deque>
#include <sstream>

enum StorageModification { STORAGE_INSERT = 0, STORAGE_ERASE };

template <class K, class V> class Journal {
public:
  static void Replay(const std::string &filepath, Map<K, V> &db);
  static void Append(std::fstream &stream, StorageModification operation,
                     K &key, V *value = nullptr);
};

template <class K, class V>
void Journal<K, V>::Replay(const std::string &filepath, Map<K, V> &db) {
  std::fstream stream;
  size_t value_bytes = 0;
  if (!FileExists(filepath)) {
    return;
  }
  size_t size = FileSize(filepath);
  stream.open(filepath, std::fstream::in | std::fstream::binary);
  uint8_t buffer;
  StorageModification operation;
  K key;
  V value;
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
      throw std::runtime_error("unknown storage modification");
    }
    bytes += Serializer<K>::Deserialize(key, stream);
    stream.read((char *)&buffer, sizeof(uint8_t));
    if (!stream) {
      throw std::runtime_error("incomplete journal message");
    }
    bytes += sizeof(uint8_t);
    if (buffer != 0) {
      value_bytes = Serializer<V>::Deserialize(value, stream);
      if (value_bytes == std::string::npos) {
        throw std::runtime_error("incomplete journal message");
        break;
      }
      bytes += value_bytes;
    }
    switch (operation) {
    case STORAGE_INSERT:
      db.Insert(key, value);
      break;
    case STORAGE_ERASE:
      db.Erase(key);
      break;
    default:
      throw std::runtime_error("unknown storage modification");
    }
  }
  stream.close();
  if (!stream) {
    throw std::runtime_error("error when loading journal");
  }
}

template <class K, class V>
void Journal<K, V>::Append(std::fstream &stream, StorageModification operation,
                           K &key, V *value) {
  uint8_t buffer = static_cast<uint8_t>(operation);
  stream.write((const char *)&buffer, sizeof(uint8_t));
  if (Serializer<K>::Serialize(key, stream) == std::string::npos) {
    throw std::runtime_error("error appending to journal");
  }
  if (value) {
    buffer = 1;
    stream.write((const char *)&buffer, sizeof(uint8_t));
    if (Serializer<V>::Serialize((*value), stream) == std::string::npos) {
      throw std::runtime_error("error appending to journal");
    }
  } else {
    buffer = 0;
    stream.write((const char *)&buffer, sizeof(uint8_t));
  }
  stream.flush();
  if (!stream) {
    throw std::runtime_error("error appending to journal");
  }
}

#endif