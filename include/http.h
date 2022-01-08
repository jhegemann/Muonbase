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

#ifndef HTTP_H
#define HTTP_H

#include <signal.h>
#include <string.h>
#include <sys/signalfd.h>
#include <sys/timerfd.h>

#include <atomic>
#include <deque>
#include <functional>
#include <map>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>

#include "log.h"
#include "service.h"
#include "tcp.h"

const std::string kHttpProtocol1_1 = "HTTP/1.1";
const std::string kHttpLineFeed = "\r\n";
const std::string kHttpDoubleLineFeed = "\r\n\r\n";
const long kHttpConnectionTimeout = 10000;
const long kHttpMaxHeaderCount = 128;
const unsigned int kHttpReservedSockets = 3;

enum HttpMethod {
  INVALID_METHOD = 0,
  POST,
  GET,
  HEAD,
  PUT,
  DELETE,
  CONNECT,
  UPDATE,
  TRACE,
  PATCH,
  OPTIONS
};

enum HttpContentType {
  INVALID_CONTENT_TYPE = 0,
  TEXT_HTML,
  TEXT_JAVASCRIPT,
  TEXT_CSV,
  APPLICATION_GZIP,
  APPLICATION_JSON,
  APPLICATION_OCTET_STREAM,
  APPLICATION_PDF,
  IMAGE_JPEG,
  IMAGE_PNG,
  MULTIPART_FORM_DATA
};

enum HttpStatus {
  CONTINUE = 100,
  SWITCHING_PROTOCOLS = 101,
  PROCESSING = 102,
  OK = 200,
  CREATED = 201,
  ACCEPTED = 202,
  NO_CONTENT = 204,
  MOVED_PERMANENTLY = 301,
  FOUND = 302,
  SEE_OTHER = 303,
  NOT_MODIFIED = 304,
  BAD_REQUEST = 400,
  UNAUTHORIZED = 401,
  FORBIDDEN = 403,
  NOT_FOUND = 404,
  METHOD_NOT_ALLOWED = 405,
  NOT_ACCEPTABLE = 406,
  REQUEST_TIMEOUT = 408,
  GONE = 410,
  LENGTH_REQUIRED = 411,
  REQUEST_ENTITY_TOO_LARGE = 413,
  REQUEST_URI_TOO_LONG = 414,
  UNSUPPORTED_MEDIA_TYPE = 415,
  EXPECTATION_FAILED = 417,
  UNPROCESSABLE_ENTITY = 422,
  LOCKED = 423,
  TOO_MANY_REQUESTS = 429,
  INTERNAL_SERVER_ERROR = 500,
  NOT_IMPLEMENTED = 501,
  BAD_GATEWAY = 502,
  SERVICE_UNAVAILABLE = 503,
};

class HttpConstants {
public:
  static std::string GetStatusString(int status);
  static std::string GetMethodString(const HttpMethod method);
  static HttpMethod GetMethod(const std::string &method_string);
  static std::string GetContentTypeString(const HttpContentType content_type);
  static HttpContentType GetContentType(const std::string &content_type_string);
};

class HttpPacket {
public:
  HttpPacket();
  virtual ~HttpPacket();
  void AddHeader(const std::string &key, const std::string &value);
  void AddHeader(const std::string &key, size_t value);
  const std::string &GetHeader(const std::string &key) const;
  size_t CountHeaders() const;
  void ClearHeaders();
  void SetBody(const std::string &body);
  void AppendToBody(const std::string &text);
  const std::string &GetBody() const;
  void ClearBody();

protected:
  std::map<std::string, std::string> headers_;
  std::string body_;
};

class HttpRequest : public HttpPacket {
public:
  HttpRequest();
  virtual ~HttpRequest();
  void Initialize();
  void SetMethod(const HttpMethod method);
  const HttpMethod &GetMethod() const;
  void SetUrl(const std::string &url);
  const std::string &GetUrl() const;
  void SetProtocol(const std::string &protocol);
  const std::string &GetProtocol() const;
  const std::string AsString() const;
  const std::string AsShortString() const;

private:
  HttpMethod method_;
  std::string url_;
  std::string protocol_;
};

