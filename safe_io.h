//
//  safe_io.h
//  readdb
//
//  Created by Cube on 13-5-16.
//  Copyright (c) 2013年 HouRui. All rights reserved.
//

#ifndef _SAFE_IO_H
#define _SAFE_IO_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "tools.h"

using namespace std;

template <bool isExit>
class SafeIO
{
private:
  static void (*func_)(const char *);
  
private:
  static void error(const char * format, ...) {
    char line[4096] = {0};
    va_list ap;
    va_start(ap, format);
    vsnprintf(line, sizeof(line), format, ap);
    va_end(ap);
    if (NULL != func_) return func_(line);
    else fprintf(stderr, "error: %s\n", line);
  } __attribute__((format(printf, 1, 2)));
  
public:
  static void setErrorInfoCb(void (*func)(const char *)) {
    func_ = func;
  }
  
public:
  // safe mkdir
  static bool mkdir(const string & dirname) {
    if (access(dirname.c_str(), F_OK) == 0) return true;
    do {
      if (!dirname.length()) break;
      if (::mkdir(dirname.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) break;
      return true;
    } while(0);
    error("mkdir('%s')", dirname.c_str());
    if (isExit) exit(EXIT_FAILURE);
    return false;
  }
  
  // safe open
  static FILE * fopen(const string & filename, const string & flag) {
    FILE * fp = ::fopen(filename.c_str(), flag.c_str());
    if (NULL != fp) return fp;
    error("fopen('%s', '%s')", filename.c_str(), flag.c_str());
    if (isExit) exit(EXIT_FAILURE);
  }
  
  // quick write
  static bool quickWrite(const string & filename, const char * format, ...) {
    FILE * fp = fopen(filename, "wb");
    va_list ap;
    va_start(ap, format);
    vfprintf(fp, format, ap);
    fclose(fp);
    va_end(ap);
    return true;
  } __attribute__((format(printf, 2, 3)));
  
  // check pidfile 
  static bool isRunning(const string & pidfile) {
    if (access(pidfile.c_str(), F_OK) != 0) return false;
    FILE *fp = fopen(pidfile, "rb");
    pid_t pid = -1;
    fread(&pid, sizeof(pid_t), 1, fp);
    fclose(fp);
    if (pid <= 1) return false;
    if (kill(pid, 0) != 0) return false;
    error("pidfile '%s', pid '%d' is running", pidfile.c_str(), pid);
    if (isExit) exit(EXIT_FAILURE);
    return true;
  }
  
  // rm file
  static bool rm(const string & filename) {
    if (access(filename.c_str(), F_OK) != 0) return true;
    if (unlink(filename.c_str()) == 0) return true;
    error("rm file: '%s'", filename.c_str());
    if (isExit) exit(EXIT_FAILURE);
  }
  
  // mv file
  static bool mv(const string & from, const string & to) {
    if (0 == rename(from.c_str(), to.c_str())) return true;
    error("mv '%s' to '%s'", from.c_str(), to.c_str());
    if (isExit) exit(EXIT_FAILURE);
  }
};

template <bool isExit>
void (*SafeIO<isExit>::func_)(const char *) = NULL;


#endif /* defined(_SAFE_IO_H) */
