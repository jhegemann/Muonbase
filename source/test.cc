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
#include <optional>

#include "client.h"
#include "clock.h"
#include "http.h"
#include "log.h"
#include "trace.h"
#include "utils.h"

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

static const size_t kOrderDefault = 32;
static const size_t kCyclesDefault = 8;
static const size_t kThreadsDefault = 4;

static void PrintVersion() {
  std::cout << "Muonbase v1.0.0" << std::endl;
  std::cout << "Copyright 2022 Jonas Hegemann <jonas.hegemann@hotmail.de>"
            << std::endl;
}

static void PrintUsage() {
  std::cout << "Usage: muonbase-client [-h] [-n <threads>] [-t] [-i <ip>] "
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
  PrintVersion();
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

  Log::GetInstance()->SetVerbose(true);

  TcpSocket socket;
  if (!socket.Connect(port, ip)) {
    LOG_INFO("no service listening on " + ip + ":" + port);
    exit(0);
  }
  if (socket.IsConnected()) {
    LOG_INFO("available service found on " + ip + ":" + port);
  }
  socket.Close();

  if (!run) {
    LOG_INFO("dry run complete - restart with -t to run tests");
    exit(0);
  }

  std::vector<std::thread> threads(num_threads);
  for (size_t index = 0; index < num_threads; index++) {
    threads[index] = std::thread([index, ip, port, order, cycles] {
      Random random(123456789 + index);
      std::map<std::string, JsonObject> mirror;
      Client client(ip, port, kUserDefault, kPasswordDefault);
      Clock clock;
      size_t count;
      try {
        LOG_INFO("thread" + kStringSpace + std::to_string(index) +
                 kStringSpace + "started");

        for (size_t i = 1; i <= cycles; i++) {
          clock.Start();
          count = 0;
          for (size_t j = 0; j < order; j++) {
            JsonArray values = json::RandomObjectArray(random);
            JsonArray result = client.Insert(values);
            for (size_t k = 0; k < values.Size(); k++) {
              if (!result.IsString(k)) {
                throw std::runtime_error("non-string insert result");
              }
              mirror.insert(
                  std::make_pair(result.GetString(k), values.GetObject(k)));
            }
            count += result.Size();
          }
          clock.Stop();
          LOG_INFO("thread" + kStringSpace + std::to_string(index) +
                   kStringSpace + "cycle" + kStringSpace + std::to_string(i) +
                   kStringSpace + "took" + kStringSpace +
                   std::to_string(clock.Time() / count) + "ms" + kStringSpace +
                   "per insertion");

          clock.Start();
          for (size_t i = 0; i < order; i++) {
            auto it = mirror.begin();
            std::advance(it, random.UniformInteger() % mirror.size());
            std::string key = it->first;
            JsonObject value = json::RandomObject(random);
            JsonObject values;
            values.PutObject(key, value);
            JsonObject result = client.Update(values);
            if (!result.Has(key) || result.IsNull(key)) {
              throw std::runtime_error("update non-existent key");
            }
            if (!result.IsObject(key)) {
              throw std::runtime_error("return value is non-object");
            }
            if (it->second.String() != result.GetObject(key).String()) {
              throw std::runtime_error("return value differs from mirror");
            }
            it->second = value;
          }
          clock.Stop();
          LOG_INFO("thread" + kStringSpace + std::to_string(index) +
                   kStringSpace + "cycle" + kStringSpace + std::to_string(i) +
                   kStringSpace + "took" + kStringSpace +
                   std::to_string(clock.Time() / count) + "ms" + kStringSpace +
                   "per update");

          clock.Start();
          for (size_t i = 0; i < order; i++) {
            auto it = mirror.begin();
            std::advance(it, random.UniformInteger() % mirror.size());
            std::string key = it->first;
            JsonArray keys;
            keys.PutString(key);
            JsonArray result = client.Find(keys);
            if (result.Size() == 0 || !result.IsObject(0)) {
              LOG_INFO(result.String());
              throw std::runtime_error("could not find key ");
            }
            if (it->second.String() != result.GetObject(0).String()) {
              throw std::runtime_error("return value differs from mirror");
            }
          }
          clock.Stop();
          LOG_INFO("thread" + kStringSpace + std::to_string(index) +
                   kStringSpace + "cycle" + kStringSpace + std::to_string(i) +
                   kStringSpace + "took" + kStringSpace +
                   std::to_string(clock.Time() / count) + "ms" + kStringSpace +
                   "per lookup");

          clock.Start();
          for (size_t i = 0; i < order; i++) {
            auto it = mirror.begin();
            std::advance(it, random.UniformInteger() % mirror.size());
            std::string key = it->first;
            JsonArray keys;
            keys.PutString(key);
            JsonArray result = client.Erase(keys);
            if (result.Size() == 0 || !result.IsString(0)) {
              throw std::runtime_error("could not erase key");
            }
            it = mirror.erase(it);
          }
          clock.Stop();
          LOG_INFO("thread" + kStringSpace + std::to_string(index) +
                   kStringSpace + "cycle" + kStringSpace + std::to_string(i) +
                   kStringSpace + "took" + kStringSpace +
                   std::to_string(clock.Time() / count) + "ms" + kStringSpace +
                   "per erasure");
        }
      } catch (std::exception &e) {
        LOG_INFO("test failed: " + std::string(e.what()));
        abort();
      }
    });
  }
  for (size_t i = 0; i < num_threads; i++) {
    threads[i].join();
  }

  LOG_INFO("all tests passed");

  return 0;
}
