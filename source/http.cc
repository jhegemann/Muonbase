#include "http.h"

std::string HttpConstants::GetStatusString(int status) {
  struct StaticMap : std::unordered_map<int, std::string> {
    StaticMap() {
      insert(std::make_pair(CONTINUE, "Continue"));
      insert(std::make_pair(SWITCHING_PROTOCOLS, "Switching Protocols"));
      insert(std::make_pair(PROCESSING, "Processing"));
      insert(std::make_pair(OK, "OK"));
      insert(std::make_pair(CREATED, "Created"));
      insert(std::make_pair(ACCEPTED, "Accepted"));
      insert(std::make_pair(NO_CONTENT, "No Content"));
      insert(std::make_pair(MOVED_PERMANENTLY, "Moved Permanently"));
      insert(std::make_pair(FOUND, "Found"));
      insert(std::make_pair(SEE_OTHER, "See Other"));
      insert(std::make_pair(NOT_MODIFIED, "Not Modfied"));
      insert(std::make_pair(BAD_REQUEST, "Bad Request"));
      insert(std::make_pair(UNAUTHORIZED, "Unauthorized"));
      insert(std::make_pair(FORBIDDEN, "Forbidden"));
      insert(std::make_pair(NOT_FOUND, "Not Found"));
      insert(std::make_pair(METHOD_NOT_ALLOWED, "Method Not Allowed"));
      insert(std::make_pair(NOT_ACCEPTABLE, "Not Acceptable"));
      insert(std::make_pair(REQUEST_TIMEOUT, "Request Timeout"));
      insert(std::make_pair(GONE, "Gone"));
      insert(std::make_pair(LENGTH_REQUIRED, "Length Required"));
      insert(
          std::make_pair(REQUEST_ENTITY_TOO_LARGE, "Request Entity Too Large"));
      insert(std::make_pair(REQUEST_URI_TOO_LONG, "Request URI Too Long"));
      insert(std::make_pair(UNSUPPORTED_MEDIA_TYPE, "Unsupported Media Type"));
      insert(std::make_pair(EXPECTATION_FAILED, "Expectation Failed"));
      insert(std::make_pair(UNPROCESSABLE_ENTITY, "Unprocessable Entity"));
      insert(std::make_pair(LOCKED, "Locked"));
      insert(std::make_pair(TOO_MANY_REQUESTS, "Too Many Requests"));
      insert(std::make_pair(INTERNAL_SERVER_ERROR, "Internal Server Error"));
      insert(std::make_pair(NOT_IMPLEMENTED, "Not Implemented"));
      insert(std::make_pair(BAD_GATEWAY, "Bad Gateway"));
      insert(std::make_pair(SERVICE_UNAVAILABLE, "Service Unavailable"));
    }
  } static status_to_string;
  auto it = status_to_string.find(status);
  if (it != status_to_string.end()) {
    return it->second;
  }
  return kStringEmpty;
}

std::string HttpConstants::GetMethodString(const HttpMethod method) {
  struct StaticMap : std::unordered_map<HttpMethod, std::string> {
    StaticMap() {
      insert(std::make_pair(POST, "POST"));
      insert(std::make_pair(GET, "GET"));
      insert(std::make_pair(HEAD, "HEAD"));
      insert(std::make_pair(PUT, "PUT"));
      insert(std::make_pair(DELETE, "DELETE"));
      insert(std::make_pair(CONNECT, "CONNECT"));
      insert(std::make_pair(UPDATE, "UPDATE"));
      insert(std::make_pair(TRACE, "TRACE"));
      insert(std::make_pair(PATCH, "PATCH"));
      insert(std::make_pair(OPTIONS, "OPTIONS"));
    }
  } static method_to_string;
  auto it = method_to_string.find(method);
  if (it != method_to_string.end()) {
    return it->second;
  }
  return kStringEmpty;
}

HttpMethod HttpConstants::GetMethod(const std::string &method_string) {
  struct StaticMap : std::unordered_map<std::string, HttpMethod> {
    StaticMap() {
      insert(std::make_pair("POST", POST));
      insert(std::make_pair("GET", GET));
      insert(std::make_pair("HEAD", HEAD));
      insert(std::make_pair("PUT", PUT));
      insert(std::make_pair("DELETE", DELETE));
      insert(std::make_pair("CONNECT", CONNECT));
      insert(std::make_pair("UPDATE", UPDATE));
      insert(std::make_pair("TRACE", TRACE));
      insert(std::make_pair("PATCH", PATCH));
      insert(std::make_pair("OPTIONS", OPTIONS));
    }
  } static string_to_method;
  auto it = string_to_method.find(method_string);
  if (it != string_to_method.end()) {
    return it->second;
  }
  return INVALID_METHOD;
}

