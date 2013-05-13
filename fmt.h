//
//  fmt.h
//  server
//
//  Created by Cube on 13-5-11.
//  Copyright (c) 2013年 HouRui. All rights reserved.
//

#ifndef _FMT_H
#define _FMT_H 1

#include <iostream>
#include <stdarg.h>
#include <time.h>
#include <stdio.h>

using namespace std;

template<int buf_size>
class fmt
{
public:
  fmt(): length_(0) {
    buf_[0] = '\0';
  }
  fmt(const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    length_ = vsnprintf(buf_, sizeof(buf_), format, ap);
    va_end(ap);
  }
  
  ~fmt() {
    buf_[0] = '\0';
  }
  
private:
  size_t length_;
  
private:
  char buf_[buf_size + 1];
  
public:
  inline size_t length(void) {
    return length_;
  }
  
  inline string str(void) {
    return string(buf_);
  }
  
  inline const char * c_str(void) {
    return buf_;
  }
  
public: // date format
  static inline fmt date(string pattern, time_t time) {
    char buf[buf_size] = {0};
    strftime(buf, sizeof(buf), pattern.c_str(), localtime(&time));
    return fmt(buf);
  }
  
  static inline fmt date(string pattern = "%Y-%m-%d %H:%M:%S") {
    return date(pattern, time(NULL));
  }
    
public: // int format
  template<typename T>
  static inline size_t itoa(char buf[], T value) {
    const static char tb[] = "9876543210123456789";
    const static char *zero = tb + 9;
    T i = value;
    char *p = buf;
    do
    {
      int lsd = static_cast<int>(i % 10);
      i /= 10;
      *p++ = zero[lsd];
    } while (i != 0);
    
    if (value < 0)
    {
      *p++ = '-';
    }
    *p = '\0';
    std::reverse(buf, p);
    return p - buf;
  }
  
  template<typename T>
  static inline fmt itoa(T value) {
    char buf[buf_size];
    itoa(buf, value);
    return fmt(buf);
  }
  
public: // double format
  template<typename T>
  static inline size_t ftoa(char buf[], T value) {
    return snprintf(buf, buf_size, "%.12g", static_cast<double>(value));
  }
  
  template<typename T>
  static inline fmt ftoa(T value) {
    char buf[buf_size];
    ftoa(buf, value);
    return fmt(buf);
  }
};


#endif /* defined(_FMT_H) */
