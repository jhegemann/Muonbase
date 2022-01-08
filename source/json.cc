/* Copyright [2022] [Jonas Hegemann]

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
  values_.insert(make_pair(key, null));
}

void JsonObject::PutBoolean(const std::string &key, JsonBoolean value) {
  values_.insert(make_pair(key, value));
}

void JsonObject::PutInteger(const std::string &key, JsonInteger value) {
  values_.insert(make_pair(key, value));
}

void JsonObject::PutFloat(const std::string &key, JsonFloat value) {
  values_.insert(make_pair(key, value));
}

void JsonObject::PutString(const std::string &key, JsonString value) {
  values_.insert(make_pair(key, value));
}

void JsonObject::PutObject(const std::string &key, JsonObject value) {
  values_.insert(make_pair(key, value));
}

void JsonObject::PutArray(const std::string &key, JsonArray value) {
  values_.insert(make_pair(key, value));
}

JsonBoolean JsonObject::GetAsBoolean(const std::string &key) {
  return std::any_cast<JsonBoolean>(values_[key]);
}

JsonInteger JsonObject::GetAsInteger(const std::string &key) {
  return std::any_cast<JsonInteger>(values_[key]);
}

JsonFloat JsonObject::GetAsFloat(const std::string &key) {
  return std::any_cast<JsonFloat>(values_[key]);
}

JsonString JsonObject::GetAsString(const std::string &key) {
  return std::any_cast<JsonString>(values_[key]);
}

JsonObject JsonObject::GetAsObject(const std::string &key) {
  return std::any_cast<JsonObject>(values_[key]);
}

JsonArray JsonObject::GetAsArray(const std::string &key) {
  return std::any_cast<JsonArray>(values_[key]);
}

bool JsonObject::IsNull(const std::string &key) {
  return !values_[key].has_value();
}

bool JsonObject::IsBoolean(const std::string &key) {
  try {
    JsonBoolean value __attribute__((unused));
    value = std::any_cast<JsonBoolean>(values_[key]);
    return true;
  } catch (std::bad_any_cast &exception) {
    return false;
  }
}

bool JsonObject::IsInteger(const std::string &key) {
  try {
    JsonInteger value __attribute__((unused));
    value = std::any_cast<JsonInteger>(values_[key]);
    return true;
  } catch (std::bad_any_cast &exception) {
    return false;
  }
}

bool JsonObject::IsFloat(const std::string &key) {
  try {
    JsonFloat value __attribute__((unused));
    value = std::any_cast<JsonFloat>(values_[key]);
    return true;
  } catch (std::bad_any_cast &exception) {
    return false;
  }
}

bool JsonObject::IsString(const std::string &key) {
  try {
    JsonString value __attribute__((unused));
    value = std::any_cast<JsonString>(values_[key]);
    return true;
  } catch (std::bad_any_cast &exception) {
    return false;
  }
}

bool JsonObject::IsObject(const std::string &key) {
  try {
    JsonObject value __attribute__((unused));
    value = std::any_cast<JsonObject>(values_[key]);
    return true;
  } catch (std::bad_any_cast &exception) {
    return false;
  }
}

bool JsonObject::IsArray(const std::string &key) {
  try {
    JsonArray value __attribute__((unused));
    value = std::any_cast<JsonArray>(values_[key]);
    return true;
  } catch (std::bad_any_cast &exception) {
    return false;
  }
}

std::vector<std::string> JsonObject::GetKeys() {
  std::vector<std::string> keys;
  keys.reserve(values_.size());
  for (auto elem : values_) {
    keys.emplace_back(elem.first);
  }
  return keys;
}

void JsonObject::Clear() { values_.clear(); }

std::string JsonObject::AsString() {
  std::stringstream ss;
  std::string sep = kStringEmpty;
  ss << kStringCurlyBracketOpen;
  for (auto it = values_.begin(); it != values_.end(); it++) {
    ss << sep;
    ss << kStringDoubleQuote << it->first << kStringDoubleQuote + kStringColon;
    if (it->second.type() == typeid(void)) {
      ss << kJsonNull;
    } else if (it->second.type() == typeid(JsonBoolean)) {
      if (std::any_cast<JsonBoolean>(it->second)) {
        ss << kJsonTrue;
      } else {
        ss << kJsonFalse;
      }
    } else if (it->second.type() == typeid(JsonInteger)) {
      ss << std::any_cast<JsonInteger>(it->second);
    } else if (it->second.type() == typeid(JsonFloat)) {
      ss << std::fixed << std::any_cast<JsonFloat>(it->second);
    } else if (it->second.type() == typeid(JsonString)) {
      ss << kStringDoubleQuote << std::any_cast<JsonString>(it->second)
         << kStringDoubleQuote;
    } else if (it->second.type() == typeid(JsonObject)) {
      ss << std::any_cast<JsonObject>(it->second).AsString();
    } else if (it->second.type() == typeid(JsonArray)) {
      ss << std::any_cast<JsonArray>(it->second).AsString();
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
  size_t off = 0;
  Parse(source, off);
}

void JsonObject::Parse(const std::string &source, size_t &offset) {
  values_.clear();
  size_t off = offset;
  size_t pos;
  std::string key;
  std::string value;
  if (!ExpectString(source, kStringCurlyBracketOpen, off)) {
    throw std::runtime_error("invalid json object");
  }
  for (;;) {
    if (!ExpectString(source, kStringDoubleQuote, off)) {
      throw std::runtime_error("invalid json object");
    }
    pos = source.find(kStringDoubleQuote, off);
    if (pos == std::string::npos) {
      throw std::runtime_error("invalid json object");
    }
    key = source.substr(off, pos - off);
    off = pos + 1;
    if (!ExpectString(source, kStringColon, off)) {
      throw std::runtime_error("invalid json object");
    }
    if (!ExpectString(source, kStringEmpty, off)) {
      throw std::runtime_error("invalid json object");
    }
    if (source.substr(off, 4).compare(kJsonNull) == 0 &&
        CharIsAnyOf(source[off + 4], kJsonWssCharset + kStringComma +
                                         kStringCurlyBracketClose)) {
      PutNull(key);
      off += 4;
    } else if (source.substr(off, 4).compare(kJsonTrue) == 0 &&
               CharIsAnyOf(source[off + 4], kJsonWssCharset + kStringComma +
                                                kStringCurlyBracketClose)) {
      PutBoolean(key, true);
      off += 4;
    } else if (source.substr(off, 5).compare(kJsonFalse) == 0 &&
               CharIsAnyOf(source[off + 5], kJsonWssCharset + kStringComma +
                                                kStringCurlyBracketClose)) {
      PutBoolean(key, false);
      off += 5;
    } else if (source[off] == kCharDoubleQuote) {
      pos = source.find(kStringDoubleQuote, off + 1);
      if (pos == std::string::npos) {
        throw std::runtime_error("invalid json object");
      }
      PutString(key, source.substr(off + 1, pos - off - 1));
      off = pos + 1;
    } else if (CharIsAnyOf(source[off], kJsonNumberCharset)) {
      size_t pos = off + 1;
      bool dot_found = false;
      while (pos < source.length()) {
        if (CharIsAnyOf(source[pos], kJsonWssCharset + kStringComma +
                                         kStringCurlyBracketClose)) {
          break;
        }
        if (source[pos] == kCharDot) {
          dot_found = true;
        }
        pos++;
      }
      if (pos == source.length()) {
        throw std::runtime_error("invalid json object");
      }
      std::string num_string = source.substr(off, pos - off);
      try {
        if (dot_found) {
          PutFloat(key, std::atof(num_string.c_str()));
        } else {
          PutInteger(key, std::atoi(num_string.c_str()));
        }
      } catch (std::invalid_argument &) {
        throw std::runtime_error("invalid number format");
      }
      off = pos;
    } else if (source[off] == kCharCurlyBracketOpen) {
      JsonObject obj;
      obj.Parse(source, off);
      PutObject(key, obj);
    } else if (source[off] == kCharSquareBracketOpen) {
      JsonArray arr;
      arr.Parse(source, off);
      PutArray(key, arr);
    } else {
      throw std::runtime_error("invalid json object");
    }
    if (!ExpectString(source, kStringEmpty, off)) {
      throw std::runtime_error("invalid json object");
    }
    if (source[off] == kCharComma) {
      off++;
      continue;
    } else if (source[off] == kCharCurlyBracketClose) {
      off++;
      break;
    } else {
      throw std::runtime_error("invalid json object");
    }
  }
  offset = off;
}

JsonArray::JsonArray() {}

JsonArray::JsonArray(const std::string &source) { Parse(source); }

JsonArray::~JsonArray() {}

size_t JsonArray::Size() { return values_.size(); }

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

JsonBoolean JsonArray::GetAsBoolean(size_t index) {
  return std::any_cast<JsonBoolean>(values_[index]);
}

JsonInteger JsonArray::GetAsInteger(size_t index) {
  return std::any_cast<JsonInteger>(values_[index]);
}

JsonFloat JsonArray::GetAsFloat(size_t index) {
  return std::any_cast<JsonFloat>(values_[index]);
}

JsonString JsonArray::GetAsString(size_t index) {
  return std::any_cast<JsonString>(values_[index]);
}

JsonObject JsonArray::GetAsObject(size_t index) {
  return std::any_cast<JsonObject>(values_[index]);
}

JsonArray JsonArray::GetAsArray(size_t index) {
  return std::any_cast<JsonArray>(values_[index]);
}

bool JsonArray::IsNull(size_t index) { return !values_[index].has_value(); }

bool JsonArray::IsBoolean(size_t index) {
  try {
    JsonBoolean value __attribute__((unused));
    value = std::any_cast<JsonBoolean>(values_[index]);
    return true;
  } catch (std::bad_any_cast &expection) {
    return false;
  }
}

bool JsonArray::IsInteger(size_t index) {
  try {
    JsonInteger value __attribute__((unused));
    value = std::any_cast<JsonInteger>(values_[index]);
    return true;
  } catch (std::bad_any_cast &exception) {
    return false;
  }
}

bool JsonArray::IsFloat(size_t index) {
  try {
    JsonFloat value __attribute__((unused));
    value = std::any_cast<JsonFloat>(values_[index]);
    return true;
  } catch (std::bad_any_cast &exception) {
    return false;
  }
}

bool JsonArray::IsString(size_t index) {
  try {
    JsonString value __attribute__((unused));
    value = std::any_cast<JsonString>(values_[index]);
    return true;
  } catch (std::bad_any_cast &exception) {
    return false;
  }
}

bool JsonArray::IsObject(size_t index) {
  try {
    JsonObject value __attribute__((unused));
    value = std::any_cast<JsonObject>(values_[index]);
    return true;
  } catch (std::bad_any_cast &exception) {
    return false;
  }
}

bool JsonArray::IsArray(size_t index) {
  try {
    JsonArray value __attribute__((unused));
    value = std::any_cast<JsonArray>(values_[index]);
    return true;
  } catch (std::bad_any_cast &exception) {
    return false;
  }
}

void JsonArray::Clear() { values_.clear(); }

std::string JsonArray::AsString() {
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
      ss << std::any_cast<JsonObject>(*it).AsString();
    } else if (it->type() == typeid(JsonArray)) {
      ss << std::any_cast<JsonArray>(*it).AsString();
    } else {
      throw std::runtime_error("incompatible json type");
    }
    sep = kStringComma;
  }
  ss << kStringSquareBracketClose;
  return ss.str();
}

void JsonArray::Parse(const std::string &source) {
  size_t off = 0;
  Parse(source, off);
}

void JsonArray::Parse(const std::string &source, size_t &offset) {
  values_.clear();
  size_t off = offset;
  size_t pos;
  std::string value;
  if (!ExpectString(source, kStringSquareBracketOpen, off)) {
    throw std::runtime_error("invalid json object:");
  }
  for (;;) {
    if (!ExpectString(source, kStringEmpty, off)) {
      throw std::runtime_error("invalid json object");
    }
    if (source.substr(off, 4).compare(kJsonNull) == 0 &&
        CharIsAnyOf(source[off + 4], kJsonWssCharset + kStringComma +
                                         kStringSquareBracketClose)) {
      PutNull();
      off += 4;
    } else if (source.substr(off, 4).compare(kJsonTrue) == 0 &&
               CharIsAnyOf(source[off + 4], kJsonWssCharset + kStringComma +
                                                kStringSquareBracketClose)) {
      PutBoolean(true);
      off += 4;
    } else if (source.substr(off, 5).compare(kJsonFalse) == 0 &&
               CharIsAnyOf(source[off + 5], kJsonWssCharset + kStringComma +
                                                kStringSquareBracketClose)) {
      PutBoolean(false);
      off += 5;
    } else if (source[off] == kCharDoubleQuote) {
      pos = source.find(kStringDoubleQuote, off + 1);
      if (pos == std::string::npos) {
        throw std::runtime_error("invalid json object");
      }
      PutString(source.substr(off + 1, pos - off - 1));
      off = pos + 1;
    } else if (CharIsAnyOf(source[off], kJsonNumberCharset)) {
      size_t pos = off + 1;
      bool dot_found = false;
      while (pos < source.length()) {
        if (CharIsAnyOf(source[pos], kJsonWssCharset + kStringComma +
                                         kStringSquareBracketClose)) {
          break;
        }
        if (source[pos] == kCharDot) {
          dot_found = true;
        }
        pos++;
      }
      if (pos == source.length()) {
        throw std::runtime_error("invalid json object");
      }
      std::string num_string = source.substr(off, pos - off);
      try {
        if (dot_found) {
          PutFloat(std::atof(num_string.c_str()));
        } else {
          PutInteger(std::atoi(num_string.c_str()));
        }
      } catch (std::invalid_argument &) {
        throw std::runtime_error("invalid number format");
      }
      off = pos;
    } else if (source[off] == kCharCurlyBracketOpen) {
      JsonObject obj;
      obj.Parse(source, off);
      PutObject(obj);
    } else if (source[off] == kCharSquareBracketOpen) {
      JsonArray arr;
      arr.Parse(source, off);
      PutArray(arr);
    } else {
      throw std::runtime_error("invalid json object");
    }
    if (!ExpectString(source, kStringEmpty, off)) {
      throw std::runtime_error("invalid json object");
    }
    if (source[off] == kCharComma) {
      off++;
      continue;
    } else if (source[off] == kCharSquareBracketClose) {
      off++;
      break;
    } else {
      throw std::runtime_error("invalid json object");
    }
  }
  offset = off;
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
  std::vector<std::string> keys = object.GetKeys();
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
      if (object.GetAsBoolean(keys[i])) {
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
      JsonFloat value = object.GetAsFloat(keys[i]);
      stream.write((const char *)&value, sizeof(JsonFloat));
      bytes += sizeof(uint8_t) + sizeof(JsonFloat);
    } else if (object.IsString(keys[i])) {
      stream.write((const char *)&kString, sizeof(uint8_t));
      JsonString value = object.GetAsString(keys[i]);
      length = value.length();
      stream.write((const char *)&length, sizeof(size_t));
      stream.write((const char *)&value[0], length);
      bytes += sizeof(uint8_t) + sizeof(size_t) + length;
    } else if (object.IsObject(keys[i])) {
      stream.write((const char *)&kObject, sizeof(uint8_t));
      JsonObject value = object.GetAsObject(keys[i]);
      bytes += sizeof(uint8_t) + SerializeJsonObject(value, stream);
    } else if (object.IsArray(keys[i])) {
      stream.write((const char *)&kArray, sizeof(uint8_t));
      JsonArray value = object.GetAsArray(keys[i]);
      bytes += sizeof(uint8_t) + SerializeJsonArray(value, stream);
    } else {
      throw std::runtime_error("incompatible json type");
    }
  }
  return bytes;
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
  return bytes;
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
      if (array.GetAsBoolean(i)) {
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
      JsonFloat value = array.GetAsFloat(i);
      stream.write((const char *)&value, sizeof(JsonFloat));
      bytes += sizeof(uint8_t) + sizeof(JsonFloat);
    } else if (array.IsString(i)) {
      stream.write((const char *)&kString, sizeof(uint8_t));
      JsonString value = array.GetAsString(i);
      size_t length = value.length();
      stream.write((const char *)&length, sizeof(size_t));
      stream.write((const char *)&value[0], length);
      bytes += sizeof(uint8_t) + sizeof(size_t) + length;
    } else if (array.IsObject(i)) {
      stream.write((const char *)&kObject, sizeof(uint8_t));
      JsonObject value = array.GetAsObject(i);
      bytes += sizeof(uint8_t) + SerializeJsonObject(value, stream);
    } else if (array.IsArray(i)) {
      stream.write((const char *)kArray, sizeof(uint8_t));
      JsonArray value = array.GetAsArray(i);
      bytes += sizeof(uint8_t) + SerializeJsonArray(value, stream);
    } else {
      throw std::runtime_error("incompatible json type");
    }
  }
  return bytes;
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
      value.resize(2);
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
  return bytes;
}

JsonObject RandomDocument() {
  static RandomGenerator rnd;
  static const std::vector<std::string> keys = {
      rnd.Uuid(), rnd.Uuid(), rnd.Uuid(), rnd.Uuid(), rnd.Uuid(),
      rnd.Uuid(), rnd.Uuid(), rnd.Uuid(), rnd.Uuid(), rnd.Uuid()};
  JsonObject result;
  size_t idx = 0;
  result.PutBoolean(keys[idx++], rnd.Uniform() > 0.5 ? true : false);
  result.PutFloat(keys[idx++], rnd.Uniform());
  result.PutInteger(keys[idx++], rnd.Uint64() % 1048576);
  result.PutNull(keys[idx++]);
  JsonObject object;
  object.PutBoolean(keys[idx++], rnd.Uniform() > 0.5 ? true : false);
  object.PutFloat(keys[idx++], rnd.Uniform());
  object.PutInteger(keys[idx++], rnd.Uint64() % 1048576);
  object.PutNull(keys[idx++]);
  result.PutObject(keys[idx++], object);
  JsonArray array;
  array.PutBoolean(rnd.Uniform() > 0.5 ? true : false);
  array.PutFloat(rnd.Uniform());
  array.PutInteger(rnd.Uint64() % 1048576);
  array.PutNull();
  result.PutArray(keys[idx++], array);
  return result;
}
