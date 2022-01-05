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
    return_value.FromString((*response).GetBody());
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
    internal.insert(
        std::make_pair(return_value.GetAsString("id"), insert_object));
  }
}

void Client::RandomRemove(const size_t count) {
  RandomGenerator rnd(time(nullptr));
  JsonObject return_value;
  for (size_t i = 0; i < count; i++) {
    auto it = internal.begin();
    std::advance(it, rnd.Uint64() % internal.size());
    std::string key = it->first;
    it = internal.erase(it);
    auto response = SendRequest(ip_, port_, POST, "/remove", "root", "0000",
                                APPLICATION_JSON, "{\"id\":\"" + key + "\"}");
    if (!response) {
      Log::GetInstance()->Info("TEST FAILED: REMOVE REQUEST");
      Log::GetInstance()->Info((*response).AsString());
      throw std::runtime_error("REMOVE REQUEST");
    }
    return_value.FromString((*response).GetBody());
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
  for (auto it = internal.begin(); it != internal.end(); it++) {
    std::string key = it->first;
    auto response = SendRequest(ip_, port_, POST, "/find", "root", "0000",
                                APPLICATION_JSON, "{\"id\":\"" + key + "\"}");
    if (!response) {
      Log::GetInstance()->Info("TEST FAILED: FIND REQUEST");
      Log::GetInstance()->Info((*response).AsString());
      throw std::runtime_error("FIND REQUEST");
    }
    return_value.FromString((*response).GetBody());
    if (!return_value.Has("success") || !return_value.IsBoolean("success") ||
        !return_value.GetAsBoolean("success")) {
      Log::GetInstance()->Info("TEST FAILED: FIND SUCCESS ATTRIBUTE");
      Log::GetInstance()->Info((*response).AsString());
      throw std::runtime_error("FIND SUCCESS ATTRIBUTE");
    }
    if (!return_value.Has("id") || !return_value.IsString("id")) {
      Log::GetInstance()->Info("TEST FAILED: FIND ID ATTRIBUTE");
      Log::GetInstance()->Info((*response).AsString());
      throw std::runtime_error("FIND ID ATTRIBUTE");
    }
    if (!return_value.Has("document") || !return_value.IsObject("document")) {
      Log::GetInstance()->Info("TEST FAILED: FIND DOCUMENT");
      Log::GetInstance()->Info((*response).AsString());
      throw std::runtime_error("FIND DOCUMENT");
    }
    if (it->second.AsString().compare(
            return_value.GetAsObject("document").AsString()) != 0) {
      Log::GetInstance()->Info("TEST FAILED: DOCUMENTS DIFFER");
      Log::GetInstance()->Info((*response).AsString());
      throw std::runtime_error("DOCUMENTS DIFFER");
    }
  }
}