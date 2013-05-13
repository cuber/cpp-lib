//
//  thread_update.h
//  server
//
//  Created by Cube on 13-4-10.
//  Copyright (c) 2013年 HouRui. All rights reserved.
//

#ifndef _THREAD_H
#define _THREAD_H 1

#include <fcntl.h>
#include <iostream>
#include <pthread.h>

#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/event_compat.h>

#include "log.h"
#include "nolock_queue.h"
#include "blizzard_hash.h"

using namespace std;

typedef void (*libevent_cb)(int, short, void *);

template<typename T>
struct ThreadAttr
{
  int pipe_in_fd;
  int pipe_out_fd;
  struct event_base * base;
  struct event        ev;
  class NolockQueue<T> *queue;
};

template <typename T>
class Thread
{ 
private:
  int current_thread_;
  
private:
  vector<struct ThreadAttr<T> > thread_;
  
private:
  libevent_cb thread_libevent_cb_;
  
public:
  Thread():
  current_thread_(0){}
  ~Thread(){}
  
public:
  void init(const int thread_num, libevent_cb func);
  bool queue(T &updata);
  vector<int>size(void);
  
private:
  void thread_init(void);
  void thread_setup(struct ThreadAttr<T> *thread);
  void thread_create(void *(*func)(void *), void *arg);
  
private:
  static int             init_count_;
  static pthread_mutex_t init_lock_;
  static pthread_cond_t  init_cond_;
  
private:
  static void *thread_start_cb (void *arg);
};
//------------------------------------------------------------------------------
//                             static member
//------------------------------------------------------------------------------
template <class T> int             Thread<T>::init_count_ = 0;;
template <class T> pthread_mutex_t Thread<T>::init_lock_;
template <class T> pthread_cond_t  Thread<T>::init_cond_;

//------------------------------------------------------------------------------
//                             static function
//------------------------------------------------------------------------------
template <class T>
void * Thread<T>::thread_start_cb(void *arg)
{
  struct ThreadAttr<T> * thread = (struct ThreadAttr<T> *)arg;
  pthread_mutex_lock  (&init_lock_);
  init_count_++;
  pthread_cond_signal (&init_cond_);
  pthread_mutex_unlock(&init_lock_);
  event_base_loop(thread->base, 0);
  return NULL;
}

//------------------------------------------------------------------------------
//                             member function
//------------------------------------------------------------------------------
template <class T>
void Thread<T>::init(const int thread_num, libevent_cb func)
{
  thread_libevent_cb_ = func;
  
  thread_.resize(thread_num);
  pthread_mutex_init(&init_lock_, NULL);
  pthread_cond_init (&init_cond_, NULL);
  
  // init
  for (size_t i = 0; i < thread_.size(); i++) {
    thread_setup(&thread_[i]);
  }
  
  // start
  for (size_t i = 0; i < thread_.size(); i++) {
    thread_create(thread_start_cb, &thread_[i]);
  }
  
  // wait all thread start
  pthread_mutex_lock  (&init_lock_);
  while (init_count_ < (int)thread_.size()) pthread_cond_wait(&init_cond_, &init_lock_);
  pthread_mutex_unlock(&init_lock_);
}

template <class T>
bool Thread<T>::queue(T &updata)
{
  register int old_thread;
  register int new_thread;
  do {
    old_thread = current_thread_;
    new_thread = (old_thread + 1) % thread_.size();
    if (__sync_bool_compare_and_swap(&current_thread_, old_thread, new_thread)) break;
  } while(true);
  
  struct ThreadAttr<T> thread = thread_[new_thread];
  if (thread.queue->push(&updata) == NQ_FULL) {
    fprintf(stderr, "queue full\n");
    return false; // 队列已满, 丢弃
  }
  if (write(thread.pipe_in_fd, "", 1) != 1) { // 通知
    fprintf(stderr, "write thread in pipe failure\n"); return false;
  }
  return true;
}

template <class T>
vector<int> Thread<T>::size(void)
{
  vector<int> size(thread_.size());
  for (size_t i = 0; i < thread_.size(); i++) {
    size[i] = (int)thread_[i].queue->get_total();
  }
  return size;
}

//------------------------------------------------------------------------------
//                         thread member function
//------------------------------------------------------------------------------
template <class T>
void Thread<T>::thread_setup(struct ThreadAttr<T> *thread)
{
  // open pipe
  int fds[2];
  if (pipe(fds)) {
    fprintf(stderr, "create pipe error\n"); exit(EXIT_FAILURE);
  }
  
#ifndef __APPLE__
  fcntl(fds[0], F_SETFL, O_NOATIME);
#endif
  
  thread->pipe_out_fd = fds[0];
  thread->pipe_in_fd  = fds[1];
  
  // setup event
  thread->base = event_init();
  if (NULL == thread->base) {
    fprintf(stderr, "can't allocate event base\n");
    exit(EXIT_FAILURE);
  }
  
  // set event callback
  event_set(&thread->ev, thread->pipe_out_fd, EV_READ | EV_PERSIST,
            thread_libevent_cb_, thread);
  event_base_set(thread->base, &thread->ev);
  if (event_add(&thread->ev, NULL) == -1) {
    fprintf(stderr, "can't monitor libevent out pipe\n");
    exit(1);
  }
  
  thread->queue = new class NolockQueue<struct UpdateData>();
}

template <class T>
void Thread<T>::thread_create(void *(*func)(void *), void *arg)
{
  pthread_t       thread;
  pthread_attr_t  attr;
  
  pthread_attr_init(&attr);
  int ret = pthread_create(&thread, &attr, func, arg);
  
  if (ret != 0) {
    fprintf(stderr, "can't create thread: %s\n", strerror(ret));
    exit(EXIT_FAILURE);
  }
}

#endif /* defined(_THREAD_H) */
