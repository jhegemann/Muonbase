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

#include "tcp.h"

Epoll::Epoll() {
  memset(&event_, 0, sizeof(epoll_event));
  memset(&events_, 0, kEpollMaximumEvents * sizeof(epoll_event));
}

Epoll::~Epoll() {}

bool Epoll::Create() {
  instance_ = epoll_create1(0);
  if (instance_ == -1) {
    return false;
  }
  return true;
}

void Epoll::Release() { close(instance_); }

int Epoll::Wait(long timeout) {
  return epoll_wait(instance_, events_, kEpollMaximumEvents, timeout);
}

bool Epoll::Add(int descriptor, int flags) {
  event_.events = flags | EPOLLERR | EPOLLHUP;
  event_.data.fd = descriptor;
  if (epoll_ctl(instance_, EPOLL_CTL_ADD, descriptor, &event_) == -1) {
    return false;
  }
  return true;
}

bool Epoll::AddReadable(int descriptor) { return Add(descriptor, EPOLLIN); }

bool Epoll::AddWritable(int descriptor) { return Add(descriptor, EPOLLOUT); }

bool Epoll::AddDuplex(int descriptor) {
  return Add(descriptor, EPOLLIN | EPOLLOUT);
}

bool Epoll::Delete(int descriptor) {
  if (epoll_ctl(instance_, EPOLL_CTL_DEL, descriptor, NULL) == -1) {
    return false;
  }
  return true;
}

bool Epoll::Modify(int descriptor, int flags) {
  event_.events = flags;
  event_.data.fd = descriptor;
  if (epoll_ctl(instance_, EPOLL_CTL_MOD, descriptor, &event_) == -1) {
    return false;
  }
  return true;
}

int Epoll::GetDescriptor(size_t index) const {
  if (index >= kEpollMaximumEvents) {
    return -1;
  }
  return events_[index].data.fd;
}

int Epoll::GetEvents(size_t index) const {
  if (index >= kEpollMaximumEvents) {
    return -1;
  }
  return events_[index].events;
}

bool Epoll::IsReadable(size_t index) const {
  if (GetEvents(index) == -1) {
    return false;
  }
  return GetEvents(index) & EPOLLIN;
}

bool Epoll::IsWritable(size_t index) const {
  if (GetEvents(index) == -1) {
    return false;
  }
  return GetEvents(index) & EPOLLOUT;
}

bool Epoll::HasErrors(size_t index) const {
  if (GetEvents(index) == -1) {
    return false;
  }
  return GetEvents(index) & EPOLLERR || GetEvents(index) & EPOLLHUP;
}

bool Epoll::SetReadable(size_t index) {
  return Modify(GetDescriptor(index), EPOLLIN | EPOLLERR | EPOLLHUP);
}

bool Epoll::SetWriteable(size_t index) {
  return Modify(GetDescriptor(index), EPOLLOUT | EPOLLERR | EPOLLHUP);
}

bool Epoll::SetDuplex(size_t index) {
  return Modify(GetDescriptor(index), EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLHUP);
}

TcpSocket::TcpSocket()
    : host_(kStringEmpty),
      service_(kStringEmpty),
      descriptor_(-1),
      listening_(false),
      connected_(false) {}

TcpSocket::~TcpSocket() { Close(); }

void TcpSocket::Close() {
  if (listening_ || connected_ || descriptor_ != -1) {
    close(descriptor_);
  }
  descriptor_ = -1;
  listening_ = false;
  connected_ = false;
  host_ = kStringEmpty;
  service_ = kStringEmpty;
}

const std::string &TcpSocket::GetHost() const { return host_; }

const std::string &TcpSocket::GetService() const { return service_; }

int TcpSocket::GetDescriptor() const { return descriptor_; }

bool TcpSocket::WaitReceive(long timeout) {
  struct pollfd event;
  event.fd = descriptor_;
  event.events = POLLIN | POLLHUP | POLLERR;
  int ready = poll(&event, 1, timeout);
  if (ready <= 0) {
    return false;
  }
  if (event.revents & POLLIN) {
    return true;
  } else if (event.revents & POLLHUP || event.revents & POLLERR) {
    return false;
  }
  return false;
}

bool TcpSocket::WaitSend(long timeout) {
  struct pollfd event;
  event.fd = descriptor_;
  event.events = POLLOUT | POLLHUP | POLLERR;
  int ready = poll(&event, 1, timeout);
  if (ready <= 0) {
    return false;
  }
  if (event.revents & POLLOUT) {
    return true;
  } else if (event.revents & POLLHUP || event.revents & POLLERR) {
    return false;
  }
  return false;
}

bool TcpSocket::IsConnected() const { return connected_; }

