#ifndef WAL_H
#define WAL_H

#include <deque>
#include <sstream>

template <class K, class V> class LogEntry;
template <class K, class V> class WriteAheadLog;

enum StorageModification { STORAGE_INSERT = 0, STORAGE_REMOVE };

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
  WriteAheadLog();
  virtual ~WriteAheadLog();
  void Load(const std::string &filepath);
  void Unload();
  void Append(const std::string &filepath, StorageModification operation,
              K &key, V *value = nullptr);
  std::deque<LogEntry<K, V>> &Log();
  size_t Size();

private:
  std::fstream stream_;
  std::deque<LogEntry<K, V>> log_;
  Serializer<K> key_serializer_;
  Serializer<V> value_serializer_;
};

template <class K, class V> WriteAheadLog<K, V>::WriteAheadLog() {}

template <class K, class V> WriteAheadLog<K, V>::~WriteAheadLog() {
  stream_.close();
}

template <class K, class V>
void WriteAheadLog<K, V>::Load(const std::string &filepath) {
  log_.clear();
  if (FileExists(filepath)) {
    size_t size = FileSize(filepath);
    stream_.open(filepath, std::fstream::in | std::fstream::binary);
    uint8_t buffer;
    StorageModification operation;
    K key;
    V *value;
    size_t bytes = 0;
    while (bytes < size) {
      stream_.read((char *)&buffer, sizeof(uint8_t));
      bytes += sizeof(uint8_t);
      switch (buffer) {
      case 0:
        operation = STORAGE_INSERT;
        break;
      case 1:
        operation = STORAGE_REMOVE;
        break;
      default:
        throw std::runtime_error("WriteAheadLog: unknown storage operation");
      }
      bytes += key_serializer_.Deserialize(key, stream_);
      stream_.read((char *)&buffer, sizeof(uint8_t));
      bytes += sizeof(uint8_t);
      if (buffer == 0) {
        value = nullptr;
      } else {
        value = new JsonObject();
        bytes += value_serializer_.Deserialize((*value), stream_);
        std::cout << value->AsString() << std::endl;
      }
      log_.emplace_back(operation, key, value);
    }
    stream_.close();
  }
}

template <class K, class V> void WriteAheadLog<K, V>::Unload() { log_.clear(); }

template <class K, class V>
void WriteAheadLog<K, V>::Append(const std::string &filepath,
                                 StorageModification operation, K &key,
                                 V *value) {
  stream_.open(filepath,
               std::fstream::out | std::fstream::binary | std::fstream::app);
  uint8_t buffer = static_cast<uint8_t>(operation);
  stream_.write((const char *)&buffer, sizeof(uint8_t));
  key_serializer_.Serialize(key, stream_);
  if (value) {
    buffer = 1;
    stream_.write((const char *)&buffer, sizeof(uint8_t));
    value_serializer_.Serialize((*value), stream_);
  } else {
    buffer = 0;
    stream_.write((const char *)&buffer, sizeof(uint8_t));
  }
  stream_.flush();
  stream_.close();
}

template <class K, class V>
std::deque<LogEntry<K, V>> &WriteAheadLog<K, V>::Log() {
  return log_;
}

template <class K, class V> size_t WriteAheadLog<K, V>::Size() {
  return log_.size();
}

#endif