std::string
HttpConstants::GetContentTypeString(const HttpContentType content_type) {
  struct StaticMap : std::unordered_map<HttpContentType, std::string> {
    StaticMap() {
      insert(std::make_pair(TEXT_HTML, "text/html"));
      insert(std::make_pair(TEXT_JAVASCRIPT, "text/javascript"));
      insert(std::make_pair(TEXT_CSV, "text/csv"));
      insert(std::make_pair(APPLICATION_GZIP, "application/gzip"));
      insert(std::make_pair(APPLICATION_JSON, "application/json"));
      insert(
          std::make_pair(APPLICATION_OCTET_STREAM, "application/octet-stream"));
      insert(std::make_pair(APPLICATION_PDF, "application/pdf"));
      insert(std::make_pair(IMAGE_JPEG, "image/jpeg"));
      insert(std::make_pair(IMAGE_PNG, "image/png"));
      insert(std::make_pair(MULTIPART_FORM_DATA, "multipart/form-data"));
    }
  } static content_type_to_string;
  auto it = content_type_to_string.find(content_type);
  if (it != content_type_to_string.end()) {
    return it->second;
  }
  return kStringEmpty;
}

HttpContentType
HttpConstants::GetContentType(const std::string &content_type_string) {
  struct StaticMap : std::unordered_map<std::string, HttpContentType> {
    StaticMap() {
      insert(std::make_pair("text/html", TEXT_HTML));
      insert(std::make_pair("text/javascript", TEXT_JAVASCRIPT));
      insert(std::make_pair("text/csv", TEXT_CSV));
      insert(std::make_pair("application/gzip", APPLICATION_GZIP));
      insert(std::make_pair("application/json", APPLICATION_JSON));
      insert(
          std::make_pair("application/octet-stream", APPLICATION_OCTET_STREAM));
      insert(std::make_pair("application/pdf", APPLICATION_PDF));
      insert(std::make_pair("image/jpeg", IMAGE_JPEG));
      insert(std::make_pair("image/png", IMAGE_PNG));
      insert(std::make_pair("multipart/form-data", MULTIPART_FORM_DATA));
    }
  } static string_to_content_type;
  auto it = string_to_content_type.find(content_type_string);
  if (it != string_to_content_type.end()) {
    return it->second;
  }
  return INVALID_CONTENT_TYPE;
}

HttpPacket::HttpPacket() : body_(kStringEmpty) {}

HttpPacket::~HttpPacket() {}

void HttpPacket::AddHeader(const std::string &key, const std::string &value) {
  headers_[StringToLower(key)] = value;
}

void HttpPacket::AddHeader(const std::string &key, size_t value) {
  headers_[StringToLower(key)] = std::to_string(value);
}

const std::string HttpPacket::GetHeader(const std::string &key) const {
  auto it = headers_.find(StringToLower(key));
  if (it != headers_.end()) {
    return it->second;
  }
  return kStringEmpty;
}

void HttpPacket::ClearHeaders() { headers_.clear(); }

void HttpPacket::SetBody(const std::string &body) { body_ = body; }

void HttpPacket::AppendToBody(const std::string &text) { body_.append(text); }

const std::string &HttpPacket::GetBody() const { return body_; }

void HttpPacket::ClearBody() { body_.clear(); }

HttpRequest::HttpRequest()
    : method_(GET), url_(kStringSlash), protocol_(kHttpProtocol1_1) {}

HttpRequest::~HttpRequest() {}

void HttpRequest::Initialize() {
  method_ = GET;
  url_ = kStringSlash;
  protocol_ = kHttpProtocol1_1;
  ClearHeaders();
  ClearBody();
}

void HttpRequest::SetMethod(const HttpMethod method) { method_ = method; }

const HttpMethod &HttpRequest::GetMethod() const { return method_; }

void HttpRequest::SetUrl(const std::string &url) { url_ = url; }

const std::string &HttpRequest::GetUrl() const { return url_; }

void HttpRequest::SetProtocol(const std::string &protocol) {
  protocol_ = protocol;
}

const std::string &HttpRequest::GetProtocol() const { return protocol_; }

const std::string HttpRequest::AsString() const {
  std::stringstream packet;
  packet << HttpConstants::GetMethodString(method_) << kStringSpace << url_
         << kStringSpace << protocol_ << kHttpLineFeed;
  for (auto it = headers_.begin(); it != headers_.end(); it++) {
    packet << it->first << kStringColon << kStringSpace << it->second
           << kHttpLineFeed;
  }
  packet << kHttpLineFeed;
  packet << body_;
  return packet.str();
}