bool TcpSocket::Connect(const std::string &service, const std::string &host) {
  Close();
  struct addrinfo hints;
  struct addrinfo *result, *iter;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_canonname = nullptr;
  hints.ai_addr = nullptr;
  hints.ai_next = nullptr;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  if (getaddrinfo(host.c_str(), service.c_str(), &hints, &result) != 0) {
    return false;
  }
  int sfd;
  for (iter = result; iter != nullptr; iter = iter->ai_next) {
    if ((sfd = socket(iter->ai_family, iter->ai_socktype, iter->ai_protocol)) ==
        -1) {
      continue;
    }
    if (connect(sfd, iter->ai_addr, iter->ai_addrlen) != -1) {
      break;
    }
    close(sfd);
  }
  freeaddrinfo(result);
  descriptor_ = (iter == nullptr) ? -1 : sfd;
  if (descriptor_ == -1) {
    return false;
  }
  host_ = host;
  service_ = service;
  connected_ = true;
  return true;
}

bool TcpSocket::IsListening() const { return listening_; }

bool TcpSocket::Listen(const std::string &service, const std::string &host) {
  Close();
  struct addrinfo hints;
  struct addrinfo *result, *iter;
  memset(&hints, 0, sizeof(struct addrinfo));
  socklen_t *addrlen = nullptr;
  hints.ai_canonname = nullptr;
  hints.ai_addr = nullptr;
  hints.ai_next = nullptr;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_UNSPEC;
  hints.ai_flags = AI_PASSIVE;
  if (getaddrinfo(host.c_str(), service.c_str(), &hints, &result) != 0) {
    return false;
  }
  int option_value = 1;
  int sfd;
  for (iter = result; iter != nullptr; iter = iter->ai_next) {
    if ((sfd = socket(iter->ai_family, iter->ai_socktype, iter->ai_protocol)) ==
        -1) {
      continue;
    }
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &option_value,
                   sizeof(option_value)) == -1) {
      close(sfd);
      freeaddrinfo(result);
      return false;
    }
    if (bind(sfd, iter->ai_addr, iter->ai_addrlen) == 0) {
      break;
    }
    close(sfd);
  }
  if (iter != nullptr) {
    if (listen(sfd, SOMAXCONN) == -1) {
      freeaddrinfo(result);
      return false;
    }
  }
  if (iter != nullptr && addrlen != nullptr) {
    *addrlen = iter->ai_addrlen;
  }
  freeaddrinfo(result);
  descriptor_ = (iter == nullptr) ? -1 : sfd;
  if (descriptor_ == -1) {
    return false;
  }
  host_ = kTcpLocalHost;
  service_ = service;
  listening_ = true;
  return true;
}

bool TcpSocket::IsBlocking() const {
  int flags = fcntl(descriptor_, F_GETFL, 0);
  if (flags == -1) {
    return false;
  }
  return !(flags & O_NONBLOCK);
}

bool TcpSocket::Unblock() {
  int flags = fcntl(descriptor_, F_GETFL, 0);
  if (flags == -1) {
    return false;
  }
  flags |= O_NONBLOCK;
  int err = fcntl(descriptor_, F_SETFL, flags);
  if (err == -1) {
    return false;
  }
  return true;
}

bool TcpSocket::Block() {
  int flags = fcntl(descriptor_, F_GETFL, 0);
  if (flags == -1) {
    return false;
  }
  flags &= ~O_NONBLOCK;
  int err = fcntl(descriptor_, F_SETFL, flags);
  if (err == -1) {
    return false;
  }
  return true;
}

bool TcpSocket::IsGood() const {
  int err;
  int option_value;
  socklen_t option_length = sizeof(int);
  err = getsockopt(descriptor_, SOL_SOCKET, SO_ERROR, &option_value,
                   &option_length);
  if (err || option_value) {
    return false;
  }
  return true;
}

TcpSocket *TcpSocket::Accept() {
  if (!IsListening() || !IsGood()) {
    return nullptr;
  }
  struct sockaddr address;
  socklen_t address_length = sizeof(address);
  memset(&address, 0, address_length);
  int cfd = accept(descriptor_, &address, &address_length);
  if (cfd == -1) {
    return nullptr;
  }
  char host[NI_MAXHOST];
  char service[NI_MAXSERV];
  if (getnameinfo(&address, address_length, host, sizeof(host), service,
                  sizeof(service), NI_NUMERICHOST | NI_NUMERICSERV) != 0) {
    return nullptr;
  }
  TcpSocket *client = new TcpSocket();
  client->descriptor_ = cfd;
  client->host_ = host;
  client->service_ = service;
  client->listening_ = false;
  client->connected_ = true;
  return client;
}

IoStatusCode TcpSocket::Receive(std::string &payload) {
  if (IsBlocking()) {
    return SOCKET_FLAGS;
  }
  if (!IsConnected()) {
    return NOT_CONNECTED;
  }
  if (!IsGood()) {
    return BAD;
  }
  ssize_t bytes;
  ssize_t length;
  char buffer[kTcpReceiveBufferSize];
  for (;;) {
    length = std::min(kTcpReceiveBufferSize,
                      kTcpMaximumPayloadSize - (long)payload.size());
    bytes = recv(descriptor_, buffer, length, 0);
    switch (bytes) {
      case -1:
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
          return BLOCKED;
        }
        if (errno == EINTR) {
          return INTERRUPTED;
        }
        return ERROR;
      case 0:
        return DISCONNECT;
      default:
        payload.insert(payload.end(), &buffer[0], &buffer[bytes]);
        if (payload.size() >= kTcpMaximumPayloadSize) {
          return OVERFLOW;
        }
        continue;
    }
  }
}

