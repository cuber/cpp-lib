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
  
  // 安全建立文件夹
  void safe_mkdir(const string &dirname)
  {
    if (dirname.length() && access(dirname.c_str(), F_OK) != 0) {
      mkdir(dirname.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }
  }
  
  // 安全打开
  FILE * safe_open(const string &filename, const string &flag)
  {
    safe_mkdir(dirname(filename));
    FILE *fp = fopen(filename.c_str(), flag.c_str());
    if (NULL == fp) {
      fprintf(stderr, "fopen('%s', '%s'); error\n", filename.c_str(), flag.c_str());
      exit(EXIT_FAILURE);
    }
    return fp;
  }
  
  // 设置run flag
  void set_run_flag(const string &filename, const int flag)
  {
    FILE *fp = safe_open(filename, "wb");
    fwrite(flag ? "1" : "0", 1, sizeof(char), fp);
    fclose(fp);
  }
  
  // 获取父级目录
  string dirname(string filename)
  {
    return filename.erase(filename.find_last_of("/"));
  }
  
  // 写进程号文件
  void write_pid(const string &pid_filename)
  {
    FILE *fp = safe_open(pid_filename, "wb");
    pid_t pid = getpid();
    fwrite(&pid, sizeof(pid_t), 1, fp);
    fclose(fp);
  }
  
  // 判断进程是否存活, 保证程序唯一性
  bool check_running(const string &pid_filename)
  {
    if (access(pid_filename.c_str(), F_OK) != 0) return false;
    FILE *fp = safe_open(pid_filename, "rb");
    pid_t pid = -1;
    fread(&pid, sizeof(pid_t), 1, fp);
    fclose(fp);
    if (pid <= 1) return false;
    return kill(pid, 0) == 0;
  }
  
  // 从shell分离
  void detach_terminal(void)
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