const std::string HttpRequest::AsShortString() const {
  std::stringstream packet;
  packet << HttpConstants::GetMethodString(method_) << kStringSpace << url_
         << kStringSpace << protocol_ << kHttpLineFeed;
  for (auto it = headers_.begin(); it != headers_.end(); it++) {
    packet << it->first << kStringColon << kStringSpace << it->second
           << kHttpLineFeed;
  }
  packet << kHttpLineFeed;
  packet << body_.substr(0, std::min<size_t>(body_.length(), (size_t)1024));
  if (body_.length() > 1024) {
    packet << "...";
  }
  return packet.str();
}

HttpResponse::HttpResponse()
    : protocol_(kHttpProtocol1_1), status_(OK),
      message_(HttpConstants::GetStatusString(OK)) {}

HttpResponse::~HttpResponse() {}

void HttpResponse::Initialize() {
  protocol_ = kHttpProtocol1_1;
  status_ = OK;
  message_ = HttpConstants::GetStatusString(OK);
  ClearHeaders();
  ClearBody();
}

void HttpResponse::SetProtocol(const std::string &protocol) {
  protocol_ = protocol;
}

const std::string &HttpResponse::GetProtocol() const { return protocol_; }

void HttpResponse::SetStatus(const int status) { status_ = status; }

int HttpResponse::GetStatus() const { return status_; }

void HttpResponse::SetMessage(const std::string &message) {
  message_ = message;
}

const std::string &HttpResponse::GetMessage() const { return message_; }

HttpResponse HttpResponse::Build(const int status) {
  HttpResponse response;
  response.SetStatus(status);
  response.SetMessage(HttpConstants::GetStatusString(status));
  response.AddHeader("date", EpochToString(time(nullptr), "%Y%m%d%H%M%S"));
  response.AddHeader("server", "baseload/1");
  response.AddHeader("access-control-allow-origin", "*");
  response.AddHeader("access-control-allow-methods", "GET, POST");
  response.AddHeader("content-length", 0);
  return response;
}

HttpResponse HttpResponse::Build(const int status,
                                 const HttpContentType content_type,
                                 const std::string &body) {
  HttpResponse response;
  response.SetStatus(status);
  response.SetMessage(HttpConstants::GetStatusString(status));
  response.AddHeader("date", EpochToString(time(nullptr), "%Y%m%d%H%M%S"));
  response.AddHeader("server", "baseload/1");
  response.AddHeader("access-control-allow-origin", "*");
  response.AddHeader("access-control-allow-methods", "GET, POST");
  response.AddHeader("content-type",
                     HttpConstants::GetContentTypeString(content_type));
  response.AddHeader("content-length", body.length());
  response.SetBody(body);
  return response;
}

const std::string HttpResponse::AsString() const {
  std::stringstream packet;
  packet << protocol_ << kStringSpace << status_ << kStringSpace << message_
         << kHttpLineFeed;
  for (auto it = headers_.begin(); it != headers_.end(); it++) {
    packet << it->first << kStringColon << kStringSpace << it->second
           << kHttpLineFeed;
  }
  packet << kHttpLineFeed;
  packet << body_;
  return packet.str();
}

const std::string HttpResponse::AsShortString() const {
  std::stringstream packet;
  packet << protocol_ << kStringSpace << status_ << kStringSpace << message_
         << kHttpLineFeed;
  for (auto it = headers_.begin(); it != headers_.end(); it++) {
    packet << it->first << kStringColon << kStringSpace << it->second
           << kHttpLineFeed;
  }
  packet << kHttpLineFeed;
  packet << body_.substr(0, std::min<size_t>(body_.length(), (size_t)1024));
  if (body_.length() > 1024) {
    packet << "...";
  }
  return packet.str();
}

HttpConnection::HttpConnection(TcpSocket *socket)
    : stage_(START), count_headers_(0), socket_(socket),
      expiry_(TimeEpochMilliseconds() + kHttpConnectionTimeout) {
  reader_ = new TcpReader(socket);
  writer_ = new TcpWriter(socket);
}

HttpConnection::~HttpConnection() {
  delete reader_;
  delete writer_;
  socket_->Close();
}

HttpStage HttpConnection::GetStage() const { return stage_; }

TcpReader *HttpConnection::GetReader() { return reader_; }

TcpWriter *HttpConnection::GetWriter() { return writer_; }

const HttpRequest &HttpConnection::GetRequest() { return request_; }

const HttpResponse &HttpConnection::GetResponse() { return response_; }

long HttpConnection::GetExpiry() { return expiry_; }

void HttpConnection::ResetExpiry() {
  expiry_ = TimeEpochMilliseconds() + kHttpConnectionTimeout;
}

