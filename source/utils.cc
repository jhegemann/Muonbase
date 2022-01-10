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

#include "utils.h"

std::string EncodeBase64(const std::string &to_encode) {
  const unsigned long predicted_len = 4 * ((to_encode.length() + 2) / 3);
  std::unique_ptr<char[]> output_buffer{
      std::make_unique<char[]>(predicted_len + 1)};
  const std::vector<unsigned char> vec_chars{to_encode.begin(),
                                             to_encode.end()};
  const int output_len =
      EVP_EncodeBlock(reinterpret_cast<unsigned char *>(output_buffer.get()),
                      vec_chars.data(), static_cast<int>(vec_chars.size()));
  if (output_len == -1) {
    throw std::runtime_error("could not base 64 encode bytes");
  }
  return output_buffer.get();
}

std::string DecodeBase64(const std::string &to_decode) {
  const unsigned long predicted_len = 3 * to_decode.length() / 4;
  const std::unique_ptr<char[]> output_buffer{
      std::make_unique<char[]>(predicted_len + 1)};
  const std::vector<unsigned char> vec_chars{to_decode.begin(),
                                             to_decode.end()};
  const int output_len =
      EVP_DecodeBlock(reinterpret_cast<unsigned char *>(output_buffer.get()),
                      vec_chars.data(), static_cast<int>(vec_chars.size()));
  if (output_len == -1) {
    throw std::runtime_error("could not base 64 encode bytes");
  }
  return output_buffer.get();
}

std::string Sha256Hash(const std::string &to_hash) {
  unsigned char hash[SHA256_DIGEST_LENGTH];
  SHA256_CTX sha256;
  SHA256_Init(&sha256);
  SHA256_Update(&sha256, to_hash.c_str(), to_hash.size());
  SHA256_Final(hash, &sha256);
  std::stringstream ss;
  for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
    ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
  }
  return ss.str();
}

bool CharIsAnyOf(char character, const std::string &charset) {
  for (size_t i = 0; i < charset.length(); i++) {
    if (character == charset[i]) {
      return true;
    }
  }
  return false;
}

bool ExpectString(const std::string &text, const std::string &what,
                  size_t &off) {
  size_t pos = off;
  while (pos < text.length()) {
    if (CharIsAnyOf(text[pos], "\b\t\n\a\r ")) {
      pos++;
      continue;
    }
    if (text.substr(pos, what.length()).compare(what) == 0) {
      off = pos + what.length();
      return true;
    }
    return false;
  }
  return false;
}

std::string DoubleToString(double number, int precision) {
  std::string result;
  double integer, fraction;
  fraction = modf(fabs(number), &integer);
  while (integer > 0) {
    result += kCharZero + (int)fmod(integer, 10);
    integer = floor(integer / 10);
  }
  std::reverse(result.begin(), result.end());
  if (result.empty()) {
    result = kStringZero;
  }
  result += kStringDot;
  int index = 0;
  while (fraction > 0 && index++ < precision) {
    fraction *= 10;
    fraction = modf(fraction, &integer);
    result += kCharZero + (int)integer;
  }
  if (!index) {
    result += kStringZero;
  }
  if (number < 0.0) {
    return kStringMinus + result;
  }
  return result;
}

double StringToDouble(const char *text) {
  double result = 0.0;
  int exponent = 0;
  int character;
  int sign = 1;
  if (*text != kCharNullTerminator && *text == kCharPlus) {
    text++;
  } else if (*text != kCharNullTerminator && *text == kCharMinus) {
    text++;
    sign = -1;
  }
  while ((character = *text++) != kCharNullTerminator &&
         (character >= kCharZero && character <= kCharNine)) {
    result = 10.0 * result + (character - kCharZero);
  }
  if (character == kCharDot) {
    while ((character = *text++) != '\0' &&
           (character >= kCharZero && character <= kCharNine)) {
      result = 10.0 * result + (character - kCharZero);
      exponent--;
    }
  }
  if (character == kCharExponentLower || character == kCharExponentUpper) {
    int exponent_sign = 1;
    int index = 0;
    character = *text++;
    if (character == kCharPlus) {
      character = *text++;
    } else if (character == kCharMinus) {
      character = *text++;
      exponent_sign = -1;
    }
    while (character >= kCharZero && character <= kCharNine) {
      index = 10 * index + (character - kCharZero);
      character = *text++;
    }
    exponent += index * exponent_sign;
  }
  while (exponent > 0) {
    result *= 10.0;
    exponent--;
  }
  while (exponent < 0) {
    result *= 0.1;
    exponent++;
  }
  return sign * result;
}

