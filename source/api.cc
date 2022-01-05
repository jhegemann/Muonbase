#include "api.h"

static const std::string kSuccess = "{\"success\":true}";
static const std::string kNoSuccess = "{\"success\":false}";

static bool AccessPermitted(const HttpRequest &request, ServiceMap services) {
  UserPool *user = static_cast<UserPool *>(services["user"]);
  std::string auth = request.GetHeader("authorization");
  if (auth.empty()) {
    return false;
  }
  std::vector<std::string> parts = StringExplode(auth, kStringSpace);
  if (parts.size() != 2 || parts[0].compare("Basic") != 0) {
    return false;
  }
  std::string auth_decoded = DecodeBase64(parts[1]);
  parts = StringExplode(auth_decoded, ":");
  if (parts.size() != 2) {
    return false;
  }
  if (!user->AccessPermitted(parts[0], parts[1])) {
    return false;
  }
  return true;
}

namespace db_api {

HttpResponse Insert(const HttpRequest &request, ServiceMap services) {
  if (services.find("user") == services.end() ||
      services.find("db") == services.end()) {
    return HttpResponse::Build(HttpStatus::INTERNAL_SERVER_ERROR,
                               APPLICATION_JSON, kNoSuccess);
  }
  if (!AccessPermitted(request, services)) {
    return HttpResponse::Build(HttpStatus::UNAUTHORIZED, APPLICATION_JSON,
                               kNoSuccess);
  }
  if (request.GetHeader("content-type")
          .compare(HttpConstants::GetContentTypeString(APPLICATION_JSON)) !=
      0) {
    return HttpResponse::Build(HttpStatus::BAD_REQUEST, APPLICATION_JSON,
                               kNoSuccess);
  }
  JsonObject object;
  try {
    object.FromString(request.GetBody());
  } catch (std::runtime_error &) {
    return HttpResponse::Build(HttpStatus::BAD_REQUEST, APPLICATION_JSON,
                               kNoSuccess);
  }
  DocumentDatabase *db = static_cast<DocumentDatabase *>(services["db"]);
  auto id = db->Insert(object);
  if (!id) {
    return HttpResponse::Build(HttpStatus::INTERNAL_SERVER_ERROR,
                               APPLICATION_JSON, kNoSuccess);
  }
  JsonObject result(kSuccess);
  result.PutString("id", *id);
  return HttpResponse::Build(HttpStatus::OK, APPLICATION_JSON,
                             result.AsString());
}

HttpResponse Remove(const HttpRequest &request, ServiceMap services) {
  if (services.find("user") == services.end() ||
      services.find("db") == services.end()) {
    return HttpResponse::Build(HttpStatus::INTERNAL_SERVER_ERROR,
                               APPLICATION_JSON, kNoSuccess);
  }
  if (!AccessPermitted(request, services)) {
    return HttpResponse::Build(HttpStatus::UNAUTHORIZED, APPLICATION_JSON,
                               kNoSuccess);
  }
  if (request.GetHeader("content-type")
          .compare(HttpConstants::GetContentTypeString(APPLICATION_JSON)) !=
      0) {
    return HttpResponse::Build(HttpStatus::BAD_REQUEST, APPLICATION_JSON,
                               kNoSuccess);
  }
  JsonObject object;
  try {
    object.FromString(request.GetBody());
  } catch (std::runtime_error &) {
    return HttpResponse::Build(HttpStatus::BAD_REQUEST, APPLICATION_JSON,
                               kNoSuccess);
  }
  if (!object.Has("id") || !object.IsString("id")) {
    return HttpResponse::Build(HttpStatus::BAD_REQUEST, APPLICATION_JSON,
                               kNoSuccess);
  }
  DocumentDatabase *db = static_cast<DocumentDatabase *>(services["db"]);
  auto id = db->Remove(object.GetAsString("id"));
  if (!id) {
    return HttpResponse::Build(HttpStatus::BAD_REQUEST, APPLICATION_JSON,
                               kNoSuccess);
  }
  JsonObject result(kSuccess);
  result.PutString("id", *id);
  return HttpResponse::Build(HttpStatus::OK, APPLICATION_JSON,
                             result.AsString());
}

HttpResponse Find(const HttpRequest &request, ServiceMap services) {
  if (services.find("user") == services.end() ||
      services.find("db") == services.end()) {
    return HttpResponse::Build(HttpStatus::INTERNAL_SERVER_ERROR,
                               APPLICATION_JSON, kNoSuccess);
  }
  if (!AccessPermitted(request, services)) {
    return HttpResponse::Build(HttpStatus::UNAUTHORIZED, APPLICATION_JSON,
                               kNoSuccess);
  }
  if (request.GetHeader("content-type")
          .compare(HttpConstants::GetContentTypeString(APPLICATION_JSON)) !=
      0) {
    return HttpResponse::Build(HttpStatus::BAD_REQUEST, APPLICATION_JSON,
                               kNoSuccess);
  }
  JsonObject object;
  try {
    object.FromString(request.GetBody());
  } catch (std::runtime_error &) {
    return HttpResponse::Build(HttpStatus::BAD_REQUEST, APPLICATION_JSON,
                               kNoSuccess);
  }
  if (!object.Has("id") || !object.IsString("id")) {
    return HttpResponse::Build(HttpStatus::BAD_REQUEST, APPLICATION_JSON,
                               kNoSuccess);
  }
  DocumentDatabase *db = static_cast<DocumentDatabase *>(services["db"]);
  auto id = db->Find(object.GetAsString("id"));
  JsonObject result(kSuccess);
  result.PutString("id", object.GetAsString("id"));
  if (id) {
    result.PutObject("document", *id);
    result.PutBoolean("found", true);
  } else {
    result.PutBoolean("found", false);
  }
  return HttpResponse::Build(HttpStatus::OK, APPLICATION_JSON,
                             result.AsString());
}

HttpResponse Keys(const HttpRequest &request, ServiceMap services) {
  if (services.find("user") == services.end() ||
      services.find("db") == services.end()) {
    return HttpResponse::Build(HttpStatus::INTERNAL_SERVER_ERROR,
                               APPLICATION_JSON, kNoSuccess);
  }
  if (!AccessPermitted(request, services)) {
    return HttpResponse::Build(HttpStatus::UNAUTHORIZED, APPLICATION_JSON,
                               kNoSuccess);
  }
  if (request.GetHeader("content-type")
          .compare(HttpConstants::GetContentTypeString(APPLICATION_JSON)) !=
      0) {
    return HttpResponse::Build(HttpStatus::BAD_REQUEST, APPLICATION_JSON,
                               kNoSuccess);
  }
  JsonObject result;
  DocumentDatabase *db = static_cast<DocumentDatabase *>(services["db"]);
  std::vector<std::string> keys = db->Keys();
  JsonArray array;
  for (size_t i = 0; i < keys.size(); i++) {
    array.PutString(keys[i]);
  }
  result.PutArray("keys", array);
  return HttpResponse::Build(HttpStatus::OK, APPLICATION_JSON,
                             result.AsString());
}

HttpResponse Dump(const HttpRequest &request, ServiceMap services) {
  if (services.find("user") == services.end() ||
      services.find("db") == services.end()) {
    return HttpResponse::Build(HttpStatus::INTERNAL_SERVER_ERROR,
                               APPLICATION_JSON, kNoSuccess);
  }
  if (!AccessPermitted(request, services)) {
    return HttpResponse::Build(HttpStatus::UNAUTHORIZED, APPLICATION_JSON,
                               kNoSuccess);
  }
  DocumentDatabase *db = static_cast<DocumentDatabase *>(services["db"]);
  return HttpResponse::Build(HttpStatus::OK, APPLICATION_JSON,
                             db->Dump().AsString());
}

} // namespace db_api
