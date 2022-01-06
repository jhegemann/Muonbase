#include "api.h"
#include "http.h"
#include "log.h"
#include "service.h"
#include "utils.h"
#include <iostream>
#include <unistd.h>

int main(int argc, char **argv) {
  int option;
  JsonObject config;
  bool daemonize = false;
  bool config_available = false;
  bool verbose = false;
  while ((option = getopt(argc, argv, "vdc:")) != -1) {
    switch (option) {
    case 'v':
      verbose = true;
      break;
    case 'c':
      std::cout << optarg << std::endl;
      config.FromString(FileToString(optarg));
      config_available = true;
      break;
    case 'd':
      daemonize = true;
      break;
    case ':':
      Log::GetInstance()->Info("option needs a value");
      exit(1);
    case '?':
      Log::GetInstance()->Info("unknown option " + std::string(optopt, 1));
      exit(1);
    default:
      printf("Usage: %s [-d] [-c <config>].\n", argv[0]);
      exit(0);
    }
  }

  if (!config_available) {
    printf("Usage: %s [-v] [-d] [-c <config>].\n", argv[0]);
    printf("\t -v : verbose\n");
    printf("\t -d : daemon\n");
    printf("\t -c <config> : configuration in json format\n");
    exit(0);
  }

  std::cout << config.AsString() << std::endl;

  if (verbose) {
    Log::GetInstance()->SetVerbose(true);
  }

  std::string working_directory = ".";
  if (daemonize) {
    if (config.Has("working_directory") &&
        config.IsString("working_directory")) {
      working_directory = config.GetAsString("working_directory");
    }
    Log::GetInstance()->Info("daemonize process");
    DaemonizeProcess(working_directory);
    if (config.Has("log_path") && config.IsString("log_path")) {
      Log::GetInstance()->SetLogfile(config.GetAsString("log_path"));
    }
  }

  std::string data_path = "core.db";
  if (config.Has("data_path") && config.IsString("data_path")) {
    data_path = config.GetAsString("data_path");
  }

  std::string user_path = "users.json";
  if (config.Has("user_path") && config.IsString("user_path")) {
    user_path = config.GetAsString("user_path");
  }

  HttpServer server;

  Log::GetInstance()->Info("set up services");
  server.RegisterService("db", new DocumentDatabase(data_path));
  server.RegisterService("user", new UserPool(user_path));

  Log::GetInstance()->Info("set up routes");
  server.RegisterHandler(HttpMethod::POST, "/insert", db_api::Insert);
  server.RegisterHandler(HttpMethod::POST, "/remove", db_api::Remove);
  server.RegisterHandler(HttpMethod::POST, "/find", db_api::Find);
  server.RegisterHandler(HttpMethod::GET, "/keys", db_api::Keys);
  server.RegisterHandler(HttpMethod::GET, "/dump", db_api::Dump);

  Log::GetInstance()->Info("start server");
  std::string ip = "127.0.0.1";
  if (config.Has("ip") && config.IsString("ip")) {
    ip = config.GetAsString("ip");
  }
  std::string port = "8080";
  if (config.Has("port") && config.IsString("port")) {
    port = config.GetAsString("port");
  }

  server.Serve(port, ip);
  return 0;
}