void HttpConnection::ParseRequest() {
  static std::string token;
  static HttpMethod method;
  switch (stage_) {
  case START:
  case METHOD:
    if (!reader_->Peak(kStringSpace)) {
      return;
    }
    token = reader_->Tok();
    if (token.empty()) {
      stage_ = FAILED;
      return;
    }
    method = HttpConstants::GetMethod(token);
    if (method == INVALID_METHOD) {
      stage_ = FAILED;
      return;
    }
    request_.SetMethod(method);
    stage_ = URL;
  case URL:
    if (!reader_->Peak(kStringSpace)) {
      return;
    }
    token = reader_->Tok();
    if (token.empty() || !StringStartsWith(token, kStringSlash) ||
        StringContains(token, kStringSlash + kStringSlash)) {
      stage_ = FAILED;
      return;
    }
    request_.SetUrl(token);
    stage_ = PROTOCOL;
  case PROTOCOL:
    if (!reader_->Peak(kHttpLineFeed)) {
      return;
    }
    token = reader_->Tok();
    if (token.compare(kHttpProtocol1_1) != 0) {
      stage_ = FAILED;
      return;
    }
    request_.SetProtocol(kHttpProtocol1_1);
    stage_ = HEADER;
  case HEADER:
  case BODY:
    ParseMessage(request_);
    break;
  default:
    return;
  }
}

void HttpConnection::ParseResponse() {
  static std::string token;
  static HttpStatus status;
  switch (stage_) {
  case START:
  case PROTOCOL:
    if (!reader_->Peak(kStringSpace)) {
      return;
    }
    token = reader_->Tok();
    if (token.compare(kHttpProtocol1_1) != 0) {
      stage_ = FAILED;
      return;
    }
    response_.SetProtocol(kHttpProtocol1_1);
    stage_ = STATUS;
  case STATUS:
    if (!reader_->Peak(kStringSpace)) {
      return;
    }
    token = reader_->Tok();
    try {
      status = static_cast<HttpStatus>(std::atoi(token.c_str()));
    } catch (std::invalid_argument &) {
      stage_ = FAILED;
      return;
    }
    if (HttpConstants::GetStatusString(status).empty()) {
      stage_ = FAILED;
      return;
    }
    response_.SetStatus(status);
    stage_ = MESSAGE;
  case MESSAGE:
    if (!reader_->Peak(kHttpLineFeed)) {
      return;
    }
    token = reader_->Tok();
    if (HttpConstants::GetStatusString(status).compare(token) != 0) {
      stage_ = FAILED;
      return;
    }
    response_.SetMessage(token);
    stage_ = HEADER;
  case HEADER:
  case BODY:
    ParseMessage(response_);
    break;
  default:
    return;
  }
}

void HttpConnection::ParseMessage(HttpPacket &packet) {
  static std::string token;
  static std::string key;
  static std::string value;
  static std::string content_length_string;
  static size_t content_length;
  static size_t bytes_left;
  switch (stage_) {
  case HEADER:
    while (count_headers_ < kMaxHttpHeaderCount) {
      if (!reader_->Peak(kHttpLineFeed)) {
        return;
      }
      token = reader_->Tok();
      if (token.empty()) {
        break;
      }
      key = StringPopSegment(token, kStringColon + kStringSpace);
      if (key.empty()) {
        stage_ = FAILED;
        return;
      }
      value = token;
      if (value.empty()) {
        stage_ = FAILED;
        return;
      }
      packet.AddHeader(key, value);
      count_headers_++;
    }
    if (count_headers_ > kMaxHttpHeaderCount) {
      stage_ = FAILED;
      return;
    }
    stage_ = BODY;
  case BODY:
    content_length_string = packet.GetHeader("content-length");
    content_length = 0;
    if (!content_length_string.empty()) {
      try {
        content_length = std::atoi(content_length_string.c_str());
      } catch (std::invalid_argument &) {
        stage_ = FAILED;
        return;
      }
    }
    if (content_length == 0) {
      stage_ = END;
      return;
    }
    bytes_left = content_length - packet.GetBody().length();
    packet.AppendToBody(reader_->Tok(bytes_left));
    if (packet.GetBody().length() < content_length) {
      return;
    }
    stage_ = END;
  case END:
    return;
  default:
    return;
  }
}

void HttpConnection::Restart() {
  ResetExpiry();
  stage_ = START;
  count_headers_ = 0;
  reader_->ClearBuffer();
}

bool HttpConnection::IsGood() { return socket_->IsGood(); }

HttpServer::HttpServer() : running_(false) {}

HttpServer::~HttpServer() {}

