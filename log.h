//
//  log.h
//
//  Created by HouRui on 13-5-12.
//  Copyright 2011年 Cube Inc. All rights reserved.
//

#ifndef _LOG_H
#define _LOG_H 1

#include <vector>
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

#define LS_EOL      "\n"
#define LS_SPACE    " "
#define LS_MAX_SIZE 4096 // 4k for direct io

#define LS_OBJ_SIZE 2

#define LS_INFO  1
#define LS_ERROR 2

using namespace std;

class Log
{
private:
  int fd_;
  string filename_;
  
private:
  Log(): fd_(-1), filename_("") {}
  
public:
  ~Log()
  {
    if (fd_ != -1) close(fd_);
  }
  
private:
  string getType(const int &type);
  
public:
  bool reopen(void);
  string filename(void);
  
public:
  bool init(const string &filename);
  bool error(const char *format, ...);
  bool info(const char *format, ...);
  
  bool write(const char *str);
  bool write(const int &type, const char *str);
  bool write(const int &type, const char *format, va_list ap);
  
  
private:
  static Log self_[LS_OBJ_SIZE];
  
public:
  static void reopenAll(void);
  static Log * get(const int logid);
  static Log * init(const int logid, string filename);
  
};


class LogStream
{ 
  typedef LogStream self;
private: // make the stream non-copyable
  LogStream(const self &);
  void operator=(const self &);
  
private:
  int  off_;
  char buf_[LS_MAX_SIZE];
  
private:
  int type_;
  int logid_;
  
public:
  LogStream(const int &logid):
  off_(0), type_(-1), logid_(logid) {
    buf_[0] = '\0';
  }
  LogStream(const int &type, const int &logid):
  off_(0), type_(type), logid_(logid) {
    buf_[0] = '\0';
  }
  ~LogStream(void) { // 末尾添加换行符
    append(LS_EOL, 1);
    Log * l = Log::get(logid_);
    if (NULL == l) return;
    if (-1 == type_) l->write(buf_);
    else l->write(type_, buf_);
  }
  
private:
  // 追加
  template<int fmt_size>
  inline self & append(fmt<fmt_size> v) {
    strncpy(buf_ + off_, v.c_str(), LS_MAX_SIZE - off_);
    off_ += v.length();
    return *this;
  }
  
  inline self & append(const char * s) {
    strncpy(buf_ + off_, s, LS_MAX_SIZE - off_);
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
