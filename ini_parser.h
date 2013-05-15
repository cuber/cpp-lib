//
// ini parser
//
//  Created by Zhibiao Pan on 11-7-5.
//  Copyright 2011年 DangDang Inc. All rights reserved.
//

#ifndef _INI_PARSER_H 
#define _INI_PARSER_H 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <fstream>

#include <ext/hash_map>

#include "tools.h"

using namespace std;
using namespace __gnu_cxx;

class IniParser {
  
  public:
  
  void load(const string &filename);
  
  IniParser(const string &comment_chars = "#;");
  ~IniParser();
  
  void   get_int   (const string &section, const string &option, int &value);
  int    get_int   (const string &section, const string &option);
  
  void   get_bool  (const string &section, const string &option, bool &value);
  bool   get_bool  (const string &section, const string &option);
  
  void   get_float (const string &section, const string &option, float &value);
  float  get_float (const string &section, const string &option);
  
  void   get_string(const string &section, const string &option, string &value);
  string get_string(const string &section, const string &option);
  
  const hash_map<string, string> get_section(const string &section);
  
  void print();
  
  private:
  
  string comment_chars_;  // 注释符
  hash_map<string, hash_map<string, string> > map_;
  
  string section_;
  
  void parse(const string &filename);

  void stripComment(string &line);
  
  void parseLine   (string line);
  bool parseSection(string line, const size_t &length);
  bool parseOption (string line, const size_t &length, string &key, string &value);
  
  void get(const string &section, const string &option, string &value);

};// IniParser()

#endif /* defined(_INI_PARSER_H) */
