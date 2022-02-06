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

#include "client.h"

Client::Client(const std::string &ip, const std::string &port,
               const std::string &user, const std::string &password)
    : ip_(ip), port_(port), user_(user), password_(password) {}

Client::~Client() {}

JsonArray Client::Insert(const JsonArray &values) {
  auto response =
      http::SendRequest(ip_, port_, POST, db_api::kRouteInsert, user_,
                        password_, APPLICATION_JSON, values.String());
  if (!response) {
    LOG_INFO("failed: insert request");
    throw std::runtime_error("insert request");
  }
  if (response->GetStatus() != HttpStatus::OK) {
    LOG_INFO("failed: insert response status");
    throw std::runtime_error("insert request");
  }
  if (response->GetBody().empty()) {
    LOG_INFO("failed: empty body");
    throw std::runtime_error("insert request");
  }
  JsonArray array;
  try {
    array.Parse((*response).GetBody());
  } catch (std::runtime_error &e) {
    LOG_INFO(std::string(e.what()));
  }
  return array;
}

JsonObject Client::Update(const JsonObject &values) {
  auto response =
      http::SendRequest(ip_, port_, POST, db_api::kRouteUpdate, user_,
                        password_, APPLICATION_JSON, values.String());
  if (!response) {
    LOG_INFO("failed: update request");
    throw std::runtime_error("update request");
  }
  if (response->GetStatus() != HttpStatus::OK) {
    LOG_INFO("failed: update response status");
    throw std::runtime_error("update request");
  }
  if (response->GetBody().empty()) {
    LOG_INFO("failed: empty body");
    throw std::runtime_error("insert request");
  }
  JsonObject object;
  try {
    object.Parse((*response).GetBody());
  } catch (std::runtime_error &e) {
    LOG_INFO(std::string(e.what()));
  }
  return object;
}

JsonArray Client::Erase(const JsonArray &keys) {
  auto response =
      http::SendRequest(ip_, port_, POST, db_api::kRouteErase, user_, password_,
                        APPLICATION_JSON, keys.String());
  if (!response) {
    LOG_INFO("failed: erase request");
    throw std::runtime_error("erase request");
  }
  if (response->GetStatus() != HttpStatus::OK) {
    LOG_INFO("failed: erase response status");
    throw std::runtime_error("erase request");
  }
  if (response->GetBody().empty()) {
    LOG_INFO("failed: empty body");
    throw std::runtime_error("insert request");
  }
  JsonArray array;
  try {
    array.Parse((*response).GetBody());
  } catch (std::runtime_error &e) {
    LOG_INFO(std::string(e.what()));
  }
  return array;
}

JsonArray Client::Find(const JsonArray &keys) {
  auto response = http::SendRequest(ip_, port_, POST, db_api::kRouteFind, user_,
                                    password_, APPLICATION_JSON, keys.String());
  if (!response) {
    LOG_INFO("failed: find request");
    throw std::runtime_error("find request");
  }
  if (response->GetStatus() != HttpStatus::OK) {
    LOG_INFO("failed: find response status");
    throw std::runtime_error("find request");
  }
  if (response->GetBody().empty()) {
    LOG_INFO("failed: empty body");
    throw std::runtime_error("insert request");
  }
  JsonArray array;
  try {
    array.Parse((*response).GetBody());
  } catch (std::runtime_error &e) {
    LOG_INFO(std::string(e.what()));
  }
  return array;
}
