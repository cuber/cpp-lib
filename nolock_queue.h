/**
 *
 * no lock queue
 *
 * @author HouRui
 * @since 2013-04-07
 *
 */
#ifndef _NOLOCK_QUEUE
#define _NOLOCK_QUEUE 1

#define NQ_OK     0
#define NQ_FULL   1
#define NQ_EMPTY  2

#define atomic_cas(val, cmpv, newv) __sync_bool_compare_and_swap((val), (cmpv), (newv))
#define likely(x)    __builtin_expect((x), 1)
#define unlikely(x)  __builtin_expect((x), 0)
#define is2n(number) (((~number + 1) & number) == number)

template <int SIZE, typename T>
class NolockQueue
{
  struct QueueNode
  {
    volatile int64_t cur_pos;
    // 采用指针避免结构体对齐, 以节约空间
    T *volatile data;
    T *volatile prev;
    QueueNode():cur_pos(0) {
      data = new T;
      prev = new T;
    }
    ~QueueNode() {
      if (NULL != data) delete data;
      if (NULL != prev) delete prev;
    }
  };
  
public:
  NolockQueue();
  ~NolockQueue();
  
public:
  inline int push(const T &ptr);
  inline int pop (T &ptr);
  
public:
  inline int64_t get_total() const;
  inline int64_t get_free () const;
  
private:
  inline int64_t get_total_(const int64_t consumer, const int64_t producer) const;
  inline int64_t get_free_ (const int64_t consumer, const int64_t producer) const;
  
private:
  int64_t queue_mask_;
  QueueNode *queue_;
  volatile int64_t consumer_;
  volatile int64_t producer_;
  
} __attribute__ ((aligned (64)));

template <int SIZE, typename T>
NolockQueue<SIZE, T>::NolockQueue():
queue_mask_(SIZE - 1), queue_(NULL), consumer_(0), producer_(0)
{
  if (SIZE < 0 || !is2n(SIZE)) {
    fprintf(stderr, "SIZE should be 2^x (x > 1)"); exit(EXIT_FAILURE);
  }
  queue_ = new QueueNode[SIZE]();
}

template <int SIZE, typename T>
NolockQueue<SIZE, T>::~NolockQueue()
{
  if (NULL != queue_) delete [] queue_;
}

template <int SIZE, typename T>
inline int64_t NolockQueue<SIZE, T>::get_total() const
{
  return get_total_(consumer_, producer_);
}

template <int SIZE, typename T>
inline int64_t NolockQueue<SIZE, T>::get_free() const
{
  return get_free_(consumer_, producer_);
}

template <int SIZE, typename T>
inline int64_t NolockQueue<SIZE, T>::get_total_(const int64_t consumer, const int64_t producer) const
{
  return (producer - consumer);
}

template <int SIZE, typename T>
inline int64_t NolockQueue<SIZE, T>::get_free_(const int64_t consumer, const int64_t producer) const
{
  return SIZE - get_total_(consumer, producer);
}

template <int SIZE, typename T>
int NolockQueue<SIZE, T>::push(const T &ptr)
{ 
  int ret = NQ_OK;
  
  register int64_t old_pos = 0;
  register int64_t new_pos = 0;
  
  while (true) {
    
    old_pos = producer_;
    new_pos = old_pos;

    if (unlikely(0 >= get_free_(consumer_, old_pos))) {
      ret = NQ_FULL;
      break;
    }

    new_pos++;
    if (atomic_cas(&producer_, old_pos, new_pos)) {
      break;
    }
  }
  
  if (likely(NQ_OK == ret)) {
    register int64_t index = old_pos & queue_mask_;
    register T * swap  = queue_[index].prev;
    queue_[index].prev = queue_[index].data;
    queue_[index].data = swap;
   *queue_[index].data = ptr;
    queue_[index].cur_pos = old_pos;
  }
  
  return ret;
}

template <int SIZE, typename T>
int NolockQueue<SIZE, T>::pop(T &ptr)
{  
  int ret = NQ_OK;
  
  register int64_t old_pos = 0;
  register int64_t new_pos = 0;
  
  while (true) {
    
    old_pos = consumer_;
    new_pos = old_pos;

    if (unlikely(0 >= get_total_(old_pos, producer_))) {
      ret = NQ_EMPTY;
      break;
    }

    new_pos += 1;
    
    if (atomic_cas(&consumer_, old_pos, new_pos)) {
      break;
    }
  }
  
  if (likely(NQ_OK == ret)) {
    
    // 取余运算 类似子网掩码
    register int64_t index = old_pos & queue_mask_; 
    
    while (true) {
      
      register int64_t cmp_ret = old_pos - queue_[index].cur_pos;
      
      if (likely(0 == cmp_ret)) {
        ptr = *queue_[index].data;
        // 二次校验数据是否被生产者覆盖
        if (likely(old_pos == queue_[index].cur_pos)) break;
        else continue;
      } else if (0 > cmp_ret) {
        // 消费者cas后还没来得及消费就被生产者再次写入, 因此要读取快照
        ptr = *queue_[index].prev;
        break;
      } else continue;
    }
  }
  
  return ret;
}
#endif
