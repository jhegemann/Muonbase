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

#include <unistd.h>

#include <iostream>

#include "api.h"
#include "http.h"
#include "log.h"
#include "service.h"
#include "utils.h"

static const char kOptionVerbose = 'v';
static const char kOptionConfig = 'c';
static const char kOptionDaemon = 'd';
static const char kOptionHelp = 'h';
static const char *kOptionString = "hvdc:";

static const std::string kIp = "ip";
static const std::string kIpDefault = "127.0.0.1";
static const std::string kPort = "port";
static const std::string kPortDefault = "8260";
static const std::string kDbPath = "dbPath";
static const std::string kDbPathDefault = "./muonbase-storage.db";
static const std::string kUserPath = "userPath";
static const std::string kUserPathDefault = "./muonbase-user.json";
static const std::string kLogPath = "logPath";
static const std::string kLogPathDefault = "./muonbase-server.log";
static const std::string kWorkingDirectory = "workingDirectory";
static const std::string kWorkingDirectoryDefault = "./";

static const bool kDaemonizeDefault = false;
static const bool kVerboseDefault = false;

static void PrintVersion() {
  std::cout << "Muonbase v1.0.0" << std::endl;
  std::cout << "Copyright 2022 Jonas Hegemann <jonas.hegemann@hotmail.de>"
            << std::endl;
}

static void PrintUsage() {
  std::cout << "Usage: muonbase-server [-h] [-v] [-d] [-c <config>]"
            << std::endl;
  std::cout << "\t -h: help" << std::endl;
  std::cout << "\t -v: verbose - default " << std::boolalpha << kVerboseDefault
            << std::endl;
  std::cout << "\t -d: daemonize - default " << std::boolalpha
            << kDaemonizeDefault << std::endl;
  std::cout << "\t -c <file>: configuration (mandatory)" << std::endl;
}

int main(int argc, char **argv) {
  PrintVersion();
  int option;
  JsonObject config;
  bool daemonize = kDaemonizeDefault;
  bool verbose = kVerboseDefault;
  bool config_available = false;
  while ((option = getopt(argc, argv, kOptionString)) != -1) {
    switch (option) {
      case kOptionVerbose:
        verbose = true;
        break;
      case kOptionConfig:
        try {
          config.Parse(FileToString(optarg));
        } catch (std::runtime_error &) {
          LOG_INFO("error parsing configuration");
          exit(1);
        }
        config_available = true;
        break;
      case kOptionDaemon:
        daemonize = true;
        break;
      case kOptionHelp:
        PrintUsage();
        exit(0);
      case kCharColon:
        LOG_INFO("option needs a value");
        PrintUsage();
        exit(1);
      case kCharQuestionMark:
        LOG_INFO("unknown option " + std::string(optopt, 1));
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

  std::string working_directory = kWorkingDirectoryDefault;
  if (daemonize) {
    if (config.Has(kWorkingDirectory) && config.IsString(kWorkingDirectory)) {
      working_directory = config.GetString(kWorkingDirectory);
    } else {
      LOG_INFO("no " + kWorkingDirectory +
               " found, fallback: " + kWorkingDirectoryDefault);
    }
    LOG_INFO("daemonize process");
    if (DaemonizeProcess(working_directory) == -1) {
      LOG_INFO("could not daemonize process");
      exit(1);
    }
    Log::GetInstance()->SetLogfile(kLogPathDefault);
    if (config.Has(kLogPath) && config.IsString(kLogPath)) {
      Log::GetInstance()->SetLogfile(config.GetString(kLogPath));
    } else {
      LOG_INFO("no " + kLogPath + " found, fallback: " + kLogPathDefault);
    }
  }

  std::string data_path = kDbPathDefault;
  if (config.Has(kDbPath) && config.IsString(kDbPath)) {
    data_path = config.GetString(kDbPath);
  } else {
    LOG_INFO("no " + kDbPath + " found, fallback: " + kDbPathDefault);
  }

  std::string user_path = kUserPathDefault;
  if (config.Has(kUserPath) && config.IsString(kUserPath)) {
    user_path = config.GetString(kUserPath);
  } else {
    LOG_INFO("no " + kUserPath + " found, fallback: " + kUserPathDefault);
  }

  HttpServer server;

  LOG_INFO("set up services");
  server.RegisterService(db_api::kServiceDatabase,
                         new DocumentDatabase(data_path));
  server.RegisterService(db_api::kServiceUser, new UserPool(user_path));

  LOG_INFO("set up routes");
  server.RegisterHandler(HttpMethod::POST, db_api::kRouteInsert,
                         db_api::Insert);
  server.RegisterHandler(HttpMethod::POST, db_api::kRouteUpdate,
                         db_api::Update);
  server.RegisterHandler(HttpMethod::POST, db_api::kRouteErase, db_api::Erase);
  server.RegisterHandler(HttpMethod::POST, db_api::kRouteFind, db_api::Find);

  LOG_INFO("start server");
  std::string ip = kIpDefault;
  if (config.Has(kIp) && config.IsString(kIp)) {
    ip = config.GetString(kIp);
  } else {
    LOG_INFO("no " + kIp + " found, fallback: " + kIpDefault);
  }
  std::string port = kPortDefault;
  if (config.Has(kPort) && config.IsString(kPort)) {
    port = config.GetString(kPort);
  } else {
    LOG_INFO("no " + kPort + " found, fallback: " + kPortDefault);
  }

  server.Serve(port, ip);
  return 0;
}
