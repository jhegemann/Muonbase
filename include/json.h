#ifndef JSON_H
#define JSON_H

#include <algorithm>
#include <any>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "utils.h"
#include "rand.h"

typedef bool JsonBoolean;
typedef int64_t JsonInteger;
typedef double JsonFloat;
typedef std::string JsonString;

class JsonArray;
class JsonObject;

class JsonArray {
public:
  friend class JsonObject;
  JsonArray();
  JsonArray(const std::string &source);
  virtual ~JsonArray();
  size_t Size();
  void PutNull();
  void PutBoolean(JsonBoolean value);
  void PutInteger(JsonInteger value);
  void PutFloat(JsonFloat value);
  void PutString(JsonString value);
  void PutObject(JsonObject value);
  void PutArray(JsonArray value);
  JsonBoolean GetAsBoolean(size_t index);
  JsonInteger GetAsInteger(size_t index);
  JsonFloat GetAsFloat(size_t index);
  JsonString GetAsString(size_t index);
  JsonObject GetAsObject(size_t index);
  JsonArray GetAsArray(size_t index);
  bool IsNull(size_t index);
  bool IsBoolean(size_t index);
  bool IsInteger(size_t index);
  bool IsFloat(size_t index);
  bool IsString(size_t index);
  bool IsObject(size_t index);
  bool IsArray(size_t index);
  void Clear();
  std::string AsString();
  void FromString(const std::string &source);

private:
  std::vector<std::any> values_;
  void FromString(const std::string &source, size_t &offset);
};

class JsonObject {
public:
  friend class JsonArray;
  JsonObject();
  JsonObject(const std::string &source);
  virtual ~JsonObject();
  bool Has(const std::string &key);
  void PutNull(const std::string &key);
  void PutBoolean(const std::string &key, JsonBoolean value);
  void PutInteger(const std::string &key, JsonInteger value);
  void PutFloat(const std::string &key, JsonFloat value);
  void PutString(const std::string &key, JsonString value);
  void PutObject(const std::string &key, JsonObject value);
  void PutArray(const std::string &key, JsonArray value);
  JsonBoolean GetAsBoolean(const std::string &key);
  JsonInteger GetAsInteger(const std::string &key);
  JsonFloat GetAsFloat(const std::string &key);
  JsonString GetAsString(const std::string &key);
  JsonObject GetAsObject(const std::string &key);
  JsonArray GetAsArray(const std::string &key);
  bool IsNull(const std::string &key);
  bool IsBoolean(const std::string &key);
  bool IsInteger(const std::string &key);
  bool IsFloat(const std::string &key);
  bool IsString(const std::string &key);
  bool IsObject(const std::string &key);
  bool IsArray(const std::string &key);
  std::vector<std::string> GetKeys();
  void Clear();
  std::string AsString();
  void FromString(const std::string &source);

private:
  std::unordered_map<std::string, std::any> values_;
  void FromString(const std::string &source, size_t &offset);
};

size_t SerializeJsonObject(JsonObject &object, std::ostream &stream);
size_t DeserializeJsonObject(JsonObject &object, std::istream &stream);
size_t SerializeJsonArray(JsonArray &array, std::ostream &stream);
size_t DeserializeJsonArray(JsonArray &array, std::istream &stream);

JsonObject RandomDocument();

#endif