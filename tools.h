//
//  tools.h
//  resysd
//
//  Created by Coffee on 12-12-17.
//  Copyright (c) 2012年 Coffee. All rights reserved.
//

#ifndef _TOOLS_H
#define _TOOLS_H 1

#include <string.h>
#include <signal.h>
#include <vector>
#include <iostream>
#include <sys/stat.h>

namespace tools {
  
  using namespace std;
  
  // 去除两端的空字符
  // @param string *str
  void trim(string *str);
  
  // 返回值重载版本
  // @param const string &str
  // @return string
  string trim(string str);
  
  // 切割字符串
  // @param vector<string> &dest
  // @param        string  source
  // @param const  string  &delimiter
  // @param const  size_t  &limit
  void explode(vector<string> &dest,            string source,
               const  string  &delimiter, const size_t &limit = 0);

  // 切割字符串
  // explode 的重载版本
  // 支持回调函数, 将字符直接转换为对应的结构
  template <class Type>
  inline void explode(vector<Type> &dest,      string source,
                      const string &delimiter, Type (func)(const char *))
  {
    dest.resize(0); dest.reserve(16);
    vector<string> dest_vt;
    explode(dest_vt, source, delimiter, 0);
    for (size_t i = 0; i < dest_vt.size(); i++) {
      dest.push_back(func(dest_vt[i].c_str()));
    }
  }
  
  // 安全创建文件夹
  // @param cosnt string &dirname
  void safe_mkdir(const string &dirname);
  
  // 安全打开
  // @param const string &filename
  // @param const string &flag
  // @return FILE *
  FILE * safe_open(const string &filename, const string &flag);
  
  // 设置run flag
  // @param const string &filename
  // @param const int flag
  void set_run_flag(const string &filename, const int flag);
  
  // 获取父级目录
  // @param string filename
  // @return string dirname
  string dirname(string filename);
  
  // 写进程号文件
  // @param const string &pid_filename
  void write_pid(const string &pid_filename);
  
  // 判断进程是否存活, 用于校验程序唯一性
  // @param const string &pid_filename
  bool check_running(const string &pid_filename);
  
  // 当前进程脱离shell
  void detach_terminal(void);
}

#endif /* defined(_TOOLS_H) */
