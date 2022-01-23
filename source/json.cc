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

#include "json.h"

namespace json {

bool IsArray(const JsonValue &value) {
  return value.type() == typeid(JsonArray);
}

bool IsObject(const JsonValue &value) {
  return value.type() == typeid(JsonObject);
}

bool IsBoolean(const JsonValue &value) {
  return value.type() == typeid(JsonBoolean);
}

bool IsInteger(const JsonValue &value) {
  return value.type() == typeid(JsonInteger);
}

bool IsFloat(const JsonValue &value) {
  return value.type() == typeid(JsonFloat);
}

bool IsString(const JsonValue &value) {
  return value.type() == typeid(JsonString);
}

} // namespace json

JsonObject::JsonObject() {}

JsonObject::JsonObject(const JsonObject &object) { values_ = object.values_; }

JsonObject::JsonObject(const std::string &source) { Parse(source); }

JsonObject::~JsonObject() {}

bool JsonObject::Has(const std::string &key) const {
  return values_.find(key) != values_.end();
}

void JsonObject::PutNull(const std::string &key) {
  static const JsonValue null = {};
  values_.insert(std::make_pair(key, null));
}

void JsonObject::PutBoolean(const std::string &key, JsonBoolean value) {
  values_.insert(std::make_pair(key, value));
}

void JsonObject::PutInteger(const std::string &key, JsonInteger value) {
  values_.insert(std::make_pair(key, value));
}

void JsonObject::PutFloat(const std::string &key, JsonFloat value) {
  values_.insert(std::make_pair(key, value));
}

void JsonObject::PutString(const std::string &key, JsonString value) {
  values_.insert(std::make_pair(key, value));
}

void JsonObject::PutObject(const std::string &key, JsonObject value) {
  values_.insert(std::make_pair(key, value));
}

void JsonObject::PutArray(const std::string &key, JsonArray value) {
  values_.insert(std::make_pair(key, value));
}

JsonValue JsonObject::GetValue(const std::string &key) const {
  return values_.at(key);
}

JsonBoolean JsonObject::GetBoolean(const std::string &key) const {
  return std::any_cast<JsonBoolean>(values_.at(key));
}

JsonInteger JsonObject::GetInteger(const std::string &key) const {
  return std::any_cast<JsonInteger>(values_.at(key));
}

JsonFloat JsonObject::GetFloat(const std::string &key) const {
  return std::any_cast<JsonFloat>(values_.at(key));
}

JsonString JsonObject::GetString(const std::string &key) const {
  return std::any_cast<JsonString>(values_.at(key));
}

JsonObject JsonObject::GetObject(const std::string &key) const {
  return std::any_cast<JsonObject>(values_.at(key));
}

JsonArray JsonObject::GetArray(const std::string &key) const {
  return std::any_cast<JsonArray>(values_.at(key));
}

bool JsonObject::IsNull(const std::string &key) const {
  return !values_.at(key).has_value();
}

bool JsonObject::IsBoolean(const std::string &key) const {
  return json::IsBoolean(values_.at(key));
}

bool JsonObject::IsInteger(const std::string &key) const {
  return json::IsInteger(values_.at(key));
}

bool JsonObject::IsFloat(const std::string &key) const {
  return json::IsFloat(values_.at(key));
}

bool JsonObject::IsString(const std::string &key) const {
  return json::IsString(values_.at(key));
}

bool JsonObject::IsObject(const std::string &key) const {
  return json::IsObject(values_.at(key));
}

bool JsonObject::IsArray(const std::string &key) const {
  return json::IsArray(values_.at(key));
}

std::vector<std::string> JsonObject::Keys() const {
  std::vector<std::string> keys;
  keys.reserve(values_.size());
  for (auto it = values_.begin(); it != values_.end(); it++) {
    keys.emplace_back(it->first);
  }
  return keys;
}

void JsonObject::Clear() { values_.clear(); }

