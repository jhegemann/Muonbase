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

#ifndef TCP_H
#define TCP_H

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdint.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "utils.h"

const unsigned kEpollMaximumEvents = 256;

class EpollInstance {
public:
  EpollInstance();
  virtual ~EpollInstance();
  bool Create();
  void Release();
  int Wait(long timeout = -1);
  bool AddDescriptor(int descriptor, int flags);
  bool AddReadableDescriptor(int descriptor);
  bool AddWritableDescriptor(int descriptor);
  bool AddDuplexDescriptor(int descriptor);
  bool DeleteDescriptor(int descriptor);
  bool ModifyDescriptor(int descriptor, int flags);
  bool SetReadable(size_t index);
  bool SetWriteable(size_t index);
  bool SetDuplex(size_t index);
  int GetDescriptor(size_t index);
  int GetEvents(size_t index);
  bool IsReadable(size_t index);
  bool IsWritable(size_t index);
  bool HasErrors(size_t index);

private:
  int instance_;
  epoll_event event_;
  epoll_event events_[kEpollMaximumEvents];
};

const std::string kTcpLocalHost = "127.0.0.1";
const long kTcpReceiveBufferSize = 65536L;
const long kTcpSendBufferSize = 65536L;
const long kTcpMaximumPayloadSize = 1073741824L;
const long kTcpTimeout = 1000L;

enum IoStatusCode {
  SUCCESS = 0,
  NONE,
  ERROR,
  DISCONNECT,
  BLOCKED,
  TIMEOUT,
  OVERFLOW,
  SOCKET_FLAGS,
  NOT_LISTENING,
  NOT_CONNECTED,
  BAD,
  INTERRUPTED,
  EMPTY_BUFFER
};

class TcpSocket {
public:
  TcpSocket();
  virtual ~TcpSocket();
  void Close();
  const std::string &GetHost() const;
  const std::string &GetService() const;
  int GetDescriptor() const;
  bool WaitReceive(long timeout = 0);
  bool WaitSend(long timeout = 0);
  bool IsConnected();
  bool Connect(const std::string &service, const std::string &host);
  bool IsListening();
  bool Listen(const std::string &service, const std::string &host);
  bool IsBlocking();
  bool Unblock();
  bool Block();
  bool IsGood();
  TcpSocket *Accept();
  IoStatusCode Receive(std::string &payload, long timeout = 0);
  IoStatusCode Send(std::string &payload, long timeout = 0);

private:
  std::string host_;
  std::string service_;
  int descriptor_;
  bool listening_;
  bool connected_;
};

class TcpReader {
public:
  TcpReader(TcpSocket *socket);
  virtual ~TcpReader();
  void ReadUntil(const std::string &token, long max_idle = kTcpTimeout);
  void ReadUntil(size_t length, long max_idle = kTcpTimeout);
  void ReadSome(long timeout = 0);
  void SyncRead(long timeout);
  IoStatusCode GetStatus();
  size_t GetPosition(const std::string &token);
  std::string PopAll();
  bool IsInBuffer(const std::string &token);
  void ClearBuffer();
  const std::string &GetBuffer();
  bool Peak(const std::string &token);
  std::string Tok();
  std::string Tok(size_t length);
  bool HasErrors();

private:
  std::string buffer_;
  TcpSocket *socket_;
  IoStatusCode status_;
  size_t peak_;
  size_t base_;
  size_t next_base_;
};

class TcpWriter {
public:
  TcpWriter(TcpSocket *socket);
  virtual ~TcpWriter();
  void Write(const std::string &payload);
  void Send();
  void SendSome();
  IoStatusCode GetStatus();
  bool IsEmpty();
  bool HasErrors();

private:
  std::string buffer_;
  TcpSocket *socket_;
  IoStatusCode status_;
};

#endif