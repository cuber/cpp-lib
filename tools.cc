//
//  tools.cc
//  resysd.cate_data
//
//  Created by Coffee on 13-1-4.
//  Copyright (c) 2013年 HouRui. All rights reserved.
//

#include "tools.h"

namespace tools {
  
  using namespace std;
  
  // 获取父级目录
  string dirname(string filename)
  {
    return filename.erase(filename.find_last_of("/"));
  }

  
  // 去除两端空字符
  void trim(string *str)
  {
    if (str->empty()) return;
    str->erase(0, str->find_first_not_of(" \r\n\t"));
    str->erase(str->find_last_not_of(" \r\n\t") + 1);
  }
  
  // 去除两端空字符
  string trim(string str)
  {
    trim(&str);
    return str;
  }
  
  // 将字符串切割为 vector<string>
  void explode(vector<string> &dest,            string source,
               const  string  &delimiter, const size_t &limit)
  {
    dest.resize(0); dest.reserve(16);
    if (source.length() == 0) return;
    if (delimiter.length() == 0) {
      dest.push_back(source); return;
    }
    string::size_type find_pos = 0, cur_pos = 0;
    while (find_pos <= source.length()) {
      find_pos = source.find(delimiter, find_pos);
      if (find_pos == string::npos || (limit > 0 && dest.size() == limit - 1)) {
        dest.push_back(source.substr(cur_pos)); return;
      }
      if (delimiter.length() == 1 && find_pos == 0 &&
          source[find_pos - 1] < 0) {
        find_pos += delimiter.length(); continue;
      }
      dest.push_back(source.substr(cur_pos, find_pos - cur_pos));
      find_pos += delimiter.length(); cur_pos = find_pos;
    }
  }
  
  // 从shell分离
  void detach_terminal(bool is_exit)
  {
    pid_t pid = fork();
    if (pid < 0) {
      fprintf(stderr, "Detach from terminal Error\n"); exit(EXIT_FAILURE);
    }
    if (pid > 0) exit(EXIT_SUCCESS); // im father
    // set child
    if (setsid() == -1) {
      fprintf(stderr, "set session id Error\n"); exit(EXIT_FAILURE);
    }
    umask(0);
  }
}
