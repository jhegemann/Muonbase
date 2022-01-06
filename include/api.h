#ifndef API_H
#define API_H

#include "http.h"
#include "json.h"
#include "log.h"

namespace db_api {

const std::string kAuthorization = "authorization";
const std::string kBasic = "Basic";
const std::string kDatabaseService = "db";
const std::string kUserService = "user";
const std::string kDocument = "document";
const std::string kId = "id";
const std::string kContentType = "content-type";
const std::string kKeys = "keys";
const std::string kFound = "found";
const std::string kSuccess = "{\"success\":true}";
const std::string kNoSuccess = "{\"success\":false}";

HttpResponse Insert(const HttpRequest &request, ServiceMap services);
HttpResponse Erase(const HttpRequest &request, ServiceMap services);
HttpResponse Find(const HttpRequest &request, ServiceMap services);
HttpResponse Keys(const HttpRequest &request, ServiceMap services);
HttpResponse Image(const HttpRequest &request, ServiceMap services);

} // namespace db_api

#endif