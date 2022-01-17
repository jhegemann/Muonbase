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

#include "json.h"

JsonObject::JsonObject() {}

JsonObject::JsonObject(const JsonObject &object) { Parse(object.String()); }

JsonObject::JsonObject(const std::string &source) { Parse(source); }

JsonObject::~JsonObject() {}

bool JsonObject::Has(const std::string &key) {
  if (values_.find(key) != values_.end()) {
    return true;
  }
  return false;
}

void JsonObject::PutNull(const std::string &key) {
  std::any null;
  null.reset();
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

JsonBoolean JsonObject::GetBoolean(const std::string &key) const {
  return std::any_cast<JsonBoolean>(values_.at(key));
}

JsonInteger JsonObject::GetAsInteger(const std::string &key) const {
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
  try {
    JsonBoolean value __attribute__((unused));
    value = std::any_cast<JsonBoolean>(values_.at(key));
    return true;
  } catch (std::bad_any_cast &exception) {
    return false;
  }
}

bool JsonObject::IsInteger(const std::string &key) const {
  try {
    JsonInteger value __attribute__((unused));
    value = std::any_cast<JsonInteger>(values_.at(key));
    return true;
  } catch (std::bad_any_cast &exception) {
    return false;
  }
}

bool JsonObject::IsFloat(const std::string &key) const {
  try {
    JsonFloat value __attribute__((unused));
    value = std::any_cast<JsonFloat>(values_.at(key));
    return true;
  } catch (std::bad_any_cast &exception) {
    return false;
  }
}

bool JsonObject::IsString(const std::string &key) const {
  try {
    JsonString value __attribute__((unused));
    value = std::any_cast<JsonString>(values_.at(key));
    return true;
  } catch (std::bad_any_cast &exception) {
    return false;
  }
}

bool JsonObject::IsObject(const std::string &key) const {
  try {
    JsonObject value __attribute__((unused));
    value = std::any_cast<JsonObject>(values_.at(key));
    return true;
  } catch (std::bad_any_cast &exception) {
    return false;
  }
}

bool JsonObject::IsArray(const std::string &key) const {
  try {
    JsonArray value __attribute__((unused));
    value = std::any_cast<JsonArray>(values_.at(key));
    return true;
  } catch (std::bad_any_cast &exception) {
    return false;
  }
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
  std::any value;
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

JsonArray::JsonArray(const JsonArray &array) { Parse(array.String()); }

JsonArray::JsonArray(const std::string &source) { Parse(source); }

JsonArray::~JsonArray() {}

size_t JsonArray::Size() const { return values_.size(); }

void JsonArray::PutNull() {
  std::any null;
  null.reset();
  values_.emplace_back(null);
}

void JsonArray::PutBoolean(JsonBoolean value) { values_.emplace_back(value); }

void JsonArray::PutInteger(JsonInteger value) { values_.emplace_back(value); }

void JsonArray::PutFloat(JsonFloat value) { values_.emplace_back(value); }

void JsonArray::PutString(JsonString value) { values_.emplace_back(value); }

void JsonArray::PutObject(JsonObject value) { values_.emplace_back(value); }

void JsonArray::PutArray(JsonArray value) { values_.emplace_back(value); }

JsonBoolean JsonArray::GetBoolean(size_t index) const {
  return std::any_cast<JsonBoolean>(values_[index]);
}

JsonInteger JsonArray::GetAsInteger(size_t index) const {
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
  try {
    JsonBoolean value __attribute__((unused));
    value = std::any_cast<JsonBoolean>(values_[index]);
    return true;
  } catch (std::bad_any_cast &expection) {
    return false;
  }
}

bool JsonArray::IsInteger(size_t index) const {
  try {
    JsonInteger value __attribute__((unused));
    value = std::any_cast<JsonInteger>(values_[index]);
    return true;
  } catch (std::bad_any_cast &exception) {
    return false;
  }
}

bool JsonArray::IsFloat(size_t index) const {
  try {
    JsonFloat value __attribute__((unused));
    value = std::any_cast<JsonFloat>(values_[index]);
    return true;
  } catch (std::bad_any_cast &exception) {
    return false;
  }
}

bool JsonArray::IsString(size_t index) const {
  try {
    JsonString value __attribute__((unused));
    value = std::any_cast<JsonString>(values_[index]);
    return true;
  } catch (std::bad_any_cast &exception) {
    return false;
  }
}

bool JsonArray::IsObject(size_t index) const {
  try {
    JsonObject value __attribute__((unused));
    value = std::any_cast<JsonObject>(values_[index]);
    return true;
  } catch (std::bad_any_cast &exception) {
    return false;
  }
}

bool JsonArray::IsArray(size_t index) const {
  try {
    JsonArray value __attribute__((unused));
    value = std::any_cast<JsonArray>(values_[index]);
    return true;
  } catch (std::bad_any_cast &exception) {
    return false;
  }
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

size_t SerializeJsonObject(JsonObject &object, std::ostream &stream) {
  size_t bytes = 0;
  static const uint8_t kNull = 0;
  static const uint8_t kBoolean = 1;
  static const uint8_t kInteger = 2;
  static const uint8_t kFloat = 3;
  static const uint8_t kString = 4;
  static const uint8_t kObject = 5;
  static const uint8_t kArray = 6;
  static const JsonBoolean kFalse = false;
  static const JsonBoolean kTrue = true;
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
      stream.write((const char *)&kNull, sizeof(uint8_t));
      bytes += sizeof(uint8_t);
    } else if (object.IsBoolean(keys[i])) {
      stream.write((const char *)&kBoolean, sizeof(uint8_t));
      if (object.GetBoolean(keys[i])) {
        stream.write((const char *)&kTrue, sizeof(JsonBoolean));
      } else {
        stream.write((const char *)&kFalse, sizeof(JsonBoolean));
      }
      bytes += sizeof(uint8_t) + sizeof(JsonBoolean);
    } else if (object.IsInteger(keys[i])) {
      stream.write((const char *)&kInteger, sizeof(uint8_t));
      JsonInteger value = object.GetAsInteger(keys[i]);
      stream.write((const char *)&value, sizeof(JsonInteger));
      bytes += sizeof(uint8_t) + sizeof(JsonInteger);
    } else if (object.IsFloat(keys[i])) {
      stream.write((const char *)&kFloat, sizeof(uint8_t));
      JsonFloat value = object.GetFloat(keys[i]);
      stream.write((const char *)&value, sizeof(JsonFloat));
      bytes += sizeof(uint8_t) + sizeof(JsonFloat);
    } else if (object.IsString(keys[i])) {
      stream.write((const char *)&kString, sizeof(uint8_t));
      JsonString value = object.GetString(keys[i]);
      length = value.length();
      stream.write((const char *)&length, sizeof(size_t));
      stream.write((const char *)&value[0], length);
      bytes += sizeof(uint8_t) + sizeof(size_t) + length;
    } else if (object.IsObject(keys[i])) {
      stream.write((const char *)&kObject, sizeof(uint8_t));
      JsonObject value = object.GetObject(keys[i]);
      bytes += sizeof(uint8_t) + SerializeJsonObject(value, stream);
    } else if (object.IsArray(keys[i])) {
      stream.write((const char *)&kArray, sizeof(uint8_t));
      JsonArray value = object.GetArray(keys[i]);
      bytes += sizeof(uint8_t) + SerializeJsonArray(value, stream);
    } else {
      throw std::runtime_error("incompatible json type");
    }
  }
  return stream ? bytes : std::string::npos;
}

size_t DeserializeJsonObject(JsonObject &object, std::istream &stream) {
  object.Clear();
  size_t bytes = 0;
  static const uint8_t kNull = 0;
  static const uint8_t kBoolean = 1;
  static const uint8_t kInteger = 2;
  static const uint8_t kFloat = 3;
  static const uint8_t kString = 4;
  static const uint8_t kObject = 5;
  static const uint8_t kArray = 6;
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
    if (type_id == kNull) {
      object.PutNull(key);
    } else if (type_id == kBoolean) {
      JsonBoolean value;
      stream.read((char *)&value, sizeof(JsonBoolean));
      object.PutBoolean(key, value);
      bytes += sizeof(JsonBoolean);
    } else if (type_id == kInteger) {
      JsonInteger value;
      stream.read((char *)&value, sizeof(JsonInteger));
      object.PutInteger(key, value);
      bytes += sizeof(JsonInteger);
    } else if (type_id == kFloat) {
      JsonFloat value;
      stream.read((char *)&value, sizeof(JsonFloat));
      object.PutFloat(key, value);
      bytes += sizeof(JsonFloat);
    } else if (type_id == kString) {
      JsonString value;
      stream.read((char *)&length, sizeof(size_t));
      value.resize(length);
      stream.read((char *)&value[0], length);
      object.PutString(key, value);
      bytes += sizeof(size_t) + length;
    } else if (type_id == kObject) {
      JsonObject value;
      bytes += DeserializeJsonObject(value, stream);
      object.PutObject(key, value);
    } else if (type_id == kArray) {
      JsonArray value;
      bytes += DeserializeJsonArray(value, stream);
      object.PutArray(key, value);
    } else {
      throw std::runtime_error("incompatible json type");
    }
  }
  return stream ? bytes : std::string::npos;
}

