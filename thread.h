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

#ifdef __APPLE__
#define QSIZE (16384 >> 1) // 16k >> 1
#else
#define QSIZE (65536 >> 1) // 64k >> 1
#endif

using namespace std;

template<class T>
struct ThreadAttr
{
  int pipe_in_fd;
  int pipe_out_fd;
  
  class NolockQueue<QSIZE, T> *queue;
  void  (*queue_cb)(const T&);
  
  struct event_base * base;
  struct event        ev;
  
  pthread_t tid;
  
  ThreadAttr() {
    base  = event_init();
    queue = new NolockQueue<QSIZE, T>;
  }
  
  ~ThreadAttr() {
    if (NULL != base) event_base_free(base);
    if (NULL != queue) delete queue;
  }
};

template <class T>
class Thread
{ 
private:
  int thread_num_;
  int current_thread_;
  
private:
  struct ThreadAttr<T> * thread_;
  
public:
  Thread() : current_thread_(0), thread_(NULL) {}
  ~Thread() {
    if (NULL != thread_) delete [] thread_;
  }
  
public:
  void init(const int thread_num, void (*queue_cb)(const T&));
  bool queue(const T &updata);
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
  static void *thread_start_cb(void *arg);
  static void  thread_libevent_cb(int fd, short which, void *arg);
};
//------------------------------------------------------------------------------
//                             static member
//------------------------------------------------------------------------------
template <class T> int             Thread<T>::init_count_;
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

template <class T>
void Thread<T>::thread_libevent_cb(int fd, short which, void *arg)
{
  struct ThreadAttr<T> * thread = (struct ThreadAttr<T> *)arg;
  
  char buffer[1];
  if (read(fd, buffer, 1) != 1) {
    fprintf(stderr, "read thread notify pipe failure\n");
  }
  T updata;
  if (thread->queue->pop(updata) == NQ_EMPTY) {
    fprintf(stderr, "empty queue\n");
    return;
  }
  if (NULL != thread->queue_cb) thread->queue_cb(updata);
}

//------------------------------------------------------------------------------
//                             member function
//------------------------------------------------------------------------------
template <class T>
void Thread<T>::init(const int thread_num, void (*queue_cb)(const T&))
{
  init_count_ = 0;
  thread_num_ = thread_num;
  
  thread_ = new struct ThreadAttr<T>[thread_num_]();

  pthread_mutex_init(&init_lock_, NULL);
  pthread_cond_init (&init_cond_, NULL);
  
  // init
  for (int i = 0; i < thread_num_; i++) {
    thread_[i].queue_cb = queue_cb;
    thread_setup(&thread_[i]);
  }
  
  // start
  for (int i = 0; i < thread_num_; i++) {
    thread_create(thread_start_cb, &thread_[i]);
  }
  
  // wait all thread start
  pthread_mutex_lock  (&init_lock_);
  while (init_count_ < thread_num_) pthread_cond_wait(&init_cond_, &init_lock_);
  pthread_mutex_unlock(&init_lock_);
  
  pthread_mutex_destroy(&init_lock_);
  pthread_cond_destroy (&init_cond_);
}

template <class T>
bool Thread<T>::queue(const T &updata)
{
  register int old_thread;
  register int new_thread;
  do {
    old_thread = current_thread_;
    new_thread = (old_thread + 1) % thread_num_;
    if (__sync_bool_compare_and_swap(&current_thread_, old_thread, new_thread)) break;
  } while(true);
  
  struct ThreadAttr<T> * thread = &thread_[new_thread];
  if (thread->queue->push(updata) == NQ_FULL) {
    fprintf(stderr, "queue full\n");
    return false; // 队列已满, 丢弃
  }
  if (write(thread->pipe_in_fd, "", 1) != 1) { // 通知
    fprintf(stderr, "write thread in pipe failure\n"); return false;
  }
  
  return true;
}

template <class T>
vector<int> Thread<T>::size(void)
{
  vector<int> size(thread_num_);
  for (int i = 0; i < thread_num_; i++) {
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
  if (NULL == thread->base) {
    fprintf(stderr, "can't allocate event base\n"); exit(EXIT_FAILURE);
  }
  
  // set event callback
  event_set(&thread->ev, thread->pipe_out_fd, EV_READ | EV_PERSIST,
            thread_libevent_cb, thread);
  event_base_set(thread->base, &thread->ev);
  if (event_add(&thread->ev, NULL) == -1) {
    fprintf(stderr, "can't monitor libevent out pipe\n");
    exit(1);
  }
}

template <class T>
void Thread<T>::thread_create(void *(*func)(void *), void *arg)
{
  struct ThreadAttr<T> * thread = (struct ThreadAttr<T> *)arg;
  
  // thread;
  pthread_attr_t  attr;
  
  pthread_attr_init(&attr);
  int ret = pthread_create(&thread->tid, &attr, func, arg);
  
  if (ret != 0) {
    fprintf(stderr, "can't create thread: %s\n", strerror(ret));
    exit(EXIT_FAILURE);
  }
}

#endif /* defined(_THREAD_H) */