time_t StringToEpoch(const std::string &text, const std::string &format) {
  struct tm tm;
  strptime(text.c_str(), format.c_str(), &tm);
  return mktime(&tm);
}

std::string EpochToString(const time_t epoch, const std::string &format) {
  char buffer[32];
  strftime(buffer, 32, format.c_str(), localtime(&epoch));
  return std::string(buffer);
}

bool StringContains(const std::string &text, const std::string &token) {
  if (text.find(token) != std::string::npos) {
    return true;
  }
  return false;
}

bool StringContains(const std::string &text, const std::string &token,
                    size_t start) {
  if (start >= text.size()) {
    return false;
  }
  if (text.find(token, start) != std::string::npos) {
    return true;
  }
  return false;
}

bool StringStartsWith(const std::string &text, const std::string &token) {
  if (text.find(token) == 0) {
    return true;
  }
  return false;
}

bool StringStopsWith(const std::string &text, const std::string &token) {
  if (text.rfind(token) == 0) {
    return true;
  }
  return false;
}

std::string StringToLower(const std::string &text) {
  std::string lowercase = text;
  std::transform(lowercase.begin(), lowercase.end(), lowercase.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return lowercase;
}

std::string StringToUpper(const std::string &text) {
  std::string lowercase = text;
  std::transform(lowercase.begin(), lowercase.end(), lowercase.begin(),
                 [](unsigned char c) { return std::toupper(c); });
  return lowercase;
}

size_t StringPosition(const std::string &text, const std::string &token,
                      size_t start) {
  return text.find(token, start);
}

size_t StringReplace(std::string &text, const std::string &from,
                     const std::string &to) {
  size_t offset = 0;
  size_t number = 0;
  size_t position = std::string::npos;
  while ((position = text.find(from, offset)) != std::string::npos) {
    text.replace(position, from.length(), to);
    number++;
    offset = position + to.length();
  }
  return number;
}

void StringReplaceAll(std::string &text, const std::string &from,
                      const std::string &to) {
  if (StringContains(to, from)) {
    return;
  }
  while (StringReplace(text, from, to) > 0) {
    continue;
  }
}

void StringLtrim(std::string &text, const std::string &token) {
  text.erase(0, text.find_first_not_of(token));
}

void StringRtrim(std::string &text, const std::string &token) {
  text.erase(text.find_last_not_of(token) + 1);
}

void StringTrim(std::string &text, const std::string &token) {
  StringLtrim(text, token);
  StringRtrim(text, token);
}

void StringLtrimCharset(std::string &text, const std::string &charset) {
  size_t counter = 0;
  bool done = false;
  while (counter < text.length() && !done) {
    bool match = false;
    for (size_t i = 0; i < charset.length(); i++) {
      if (text[counter] == charset[i]) {
        match = true;
        break;
      }
    }
    if (match) {
      counter++;
    } else {
      done = true;
    }
  }
  text = text.substr(counter);
}

void StringRtrimCharset(std::string &text, const std::string &charset) {
  size_t counter = 0;
  bool done = false;
  while (counter < text.length() && !done) {
    bool match = false;
    size_t position = text.length() - 1 - counter;
    for (size_t i = 0; i < charset.length(); i++) {
      if (text[position] == charset[i]) {
        match = true;
        break;
      }
    }
    if (match) {
      counter++;
    } else {
      done = true;
    }
  }
  text = text.substr(0, text.length() - counter);
}

void StringTrimCharset(std::string &text, const std::string &charset) {
  StringLtrimCharset(text, charset);
  StringRtrimCharset(text, charset);
}

std::vector<std::string> StringExplode(const std::string &text,
                                       const std::string &delimiter) {
  std::vector<std::string> parts;
  std::string copy = text;
  StringTrim(copy, delimiter);
  size_t position;
  std::string segment;
  while ((position = copy.find(delimiter)) != std::string::npos) {
    segment = copy.substr(0, position);
    parts.push_back(segment);
    copy = copy.substr(position + delimiter.length());
  }
  segment = copy;
  parts.push_back(segment);
  std::vector<std::string>::iterator iter = parts.begin();
  while (iter != parts.end()) {
    if (iter->empty()) {
      iter = parts.erase(iter);
      continue;
    }
    iter++;
  }
  return parts;
}

std::string StringImplode(const std::vector<std::string> &segments,
                          const std::string &delimiter) {
  std::string joined(segments[0]);
  for (size_t i = 1; i < segments.size(); i++) {
    joined += delimiter;
    joined += segments[i];
  }
  return joined;
}

std::string StringPopSegment(std::string &text, const std::string &delimiter) {
  size_t position = text.find(delimiter);
  if (position == std::string::npos) {
    return kStringEmpty;
  }
  std::string segment = text.substr(0, position);
  text.erase(0, position + delimiter.length());
  return segment;
}

std::string StringPopSegment(std::string &text, size_t position) {
  if (position == std::string::npos) {
    return kStringEmpty;
  }
  if (position >= text.length()) {
    std::string segment = text;
    text = std::string();
    return segment;
  }
  std::string segment = text.substr(0, position);
  text.erase(0, position + 1);
  return segment;
}

std::string FileToString(const std::string &filename) {
  const size_t chunk_size = 4096;
  char buffer[chunk_size];
  std::string content;
  FILE *stream = fopen(filename.c_str(), "r");
  if (!stream) {
    return kStringEmpty;
  }
  size_t bytes_read;
  while ((bytes_read = fread(buffer, sizeof(char), chunk_size, stream)) > 0) {
    content.insert(content.length(), buffer, bytes_read);
  }
  fclose(stream);
  return content;
}

void StringToFile(const std::string &filename, const std::string &content) {
  const size_t chunk_size = 4096;
  FILE *stream = fopen(filename.c_str(), "w");
  if (!stream) {
    return;
  }
  size_t total_written = 0;
  size_t bytes_written;
  while (total_written < content.length()) {
    if ((bytes_written =
             fwrite(&content[total_written], sizeof(char),
                    std::min(chunk_size, content.length() - total_written),
                    stream)) < 1) {
      break;
    }
    total_written += bytes_written;
  }
  fclose(stream);
}

long TimeElapsedMilliseconds(struct timeval *from, struct timeval *to) {
  const suseconds_t sec = 1000000;
  if (from->tv_sec > to->tv_sec) {
    return 0;
  }
  if (from->tv_sec == to->tv_sec) {
    if (from->tv_usec > to->tv_usec) {
      return 0;
    }
    return (to->tv_usec - from->tv_usec) / 1000;
  }
  return ((sec - from->tv_usec) + (to->tv_sec - from->tv_sec - 1) * sec +
          (to->tv_usec)) /
         1000;
}

long TimeEpochMilliseconds() {
  struct timeval epoch;
  gettimeofday(&epoch, nullptr);
  return epoch.tv_sec * 1000 + epoch.tv_usec / 1000;
}

std::string StripFileExtension(const std::string &filename) {
  size_t position = filename.find_last_of(".");
  if (position == std::string::npos) {
    return kStringEmpty;
  }
  return filename.substr(0, position + 1);
}

bool IsDirectory(const std::string &path) {
  struct stat info;
  if (stat(path.c_str(), &info) == 0 && info.st_mode & S_IFDIR) {
    return true;
  }
  return false;
}

bool IsFile(const std::string &path) {
  struct stat info;
  if (stat(path.c_str(), &info) == 0 && info.st_mode & S_IFREG) {
    return true;
  }
  return false;
}

bool FileExists(const std::string &filename) {
  if (access(filename.c_str(), F_OK) != -1) {
    return true;
  }
  return false;
}

time_t FileModificationTime(const std::string &filename) {
  struct stat info;
  if (stat(filename.c_str(), &info) != 0) {
    return -1;
  }
  return info.st_mtim.tv_sec;
}

off_t FileSize(const std::string &filename) {
  struct stat info;
  if (stat(filename.c_str(), &info) != 0) {
    return (off_t)-1;
  }
  return info.st_size;
}

void CopyFile(const std::string &from, const std::string &to) {
  const size_t chunk_size = 4096;
  char buffer[chunk_size];
  ssize_t bytes_read;
  ssize_t bytes_written;
  FILE *source = fopen(from.c_str(), "r");
  FILE *destination = fopen(to.c_str(), "w");
  while ((bytes_read = fread(buffer, sizeof(char), chunk_size, source)) > 0) {
    bytes_written = fwrite(buffer, sizeof(char), bytes_read, destination);
    if (bytes_written == -1) {
      break;
    }
  }
  fclose(source);
  fclose(destination);
}

std::vector<std::string> FindFiles(const std::string &directory,
                                   const std::string &pattern) {
  std::vector<std::string> files;
  struct dirent **namelist;
  if (!IsDirectory(directory)) {
    return files;
  }
  int number = scandir(directory.c_str(), &namelist, nullptr, alphasort);
  if (number == -1) {
    return files;
  }
  for (int i = 0; i < number; i++) {
    std::string filename = namelist[i]->d_name;
    if (StringContains(filename, pattern)) {
      files.push_back(JoinPath(directory, filename));
    }
    free(namelist[i]);
  }
  free(namelist);
  return files;
}

std::string JoinPath(const std::string &directory,
                     const std::string &filename) {
  std::string folder = directory;
  StringRtrim(folder, kStringSlash);
  std::string file = filename;
  StringLtrim(file, kStringSlash);
  return folder + kStringSlash + file;
}

bool MakePath(const std::string &path, mode_t mode) {
  std::string copy = path;
  StringRtrim(copy, kStringSlash);
  int error = mkdir(copy.c_str(), mode);
  if (error == 0) {
    return true;
  }
  if (errno == ENOENT) {
    size_t position = copy.find_last_of(kStringSlash);
    if (position == std::string::npos) {
      return false;
    }
    if (!MakePath(copy.substr(0, position), mode)) {
      return false;
    }
    return 0 == mkdir(copy.c_str(), mode);
  } else if (errno == EEXIST) {
    return IsDirectory(copy);
  } else {
    return false;
  }
}

bool UnblockDescriptor(int descriptor) {
  int flags = fcntl(descriptor, F_GETFL, 0);
  if (flags == -1) {
    return false;
  }
  flags |= O_NONBLOCK;
  int error = fcntl(descriptor, F_SETFL, flags);
  if (error == -1) {
    return false;
  }
  return true;
}

std::string ExecuteProcess(const std::string &command) {
  const size_t chunk_size = 4096;
  char buffer[chunk_size];
  std::string output;
  FILE *stream = popen(command.c_str(), "r");
  if (!stream) {
    return kStringEmpty;
  }
  size_t bytes_read;
  while ((bytes_read = fread(buffer, sizeof(char), chunk_size, stream)) > 0) {
    output.insert(output.length(), buffer, bytes_read);
  }
  int err __attribute__((unused));
  err = pclose(stream);
  return output;
}

int DaemonizeProcess(const std::string &directory) {
  const std::string null_device = kNullDevice;
  const std::string current_directory = kStringDot;
  if (!IsDirectory(directory)) {
    return -1;
  }
  switch (fork()) {
  case -1:
    return -1;
  case 0:
    break;
  default:
    _exit(EXIT_SUCCESS);
  }
  if (setsid() == -1) {
    return -1;
  }
  switch (fork()) {
  case -1:
    return -1;
  case 0:
    break;
  default:
    _exit(EXIT_SUCCESS);
  }
  umask(0);
  if (strncmp(directory.c_str(), current_directory.c_str(), 1) != 0) {
    if (chdir(directory.c_str()) == -1) {
      return -1;
    }
  }
  int maxfd = sysconf(_SC_OPEN_MAX);
  if (maxfd == -1) {
    maxfd = 8192;
  }
  for (int fd = 0; fd < maxfd; fd++) {
    close(fd);
  }
  close(STDIN_FILENO);
  int fd = open(null_device.c_str(), O_RDWR);
  if (fd != STDIN_FILENO) {
    return -1;
  }
  if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO) {
    return -1;
  }
  if (dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO) {
    return -1;
  }
  return 0;
}