std::string JsonObject::String() const {
  std::stringstream ss;
  std::string sep = kStringEmpty;
  ss << kStringCurlyBracketOpen;
  std::string key;
  JsonValue value;
  for (auto it = values_.begin(); it != values_.end(); it++) {
    key = it->first;
    value = it->second;
    ss << sep;
    ss << kStringDoubleQuote << key << kStringDoubleQuote + kStringColon;
    if (value.type() == typeid(void)) {
      ss << kJsonNull;
    } else if (value.type() == typeid(JsonBoolean)) {
      if (std::any_cast<JsonBoolean>(value)) {
        ss << kJsonTrue;
      } else {
        ss << kJsonFalse;
      }
    } else if (value.type() == typeid(JsonInteger)) {
      ss << std::any_cast<JsonInteger>(value);
    } else if (value.type() == typeid(JsonFloat)) {
      ss << std::fixed << std::any_cast<JsonFloat>(value);
    } else if (value.type() == typeid(JsonString)) {
      ss << kStringDoubleQuote << std::any_cast<JsonString>(value)
         << kStringDoubleQuote;
    } else if (value.type() == typeid(JsonObject)) {
      ss << std::any_cast<JsonObject>(value).String();
    } else if (value.type() == typeid(JsonArray)) {
      ss << std::any_cast<JsonArray>(value).String();
    } else {
      std::cout << ss.str() << std::endl;
      throw std::runtime_error("incompatible json type");
    }
    sep = kStringComma;
  }
  ss << kStringCurlyBracketClose;
  return ss.str();
}

void JsonObject::Parse(const std::string &source) {
  size_t offset = 0;
  Parse(source, offset);
}

