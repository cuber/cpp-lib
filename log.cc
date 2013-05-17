//
//  log.cc
//  resysd.cate_data
//
//  Created by Coffee on 13-1-5.
//  Copyright (c) 2013年 HouRui. All rights reserved.
//

#include "log.h"

using namespace tools;
//------------------------------------------------------------------------------
//                         member function
//------------------------------------------------------------------------------
bool Log::init(const string &filename)
{
  if (!filename.length()) return false;
  
  filename_ = filename;
  SafeIO<true>::mkdir(dirname(filename_));
  
  fd_ = open(filename_.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
  return fd_ != -1;
}

bool Log::error(const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  int res = write("error", format, ap);
  va_end(ap);
  return res;
}

bool Log::info(const char *format, ...)
{
  va_list ap;
  va_start(ap, format);
  int res = write("info", format, ap);
  va_end(ap);
  return res;
}

bool Log::write(const char *str)
{
  if (fd_ == -1) return false;
  return ::write(fd_, str, strlen(str));
}

bool Log::write(const char *type, const char *str)
{
  char line[LOG_MAX_SIZE];
  snprintf(line, LOG_MAX_SIZE, "%s [%s] %s", fmt<32>::date().c_str(), type, str);
#ifdef _DEBUG
  fprintf(strncmp(type, "error", strlen(type)) ? stderr : stdout, "%s", line);
#endif
  return this->write(line);
}

bool Log::write(const char *type, const char *format, va_list ap)
{
  char line[LOG_MAX_SIZE];
  vsnprintf(line, LOG_MAX_SIZE, format, ap);
  return this->write(type, line);
}

bool Log::reopen(void)
{
  int new_fd, tmp_fd;
  
  // 1. open new
  if (!filename_.length()) return false;
  new_fd = open(filename_.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
  if (new_fd == -1) return false;
  
  // 2. switch hd
  tmp_fd = fd_;
  fd_ = new_fd;
  
  // 3. close old hd
  close(new_fd);
  return true;
}