void HttpServer::RegisterHandler(HttpMethod method, const std::string &url,
                                 HttpCallback callback) {
  if (running_) {
    return;
  }
  std::string handler_id = HttpConstants::GetMethodString(method) + url;
  if (handlers_.find(handler_id) != handlers_.end()) {
    Log::GetInstance()->Info("handler already registered");
    return;
  }
  handlers_.insert(std::make_pair(handler_id, callback));
}

void HttpServer::RegisterService(const std::string &name, ApiService *service) {
  if (running_) {
    return;
  }
  if (services_.find(name) != services_.end()) {
    Log::GetInstance()->Info("service already registered");
    return;
  }
  services_.insert(std::make_pair(name, service));
}

HttpResponse HttpServer::ExecuteHandler(const HttpRequest &request,
                                        ServiceMap services) {
  HttpResponse response = HttpResponse::Build(NOT_FOUND);
  std::string handler_id(HttpConstants::GetMethodString(request.GetMethod()) +
                         request.GetUrl());
  auto it = handlers_.find(handler_id);
  if (it != handlers_.end()) {
    response = it->second(request, services);
  }
  return response;
}

void HttpServer::Serve(const std::string &service, const std::string &host) {
  for (auto service : services_) {
    Log::GetInstance()->Info("initialize service " + service.first);
    service.second->Initialize();
  }
  if (!epoll_instance_.Create()) {
    Log::GetInstance()->Info("cannot not set up epoll instance");
    return;
  }
  if (!SetupServerSocket(service, host)) {
    Log::GetInstance()->Info("cannot not set up server socket");
    return;
  }
  if (!epoll_instance_.AddReadableDescriptor(server_socket_.GetDescriptor())) {
    Log::GetInstance()->Info(
        "cannot not add listening socket to epoll instance");
    return;
  }
  Log::GetInstance()->Info("setup signal descriptor");
  if (!SetupSignalDescriptor()) {
    Log::GetInstance()->Info("cannot setup signal descriptor");
    return;
  }
  if (!epoll_instance_.AddReadableDescriptor(signal_descriptor_)) {
    Log::GetInstance()->Info("cannot add signal descriptor to epoll instance");
    return;
  }
  Log::GetInstance()->Info("setup timer descriptor");
  if (!SetupTimerDescriptor()) {
    Log::GetInstance()->Info("cannot setup timer descriptor");
    return;
  }
  if (!ScheduleTimer(kHttpConnectionTimeout)) {
    Log::GetInstance()->Info("cannot schedule timer");
    return;
  }
  if (!epoll_instance_.AddReadableDescriptor(timer_descriptor_)) {
    Log::GetInstance()->Info("cannot add timer descriptor to epoll instance");
    return;
  }
  running_ = true;
  while (running_) {
    int ready = epoll_instance_.Wait();
    for (size_t idx = 0; idx < (size_t)ready; idx++) {
      int current_descriptor = epoll_instance_.GetDescriptor(idx);
      if (timer_descriptor_ == current_descriptor) {
        if (epoll_instance_.HasErrors(idx)) {
          HandleTimerError();
          continue;
        }
        HandleTimerEvent();
      } else if (signal_descriptor_ == current_descriptor) {
        if (epoll_instance_.HasErrors(idx)) {
          HandleSignalError();
          continue;
        }
        HandleSignalEvent();
      } else if (server_socket_.GetDescriptor() == current_descriptor) {
        if (epoll_instance_.HasErrors(idx)) {
          HandleServerError(service, host);
          continue;
        }
        HandleServerEvent();
      } else {
        HandleClientEvent(idx);
      }
    }
  }
  Log::GetInstance()->Info("shut down services");
  for (auto service : services_) {
    Log::GetInstance()->Info("shut down service " + service.first);
    service.second->Shutdown();
    delete service.second;
  }
  Log::GetInstance()->Info("close timer descriptor");
  epoll_instance_.DeleteDescriptor(timer_descriptor_);
  close(timer_descriptor_);
  Log::GetInstance()->Info("close signal descriptor");
  epoll_instance_.DeleteDescriptor(signal_descriptor_);
  close(signal_descriptor_);
  Log::GetInstance()->Info("close server socket");
  server_socket_.Close();
  Log::GetInstance()->Info("delete connections");
  DeleteAllConnections();
  Log::GetInstance()->Info("release epoll instance");
  epoll_instance_.Release();
  running_ = false;
  Log::GetInstance()->Info("clean http server shutdown succeeded");
}

