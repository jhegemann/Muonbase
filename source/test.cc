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
  const size_t count = 1024;
  const size_t cycles = 8;
  try {
    Log::GetInstance()->Info("TEST INIT");
    client.RandomInsert(count);
    client.CompleteLookup();
    for (size_t i = 1; i <= cycles; i++) {
      Log::GetInstance()->Info("TEST CYCLE " + std::to_string(i));
      client.RandomInsert(count / cycles);
      client.CompleteLookup();
      client.RandomRemove(count / cycles);
      client.CompleteLookup();
    }
  } catch (std::runtime_error &) {
    Log::GetInstance()->Info("TEST FAILED!");
    exit(1);
  }
  Log::GetInstance()->Info("ALL TESTS PASSED!");

  return 0;
}
