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

#ifndef API_H
#define API_H

#include "http.h"
#include "json.h"
#include "log.h"

namespace db_api {

const std::string kAuthorization = "authorization";
const std::string kBasic = "Basic";
const std::string kSuccess = "success";
const std::string kDocument = "document";
const std::string kId = "id";
const std::string kContentType = "content-type";
const std::string kKeys = "keys";
const std::string kFound = "found";
const std::string kSuccessObject = "{\"success\":true}";
const std::string kNoSuccessObject = "{\"success\":false}";

const std::string kRouteInsert = "/insert";
const std::string kRouteErase = "/erase";
const std::string kRouteFind = "/find";
const std::string kRouteKeys = "/keys";
const std::string kRouteImage = "/image";

const std::string kDatabaseService = "db";
const std::string kUserService = "user";

HttpResponse Insert(const HttpRequest &request, ServiceMap services);
HttpResponse Erase(const HttpRequest &request, ServiceMap services);
HttpResponse Find(const HttpRequest &request, ServiceMap services);
HttpResponse Keys(const HttpRequest &request, ServiceMap services);
HttpResponse Image(const HttpRequest &request, ServiceMap services);

} // namespace db_api

#endif