void JsonObject::Parse(const std::string &source, size_t &source_offset) {
  values_.clear();
  size_t offset = source_offset;
  size_t position;
  const std::string kValueBorder =
      kStringWss + kStringComma + kStringCurlyBracketClose;
  std::string key;
  std::string value;
  JsonObject object;
  JsonArray array;
  bool dot;
  std::string text;
  char border;
  if (!ExpectString(source, kStringCurlyBracketOpen, offset)) {
    throw std::runtime_error("invalid json object");
  }
  for (;;) {
    if (!ExpectString(source, kStringDoubleQuote, offset)) {
      throw std::runtime_error("invalid json object");
    }
    position = source.find(kStringDoubleQuote, offset);
    if (position == std::string::npos) {
      throw std::runtime_error("invalid json object");
    }
    key = source.substr(offset, position - offset);
    offset = position + 1;
    if (!ExpectString(source, kStringColon, offset)) {
      throw std::runtime_error("invalid json object");
    }
    if (!ExpectString(source, kStringEmpty, offset)) {
      throw std::runtime_error("invalid json object");
    }
    switch (source[offset]) {
    case kCharN:
      if (source.substr(offset, kJsonNullLength).compare(kJsonNull)) {
        throw std::runtime_error("json: parse null value");
      }
      border = source[offset + kJsonNullLength];
      if (!CharIsAnyOf(border, kValueBorder)) {
        throw std::runtime_error("json: parse null value");
      }
      PutNull(key);
      offset += kJsonNullLength;
      break;
    case kCharT:
      if (source.substr(offset, kJsonTrueLength).compare(kJsonTrue)) {
        throw std::runtime_error("json: parse true value");
      }
      border = source[offset + kJsonTrueLength];
      if (!CharIsAnyOf(border, kValueBorder)) {
        throw std::runtime_error("json: parse true value");
      }
      PutBoolean(key, true);
      offset += kJsonTrueLength;
      break;
    case kCharF:
      if (source.substr(offset, kJsonFalseLength).compare(kJsonFalse)) {
        throw std::runtime_error("json: parse false value");
      }
      border = source[offset + kJsonFalseLength];
      if (!CharIsAnyOf(border, kValueBorder)) {
        throw std::runtime_error("json: parse false value");
      }
      PutBoolean(key, false);
      offset += kJsonFalseLength;
      break;
    case kCharDoubleQuote:
      position = source.find(kStringDoubleQuote, offset + 1);
      if (position == std::string::npos) {
        throw std::runtime_error("json: parse string value");
      }
      PutString(key, source.substr(offset + 1, position - offset - 1));
      offset = position + 1;
      break;
    case kCharZero:
      [[fallthrough]];
    case kCharOne:
      [[fallthrough]];
    case kCharTwo:
      [[fallthrough]];
    case kCharThree:
      [[fallthrough]];
    case kCharFour:
      [[fallthrough]];
    case kCharFive:
      [[fallthrough]];
    case kCharSix:
      [[fallthrough]];
    case kCharSeven:
      [[fallthrough]];
    case kCharEight:
      [[fallthrough]];
    case kCharNine:
      [[fallthrough]];
    case kCharPlus:
      [[fallthrough]];
    case kCharMinus:
      position = offset + 1;
      dot = false;
      while (position < source.length()) {
        if (CharIsAnyOf(source[position], kValueBorder)) {
          break;
        }
        if (source[position] == kCharDot) {
          dot = true;
        }
        position++;
      }
      if (position == source.length()) {
        throw std::runtime_error("json: parse number value");
      }
      text = source.substr(offset, position - offset);
      try {
        if (dot) {
          PutFloat(key, std::atof(text.c_str()));
        } else {
          PutInteger(key, std::atoi(text.c_str()));
        }
      } catch (std::invalid_argument &) {
        throw std::runtime_error("json: parse number value");
      }
      offset = position;
      break;
    case kCharCurlyBracketOpen:
      object.Parse(source, offset);
      PutObject(key, object);
      break;
    case kCharSquareBracketOpen:
      array.Parse(source, offset);
      PutArray(key, array);
      break;
    default:
      throw std::runtime_error("json: invalid value");
    }
    if (!ExpectString(source, kStringEmpty, offset)) {
      throw std::runtime_error("invalid json object");
    }
    if (source[offset] == kCharComma) {
      offset++;
      continue;
    } else if (source[offset] == kCharCurlyBracketClose) {
      offset++;
      break;
    } else {
      throw std::runtime_error("invalid json object");
    }
  }
  source_offset = offset;
}

JsonArray::JsonArray() {}

JsonArray::JsonArray(const JsonArray &array) { values_ = array.values_; }

JsonArray::JsonArray(const std::string &source) { Parse(source); }

JsonArray::~JsonArray() {}

size_t JsonArray::Size() const { return values_.size(); }

void JsonArray::PutNull() {
  static const JsonValue null = {};
  values_.emplace_back(null);
}

void JsonArray::PutBoolean(JsonBoolean value) { values_.emplace_back(value); }

void JsonArray::PutInteger(JsonInteger value) { values_.emplace_back(value); }

void JsonArray::PutFloat(JsonFloat value) { values_.emplace_back(value); }

void JsonArray::PutString(JsonString value) { values_.emplace_back(value); }

void JsonArray::PutObject(JsonObject value) { values_.emplace_back(value); }

void JsonArray::PutArray(JsonArray value) { values_.emplace_back(value); }

JsonValue JsonArray::GetValue(size_t index) const { return values_[index]; }

JsonBoolean JsonArray::GetBoolean(size_t index) const {
  return std::any_cast<JsonBoolean>(values_[index]);
}

JsonInteger JsonArray::GetInteger(size_t index) const {
  return std::any_cast<JsonInteger>(values_[index]);
}

JsonFloat JsonArray::GetFloat(size_t index) const {
  return std::any_cast<JsonFloat>(values_[index]);
}

JsonString JsonArray::GetString(size_t index) const {
  return std::any_cast<JsonString>(values_[index]);
}

