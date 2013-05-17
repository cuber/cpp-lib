//
//  log.h
//
//  Created by HouRui on 13-5-12.
//  Copyright 2011年 Cube Inc. All rights reserved.
//

#ifndef _LOG_H
#define _LOG_H 1

#include <map>
#include <fcntl.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>
#include <unistd.h>

#include <iostream>

#include "fmt.h"
#include "tools.h"
#include "safe_io.h"

#define LOG_EOL      "\n"
#define LOG_SPACE    " "
#define LOG_MAX_SIZE 4096 // 4k for direct io

using namespace std;

class Log
{
private:
  int    fd_;
  string filename_;
  
public:
  Log(): fd_(-1), filename_("") {}
  Log(const string &filename): fd_(-1) {
    init(filename);
  }
  
public:
  ~Log() {
    if (fd_ != -1) close(fd_);
  }
  
public:
  bool reopen(void);
  
public:
  bool init(const string &filename);
  bool error(const char *format, ...) __attribute__((format(printf, 2, 3)));
  bool info (const char *format, ...) __attribute__((format(printf, 2, 3)));
  
  bool write(const char *str);
  bool write(const char *type, const char *str);
  bool write(const char *type, const char *format, va_list ap);
};


class LogStream
{ 
  typedef LogStream self;
private: // make the stream non-copyable
  LogStream(const self &);
  void operator=(const self &);
  
private:
  static map<string, class Log> logmap_;
  
private:
  int  off_;
  
private:
  class Log * log_;
  
private:
  string type_;
  
private:
  char buf_[LOG_MAX_SIZE];
  
public:
  LogStream(class Log * log, const string type = ""):
  off_(0), log_(log), type_(type) {
    buf_[0] = '\0';
  }
  
  ~LogStream(void) { // 末尾添加换行符
    append(LOG_EOL, 1);
    if (type_.length()) {
      if (NULL == log_) fprintf(stdout, "[%s] %s", type_.c_str(), buf_);
      else log_->write(type_.c_str(), buf_);
    } else {
      if (NULL == log_) fprintf(stdout, "%s", buf_);
      else log_->write(buf_);
    }
  }
  
private:
  // 追加
  template<int fmt_size>
  inline self & append(fmt<fmt_size> v) {
    strncpy(buf_ + off_, v.c_str(), LOG_MAX_SIZE - off_);
    off_ += v.length();
    return *this;
  }
  
  inline self & append(const char * s) {
    strncpy(buf_ + off_, s, LOG_MAX_SIZE - off_);
    off_ += strlen(s);
    return *this;
  }
  
  inline self & append(const char * s, const size_t & length) {
    strncpy(buf_ + off_, s, length);
    off_ += length;
    return *this;
  }
  
public:
  // 字符系列
  template<int fmt_size>
  inline self & operator << (const fmt<fmt_size> &v) {
    return append<fmt_size>(v);
  }
  inline self & operator << (const char v) {
    return append(&v, 1);
  }
  inline self & operator << (const bool v) {
    return append(v ? "1" : "0", 1);
  }
  // 整数系列
  template<typename T>
  inline self & operator << (const T v) {
    return append(fmt<sizeof(T) * 2>::itoa(v));
  }
  // 浮点
  inline self & operator << (const float v) {
    return append(fmt<32>::ftoa(v));
  }
  inline self & operator << (const double v) {
    return append(fmt<32>::ftoa(v));
  }
  // 字符串
  inline self & operator << (char* v) {
    return append(v);
  }
  inline self & operator << (const char* v) {
    return append(v);
  }
  inline self & operator << (string& v) {
    return append(v.c_str(), v.length());
  }
  inline self & operator << (const string& v) {
    return append(v.c_str(), v.length());
  }
public:
  inline size_t length(void) {
    return off_;
  }
  inline const char * c_str(void) {
    return buf_;
  }
};

#endif /* defined(_LOG_H) */
