//
// ini parser
//
//  Created by Zhibiao Pan on 11-7-5.
//  Copyright 2011年 DangDang Inc. All rights reserved.
//

#ifndef _INI_H 
#define _INI_H 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <fstream>

#include <map>

#include "tools.h"

using namespace std;
using namespace tools;

class ini {
  class value {
  private:
    string content_;
    
  public:
    value(const string &content) : content_(content) {}
    
  public:
    inline int c_int(int * val = NULL) {
      int t = atoi(content_.c_str());
      return val == NULL ? t : (*val = t);
    }
    
    inline float c_float(float * val = NULL) {
      float t = atof(content_.c_str());
      return val == NULL ? t : (*val = t);
    }
    
    inline string str(string * val = NULL) {
      return val == NULL ? content_ : (*val = content_);
    }
    
    inline const char * c_str(const char ** val = NULL) {
      return val == NULL ? content_.c_str() : (*val = content_.c_str());
    }
    
    inline bool asBool(bool * val = NULL) {
      bool t = true;
      if (strcasecmp(content_.c_str(), "false") == 0 ||
          strcasecmp(content_.c_str(), "no"   ) == 0 ||
          strcasecmp(content_.c_str(), "off"  ) == 0 ||
          strcasecmp(content_.c_str(), "0"    ) == 0) {
        t = false;
      }
      return val == NULL ? t : (*val = t);
    }
  };
  
  typedef map<string, class value> section_t;
  
  class section : public section_t {
  public:
    inline class value& operator[] (const string &key) {
      section_t::iterator it = find(key);
      if (it != end()) return it->second;
      fprintf(stderr, "Key: '%s' not exists\n", key.c_str());
      exit(EXIT_FAILURE);
    }
    
    inline class value get(const string &key) {
      return (*this)[key];
    }
  };
  
  typedef map<string, class section> ini_t;
  
private:
  string comment_;
  
private:
  string section_;
  string filename_;

  
private:
  ini_t map_;
  
public:
  ini(void) {
    comment_ = "#!";
  }
  
  inline void load(const string &filename) {
    filename_ = filename;
    if (access(filename.c_str(), 00) == F_OK) return parse();
    fprintf(stderr, "ini file '%s' not exists\n", filename.c_str());
    exit(EXIT_FAILURE);
  }
  
public:
  inline class section section(const string &section) {
    ini_t::iterator it = map_.find(section);
    if (it != map_.end()) return it->second;
    fprintf(stderr, "Section: '%s' not exists\n", section.c_str());
    exit(EXIT_FAILURE);
  }
  
  inline class value get(const string &section, const string &key) {
    class section s = this->section(section);
    return s.get(key);
  }
  
  inline class section operator[](const string &section) {
    return map_[section];
  }
public:
  void print(void);
  
private:
  void parse(void);
  void parseLine(string line);
  bool parseValue(string line, const size_t &length, string &key, string &value);
  bool parseSection(string line, const size_t &length);
  
private:
  void stripComment(string &line);

};

#endif /* defined(_INI_H) */