JsonObject JsonArray::GetObject(size_t index) const {
  return std::any_cast<JsonObject>(values_[index]);
}

JsonArray JsonArray::GetArray(size_t index) const {
  return std::any_cast<JsonArray>(values_[index]);
}

bool JsonArray::IsNull(size_t index) const {
  return !values_[index].has_value();
}

bool JsonArray::IsBoolean(size_t index) const {
  return json::IsBoolean(values_[index]);
}

bool JsonArray::IsInteger(size_t index) const {
  return json::IsInteger(values_[index]);
}

bool JsonArray::IsFloat(size_t index) const {
  return json::IsFloat(values_[index]);
}

bool JsonArray::IsString(size_t index) const {
  return json::IsString(values_[index]);
}

bool JsonArray::IsObject(size_t index) const {
  return json::IsObject(values_[index]);
}

bool JsonArray::IsArray(size_t index) const {
  return json::IsArray(values_[index]);
}

void JsonArray::Clear() { values_.clear(); }

std::string JsonArray::String() const {
  std::stringstream ss;
  std::string sep = kStringEmpty;
  ss << kStringSquareBracketOpen;
  for (auto it = values_.begin(); it != values_.end(); it++) {
    ss << sep;
    if (it->type() == typeid(void)) {
      ss << kJsonNull;
    } else if (it->type() == typeid(JsonBoolean)) {
      if (std::any_cast<JsonBoolean>(*it)) {
        ss << kJsonTrue;
      } else {
        ss << kJsonFalse;
      }
    } else if (it->type() == typeid(JsonInteger)) {
      ss << std::any_cast<JsonInteger>(*it);
    } else if (it->type() == typeid(JsonFloat)) {
      ss << std::fixed << std::any_cast<JsonFloat>(*it);
    } else if (it->type() == typeid(JsonString)) {
      ss << kStringDoubleQuote << std::any_cast<JsonString>(*it)
         << kStringDoubleQuote;
    } else if (it->type() == typeid(JsonObject)) {
      ss << std::any_cast<JsonObject>(*it).String();
    } else if (it->type() == typeid(JsonArray)) {
      ss << std::any_cast<JsonArray>(*it).String();
    } else {
      throw std::runtime_error("incompatible json type");
    }
    sep = kStringComma;
  }
  ss << kStringSquareBracketClose;
  return ss.str();
}

void JsonArray::Parse(const std::string &source) {
  size_t offset = 0;
  Parse(source, offset);
}

