/* Copyright 2022 Jonas Hegemann 26 Oct 1988

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#ifndef JSON_H
#define JSON_H

#include <algorithm>
#include <any>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <map>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

#include "rand.h"
#include "utils.h"

const std::string kJsonNull = "null";
const std::string kJsonFalse = "false";
const std::string kJsonTrue = "true";
const size_t kJsonNullLength = 4;
const size_t kJsonFalseLength = 5;
const size_t kJsonTrueLength = 4;

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
  JsonArray(const JsonArray &array);
  JsonArray(const std::string &source);
  virtual ~JsonArray();
  size_t Size() const;
  void PutNull();
  void PutBoolean(JsonBoolean value);
  void PutInteger(JsonInteger value);
  void PutFloat(JsonFloat value);
  void PutString(JsonString value);
  void PutObject(JsonObject value);
  void PutArray(JsonArray value);
  JsonBoolean GetBoolean(size_t index) const;
  JsonInteger GetInteger(size_t index) const;
  JsonFloat GetFloat(size_t index) const;
  JsonString GetString(size_t index) const;
  JsonObject GetObject(size_t index) const;
  JsonArray GetArray(size_t index) const;
  bool IsNull(size_t index) const;
  bool IsBoolean(size_t index) const;
  bool IsInteger(size_t index) const;
  bool IsFloat(size_t index) const;
  bool IsString(size_t index) const;
  bool IsObject(size_t index) const;
  bool IsArray(size_t index) const;
  void Clear();
  std::string String() const;
  void Parse(const std::string &source);

private:
  std::vector<std::any> values_;
  void Parse(const std::string &source, size_t &source_offset);
};

class JsonObject {
public:
  friend class JsonArray;
  JsonObject();
  JsonObject(const JsonObject &object);
  JsonObject(const std::string &source);
  virtual ~JsonObject();
  bool Has(const std::string &key) const;
  void PutNull(const std::string &key);
  void PutBoolean(const std::string &key, JsonBoolean value);
  void PutInteger(const std::string &key, JsonInteger value);
  void PutFloat(const std::string &key, JsonFloat value);
  void PutString(const std::string &key, JsonString value);
  void PutObject(const std::string &key, JsonObject value);
  void PutArray(const std::string &key, JsonArray value);
  JsonBoolean GetBoolean(const std::string &key) const;
  JsonInteger GetInteger(const std::string &key) const;
  JsonFloat GetFloat(const std::string &key) const;
  JsonString GetString(const std::string &key) const;
  JsonObject GetObject(const std::string &key) const;
  JsonArray GetArray(const std::string &key) const;
  bool IsNull(const std::string &key) const;
  bool IsBoolean(const std::string &key) const;
  bool IsInteger(const std::string &key) const;
  bool IsFloat(const std::string &key) const;
  bool IsString(const std::string &key) const;
  bool IsObject(const std::string &key) const;
  bool IsArray(const std::string &key) const;
  std::vector<std::string> Keys() const;
  void Clear();
  std::string String() const;
  void Parse(const std::string &source);

private:
  std::unordered_map<std::string, std::any> values_;
  void Parse(const std::string &source, size_t &source_offset);
};

size_t SerializeJsonObject(const JsonObject &object, std::ostream &stream);
size_t DeserializeJsonObject(JsonObject &object, std::istream &stream);
size_t SerializeJsonArray(const JsonArray &object, std::ostream &stream);
size_t DeserializeJsonArray(JsonArray &object, std::istream &stream);

uint64_t MemoryJsonObject(const JsonObject &object);
uint64_t MemoryJsonArray(const JsonArray &object);

JsonObject RandomDocument();
JsonArray RandomDocumentArray();

#endif