class HttpResponse : public HttpPacket {
public:
  HttpResponse();
  virtual ~HttpResponse();
  void Initialize();
  void SetProtocol(const std::string &protocol);
  const std::string &GetProtocol() const;
  void SetStatus(const int status);
  int GetStatus() const;
  void SetMessage(const std::string &message);
  const std::string &GetMessage() const;
  static HttpResponse Build(const int status);
  static HttpResponse Build(const int status,
                            const HttpContentType content_type,
                            const std::string &body);
  const std::string AsString() const;
  const std::string AsShortString() const;

private:
  std::string protocol_;
  int status_;
  std::string message_;
};

enum HttpStage {
  START = 0,
  METHOD,
  STATUS,
  MESSAGE,
  URL,
  PROTOCOL,
  HEADER,
  BODY,
  END,
  FAILED
};

class HttpConnection {
public:
  HttpConnection(TcpSocket *socket);
  virtual ~HttpConnection();
  HttpStage GetStage() const;
  TcpReader *GetReader();
  TcpWriter *GetWriter();
  const HttpRequest &GetRequest();
  const HttpResponse &GetResponse();
  void ParseRequest();
  void ParseResponse();
  void Restart();
  bool IsGood();
  long GetExpiry();
  void ResetExpiry();

private:
  void ParseMessage(HttpPacket &packet);
  HttpRequest request_;
  HttpResponse response_;
  HttpStage stage_;
  size_t count_headers_;
  TcpReader *reader_;
  TcpWriter *writer_;
  TcpSocket *socket_;
  long expiry_;
};

typedef std::map<std::string, ApiService *> ServiceMap;
typedef std::function<HttpResponse(const HttpRequest &, ServiceMap services)>
    HttpCallback;

class HttpServer {
public:
  HttpServer();
  virtual ~HttpServer();
  void RegisterHandler(HttpMethod method, const std::string &url,
                       HttpCallback callback);
  void RegisterService(const std::string &name, ApiService *service);
  void Serve(const std::string &service, const std::string &host);

private:
  HttpResponse ExecuteHandler(const HttpRequest &request, ServiceMap services);
  bool SetupTimerDescriptor();
  bool SetupSignalDescriptor();
  bool SetupServerSocket(const std::string &service, const std::string &host);
  bool ClearTimer();
  bool ScheduleTimer(long duration);
  bool IsTimerScheduled();
  bool PopTimerEvent();
  bool PopSignalEvent();
  bool SignalReceived();
  void DeleteConnection(int descriptor);
  void DeleteAllConnections();
  void DeleteExpiredConnections();
  void HandleTimerError();
  void HandleTimerEvent();
  void HandleSignalError();
  void HandleSignalEvent();
  void HandleServerError(const std::string &service, const std::string &host);
  void HandleServerEvent();
  void HandleClientEvent(int index);
  std::atomic<bool> running_;
  TcpSocket server_socket_;
  std::map<std::string, HttpCallback> handlers_;
  EpollInstance epoll_instance_;
  std::map<int, HttpConnection *> connections_;
  sigset_t sigset_;
  int signal_descriptor_;
  struct signalfd_siginfo signal_info_;
  int timer_descriptor_;
  struct itimerspec timer_current_;
  struct itimerspec timer_schedule_;
  ServiceMap services_;
};

std::optional<HttpResponse>
SendRequest(const std::string &ip, const std::string &port, HttpMethod method,
            const std::string &url, const std::string &user = kStringEmpty,
            const std::string &password = kStringEmpty,
            HttpContentType content_type = INVALID_CONTENT_TYPE,
            const std::string &content = kStringEmpty);

#endif