void JsonArray::Parse(const std::string &source, size_t &source_offset) {
  values_.clear();
  const std::string kValueBorder =
      kStringWss + kStringComma + kStringSquareBracketClose;
  size_t offset = source_offset;
  size_t position;
  std::string value;
  JsonObject object;
  JsonArray array;
  char border;
  bool dot;
  std::string text;
  if (!ExpectString(source, kStringSquareBracketOpen, offset)) {
    throw std::runtime_error("invalid json object:");
  }
  for (;;) {
    if (!ExpectString(source, kStringEmpty, offset)) {
      throw std::runtime_error("invalid json object");
    }
    switch (source[offset]) {
    case kCharN:
      if (source.substr(offset, kJsonNullLength).compare(kJsonNull)) {
        throw std::runtime_error("json: parse null value");
      }
      border = source[offset + kJsonNullLength];
      if (!CharIsAnyOf(border, kValueBorder)) {
        throw std::runtime_error("json: parse null value");
      }
      PutNull();
      offset += kJsonNullLength;
      break;
    case kCharT:
      if (source.substr(offset, kJsonTrueLength).compare(kJsonTrue)) {
        throw std::runtime_error("json: parse true value");
      }
      border = source[offset + kJsonTrueLength];
      if (!CharIsAnyOf(border, kValueBorder)) {
        throw std::runtime_error("json: parse true value");
      }
      PutBoolean(true);
      offset += kJsonTrueLength;
      break;
    case kCharF:
      if (source.substr(offset, kJsonFalseLength).compare(kJsonFalse)) {
        throw std::runtime_error("json: parse false value");
      }
      border = source[offset + kJsonFalseLength];
      if (!CharIsAnyOf(border, kValueBorder)) {
        throw std::runtime_error("json: parse false value");
      }
      PutBoolean(false);
      offset += kJsonFalseLength;
      break;
    case kCharDoubleQuote:
      position = source.find(kStringDoubleQuote, offset + 1);
      if (position == std::string::npos) {
        throw std::runtime_error("json: parse string value");
      }
      PutString(source.substr(offset + 1, position - offset - 1));
      offset = position + 1;
      break;
    case kCharZero:
      [[fallthrough]];
    case kCharOne:
      [[fallthrough]];
    case kCharTwo:
      [[fallthrough]];
    case kCharThree:
      [[fallthrough]];
    case kCharFour:
      [[fallthrough]];
    case kCharFive:
      [[fallthrough]];
    case kCharSix:
      [[fallthrough]];
    case kCharSeven:
      [[fallthrough]];
    case kCharEight:
      [[fallthrough]];
    case kCharNine:
      [[fallthrough]];
    case kCharPlus:
      [[fallthrough]];
    case kCharMinus:
      position = offset + 1;
      dot = false;
      while (position < source.length()) {
        if (CharIsAnyOf(source[position], kValueBorder)) {
          break;
        }
        if (source[position] == kCharDot) {
          dot = true;
        }
        position++;
      }
      if (position == source.length()) {
        throw std::runtime_error("json: parse number value");
      }
      text = source.substr(offset, position - offset);
      try {
        if (dot) {
          PutFloat(std::atof(text.c_str()));
        } else {
          PutInteger(std::atoi(text.c_str()));
        }
      } catch (std::invalid_argument &) {
        throw std::runtime_error("json: parse number value");
      }
      offset = position;
      break;
    case kCharCurlyBracketOpen:
      object.Parse(source, offset);
      PutObject(object);
      break;
    case kCharSquareBracketOpen:
      array.Parse(source, offset);
      PutArray(array);
      break;
    default:
      throw std::runtime_error("json: invalid value");
    }
    if (!ExpectString(source, kStringEmpty, offset)) {
      throw std::runtime_error("invalid json object");
    }
    if (source[offset] == kCharComma) {
      offset++;
      continue;
    } else if (source[offset] == kCharSquareBracketClose) {
      offset++;
      break;
    } else {
      throw std::runtime_error("invalid json object");
    }
  }
  source_offset = offset;
}