void HttpServer::HandleTimerError() {
  Log::GetInstance()->Info("error on timer descriptor; close timer descriptor");
  epoll_instance_.DeleteDescriptor(timer_descriptor_);
  close(timer_descriptor_);
  Log::GetInstance()->Info("setup timer descriptor");
  if (!SetupTimerDescriptor()) {
    Log::GetInstance()->Info("cannot setup timer descriptor");
    running_ = false;
    return;
  }
  if (!ScheduleTimer(kHttpConnectionTimeout)) {
    Log::GetInstance()->Info("cannot schedule timer");
    running_ = false;
    return;
  }
  if (!epoll_instance_.AddReadableDescriptor(timer_descriptor_)) {
    Log::GetInstance()->Info("cannot add timer descriptor to epoll instance");
    running_ = false;
    return;
  }
}

bool HttpServer::SetupTimerDescriptor() {
  if ((timer_descriptor_ = timerfd_create(CLOCK_MONOTONIC, 0)) == -1) {
    Log::GetInstance()->Info("cannot open timer descriptor");
    return false;
  }
  if (!UnblockDescriptor(timer_descriptor_)) {
    Log::GetInstance()->Info("cannot set timer descriptor to nonblocking mode");
    return false;
  }
  return true;
}

void HttpServer::HandleSignalError() {
  Log::GetInstance()->Info(
      "error on signal descriptor; close signal descriptor");
  epoll_instance_.DeleteDescriptor(signal_descriptor_);
  close(signal_descriptor_);
  Log::GetInstance()->Info("setup signal descriptor");
  if (!SetupSignalDescriptor()) {
    Log::GetInstance()->Info("cannot setup signal descriptor");
    running_ = false;
    return;
  }
  if (!epoll_instance_.AddReadableDescriptor(signal_descriptor_)) {
    Log::GetInstance()->Info("cannot add signal descriptor to epoll instance");
    running_ = false;
    return;
  }
}

bool HttpServer::SetupSignalDescriptor() {
  if (sigemptyset(&sigset_) == -1) {
    Log::GetInstance()->Info("cannot clear signal set");
    return false;
  }
  if (sigaddset(&sigset_, SIGINT) == -1 || sigaddset(&sigset_, SIGKILL) == -1 ||
      sigaddset(&sigset_, SIGTERM) == -1) {
    Log::GetInstance()->Info("cannot add signal to signal set");
    return false;
  }
  if (sigprocmask(SIG_BLOCK, &sigset_, nullptr) == -1) {
    Log::GetInstance()->Info("cannot block signals");
    return false;
  }
  if ((signal_descriptor_ = signalfd(-1, &sigset_, 0)) == -1) {
    Log::GetInstance()->Info("cannot open signal descriptor");
    return false;
  }
  if (!UnblockDescriptor(signal_descriptor_)) {
    Log::GetInstance()->Info(
        "cannot set signal descriptor to nonblocking mode");
    return false;
  }
  return true;
}

bool HttpServer::SetupServerSocket(const std::string &service,
                                   const std::string &host) {
  if (!server_socket_.Listen(service, host)) {
    return false;
  }
  server_socket_.Unblock();
  return true;
}

void HttpServer::DeleteConnection(int descriptor) {
  auto it_connection = connections_.find(descriptor);
  if (it_connection == connections_.end()) {
    return;
  }
  Log::GetInstance()->Info("delete connection " + std::to_string(descriptor));
  epoll_instance_.DeleteDescriptor(descriptor);
  delete it_connection->second;
  it_connection = connections_.erase(it_connection);
}

void HttpServer::DeleteAllConnections() {
  auto it_connection = connections_.begin();
  while (it_connection != connections_.end()) {
    Log::GetInstance()->Info("delete connection " +
                             std::to_string(it_connection->first));
    epoll_instance_.DeleteDescriptor(it_connection->first);
    delete it_connection->second;
    it_connection = connections_.erase(it_connection);
  }
  connections_.clear();
}

void HttpServer::DeleteExpiredConnections() {
  Log::GetInstance()->Info("check for expired connections");
  int descriptor;
  auto it_connection = connections_.begin();
  while (it_connection != connections_.end()) {
    if (it_connection->second->GetExpiry() <= TimeEpochMilliseconds()) {
      descriptor = it_connection->first;
      Log::GetInstance()->Info("delete expired connection " +
                               std::to_string(descriptor));
      epoll_instance_.DeleteDescriptor(descriptor);
      delete it_connection->second;
      it_connection = connections_.erase(it_connection);
    } else {
      it_connection++;
    }
  }
}

bool HttpServer::ClearTimer() {
  Log::GetInstance()->Info("clear timer");
  timer_schedule_.it_interval.tv_sec = 0;
  timer_schedule_.it_interval.tv_nsec = 0;
  timer_schedule_.it_value.tv_sec = 0;
  timer_schedule_.it_value.tv_nsec = 0;
  if (timerfd_settime(timer_descriptor_, 0, &timer_schedule_, 0) == -1) {
    Log::GetInstance()->Info("cannot clear timer");
    return false;
  }
  return true;
}