IoStatusCode TcpSocket::Send(std::string &payload) {
  if (IsBlocking()) {
    return SOCKET_FLAGS;
  }
  if (!IsConnected()) {
    return NOT_CONNECTED;
  }
  if (!IsGood()) {
    return BAD;
  }
  if (payload.size() > kTcpMaximumPayloadSize) {
    return OVERFLOW;
  }
  ssize_t bytes;
  ssize_t length;
  for (;;) {
    length = std::min(kTcpSendBufferSize, (long)payload.size());
    bytes = send(descriptor_, &payload[0], length, 0);
    switch (bytes) {
      case -1:
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
          return BLOCKED;
        }
        if (errno == EINTR) {
          return INTERRUPTED;
        }
        return ERROR;
      case 0:
        return ERROR;
      default:
        payload.erase(0, bytes);
        if (payload.empty()) {
          return SUCCESS;
        }
        continue;
    }
  }
}

TcpReader::TcpReader(TcpSocket *socket)
    : buffer_(kStringEmpty),
      socket_(socket),
      status_(NONE),
      peak_(0),
      base_(0),
      next_base_(0) {}

TcpReader::~TcpReader() {}

void TcpReader::ReadUntil(const std::string &token) {
  size_t start = 0;
  while (!StringContains(buffer_, token, start)) {
    start = buffer_.size();
    if (!socket_->WaitReceive(kTcpTimeout)) {
      status_ = EMPTY_BUFFER;
      break;
    }
    status_ = socket_->Receive(buffer_);
    if (HasErrors()) {
      break;
    }
  }
}

void TcpReader::ReadUntil(size_t length) {
  while (buffer_.length() < length) {
    if (!socket_->WaitReceive(kTcpTimeout)) {
      status_ = EMPTY_BUFFER;
      break;
    }
    status_ = socket_->Receive(buffer_);
    if (HasErrors()) {
      break;
    }
  }
}

bool TcpReader::HasErrors() const { return status_ != BLOCKED; }

void TcpReader::ReadSome() { status_ = socket_->Receive(buffer_); }

void TcpReader::SyncRead() {
  while (true) {
    if (!socket_->WaitReceive(kTcpTimeout)) {
      break;
    }
    status_ = socket_->Receive(buffer_);
    if (HasErrors()) {
      break;
    }
  }
}

size_t TcpReader::GetPosition(const std::string &token) const {
  return StringPosition(buffer_, token);
}

std::string TcpReader::PopAll() {
  std::string temp = buffer_;
  ClearBuffer();
  return temp;
}

IoStatusCode TcpReader::GetStatus() const { return status_; }

bool TcpReader::IsInBuffer(const std::string &token) const {
  return StringContains(buffer_, token);
}

void TcpReader::ClearBuffer() {
  buffer_.clear();
  status_ = NONE;
  peak_ = 0;
  base_ = 0;
  next_base_ = 0;
}

const std::string &TcpReader::GetBuffer() const { return buffer_; }

bool TcpReader::Peak(const std::string &token) {
  if ((peak_ = StringPosition(buffer_, token, base_)) == std::string::npos) {
    peak_ = base_;
    next_base_ = base_;
    return false;
  }
  next_base_ = peak_ + token.length();
  return true;
}

std::string TcpReader::Tok() {
  std::string result(buffer_.begin() + base_, buffer_.begin() + peak_);
  base_ = next_base_;
  peak_ = base_;
  return result;
}

std::string TcpReader::Tok(size_t length) {
  if (base_ == buffer_.length()) {
    return kStringEmpty;
  }
  if (base_ + length > buffer_.length()) {
    peak_ = buffer_.length();
  } else {
    peak_ = base_ + length;
  }
  std::string result(buffer_.begin() + base_, buffer_.begin() + peak_);
  base_ = peak_;
  next_base_ = peak_;
  return result;
}

TcpWriter::TcpWriter(TcpSocket *socket)
    : buffer_(kStringEmpty), socket_(socket), status_(NONE) {}

TcpWriter::~TcpWriter() {}

void TcpWriter::Write(const std::string &payload) {
  buffer_.insert(buffer_.end(), payload.begin(), payload.end());
}

void TcpWriter::Send() {
  while (!buffer_.empty()) {
    if (!socket_->WaitSend(kTcpTimeout)) {
      break;
    }
    status_ = socket_->Send(buffer_);
    if (HasErrors()) {
      break;
    }
  }
}

bool TcpWriter::HasErrors() const {
  return status_ != SUCCESS && status_ != BLOCKED;
}

void TcpWriter::SendSome() { status_ = socket_->Send(buffer_); }

IoStatusCode TcpWriter::GetStatus() const { return status_; }

bool TcpWriter::IsEmpty() const { return buffer_.empty(); }
