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

Client::Client(const std::string &ip, const std::string &port,
               const std::string &user, const std::string &password)
    : ip_(ip), port_(port), user_(user), password_(password) {}

Client::~Client() {}

JsonArray Client::Insert(const JsonArray &values) {
  auto response = SendRequest(ip_, port_, POST, db_api::kRouteInsert, user_,
                              password_, APPLICATION_JSON, values.String());
  if (!response) {
    Log::GetInstance()->Info("failed: insert request");
    throw std::runtime_error("insert request");
  }
  if (response->GetStatus() != OK) {
    Log::GetInstance()->Info("failed: insert response status");
    throw std::runtime_error("insert request");
  }
  return JsonArray((*response).GetBody());
}

JsonArray Client::Erase(const JsonArray &keys) {
  auto response = SendRequest(ip_, port_, POST, db_api::kRouteErase, user_,
                              password_, APPLICATION_JSON, keys.String());
  if (!response) {
    Log::GetInstance()->Info("failed: erase request");
    throw std::runtime_error("erase request");
  }
  if (response->GetStatus() != OK) {
    Log::GetInstance()->Info("failed: erase response status");
    throw std::runtime_error("erase request");
  }
  return JsonArray((*response).GetBody());
}

JsonArray Client::Find(const JsonArray &keys) {
  auto response = SendRequest(ip_, port_, POST, db_api::kRouteFind, user_,
                              password_, APPLICATION_JSON, keys.String());
  if (!response) {
    Log::GetInstance()->Info("failed: find request");
    throw std::runtime_error("find request");
  }
  if (response->GetStatus() != OK) {
    Log::GetInstance()->Info("failed: find response status");
    throw std::runtime_error("find request");
  }
  return JsonArray((*response).GetBody());
}

JsonArray Client::Keys() {
  auto response =
      SendRequest(ip_, port_, GET, db_api::kRouteKeys, user_, password_);
  if (!response) {
    Log::GetInstance()->Info("failed: keys request");
    throw std::runtime_error("keys request");
  }
  if (response->GetStatus() != OK) {
    Log::GetInstance()->Info("failed: keys response status");
    throw std::runtime_error("keys request");
  }
  return JsonArray((*response).GetBody());
}

JsonArray Client::Values() {
  auto response =
      SendRequest(ip_, port_, GET, db_api::kRouteValues, user_, password_);
  if (!response) {
    Log::GetInstance()->Info("failed: values request");
    throw std::runtime_error("values request");
  }
  if (response->GetStatus() != OK) {
    Log::GetInstance()->Info("failed: values response status");
    throw std::runtime_error("values request");
  }
  return JsonArray((*response).GetBody());
}

JsonObject Client::Image() {
  auto response =
      SendRequest(ip_, port_, GET, db_api::kRouteImage, user_, password_);
  if (!response) {
    Log::GetInstance()->Info("failed: image request");
    throw std::runtime_error("image request");
  }
  if (response->GetStatus() != OK) {
    Log::GetInstance()->Info("failed: image response status");
    throw std::runtime_error("image request");
  }
  return JsonObject((*response).GetBody());
}