size_t SerializeJsonArray(JsonArray &array, std::ostream &stream) {
  size_t bytes = 0;
  static const uint8_t kNull = 0;
  static const uint8_t kBoolean = 1;
  static const uint8_t kInteger = 2;
  static const uint8_t kFloat = 3;
  static const uint8_t kString = 4;
  static const uint8_t kObject = 5;
  static const uint8_t kArray = 6;
  static const JsonBoolean kFalse = 0;
  static const JsonBoolean kTrue = 1;
  size_t size = array.Size();
  stream.write((const char *)&size, sizeof(size_t));
  bytes += sizeof(size_t);
  for (size_t i = 0; i < size; i++) {
    if (array.IsNull(i)) {
      stream.write((const char *)&kNull, sizeof(uint8_t));
      bytes += sizeof(uint8_t);
    } else if (array.IsBoolean(i)) {
      stream.write((const char *)&kBoolean, sizeof(uint8_t));
      if (array.GetBoolean(i)) {
        stream.write((const char *)&kTrue, sizeof(JsonBoolean));
      } else {
        stream.write((const char *)&kFalse, sizeof(JsonBoolean));
      }
      bytes += sizeof(uint8_t) + sizeof(JsonBoolean);
    } else if (array.IsInteger(i)) {
      stream.write((const char *)&kInteger, sizeof(uint8_t));
      JsonInteger value = array.GetAsInteger(i);
      stream.write((const char *)&value, sizeof(JsonInteger));
      bytes += sizeof(uint8_t) + sizeof(JsonInteger);
    } else if (array.IsFloat(i)) {
      stream.write((const char *)&kFloat, sizeof(uint8_t));
      JsonFloat value = array.GetFloat(i);
      stream.write((const char *)&value, sizeof(JsonFloat));
      bytes += sizeof(uint8_t) + sizeof(JsonFloat);
    } else if (array.IsString(i)) {
      stream.write((const char *)&kString, sizeof(uint8_t));
      JsonString value = array.GetString(i);
      size_t length = value.length();
      stream.write((const char *)&length, sizeof(size_t));
      stream.write((const char *)&value[0], length);
      bytes += sizeof(uint8_t) + sizeof(size_t) + length;
    } else if (array.IsObject(i)) {
      stream.write((const char *)&kObject, sizeof(uint8_t));
      JsonObject value = array.GetObject(i);
      bytes += sizeof(uint8_t) + SerializeJsonObject(value, stream);
    } else if (array.IsArray(i)) {
      stream.write((const char *)kArray, sizeof(uint8_t));
      JsonArray value = array.GetArray(i);
      bytes += sizeof(uint8_t) + SerializeJsonArray(value, stream);
    } else {
      throw std::runtime_error("incompatible json type");
    }
  }
  return stream ? bytes : std::string::npos;
}

