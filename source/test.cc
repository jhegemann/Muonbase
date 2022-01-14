/* Copyright [2022] [Jonas Hegemann, 26 Oct 1988]

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
#include "clock.h"
#include "http.h"
#include "log.h"
#include "utils.h"
#include <iostream>
#include <optional>
#include <unistd.h>

static const char kOptionRun = 't';
static const char kOptionIp = 'i';
static const char kOptionPort = 'p';
static const char kOptionOrder = 'o';
static const char kOptionCycles = 'c';
static const char kOptionHelp = 'h';
static const char kOptionThreads = 'n';
static const char *kOptionString = "htn:i:p:o:c:";

static const std::string kIp = "ip";
static const std::string kIpDefault = "127.0.0.1";
static const std::string kPort = "port";
static const std::string kPortDefault = "8260";
static const std::string kUserDefault = "root";
static const std::string kPasswordDefault = "0000";

static const size_t kOrderDefault = 128;
static const size_t kCyclesDefault = 16;
static const size_t kThreadsDefault = 8;

static void PrintUsage() {
  std::cout << "Usage: muonbase-client.app [-h] [-n <threads>] [-t] [-i <ip>] "
               "[-p <port>] [-o <order>] "
               "[-c <cycles>]"
            << std::endl;
  std::cout << "\t -h: help" << std::endl;
  std::cout << "\t -t: test " << std::endl;
  std::cout << "\t -n <threads>: threads - default " << kThreadsDefault
            << std::endl;
  std::cout << "\t -i <ip>: ip - default " << kIpDefault << std::endl;
  std::cout << "\t -p <port>: port - default " << kPortDefault << std::endl;
  std::cout << "\t -o <order>: order - default " << kOrderDefault << std::endl;
  std::cout << "\t -c <cycles>: cycles - default " << kCyclesDefault
            << std::endl;
}

int main(int argc, char **argv) {
  int option;
  bool run = false;
  std::string ip = kIpDefault;
  std::string port = kPortDefault;
  size_t order = kOrderDefault;
  size_t cycles = kCyclesDefault;
  size_t num_threads = kThreadsDefault;
  while ((option = getopt(argc, argv, kOptionString)) != -1) {
    switch (option) {
    case kOptionRun:
      run = true;
      break;
    case kOptionIp:
      ip = optarg;
      break;
    case kOptionPort:
      port = optarg;
      break;
    case kOptionThreads:
      try {
        num_threads = std::atoi(optarg);
      } catch (std::invalid_argument &) {
        PrintUsage();
        exit(1);
      }
      break;
    case kOptionHelp:
      PrintUsage();
      exit(0);
    case kOptionOrder:
      try {
        order = std::atoi(optarg);
      } catch (std::invalid_argument &) {
        PrintUsage();
        exit(1);
      }
      break;
    case kOptionCycles:
      try {
        cycles = std::atoi(optarg);
      } catch (std::invalid_argument &) {
        PrintUsage();
        exit(1);
      }
      break;
    case kCharColon:
      Log::GetInstance()->Info("option needs a value");
      PrintUsage();
      exit(1);
    case kCharQuestionMark:
      Log::GetInstance()->Info("unknown option " + std::string(optopt, 1));
      PrintUsage();
      exit(1);
    default:
      PrintUsage();
      exit(0);
    }
  }

  Log::GetInstance()->SetVerbose(true);

  TcpSocket socket;
  if (!socket.Connect(port, ip)) {
    Log::GetInstance()->Info("no service listening on " + ip + ":" + port);
    exit(0);
  }
  if (socket.IsConnected()) {
    Log::GetInstance()->Info("available service found on " + ip + ":" + port);
  }
  socket.Close();

  if (!run) {
    Log::GetInstance()->Info("dry run complete - restart with -t to run tests");
    exit(0);
  }

  std::vector<std::thread> threads(num_threads);
  for (size_t index = 0; index < num_threads; index++) {
    threads[index] = std::thread([index, ip, port, order, cycles] {
      RandomGenerator random(index);
      std::map<std::string, JsonObject> mirrored_state;
      Client client(ip, port, kUserDefault, kPasswordDefault);
      Clock clock;
      size_t count;
      try {
        Log::GetInstance()->Info("thread" + kStringSpace +
                                 std::to_string(index) + kStringSpace +
                                 "fill db");
        clock.Start();
        count = 0;
        for (size_t i = 0; i < order; i++) {
          JsonArray values = RandomDocumentArray();
          JsonArray keys = client.Insert(values);
          for (size_t j = 0; j < keys.Size(); j++) {
            mirrored_state.insert(
                std::make_pair(keys.GetString(j), values.GetObject(j)));
          }
          count += keys.Size();
        }
        clock.Stop();
        Log::GetInstance()->Info(
            "thread" + kStringSpace + std::to_string(index) + kStringSpace +
            "took" + kStringSpace + std::to_string(clock.Time() / count) +
            "ms" + kStringSpace + "per insertion");

        for (size_t i = 1; i <= cycles; i++) {
          clock.Start();
          size_t count = 0;
          for (size_t j = 0; j < order; j++) {
            JsonArray values = RandomDocumentArray();
            JsonArray keys = client.Insert(values);
            for (size_t k = 0; k < keys.Size(); k++) {
              mirrored_state.insert(
                  std::make_pair(keys.GetString(k), values.GetObject(k)));
            }
            count += keys.Size();
          }
          clock.Stop();
          Log::GetInstance()->Info(
              "thread" + kStringSpace + std::to_string(index) + kStringSpace +
              "cycle" + kStringSpace + std::to_string(i) + kStringSpace +
              "took" + kStringSpace + std::to_string(clock.Time() / count) +
              "ms" + kStringSpace + "per insertion");

          clock.Start();
          for (size_t j = 0; j < count; j++) {
            auto it = mirrored_state.begin();
            std::advance(it, random.Uint64() % mirrored_state.size());
            std::string key = it->first;
            it = mirrored_state.erase(it);
            JsonArray keys;
            keys.PutString(key);
            client.Erase(keys);
          }
          clock.Stop();
          Log::GetInstance()->Info(
              "thread" + kStringSpace + std::to_string(index) + kStringSpace +
              "cycle" + kStringSpace + std::to_string(i) + kStringSpace +
              "took" + kStringSpace + std::to_string(clock.Time() / count) +
              "ms" + kStringSpace + "per erasure");
        }
      } catch (std::runtime_error &) {
        Log::GetInstance()->Info("test failed");
        return;
      }
    });
  }
  for (size_t i = 0; i < num_threads; i++) {
    threads[i].join();
  }

  try {
    Client client(ip, port, kUserDefault, kPasswordDefault);
    JsonArray keys = client.Keys();
    JsonArray values = client.Values();
    JsonObject image = client.Image();
    JsonArray added_keys = client.Insert(RandomDocumentArray());
    client.Erase(added_keys);
    client.Find(keys);
  } catch (std::runtime_error &) {
    Log::GetInstance()->Info("test failed");
  }

  Log::GetInstance()->Info("all tests passed");

  return 0;
}
