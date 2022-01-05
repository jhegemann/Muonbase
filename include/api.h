#ifndef API_H
#define API_H

#include "http.h"
#include "json.h"
#include "log.h"

namespace db_api {

HttpResponse Insert(const HttpRequest &request, ServiceMap services);
HttpResponse Remove(const HttpRequest &request, ServiceMap services);
HttpResponse Find(const HttpRequest &request, ServiceMap services);
HttpResponse Keys(const HttpRequest &request, ServiceMap services);
HttpResponse Dump(const HttpRequest &request, ServiceMap services);

} // namespace db_api

#endif