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

#include "client.h"

Client::Client(const std::string &ip, const std::string &port)
    : ip_(ip), port_(port) {}

Client::~Client() {}

void Client::RandomInsert(const size_t count) {
  JsonObject insert_object;
  JsonObject return_value;
  for (size_t i = 0; i < count; i++) {
    insert_object = RandomDocument();
    auto response =
        SendRequest(ip_, port_, POST, db_api::kRouteInsert, "root", "0000",
                    APPLICATION_JSON, insert_object.AsString());
    if (!response) {
      Log::GetInstance()->Info("test failed: insert request");
      throw std::runtime_error("insert request");
    }
    if (response->GetStatus() != OK) {
      Log::GetInstance()->Info("test failed: insert response status");
      throw std::runtime_error("insert request");
    }
    return_value.Parse((*response).GetBody());
    if (!return_value.Has(db_api::kSuccess) ||
        !return_value.IsBoolean(db_api::kSuccess) ||
        !return_value.GetAsBoolean(db_api::kSuccess)) {
      Log::GetInstance()->Info("test failed: insert success attribute");
      Log::GetInstance()->Info((*response).AsString());
      throw std::runtime_error("insert success attribute");
    }
    if (!return_value.Has(db_api::kId) || !return_value.IsString(db_api::kId)) {
      Log::GetInstance()->Info("test failed: insert id attribute");
      Log::GetInstance()->Info((*response).AsString());
      throw std::runtime_error("insert id attribute");
    }
    internal_.insert(
        std::make_pair(return_value.GetAsString(db_api::kId), insert_object));
  }
}

void Client::RandomErase(const size_t count) {
  RandomGenerator rnd(time(nullptr));
  JsonObject return_value;
  for (size_t i = 0; i < count; i++) {
    auto it = internal_.begin();
    std::advance(it, rnd.Uint64() % internal_.size());
    std::string key = it->first;
    it = internal_.erase(it);
    auto response =
        SendRequest(ip_, port_, POST, db_api::kRouteErase, "root", "0000",
                    APPLICATION_JSON, "{\"id\":\"" + key + "\"}");
    if (!response) {
      Log::GetInstance()->Info("test failed: erase request");
      throw std::runtime_error("erase request");
    }
    if (response->GetStatus() != OK) {
      Log::GetInstance()->Info("test failed: erase response status");
      throw std::runtime_error("erase request");
    }
    return_value.Parse((*response).GetBody());
    if (!return_value.Has(db_api::kSuccess) ||
        !return_value.IsBoolean(db_api::kSuccess) ||
        !return_value.GetAsBoolean(db_api::kSuccess)) {
      Log::GetInstance()->Info("test failed: erase success attribute");
      Log::GetInstance()->Info((*response).AsString());
      throw std::runtime_error("erase success attribute");
    }
    if (!return_value.Has(db_api::kId) || !return_value.IsString(db_api::kId)) {
      Log::GetInstance()->Info("test failed: erase id attribute");
      Log::GetInstance()->Info((*response).AsString());
      throw std::runtime_error("erase id attribute");
    }
  }
}

void Client::FindAll() {
  JsonObject return_value;
  for (auto it = internal_.begin(); it != internal_.end(); it++) {
    std::string key = it->first;
    auto response =
        SendRequest(ip_, port_, POST, db_api::kRouteFind, "root", "0000",
                    APPLICATION_JSON, "{\"id\":\"" + key + "\"}");
    if (!response) {
      Log::GetInstance()->Info("test failed: find request");
      throw std::runtime_error("find request");
    }
    if (response->GetStatus() != OK) {
      Log::GetInstance()->Info("test failed: find response status");
      throw std::runtime_error("find request");
    }
    return_value.Parse((*response).GetBody());
    if (!return_value.Has(db_api::kSuccess) ||
        !return_value.IsBoolean(db_api::kSuccess) ||
        !return_value.GetAsBoolean(db_api::kSuccess)) {
      Log::GetInstance()->Info("test failed: find success attribute");
      Log::GetInstance()->Info((*response).AsString());
      throw std::runtime_error("find success attribute");
    }
    if (!return_value.Has(db_api::kId) || !return_value.IsString(db_api::kId)) {
      Log::GetInstance()->Info("test failed: find id attribute");
      Log::GetInstance()->Info((*response).AsString());
      throw std::runtime_error("find id attribute");
    }
    if (!return_value.Has(db_api::kDocument) ||
        !return_value.IsObject(db_api::kDocument)) {
      Log::GetInstance()->Info("test failed: find document");
      Log::GetInstance()->Info((*response).AsString());
      throw std::runtime_error("find document");
    }
    if (it->second.AsString().compare(
            return_value.GetAsObject(db_api::kDocument).AsString()) != 0) {
      Log::GetInstance()->Info("test failed: documents differ");
      Log::GetInstance()->Info((*response).AsString());
      throw std::runtime_error("documents differ");
    }
  }
}