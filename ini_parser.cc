//
//  ini_parser.cc
//
//  Created by Coffee on 13-1-5.
//  Copyright (c) 2013年 HouRui. All rights reserved.
//

#include "ini_parser.h"

IniParser::IniParser(const string &comment_chars)
{
  comment_chars_ = comment_chars;
  section_ = "";
}

IniParser::~IniParser() {}

void IniParser::load(const string &filename)
{
  if (access(filename.c_str(), 00) != F_OK) {
    printf("file '%s' not exists\n", filename.c_str()); exit(EXIT_FAILURE);
  }
  parse(filename);
}

void IniParser::parse(const string &filename)
{
  string line(""), realline("");
  ifstream ifs(filename.c_str());
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

void IniParser::parseLine(string line)
{
  tools::trim(&line); size_t length = line.length();
  if (length == 0) return;
  string key(""), value("");
  if (parseSection(line, length)) return;
  if (parseOption(line, length, key, value)) {
    if (section_.length() == 0) return;
    map_[section_][key] = value;
  }
}

bool IniParser::parseSection(string line, const size_t &length)
{
  if (line[0] != '[' || line[length - 1] != ']') return false;
  section_ = line.substr(1, length - 2); return true;
}

bool IniParser::parseOption(string line, const size_t &length,
                               string &key, string &value)
{
  string::size_type equal_pos = line.find_first_of("=");
  if (equal_pos == string::npos) return false;
  key   = tools::trim(line.substr(0, equal_pos - 1));
  value = tools::trim(line.substr(equal_pos + 1));
  if (key.length() > 0) return true;
  return false;
}

void IniParser::stripComment(string &line)
{
  string::size_type comment_pos = line.find_first_of(comment_chars_);
  if (comment_pos == string::npos) return;
  line.erase(comment_pos);
}

void IniParser::get(const string &section, const string &option, string &value)
{
  map<string, string>::const_iterator it_option;
  map<string, map<string, string> >::const_iterator it_section;
  do {
    it_section = map_.find(section);
    if (it_section == map_.end()) break;
    it_option = it_section->second.find(option);
    if (it_option == it_section->second.end()) break;
    value = it_option->second;
    return;
  } while(0);
  printf("[%s].%s not found\n", section.c_str(), option.c_str());
  exit(EXIT_FAILURE);
}

int IniParser::get_int(const string &section, const string &option)
{
  string value;
  get(section, option, value);
  return atoi(value.c_str());
}

void IniParser::get_int(const string &section, const string &option, int &value)
{
  value = get_int(section, option);
}

bool IniParser::get_bool(const string &section, const string &option)
{
  string value;
  get(section, option, value);
  
  if (strcasecmp(value.c_str(), "false") == 0 ||
      strcasecmp(value.c_str(), "no"   ) == 0 ||
      strcasecmp(value.c_str(), "off"  ) == 0 ||
      strcasecmp(value.c_str(), "0"    ) == 0) {
    return false;
  }
  
  if (strcasecmp(value.c_str(), "true") == 0 ||
      strcasecmp(value.c_str(), "yes" ) == 0 ||
      strcasecmp(value.c_str(), "on"  ) == 0 ||
      strcasecmp(value.c_str(), "1"   ) == 0) {
    return true;
  }
  
  return false;
}

void IniParser::get_bool(const string &section, const string &option, bool &value)
{
  value = get_bool(section, option);
}

float IniParser::get_float(const string &section, const string &option)
{
  string value;
  get(section, option, value);
  return atof(value.c_str());
}

void IniParser::get_float(const string &section, const string &option, float &value)
{
  value = get_float(section, option);
}

string IniParser::get_string(const string &section, const string &option)
{
  string value;
  get(section, option, value);
  return value;
}

void IniParser::get_string(const string &section, const string &option, string &value)
{
  get(section, option, value);
}

void IniParser::print()
{
  if (map_.size() == 0) {
    printf("NULL\n"); return;
  }
  for (std::map<std::string, std::map<std::string, std::string> >::const_iterator it_section
       = map_.begin(); it_section != map_.end(); it_section++) {
    // section
    printf("[%s]\n", it_section->first.c_str());
    // options
    for (std::map<std::string, std::string>::const_iterator it_option = it_section->second.begin();
         it_option != it_section->second.end(); ++it_option) {
      printf("%30s = %s\n", it_option->first.c_str(), it_option->second.c_str());
    }
    printf("\n");
  }
}