bool HttpServer::ScheduleTimer(long duration) {
  Log::GetInstance()->Info("schedule timer");
  timer_schedule_.it_interval.tv_sec = duration / 1000;
  timer_schedule_.it_interval.tv_nsec = 0;
  timer_schedule_.it_value.tv_sec = duration / 1000;
  timer_schedule_.it_value.tv_nsec = 0;
  if (timerfd_settime(timer_descriptor_, 0, &timer_schedule_, 0) == -1) {
    Log::GetInstance()->Info("cannot schedule timer");
    return false;
  }
  return true;
}

bool HttpServer::IsTimerScheduled() {
  if (timerfd_gettime(timer_descriptor_, &timer_current_) == -1) {
    return false;
  }
  if (timer_current_.it_interval.tv_sec == 0 &&
      timer_current_.it_interval.tv_nsec == 0 &&
      timer_current_.it_value.tv_sec == 0 &&
      timer_current_.it_value.tv_nsec == 0) {
    return false;
  }
  return true;
}

bool HttpServer::PopTimerEvent() {
  uint64_t expired = 0;
  ssize_t bytes = read(timer_descriptor_, &expired, sizeof(uint64_t));
  if (bytes == -1) {
    return false;
  }
  return true;
}

bool HttpServer::PopSignalEvent() {
  memset(&signal_info_, 0, sizeof(struct signalfd_siginfo));
  ssize_t bytes =
      read(signal_descriptor_, &signal_info_, sizeof(struct signalfd_siginfo));
  if (bytes == -1) {
    return false;
  }
  return true;
}

bool HttpServer::SignalReceived() {
  return (signal_info_.ssi_signo == SIGINT ||
          signal_info_.ssi_signo == SIGKILL ||
          signal_info_.ssi_signo == SIGTERM);
}

void HttpServer::HandleTimerEvent() {
  Log::GetInstance()->Info("event on timer descriptor");
  if (!PopTimerEvent()) {
    Log::GetInstance()->Info("error reading time from timer descriptor");
    return;
  }
  for (auto service : services_) {
    Log::GetInstance()->Info("tick service " + service.first);
    service.second->Tick();
  }
  DeleteExpiredConnections();
  std::string connection_list;
  if (connections_.size() == 0) {
    connection_list = "none";
  }
  std::string sep = kStringEmpty;
  for (auto it = connections_.begin(); it != connections_.end(); it++) {
    connection_list += sep + std::to_string(it->first);
    sep = ", ";
  }
  Log::GetInstance()->Info("open connections: " + connection_list);
}

void HttpServer::HandleSignalEvent() {
  Log::GetInstance()->Info("event on signal descriptor");
  if (!PopSignalEvent()) {
    Log::GetInstance()->Info(
        "error reading signal info from signal descriptor");
    return;
  }
  if (SignalReceived()) {
    Log::GetInstance()->Info("process stopped by signal");
    running_ = false;
    return;
  }
}

void HttpServer::HandleServerError(const std::string &service,
                                   const std::string &host) {
  Log::GetInstance()->Info(
      "error condition on server socket; close server socket");
  epoll_instance_.DeleteDescriptor(server_socket_.GetDescriptor());
  server_socket_.Close();
  Log::GetInstance()->Info("try to restart server socket");
  if (!SetupServerSocket(service, host)) {
    Log::GetInstance()->Info("cannot set up server socket");
    running_ = false;
    return;
  }
  if (!epoll_instance_.AddReadableDescriptor(server_socket_.GetDescriptor())) {
    Log::GetInstance()->Info("cannot add listening socket to epoll instance");
    running_ = false;
    return;
  }
  Log::GetInstance()->Info("server socket has been restarted");
  DeleteAllConnections();
  Log::GetInstance()->Info("all connections dropped");
}

void HttpServer::HandleServerEvent() {
  Log::GetInstance()->Info("event on server socket");
  if (connections_.size() >= kMaximumEvents - kReservedSockets) {
    Log::GetInstance()->Info("cannot accept more connections");
    return;
  }
  Log::GetInstance()->Info("accept a new client socket");
  TcpSocket *client_socket;
  client_socket = server_socket_.Accept();
  if (client_socket == nullptr) {
    Log::GetInstance()->Info("error accepting new client socket");
    return;
  }
  if (!epoll_instance_.AddReadableDescriptor(client_socket->GetDescriptor())) {
    Log::GetInstance()->Info("cannot add new client socket to epoll instance");
    client_socket->Close();
    delete client_socket;
    return;
  }
  client_socket->Unblock();
  HttpConnection *connection = new HttpConnection(client_socket);
  connections_.insert(
      std::make_pair(client_socket->GetDescriptor(), connection));
}