namespace json {

size_t Serialize(const JsonObject &object, std::ostream &stream) {
  size_t bytes = 0;
  std::vector<std::string> keys = object.Keys();
  size_t size = keys.size();
  stream.write((const char *)&size, sizeof(size_t));
  bytes += sizeof(size_t);
  size_t length;
  for (size_t i = 0; i < keys.size(); i++) {
    length = keys[i].length();
    stream.write((const char *)&length, sizeof(size_t));
    stream.write((const char *)&keys[i][0], length);
    bytes += sizeof(size_t) + length;
    if (object.IsNull(keys[i])) {
      stream.write((const char *)&kJsonTypeNull, sizeof(uint8_t));
      bytes += sizeof(uint8_t);
    } else if (object.IsBoolean(keys[i])) {
      stream.write((const char *)&kJsonTypeBoolean, sizeof(uint8_t));
      if (object.GetBoolean(keys[i])) {
        stream.write((const char *)&kJsonTrue, sizeof(JsonBoolean));
      } else {
        stream.write((const char *)&kJsonFalse, sizeof(JsonBoolean));
      }
      bytes += sizeof(uint8_t) + sizeof(JsonBoolean);
    } else if (object.IsInteger(keys[i])) {
      stream.write((const char *)&kJsonTypeInteger, sizeof(uint8_t));
      JsonInteger value = object.GetInteger(keys[i]);
      stream.write((const char *)&value, sizeof(JsonInteger));
      bytes += sizeof(uint8_t) + sizeof(JsonInteger);
    } else if (object.IsFloat(keys[i])) {
      stream.write((const char *)&kJsonTypeFloat, sizeof(uint8_t));
      JsonFloat value = object.GetFloat(keys[i]);
      stream.write((const char *)&value, sizeof(JsonFloat));
      bytes += sizeof(uint8_t) + sizeof(JsonFloat);
    } else if (object.IsString(keys[i])) {
      stream.write((const char *)&kJsonTypeString, sizeof(uint8_t));
      JsonString value = object.GetString(keys[i]);
      length = value.length();
      stream.write((const char *)&length, sizeof(size_t));
      stream.write((const char *)&value[0], length);
      bytes += sizeof(uint8_t) + sizeof(size_t) + length;
    } else if (object.IsObject(keys[i])) {
      stream.write((const char *)&kJsonTypeObject, sizeof(uint8_t));
      JsonObject value = object.GetObject(keys[i]);
      bytes += sizeof(uint8_t) + json::Serialize(value, stream);
    } else if (object.IsArray(keys[i])) {
      stream.write((const char *)&kJsonTypeArray, sizeof(uint8_t));
      JsonArray value = object.GetArray(keys[i]);
      bytes += sizeof(uint8_t) + json::Serialize(value, stream);
    } else {
      throw std::runtime_error("incompatible json type");
    }
  }
  return stream ? bytes : std::string::npos;
}

size_t Deserialize(JsonObject &object, std::istream &stream) {
  object.Clear();
  size_t bytes = 0;
  size_t size;
  stream.read((char *)&size, sizeof(size_t));
  size_t length;
  bytes += sizeof(size_t);
  std::string key;
  for (size_t i = 0; i < size; i++) {
    stream.read((char *)&length, sizeof(size_t));
    key.resize(length);
    stream.read((char *)&key[0], length);
    bytes += sizeof(size_t) + length;
    uint8_t type_id;
    stream.read((char *)&type_id, sizeof(uint8_t));
    bytes += sizeof(uint8_t);
    if (type_id == kJsonTypeNull) {
      object.PutNull(key);
    } else if (type_id == kJsonTypeBoolean) {
      JsonBoolean value;
      stream.read((char *)&value, sizeof(JsonBoolean));
      object.PutBoolean(key, value);
      bytes += sizeof(JsonBoolean);
    } else if (type_id == kJsonTypeInteger) {
      JsonInteger value;
      stream.read((char *)&value, sizeof(JsonInteger));
      object.PutInteger(key, value);
      bytes += sizeof(JsonInteger);
    } else if (type_id == kJsonTypeFloat) {
      JsonFloat value;
      stream.read((char *)&value, sizeof(JsonFloat));
      object.PutFloat(key, value);
      bytes += sizeof(JsonFloat);
    } else if (type_id == kJsonTypeString) {
      JsonString value;
      stream.read((char *)&length, sizeof(size_t));
      value.resize(length);
      stream.read((char *)&value[0], length);
      object.PutString(key, value);
      bytes += sizeof(size_t) + length;
    } else if (type_id == kJsonTypeObject) {
      JsonObject value;
      bytes += json::Deserialize(value, stream);
      object.PutObject(key, value);
    } else if (type_id == kJsonTypeArray) {
      JsonArray value;
      bytes += json::Deserialize(value, stream);
      object.PutArray(key, value);
    } else {
      throw std::runtime_error("incompatible json type");
    }
  }
  return stream ? bytes : std::string::npos;
}

size_t Serialize(const JsonArray &object, std::ostream &stream) {
  size_t bytes = 0;
  size_t size = object.Size();
  stream.write((const char *)&size, sizeof(size_t));
  bytes += sizeof(size_t);
  for (size_t i = 0; i < size; i++) {
    if (object.IsNull(i)) {
      stream.write((const char *)&kJsonTypeNull, sizeof(uint8_t));
      bytes += sizeof(uint8_t);
    } else if (object.IsBoolean(i)) {
      stream.write((const char *)&kJsonTypeBoolean, sizeof(uint8_t));
      if (object.GetBoolean(i)) {
        stream.write((const char *)&kJsonTrue, sizeof(JsonBoolean));
      } else {
        stream.write((const char *)&kJsonFalse, sizeof(JsonBoolean));
      }
      bytes += sizeof(uint8_t) + sizeof(JsonBoolean);
    } else if (object.IsInteger(i)) {
      stream.write((const char *)&kJsonTypeInteger, sizeof(uint8_t));
      JsonInteger value = object.GetInteger(i);
      stream.write((const char *)&value, sizeof(JsonInteger));
      bytes += sizeof(uint8_t) + sizeof(JsonInteger);
    } else if (object.IsFloat(i)) {
      stream.write((const char *)&kJsonTypeFloat, sizeof(uint8_t));
      JsonFloat value = object.GetFloat(i);
      stream.write((const char *)&value, sizeof(JsonFloat));
      bytes += sizeof(uint8_t) + sizeof(JsonFloat);
    } else if (object.IsString(i)) {
      stream.write((const char *)&kJsonTypeString, sizeof(uint8_t));
      JsonString value = object.GetString(i);
      size_t length = value.length();
      stream.write((const char *)&length, sizeof(size_t));
      stream.write((const char *)&value[0], length);
      bytes += sizeof(uint8_t) + sizeof(size_t) + length;
    } else if (object.IsObject(i)) {
      stream.write((const char *)&kJsonTypeObject, sizeof(uint8_t));
      JsonObject value = object.GetObject(i);
      bytes += sizeof(uint8_t) + json::Serialize(value, stream);
    } else if (object.IsArray(i)) {
      stream.write((const char *)kJsonTypeArray, sizeof(uint8_t));
      JsonArray value = object.GetArray(i);
      bytes += sizeof(uint8_t) + json::Serialize(value, stream);
    } else {
      throw std::runtime_error("incompatible json type");
    }
  }
  return stream ? bytes : std::string::npos;
}

size_t Deserialize(JsonArray &object, std::istream &stream) {
  object.Clear();
  size_t bytes = 0;
  size_t size;
  stream.read((char *)&size, sizeof(size_t));
  bytes += sizeof(size_t);
  for (size_t i = 0; i < size; i++) {
    uint8_t type_id;
    stream.read((char *)&type_id, sizeof(uint8_t));
    bytes += sizeof(uint8_t);
    if (type_id == kJsonTypeNull) {
      object.PutNull();
    } else if (type_id == kJsonTypeBoolean) {
      JsonBoolean value;
      stream.read((char *)&value, sizeof(JsonBoolean));
      object.PutBoolean(value);
      bytes += sizeof(JsonBoolean);
    } else if (type_id == kJsonTypeInteger) {
      JsonInteger value;
      stream.read((char *)&value, sizeof(JsonInteger));
      object.PutInteger(value);
      bytes += sizeof(JsonInteger);
    } else if (type_id == kJsonTypeFloat) {
      JsonFloat value;
      stream.read((char *)&value, sizeof(JsonFloat));
      object.PutFloat(value);
      bytes += sizeof(JsonFloat);
    } else if (type_id == kJsonTypeString) {
      JsonString value;
      size_t length;
      stream.read((char *)&length, sizeof(size_t));
      value.resize(length);
      stream.read((char *)&value[0], length);
      object.PutString(value);
      bytes += sizeof(size_t) + length;
    } else if (type_id == kJsonTypeObject) {
      JsonObject value;
      bytes += json::Deserialize(value, stream);
      object.PutObject(value);
    } else if (type_id == kJsonTypeArray) {
      JsonArray value;
      bytes += json::Deserialize(value, stream);
      object.PutArray(value);
    } else {
      throw std::runtime_error("incompatible json type");
    }
  }
  return stream ? bytes : std::string::npos;
}

JsonObject RandomObject() {
  static RandomGenerator random((uint64_t)time(nullptr));
  const std::vector<std::string> keys = {
      random.Uuid(), random.Uuid(), random.Uuid(), random.Uuid(),
      random.Uuid(), random.Uuid(), random.Uuid(), random.Uuid(),
      random.Uuid(), random.Uuid(), random.Uuid(), random.Uuid()};
  JsonObject object;
  size_t n = 0;
  object.PutBoolean(keys[n++], random.Uniform() > 0.5 ? true : false);
  object.PutFloat(keys[n++], random.Uniform());
  object.PutInteger(keys[n++], random.Uint64() % 1048576);
  object.PutString(keys[n++], random.Uuid());
  object.PutNull(keys[n++]);
  JsonObject nested;
  nested.PutBoolean(keys[n++], random.Uniform() > 0.5 ? true : false);
  nested.PutFloat(keys[n++], random.Uniform());
  nested.PutInteger(keys[n++], random.Uint64() % 1048576);
  nested.PutString(keys[n++], random.Uuid());
  nested.PutNull(keys[n++]);
  object.PutObject(keys[n++], nested);
  JsonArray array;
  array.PutBoolean(random.Uniform() > 0.5 ? true : false);
  array.PutFloat(random.Uniform());
  array.PutInteger(random.Uint64() % 1048576);
  array.PutString(random.Uuid());
  array.PutNull();
  object.PutArray(keys[n++], array);
  return object;
}

JsonArray RandomObjectArray() {
  static RandomGenerator random((uint64_t)time(nullptr));
  JsonArray array;
  for (size_t i = 0; i < 1 + random.Uint64() % 10; i++) {
    array.PutObject(RandomObject());
  }
  return array;
}

uint64_t Memory(const JsonObject &object) {
  uint64_t result = sizeof(std::unordered_map<std::string, std::any>);
  for (std::string key : object.Keys()) {
    result += sizeof(std::string) + key.length();
    if (object.IsArray(key)) {
      result += json::Memory(object.GetArray(key));
    } else if (object.IsBoolean(key)) {
      result += sizeof(std::any);
    } else if (object.IsFloat(key)) {
      result += sizeof(std::any);
    } else if (object.IsInteger(key)) {
      result += sizeof(std::any);
    } else if (object.IsNull(key)) {
      result += sizeof(std::any);
    } else if (object.IsObject(key)) {
      result += json::Memory(object.GetObject(key));
    } else if (object.IsString(key)) {
      result += sizeof(std::any) + sizeof(std::string) +
                object.GetString(key).capacity();
    } else {
      throw std::runtime_error("json: object memory");
    }
  }
  return result;
}

uint64_t Memory(const JsonArray &object) {
  uint64_t result = sizeof(std::vector<std::any>);
  for (size_t i = 0; i < object.Size(); i++) {
    if (object.IsArray(i)) {
      result += json::Memory(object.GetArray(i));
    } else if (object.IsBoolean(i)) {
      result += sizeof(std::any);
    } else if (object.IsFloat(i)) {
      result += sizeof(std::any);
    } else if (object.IsInteger(i)) {
      result += sizeof(std::any);
    } else if (object.IsNull(i)) {
      result += sizeof(std::any);
    } else if (object.IsObject(i)) {
      result += json::Memory(object.GetObject(i));
    } else if (object.IsString(i)) {
      result += sizeof(std::any) + sizeof(std::string) +
                object.GetString(i).capacity();
    } else {
      throw std::runtime_error("json: array memory");
    }
  }
  return result;
}

} // namespace json