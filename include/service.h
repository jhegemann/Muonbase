#ifndef SERVICE_H
#define SERVICE_H

#include "json.h"
#include "map.h"
#include "rand.h"
#include "utils.h"
#include "wal.h"
#include <fstream>
#include <map>
#include <optional>

template <class K, class V> class Map;

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
  void Rollover();
  std::optional<std::string> Insert(JsonObject &document);
  std::optional<std::string> Remove(std::string id);
  std::optional<JsonObject> Fetch(std::string id);
  std::vector<std::string> Keys();
  JsonObject Dump();

private:
  std::string filepath_;
  std::string filepath_journal_;
  std::string filepath_snapshot_;
  Map<std::string, JsonObject> db_;
  Serializer<Map<std::string, JsonObject>> serializer_;
  RandomGenerator random_;
  WriteAheadLog<std::string, JsonObject> log_;
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