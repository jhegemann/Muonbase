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

#include "api.h"
#include "http.h"
#include "log.h"
#include "service.h"
#include "utils.h"
#include <iostream>
#include <unistd.h>

static const char kOptionVerbose = 'v';
static const char kOptionConfig = 'c';
static const char kOptionDaemon = 'd';
static const char *kOptionString = "vdc:";

static const std::string kIp = "ip";
static const std::string kIpDefault = "127.0.0.1";
static const std::string kPort = "port";
static const std::string kPortDefault = "8260";
static const std::string kDataPath = "data_path";
static const std::string kDataPathDefault = "./storage.db";
static const std::string kUserPath = "user_path";
static const std::string kUserPathDefault = "./users.json";
static const std::string kLogPath = "log_path";
static const std::string kWorkingDirectory = "working_directory";

static void PrintUsage() {
  std::cout << "Usage: database.app [-v] [-d] [-c <config>]" << std::endl;
  std::cout << "\t -v: verbose" << std::endl;
  std::cout << "\t -d: daemon" << std::endl;
  std::cout << "\t -c <file>: configuration (mandatory)" << std::endl;
}

int main(int argc, char **argv) {
  int option;
  JsonObject config;
  bool daemonize = false;
  bool config_available = false;
  bool verbose = false;
  while ((option = getopt(argc, argv, kOptionString)) != -1) {
    switch (option) {
    case kOptionVerbose:
      verbose = true;
      break;
    case kOptionConfig:
      std::cout << optarg << std::endl;
      config.Parse(FileToString(optarg));
      config_available = true;
      break;
    case kOptionDaemon:
      daemonize = true;
      break;
    case ':':
      Log::GetInstance()->Info("option needs a value");
      PrintUsage();
      exit(1);
    case '?':
      Log::GetInstance()->Info("unknown option " + std::string(optopt, 1));
      PrintUsage();
      exit(1);
    default:
      PrintUsage();
      exit(0);
    }
  }

  if (!config_available) {
    PrintUsage();
    exit(0);
  }

  if (verbose) {
    Log::GetInstance()->SetVerbose(true);
  }

  std::string working_directory = ".";
  if (daemonize) {
    if (config.Has(kWorkingDirectory) && config.IsString(kWorkingDirectory)) {
      working_directory = config.GetAsString(kWorkingDirectory);
    }
    Log::GetInstance()->Info("daemonize process");
    DaemonizeProcess(working_directory);
    if (config.Has(kLogPath) && config.IsString(kLogPath)) {
      Log::GetInstance()->SetLogfile(config.GetAsString(kLogPath));
    }
  }

  std::string data_path = kDataPathDefault;
  if (config.Has(kDataPath) && config.IsString(kDataPath)) {
    data_path = config.GetAsString(kDataPath);
  }

  std::string user_path = kUserPathDefault;
  if (config.Has(kUserPath) && config.IsString(kUserPath)) {
    user_path = config.GetAsString(kUserPath);
  }

  HttpServer server;

  Log::GetInstance()->Info("set up services");
  server.RegisterService(db_api::kDatabaseService,
                         new DocumentDatabase(data_path));
  server.RegisterService(db_api::kUserService, new UserPool(user_path));

  Log::GetInstance()->Info("set up routes");
  server.RegisterHandler(HttpMethod::POST, db_api::kRouteInsert,
                         db_api::Insert);
  server.RegisterHandler(HttpMethod::POST, db_api::kRouteErase, db_api::Erase);
  server.RegisterHandler(HttpMethod::POST, db_api::kRouteFind, db_api::Find);
  server.RegisterHandler(HttpMethod::GET, db_api::kRouteKeys, db_api::Keys);
  server.RegisterHandler(HttpMethod::GET, db_api::kRouteImage, db_api::Image);

  Log::GetInstance()->Info("start server");
  std::string ip = kIpDefault;
  if (config.Has(kIp) && config.IsString(kIp)) {
    ip = config.GetAsString(kIp);
  }
  std::string port = kPortDefault;
  if (config.Has(kPort) && config.IsString(kPort)) {
    port = config.GetAsString(kPort);
  }

  server.Serve(port, ip);
  return 0;
}
