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

static bool AccessPermitted(const HttpRequest &request, ServiceMap &services) {
  UserPool *user = static_cast<UserPool *>(services[kServiceUser]);
  std::string auth = request.GetHeader(kHttpAuthorization);
  if (auth.empty()) {
    return false;
  }
  std::vector<std::string> parts = StringExplode(auth, kStringSpace);
  if (parts.size() != 2 || parts[0].compare(kHttpBasic) != 0) {
    return false;
  }
  std::string auth_decoded = DecodeBase64(parts[1]);
  parts = StringExplode(auth_decoded, kStringColon);
  if (parts.size() != 2) {
    return false;
  }
  if (user->AccessPermitted(parts[0], parts[1])) {
    return true;
  }
  return false;
}

static bool ServicesAvailable(ServiceMap &services) {
  return services.find(kServiceUser) != services.end() &&
         services.find(kServiceDatabase) != services.end();
}

static bool JsonContent(const HttpRequest &request) {
  if (!request.GetHeader(kHttpContentType)
           .compare(HttpConstants::GetContentTypeString(APPLICATION_JSON))) {
    return true;
  }
  return false;
}

HttpResponse Insert(const HttpRequest &request, ServiceMap &services) {
  if (!ServicesAvailable(services)) {
    return HttpResponse::Build(HttpStatus::INTERNAL_SERVER_ERROR);
  }
  if (!AccessPermitted(request, services)) {
    return HttpResponse::Build(HttpStatus::UNAUTHORIZED);
  }
  if (!JsonContent(request)) {
    return HttpResponse::Build(HttpStatus::BAD_REQUEST);
  }
  JsonArray array;
  try {
    array.Parse(request.GetBody());
  } catch (std::runtime_error &) {
    return HttpResponse::Build(HttpStatus::BAD_REQUEST);
  }
  DocumentDatabase *db =
      static_cast<DocumentDatabase *>(services[kServiceDatabase]);
  return HttpResponse::Build(HttpStatus::OK, APPLICATION_JSON,
                             db->Insert(array).String());
}

HttpResponse Erase(const HttpRequest &request, ServiceMap &services) {
  if (!ServicesAvailable(services)) {
    return HttpResponse::Build(HttpStatus::INTERNAL_SERVER_ERROR);
  }
  if (!AccessPermitted(request, services)) {
    return HttpResponse::Build(HttpStatus::UNAUTHORIZED);
  }
  if (!JsonContent(request)) {
    return HttpResponse::Build(HttpStatus::BAD_REQUEST);
  }
  JsonArray array;
  try {
    array.Parse(request.GetBody());
  } catch (std::runtime_error &) {
    return HttpResponse::Build(HttpStatus::BAD_REQUEST);
  }
  DocumentDatabase *db =
      static_cast<DocumentDatabase *>(services[kServiceDatabase]);
  return HttpResponse::Build(HttpStatus::OK, APPLICATION_JSON,
                             db->Erase(array).String());
}

HttpResponse Find(const HttpRequest &request, ServiceMap &services) {
  if (!ServicesAvailable(services)) {
    return HttpResponse::Build(HttpStatus::INTERNAL_SERVER_ERROR);
  }
  if (!AccessPermitted(request, services)) {
    return HttpResponse::Build(HttpStatus::UNAUTHORIZED);
  }
  if (!JsonContent(request)) {
    return HttpResponse::Build(HttpStatus::BAD_REQUEST);
  }
  JsonArray array;
  try {
    array.Parse(request.GetBody());
  } catch (std::runtime_error &) {
    return HttpResponse::Build(HttpStatus::BAD_REQUEST);
  }
  DocumentDatabase *db =
      static_cast<DocumentDatabase *>(services[kServiceDatabase]);
  return HttpResponse::Build(HttpStatus::OK, APPLICATION_JSON,
                             db->Find(array).String());
}

HttpResponse Keys(const HttpRequest &request, ServiceMap &services) {
  if (!ServicesAvailable(services)) {
    return HttpResponse::Build(HttpStatus::INTERNAL_SERVER_ERROR);
  }
  if (!AccessPermitted(request, services)) {
    return HttpResponse::Build(HttpStatus::UNAUTHORIZED);
  }
  DocumentDatabase *db =
      static_cast<DocumentDatabase *>(services[kServiceDatabase]);
  return HttpResponse::Build(HttpStatus::OK, APPLICATION_JSON,
                             db->Keys().String());
}

HttpResponse Values(const HttpRequest &request, ServiceMap &services) {
  if (!ServicesAvailable(services)) {
    return HttpResponse::Build(HttpStatus::INTERNAL_SERVER_ERROR);
  }
  if (!AccessPermitted(request, services)) {
    return HttpResponse::Build(HttpStatus::UNAUTHORIZED);
  }
  DocumentDatabase *db =
      static_cast<DocumentDatabase *>(services[kServiceDatabase]);
  return HttpResponse::Build(HttpStatus::OK, APPLICATION_JSON,
                             db->Values().String());
}

HttpResponse Image(const HttpRequest &request, ServiceMap &services) {
  if (!ServicesAvailable(services)) {
    return HttpResponse::Build(HttpStatus::INTERNAL_SERVER_ERROR);
  }
  if (!AccessPermitted(request, services)) {
    return HttpResponse::Build(HttpStatus::UNAUTHORIZED);
  }
  JsonObject result;
  DocumentDatabase *db =
      static_cast<DocumentDatabase *>(services[kServiceDatabase]);
  return HttpResponse::Build(HttpStatus::OK, APPLICATION_JSON,
                             db->Image().String());
}

} // namespace db_api