void HttpServer::HandleClientEvent(int index) {
  Log::GetInstance()->Info(
      "event on client socket - connection " +
      std::to_string(epoll_instance_.GetDescriptor(index)));
  auto lookup = connections_.find(epoll_instance_.GetDescriptor(index));
  if (lookup == connections_.end()) {
    Log::GetInstance()->Info("cannot not find connection");
    return;
  }
  int descriptor = lookup->first;
  HttpConnection *connection = lookup->second;
  if (!connection->IsGood() || epoll_instance_.HasErrors(index)) {
    Log::GetInstance()->Info(
        "error condition on client socket - remove client");
    DeleteConnection(descriptor);
    return;
  }
  if (epoll_instance_.IsReadable(index)) {
    connection->ResetExpiry();
    if (connection->GetStage() == END) {
      Log::GetInstance()->Info(
          "connection still readable though successfully parsed");
      DeleteConnection(descriptor);
      return;
    }
    connection->GetReader()->ReadSome();
    if (connection->GetReader()->HasErrors()) {
      Log::GetInstance()->Info("error condition on reader - connection closed");
      DeleteConnection(descriptor);
      return;
    }
    connection->ParseRequest();
    if (connection->GetStage() == FAILED) {
      Log::GetInstance()->Info("parsing of request failed");
      DeleteConnection(descriptor);
      return;
    }
    if (connection->GetStage() == END) {
      Log::GetInstance()->Info("incoming request: " +
                               connection->GetRequest().AsShortString());
      Log::GetInstance()->Info("execute handler for connection " +
                               std::to_string(descriptor));
      HttpResponse response =
          ExecuteHandler(connection->GetRequest(), services_);
      Log::GetInstance()->Info("response: " + response.AsShortString());
      connection->GetWriter()->Write(response.AsString());
      if (!epoll_instance_.SetWriteable(index)) {
        Log::GetInstance()->Info("could not set descriptor to write mode");
        DeleteConnection(descriptor);
        return;
      }
    }
  } else if (epoll_instance_.IsWritable(index)) {
    connection->ResetExpiry();
    connection->GetWriter()->SendSome();
    if (connection->GetWriter()->HasErrors()) {
      Log::GetInstance()->Info(
          "error occurred when sending response " +
          std::to_string(connection->GetWriter()->GetStatus()));
      DeleteConnection(descriptor);
      return;
    }
    if (connection->GetWriter()->IsEmpty()) {
      Log::GetInstance()->Info("response has been sent for connection " +
                               std::to_string(descriptor));
      if (connection->GetRequest()
              .GetHeader("connection")
              .compare("keep-alive") == 0) {
        Log::GetInstance()->Info("keep-alive request detected");
        connection->Restart();
        if (!epoll_instance_.SetReadable(index)) {
          Log::GetInstance()->Info("could not set descriptor to read mode");
          DeleteConnection(descriptor);
          return;
        }
        Log::GetInstance()->Info("connection restart due to keep-alive header");
        return;
      }
      DeleteConnection(descriptor);
      return;
    }
  } else {
    Log::GetInstance()->Info("unknown event on client socket");
    DeleteConnection(descriptor);
    return;
  }
}

std::optional<HttpResponse>
SendRequest(const std::string ip, const std::string port, HttpMethod method,
            const std::string &url, const std::string &user,
            const std::string &password, HttpContentType content_type,
            const std::string &content) {
  HttpRequest request;
  request.SetMethod(method);
  request.SetUrl(url);
  if (!user.empty() && !password.empty()) {
    request.AddHeader("authorization",
                      "Basic" + kStringSpace +
                          EncodeBase64(user + kStringColon + password));
  }
  if (content_type != INVALID_CONTENT_TYPE) {
    request.AddHeader("content-type",
                      HttpConstants::GetContentTypeString(content_type));
  }
  if (!content.empty()) {
    request.SetBody(content);
    request.AddHeader("content-length", content.length());
  }
  TcpSocket *socket = new TcpSocket();
  if (!socket->Connect(port, ip)) {
    return {};
  }
  socket->Unblock();
  HttpConnection connection(socket);
  connection.GetWriter()->Write(request.AsString());
  connection.GetWriter()->Send();
  while (connection.GetStage() != END) {
    connection.GetReader()->SyncRead(kTcpTimeout);
    connection.ParseResponse();
  }
  return connection.GetResponse();
}