size_t DeserializeJsonArray(JsonArray &array, std::istream &stream) {
  array.Clear();
  size_t bytes = 0;
  static const uint8_t kNull = 0;
  static const uint8_t kBoolean = 1;
  static const uint8_t kInteger = 2;
  static const uint8_t kFloat = 3;
  static const uint8_t kString = 4;
  static const uint8_t kObject = 5;
  static const uint8_t kArray = 6;
  size_t size;
  stream.read((char *)&size, sizeof(size_t));
  bytes += sizeof(size_t);
  for (size_t i = 0; i < size; i++) {
    uint8_t type_id;
    stream.read((char *)&type_id, sizeof(uint8_t));
    bytes += sizeof(uint8_t);
    if (type_id == kNull) {
      array.PutNull();
    } else if (type_id == kBoolean) {
      JsonBoolean value;
      stream.read((char *)&value, sizeof(JsonBoolean));
      array.PutBoolean(value);
      bytes += sizeof(JsonBoolean);
    } else if (type_id == kInteger) {
      JsonInteger value;
      stream.read((char *)&value, sizeof(JsonInteger));
      array.PutInteger(value);
      bytes += sizeof(JsonInteger);
    } else if (type_id == kFloat) {
      JsonFloat value;
      stream.read((char *)&value, sizeof(JsonFloat));
      array.PutFloat(value);
      bytes += sizeof(JsonFloat);
    } else if (type_id == kString) {
      JsonString value;
      size_t length;
      stream.read((char *)&length, sizeof(size_t));
      value.resize(length);
      stream.read((char *)&value[0], length);
      array.PutString(value);
      bytes += sizeof(size_t) + length;
    } else if (type_id == kObject) {
      JsonObject value;
      bytes += DeserializeJsonObject(value, stream);
      array.PutObject(value);
    } else if (type_id == kArray) {
      JsonArray value;
      bytes += DeserializeJsonArray(value, stream);
      array.PutArray(value);
    } else {
      throw std::runtime_error("incompatible json type");
    }
  }
  return stream ? bytes : std::string::npos;
}

