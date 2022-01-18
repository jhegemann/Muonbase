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

#ifndef API_H
#define API_H

#include "http.h"
#include "json.h"
#include "log.h"

namespace db_api {

const std::string kRouteInsert = "/insert";
const std::string kRouteErase = "/erase";
const std::string kRouteFind = "/find";
const std::string kRouteKeys = "/keys";
const std::string kRouteValues = "/values";
const std::string kRouteImage = "/image";

const std::string kServiceDatabase = "db";
const std::string kServiceUser = "user";

HttpResponse Insert(const HttpRequest &request, ServiceMap &services);
HttpResponse Erase(const HttpRequest &request, ServiceMap &services);
HttpResponse Find(const HttpRequest &request, ServiceMap &services);
HttpResponse Keys(const HttpRequest &request, ServiceMap &services);
HttpResponse Values(const HttpRequest &request, ServiceMap &services);
HttpResponse Image(const HttpRequest &request, ServiceMap &services);

} // namespace db_api

#endif