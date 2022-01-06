#include "client.h"
#include "http.h"
#include "log.h"
#include "utils.h"
#include <iostream>
#include <optional>
#include <unistd.h>

int main(int argc, char **argv) {
  int option;
  std::string ip = "127.0.0.1";
  std::string port = "8260";
  while ((option = getopt(argc, argv, "i:p:")) != -1) {
    switch (option) {
    case 'i':
      ip = optarg;
      break;
    case 'p':
      port = optarg;
      break;
    case ':':
      Log::GetInstance()->Info("option needs a value");
      exit(0);
    case '?':
      Log::GetInstance()->Info("unknown option " + std::string(optopt, 1));
      exit(0);
    default:
      printf("Usage: %s [-i <ip>] [-p <port>].\n", argv[0]);
      exit(0);
    }
  }

  Log::GetInstance()->SetVerbose(true);

  Client client(ip, port);
  const size_t count = 128;
  try {
    Log::GetInstance()->Info("TEST INIT");
    client.RandomInsert(count);
    client.CompleteLookup();
    for (size_t i = 1; i <= 8; i++) {
      Log::GetInstance()->Info("TEST CYCLE " + std::to_string(i));
      client.RandomInsert(count);
      client.CompleteLookup();
      client.RandomRemove(count);
      client.CompleteLookup();
    }
  } catch (std::runtime_error&) {
    Log::GetInstance()->Info("TEST FAILED!");
    exit(1);
  }
  Log::GetInstance()->Info("ALL TESTS PASSED!");

  return 0;
}
