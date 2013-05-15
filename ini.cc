//
//  ini_parser.cc
//
//  Created by Coffee on 13-1-5.
//  Copyright (c) 2013年 HouRui. All rights reserved.
//

#include "ini.h"

void ini::stripComment(string &line) {
  string::size_type comment_pos = line.find_first_of(comment_);
  if (comment_pos == string::npos) return;
  line.erase(comment_pos);
}


void ini::parse(void)
{
  string line(""), realline("");
  ifstream ifs(filename_.c_str());
  while (getline(ifs, line)) {
    stripComment(line);
    size_t length = line.length();
    if (line[length - 1] == '\\') {
      realline += line.substr(0, length - 1); continue;
    } else {
      realline += line;
    }
    parseLine(realline);
    realline = "";
  }
}

void ini::parseLine(string line)
{
  tools::trim(&line); size_t length = line.length();
  if (length == 0) return;
  string key(""), value("");
  if (parseSection(line, length)) return;
  if (parseValue(line, length, key, value)) {
    if (section_.length() == 0) return;
    map_[section_].insert(make_pair(key, value));
  }
}

bool ini::parseValue(string line, const size_t &length,
                     string &key, string &value)
{
  string::size_type equal_pos = line.find_first_of("=");
  if (equal_pos == string::npos) return false;
  key   = trim(line.substr(0, equal_pos - 1));
  value = trim(line.substr(equal_pos + 1));
  if (key.length() > 0) return true;
  return false;
}

bool ini::parseSection(string line, const size_t &length)
{
  if (line[0] != '[' || line[length - 1] != ']') return false;
  section_ = line.substr(1, length - 2); return true;
}


void ini::print()
{
  if (map_.size() == 0) {
    printf("NULL\n"); return;
  }
  for (ini_t::iterator it_section = map_.begin();
       it_section != map_.end(); ++it_section) {
    // section
    printf("[%s]\n", it_section->first.c_str());
    // value
    for (section_t::iterator it_value = it_section->second.begin();
         it_value != it_section->second.end(); ++it_value) {
      printf("%30s = %s\n", it_value->first.c_str(), it_value->second.c_str());
    }
    printf("\n");
  }
}

