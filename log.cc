//
//  log.cc
//  resysd.cate_data
//
//  Created by Coffee on 13-1-5.
//  Copyright (c) 2013年 HouRui. All rights reserved.
//

#include "log.h"

using namespace tools;

Log Log::self_[LS_OBJ_SIZE];

Log * Log::init(const int logid, string filename)
{
  if (logid < 0 || logid >= LS_OBJ_SIZE) return NULL;
  Log *ptr = self_ + logid;
  ptr->init(filename);
  return ptr;
}

Log * Log::get(const int logid)
{
  if (logid < 0 || logid >= LS_OBJ_SIZE) return NULL;
  Log *ptr = self_ + logid;
  return ptr;
}

void Log::reopenAll(void)
{
  for (int i = 0; i < LS_MAX_SIZE; i++) {
    if (self_[i].reopen()) continue;
    fprintf(stderr, "reopen('%s') failed\n", self_[i].filename().c_str());
  }
}

string Log::filename(void)
{
  return filename_;
}

string Log::getType(const int &type)
{
  switch (type) {
    case LS_ERROR:
      return "error";
      break;
    case LS_INFO: default:
      return "info";
      break;
  }
}

bool Log::init(const string &filename)
{
  if (!filename.length()) return false;
  
  filename_ = filename;
  safe_mkdir(dirname(filename_));
  
  fd_ = open(filename_.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
  return fd_ != -1;
}

bool Log::error(const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  int res = write(LS_ERROR, format, ap);
  va_end(ap);
  return res;
}

bool Log::info(const char *format, ...)
{
  va_list ap;
  va_start(ap, format);
  int res = write(LS_INFO, format, ap);
  va_end(ap);
  return res;
}

bool Log::write(const char *str)
{
  if (fd_ == -1) return false;
  return ::write(fd_, str, strlen(str));
}

bool Log::write(const int &type, const char *str)
{
  char line[LS_MAX_SIZE];
  snprintf(line, sizeof(line), "%s [%s] %s", fmt<32>::date().c_str(),
           getType(type).c_str(), str);
#ifdef _DEBUG
  fprintf(type == LS_ERROR ? stderr : stdout, "%s", line);
#endif
  return write(line);
}

bool Log::write(const int &type, const char *format, va_list ap)
{
  char line[LS_MAX_SIZE];
  vsnprintf(line, sizeof(line), format, ap);
  return write(type, line);
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
