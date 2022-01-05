#include "service.h"

ApiService::ApiService() {}

ApiService::~ApiService() {}

DocumentDatabase::DocumentDatabase(const std::string &filepath)
    : filepath_(filepath), filepath_journal_(filepath + ".journal"),
      filepath_snapshot_(filepath + ".snapshot") {}

DocumentDatabase::~DocumentDatabase() {}

void DocumentDatabase::Initialize() {
  random_.Seed(time(nullptr));
  if (FileExists(filepath_)) {
    std::fstream stream;
    stream.open(filepath_, std::fstream::in | std::fstream::binary);
    serializer_.Deserialize(db_, stream);
    stream.close();
  }
  log_.Load(filepath_journal_);
  Log::GetInstance()->Info("replay write ahead log");
  for (auto it = log_.Log().begin(); it != log_.Log().end(); it++) {
    switch (it->GetOperation()) {
    case STORAGE_INSERT:
      db_.Put(it->GetKey(), *it->GetValue());
      break;
    case STORAGE_REMOVE:
      db_.Erase(it->GetKey());
      break;
    default:
      throw std::runtime_error("unknown storage operation");
    }
  }
  log_.Unload();
  Rollover();
}

void DocumentDatabase::Tick() {
  Log::GetInstance()->Info("rollover database journal");
  Rollover();
}

void DocumentDatabase::Shutdown() {}

void DocumentDatabase::Rollover() {
  if (db_.GetSize() == 0) {
    return;
  }
  if (!FileExists(filepath_journal_)) {
    return;
  }
  if (FileSize(filepath_journal_) < FileSize(filepath_)) {
    return;
  }
  std::fstream stream;
  stream.open(filepath_snapshot_, std::fstream::out | std::fstream::binary);
  serializer_.Serialize(db_, stream);
  stream.close();
  rename(filepath_snapshot_.c_str(), filepath_.c_str());
  unlink(filepath_journal_.c_str());
}

std::optional<std::string> DocumentDatabase::Insert(JsonObject &document) {
  std::string uuid;
  bool unique = false;
  while (!unique) {
    uuid = random_.Uuid();
    if (db_.Find(uuid) == db_.End()) {
      unique = true;
    }
  }
  log_.Append(filepath_journal_, STORAGE_INSERT, uuid, &document);
  db_.Put(uuid, document);
  return uuid;
}

std::optional<std::string> DocumentDatabase::Remove(std::string id) {
  MapIterator<std::string, JsonObject> it = db_.Find(id);
  if (it == db_.End()) {
    return {};
  }
  log_.Append(filepath_journal_, STORAGE_REMOVE, id, nullptr);
  db_.Erase(it);
  return id;
}

std::optional<JsonObject> DocumentDatabase::Find(std::string id) {
  MapIterator<std::string, JsonObject> it = db_.Find(id);
  if (it == db_.End()) {
    return {};
  }
  return it.GetValue();
}

std::vector<std::string> DocumentDatabase::Keys() {
  MapIterator<std::string, JsonObject> it = db_.Begin();
  std::vector<std::string> keys;
  while (it != db_.End()) {
    keys.emplace_back(it.GetKey());
    it++;
  }
  return keys;
}

JsonObject DocumentDatabase::Dump() {
  JsonObject dump;
  MapIterator<std::string, JsonObject> it = db_.Begin();
  while (it != db_.End()) {
    dump.PutObject(it.GetKey(), it.GetValue());
    it++;
  }
  return dump;
}

UserPool::UserPool(const std::string &filepath) : filepath_(filepath) {}

UserPool::~UserPool() {}

void UserPool::Initialize() { users_.FromString(FileToString(filepath_)); }

void UserPool::Tick() {}

void UserPool::Shutdown() { StringToFile(filepath_, users_.AsString()); }

bool UserPool::AccessPermitted(const std::string &user,
                               const std::string &passwd) {
  if (users_.Has(user) && users_.IsString(user)) {
    if (Sha256Hash(passwd).compare(users_.GetAsString(user)) == 0) {
      return true;
    }
  }
  return false;
}