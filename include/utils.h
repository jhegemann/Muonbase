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

#ifndef UTILS_H
#define UTILS_H

#include <dirent.h>
#include <fcntl.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

const std::string kStringEmpty = "";
const std::string kStringSpace = " ";
const std::string kStringDoubleSpace = "  ";
const std::string kStringLineFeed = "\n";
const std::string kStringCarriageReturn = "\r";
const std::string kStringWindowsLineFeed = "\r\n";
const std::string kStringTab = "\t";
const std::string kStringSlash = "/";
const std::string kStringColon = ":";
const std::string kStringSemicolon = ";";
const std::string kStringComma = ",";
const std::string kStringDot = ".";
const std::string kStringDots = "...";
const std::string kStringUnderscore = "_";
const std::string kStringZero = "0";
const std::string kStringMinus = "-";
const std::string kStringPlus = "+";
const std::string kStringCurlyBracketOpen = "{";
const std::string kStringCurlyBracketClose = "}";
const std::string kStringSquareBracketOpen = "[";
const std::string kStringSquareBracketClose = "]";
const std::string kStringDoubleQuote = "\"";
const std::string kStringSingleQuote = "'";
const std::string kStringPipe = "|";
const std::string kStringWss = "\b\t\n\a\r ";
const std::string kStringAlphabet = "abcdefghijklmnopqrstuvwxyz";
const std::string kStringDigits = "0123456789";

const char kCharZero = '0';
const char kCharOne = '1';
const char kCharTwo = '2';
const char kCharThree = '3';
const char kCharFour = '4';
const char kCharFive = '5';
const char kCharSix = '6';
const char kCharSeven = '7';
const char kCharEight = '8';
const char kCharNine = '9';
const char kCharMinus = '-';
const char kCharPlus = '+';
const char kCharExponentUpper = 'E';
const char kCharExponentLower = 'e';
const char kCharNullTerminator = '\0';
const char kCharDot = '.';
const char kCharComma = ',';
const char kCharColon = ':';
const char kCharQuestionMark = '?';
const char kCharDoubleQuote = '\"';
const char kCharSingleQuote = '\'';
const char kCharCurlyBracketOpen = '{';
const char kCharCurlyBracketClose = '}';
const char kCharSquareBracketOpen = '[';
const char kCharSquareBracketClose = ']';
const char kCharN = 'n';
const char kCharT = 't';
const char kCharF = 'f';

const std::string kReadMode = "r";
const std::string kWriteMode = "w";

const std::string kNullDevice = "/dev/null";

std::string EncodeBase64(const std::string &to_encode);
std::string DecodeBase64(const std::string &to_decode);
std::string Sha256Hash(const std::string &to_hash);
bool CharIsAnyOf(char character, const std::string &charset);
bool StringExpect(const std::string &text, const std::string &what,
                  size_t &offset);
std::string DoubleToString(double number, int precision = 6);
double StringToDouble(const char *text);
time_t StringToEpoch(const std::string &text, const std::string &format);
std::string EpochToString(const time_t epoch, const std::string &format);
bool StringContains(const std::string &text, const std::string &token);
bool StringContains(const std::string &text, const std::string &token,
                    size_t start);
bool StringStartsWith(const std::string &text, const std::string &token);
bool StringStopsWith(const std::string &text, const std::string &token);
std::string StringToLower(const std::string &text);
std::string StringToUpper(const std::string &text);
size_t StringPosition(const std::string &text, const std::string &token,
                      size_t start = 0);
size_t StringReplace(std::string &text, const std::string &from,
                     const std::string &to);
void StringReplaceAll(std::string &text, const std::string &from,
                      const std::string &to);
void StringLtrim(std::string &text, const std::string &token);
void StringRtrim(std::string &text, const std::string &token);
void StringTrim(std::string &text, const std::string &token);
void StringLtrimCharset(std::string &text, const std::string &charset);
void StringRtrimCharset(std::string &text, const std::string &charset);
void StringTrimCharset(std::string &text, const std::string &charset);
std::vector<std::string> StringExplode(const std::string &text,
                                       const std::string &delimiter);
std::string StringImplode(const std::vector<std::string> &segments,
                          const std::string &delimiter);
std::string StringPopSegment(std::string &text, const std::string &delimiter);
std::string StringPopSegment(std::string &text, size_t position);
std::string FileToString(const std::string &filename);
void StringToFile(const std::string &filename, const std::string &content);
long TimeElapsedMilliseconds(struct timeval *from, struct timeval *to);
long TimeEpochMilliseconds();
std::string StripFileExtension(const std::string &filename);
bool IsDirectory(const std::string &path);
bool IsFile(const std::string &path);
bool FileExists(const std::string &filename);
time_t FileModificationTime(const std::string &filename);
off_t FileSize(const std::string &filename);
void CopyFile(const std::string &from, const std::string &to);
std::vector<std::string> FindFiles(const std::string &directory,
                                   const std::string &pattern = kStringEmpty);
std::string JoinPath(const std::string &directory, const std::string &filename);
bool MakePath(const std::string &path, mode_t mode);
bool UnblockDescriptor(int descriptor);
std::string ExecuteProcess(const std::string &command);
int DaemonizeProcess(const std::string &directory);

#endif