JsonObject RandomDocument() {
  static RandomGenerator random((uint64_t)time(nullptr));
  const std::vector<std::string> keys = {
      random.Uuid(), random.Uuid(), random.Uuid(), random.Uuid(),
      random.Uuid(), random.Uuid(), random.Uuid(), random.Uuid(),
      random.Uuid(), random.Uuid(), random.Uuid(), random.Uuid()};
  JsonObject result;
  size_t n = 0;
  result.PutBoolean(keys[n++], random.Uniform() > 0.5 ? true : false);
  result.PutFloat(keys[n++], random.Uniform());
  result.PutInteger(keys[n++], random.Uint64() % 1048576);
  result.PutString(keys[n++], random.Uuid());
  result.PutNull(keys[n++]);
  JsonObject object;
  object.PutBoolean(keys[n++], random.Uniform() > 0.5 ? true : false);
  object.PutFloat(keys[n++], random.Uniform());
  object.PutInteger(keys[n++], random.Uint64() % 1048576);
  object.PutString(keys[n++], random.Uuid());
  object.PutNull(keys[n++]);
  result.PutObject(keys[n++], object);
  JsonArray array;
  array.PutBoolean(random.Uniform() > 0.5 ? true : false);
  array.PutFloat(random.Uniform());
  array.PutInteger(random.Uint64() % 1048576);
  array.PutString(random.Uuid());
  array.PutNull();
  result.PutArray(keys[n++], array);
  return result;
}

JsonArray RandomDocumentArray() {
  static RandomGenerator random((uint64_t)time(nullptr));
  JsonArray array;
  for (size_t i = 0; i < 1 + random.Uint64() % 10; i++) {
    array.PutObject(RandomDocument());
  }
  return array;
}