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

#include "api.h"

namespace db_api {

static bool AccessPermitted(const HttpRequest &request, ServiceMap services) {
  UserPool *user = static_cast<UserPool *>(services[kUserService]);
  std::string auth = request.GetHeader(kAuthorization);
  if (auth.empty()) {
    return false;
  }
  std::vector<std::string> parts = StringExplode(auth, kStringSpace);
  if (parts.size() != 2 || parts[0].compare(kBasic) != 0) {
    return false;
  }
  std::string auth_decoded = DecodeBase64(parts[1]);
  parts = StringExplode(auth_decoded, kStringColon);
  if (parts.size() != 2) {
    return false;
  }
  if (!user->AccessPermitted(parts[0], parts[1])) {
    return false;
  }
  return true;
}

HttpResponse Insert(const HttpRequest &request, ServiceMap services) {
  if (services.find(kUserService) == services.end() ||
      services.find(kDatabaseService) == services.end()) {
    return HttpResponse::Build(HttpStatus::INTERNAL_SERVER_ERROR,
                               APPLICATION_JSON, kNoSuccessObject);
  }
  if (!AccessPermitted(request, services)) {
    return HttpResponse::Build(HttpStatus::UNAUTHORIZED, APPLICATION_JSON,
                               kNoSuccessObject);
  }
  if (request.GetHeader(kContentType)
          .compare(HttpConstants::GetContentTypeString(APPLICATION_JSON)) !=
      0) {
    return HttpResponse::Build(HttpStatus::BAD_REQUEST, APPLICATION_JSON,
                               kNoSuccessObject);
  }
  JsonObject object;
  try {
    object.Parse(request.GetBody());
  } catch (std::runtime_error &) {
    return HttpResponse::Build(HttpStatus::BAD_REQUEST, APPLICATION_JSON,
                               kNoSuccessObject);
  }
  DocumentDatabase *db =
      static_cast<DocumentDatabase *>(services[kDatabaseService]);
  auto id = db->Insert(object);
  if (!id) {
    return HttpResponse::Build(HttpStatus::INTERNAL_SERVER_ERROR,
                               APPLICATION_JSON, kNoSuccessObject);
  }
  JsonObject result(kSuccessObject);
  result.PutString(kId, *id);
  return HttpResponse::Build(HttpStatus::OK, APPLICATION_JSON,
                             result.AsString());
}

HttpResponse Erase(const HttpRequest &request, ServiceMap services) {
  if (services.find(kUserService) == services.end() ||
      services.find(kDatabaseService) == services.end()) {
    return HttpResponse::Build(HttpStatus::INTERNAL_SERVER_ERROR,
                               APPLICATION_JSON, kNoSuccessObject);
  }
  if (!AccessPermitted(request, services)) {
    return HttpResponse::Build(HttpStatus::UNAUTHORIZED, APPLICATION_JSON,
                               kNoSuccessObject);
  }
  if (request.GetHeader(kContentType)
          .compare(HttpConstants::GetContentTypeString(APPLICATION_JSON)) !=
      0) {
    return HttpResponse::Build(HttpStatus::BAD_REQUEST, APPLICATION_JSON,
                               kNoSuccessObject);
  }
  JsonObject object;
  try {
    object.Parse(request.GetBody());
  } catch (std::runtime_error &) {
    return HttpResponse::Build(HttpStatus::BAD_REQUEST, APPLICATION_JSON,
                               kNoSuccessObject);
  }
  if (!object.Has(kId) || !object.IsString(kId)) {
    return HttpResponse::Build(HttpStatus::BAD_REQUEST, APPLICATION_JSON,
                               kNoSuccessObject);
  }
  DocumentDatabase *db =
      static_cast<DocumentDatabase *>(services[kDatabaseService]);
  auto id = db->Erase(object.GetAsString(kId));
  if (!id) {
    return HttpResponse::Build(HttpStatus::BAD_REQUEST, APPLICATION_JSON,
                               kNoSuccessObject);
  }
  JsonObject result(kSuccessObject);
  result.PutString(kId, *id);
  return HttpResponse::Build(HttpStatus::OK, APPLICATION_JSON,
                             result.AsString());
}

HttpResponse Find(const HttpRequest &request, ServiceMap services) {
  if (services.find(kUserService) == services.end() ||
      services.find(kDatabaseService) == services.end()) {
    return HttpResponse::Build(HttpStatus::INTERNAL_SERVER_ERROR,
                               APPLICATION_JSON, kNoSuccessObject);
  }
  if (!AccessPermitted(request, services)) {
    return HttpResponse::Build(HttpStatus::UNAUTHORIZED, APPLICATION_JSON,
                               kNoSuccessObject);
  }
  if (request.GetHeader(kContentType)
          .compare(HttpConstants::GetContentTypeString(APPLICATION_JSON)) !=
      0) {
    return HttpResponse::Build(HttpStatus::BAD_REQUEST, APPLICATION_JSON,
                               kNoSuccessObject);
  }
  JsonObject object;
  try {
    object.Parse(request.GetBody());
  } catch (std::runtime_error &) {
    return HttpResponse::Build(HttpStatus::BAD_REQUEST, APPLICATION_JSON,
                               kNoSuccessObject);
  }
  if (!object.Has(kId) || !object.IsString(kId)) {
    return HttpResponse::Build(HttpStatus::BAD_REQUEST, APPLICATION_JSON,
                               kNoSuccessObject);
  }
  DocumentDatabase *db =
      static_cast<DocumentDatabase *>(services[kDatabaseService]);
  auto id = db->Find(object.GetAsString(kId));
  JsonObject result(kSuccessObject);
  result.PutString(kId, object.GetAsString(kId));
  if (id) {
    result.PutObject(kDocument, *id);
    result.PutBoolean(kFound, true);
  } else {
    result.PutBoolean(kFound, false);
  }
  return HttpResponse::Build(HttpStatus::OK, APPLICATION_JSON,
                             result.AsString());
}

HttpResponse Keys(const HttpRequest &request, ServiceMap services) {
  if (services.find(kUserService) == services.end() ||
      services.find(kDatabaseService) == services.end()) {
    return HttpResponse::Build(HttpStatus::INTERNAL_SERVER_ERROR,
                               APPLICATION_JSON, kNoSuccessObject);
  }
  if (!AccessPermitted(request, services)) {
    return HttpResponse::Build(HttpStatus::UNAUTHORIZED, APPLICATION_JSON,
                               kNoSuccessObject);
  }
  if (request.GetHeader(kContentType)
          .compare(HttpConstants::GetContentTypeString(APPLICATION_JSON)) !=
      0) {
    return HttpResponse::Build(HttpStatus::BAD_REQUEST, APPLICATION_JSON,
                               kNoSuccessObject);
  }
  JsonObject result;
  DocumentDatabase *db =
      static_cast<DocumentDatabase *>(services[kDatabaseService]);
  std::vector<std::string> keys = db->Keys();
  JsonArray array;
  for (size_t i = 0; i < keys.size(); i++) {
    array.PutString(keys[i]);
  }
  result.PutArray(kKeys, array);
  return HttpResponse::Build(HttpStatus::OK, APPLICATION_JSON,
                             result.AsString());
}

HttpResponse Image(const HttpRequest &request, ServiceMap services) {
  if (services.find(kUserService) == services.end() ||
      services.find(kDatabaseService) == services.end()) {
    return HttpResponse::Build(HttpStatus::INTERNAL_SERVER_ERROR,
                               APPLICATION_JSON, kNoSuccessObject);
  }
  if (!AccessPermitted(request, services)) {
    return HttpResponse::Build(HttpStatus::UNAUTHORIZED, APPLICATION_JSON,
                               kNoSuccessObject);
  }
  if (request.GetHeader(kContentType)
          .compare(HttpConstants::GetContentTypeString(APPLICATION_JSON)) !=
      0) {
    return HttpResponse::Build(HttpStatus::BAD_REQUEST, APPLICATION_JSON,
                               kNoSuccessObject);
  }
  DocumentDatabase *db =
      static_cast<DocumentDatabase *>(services[kDatabaseService]);
  return HttpResponse::Build(HttpStatus::OK, APPLICATION_JSON,
                             db->Image().AsString());
}

} // namespace db_api
