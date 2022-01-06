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

#include "client.h"

Client::Client(const std::string &ip, const std::string &port)
    : ip_(ip), port_(port) {}

Client::~Client() {}

void Client::RandomInsert(const size_t count) {
  JsonObject insert_object;
  JsonObject return_value;
  for (size_t i = 0; i < count; i++) {
    insert_object = RandomDocument();
    auto response = SendRequest(ip_, port_, POST, "/insert", "root", "0000",
                                APPLICATION_JSON, insert_object.AsString());
    if (!response) {
      Log::GetInstance()->Info("TEST FAILED: INSERTION REQUEST");
      Log::GetInstance()->Info((*response).AsString());
      throw std::runtime_error("INSERTION REQUEST");
    }
    return_value.Parse((*response).GetBody());
    if (!return_value.Has("success") || !return_value.IsBoolean("success") ||
        !return_value.GetAsBoolean("success")) {
      Log::GetInstance()->Info("TEST FAILED: INSERTION SUCCESS ATTRIBUTE");
      Log::GetInstance()->Info((*response).AsString());
      throw std::runtime_error("INSERTION SUCCESS ATTRIBUTE");
    }
    if (!return_value.Has("id") || !return_value.IsString("id")) {
      Log::GetInstance()->Info("TEST FAILED: INSERTION ID ATTRIBUTE");
      Log::GetInstance()->Info((*response).AsString());
      throw std::runtime_error("INSERTION ID ATTRIBUTE");
    }
    internal_.insert(
        std::make_pair(return_value.GetAsString("id"), insert_object));
  }
}

void Client::RandomRemove(const size_t count) {
  RandomGenerator rnd(time(nullptr));
  JsonObject return_value;
  for (size_t i = 0; i < count; i++) {
    auto it = internal_.begin();
    std::advance(it, rnd.Uint64() % internal_.size());
    std::string key = it->first;
    it = internal_.erase(it);
    auto response = SendRequest(ip_, port_, POST, "/remove", "root", "0000",
                                APPLICATION_JSON, "{\"id\":\"" + key + "\"}");
    if (!response) {
      Log::GetInstance()->Info("TEST FAILED: REMOVE REQUEST");
      Log::GetInstance()->Info((*response).AsString());
      throw std::runtime_error("REMOVE REQUEST");
    }
    return_value.Parse((*response).GetBody());
    if (!return_value.Has("success") || !return_value.IsBoolean("success") ||
        !return_value.GetAsBoolean("success")) {
      Log::GetInstance()->Info("TEST FAILED: FIND SUCCESS ATTRIBUTE");
      Log::GetInstance()->Info((*response).AsString());
      throw std::runtime_error("FIND SUCCESS ATTRIBUTE");
    }
    if (!return_value.Has("id") || !return_value.IsString("id")) {
      Log::GetInstance()->Info("TEST FAILED: REMOVE ID ATTRIBUTE");
      Log::GetInstance()->Info((*response).AsString());
      throw std::runtime_error("REMOVE ID ATTRIBUTE");
    }
  }
}

void Client::CompleteLookup() {
  JsonObject return_value;
  for (auto it = internal_.begin(); it != internal_.end(); it++) {
    std::string key = it->first;
    auto response = SendRequest(ip_, port_, POST, "/fetch", "root", "0000",
                                APPLICATION_JSON, "{\"id\":\"" + key + "\"}");
    if (!response) {
      Log::GetInstance()->Info("TEST FAILED: FETCH REQUEST");
      Log::GetInstance()->Info((*response).AsString());
      throw std::runtime_error("FETCH REQUEST");
    }
    return_value.Parse((*response).GetBody());
    if (!return_value.Has("success") || !return_value.IsBoolean("success") ||
        !return_value.GetAsBoolean("success")) {
      Log::GetInstance()->Info("TEST FAILED: FETCH SUCCESS ATTRIBUTE");
      Log::GetInstance()->Info((*response).AsString());
      throw std::runtime_error("FETCH SUCCESS ATTRIBUTE");
    }
    if (!return_value.Has("id") || !return_value.IsString("id")) {
      Log::GetInstance()->Info("TEST FAILED: FETCH ID ATTRIBUTE");
      Log::GetInstance()->Info((*response).AsString());
      throw std::runtime_error("FETCH ID ATTRIBUTE");
    }
    if (!return_value.Has("document") || !return_value.IsObject("document")) {
      Log::GetInstance()->Info("TEST FAILED: FETCH DOCUMENT");
      Log::GetInstance()->Info((*response).AsString());
      throw std::runtime_error("FETCH DOCUMENT");
    }
    if (it->second.AsString().compare(
            return_value.GetAsObject("document").AsString()) != 0) {
      Log::GetInstance()->Info("TEST FAILED: DOCUMENTS DIFFER");
      Log::GetInstance()->Info((*response).AsString());
      throw std::runtime_error("DOCUMENTS DIFFER");
    }
  }
}