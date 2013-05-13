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

#define atomic_cas(val, cmpv, newv) __sync_val_compare_and_swap((val), (cmpv), (newv))
#define likely(x)    __builtin_expect((x), 1)
#define unlikely(x)  __builtin_expect((x), 0)
#define is2n(number) (((~number + 1) & number) == number)

#ifdef __APPLE__
#define NQ_MAX_SIZE 16384 >> 1 // 16k >> 1
#else
#define NQ_MAX_SIZE 65536 >> 1 // 64k >> 1
#endif

template <typename T>
class NolockQueue
{
  struct QueueNode
  {
    volatile int64_t cur_pos;
    T *volatile data;
    T *volatile prev;
  };
public:
  NolockQueue(const int64_t queue_size = NQ_MAX_SIZE);
  ~NolockQueue();
public:
  void destroy();
public:
  inline int push(T *ptr);
  inline int pop(T *&ptr);
  inline int64_t get_total() const;
  inline int64_t get_free() const;
private:
  int init(const int64_t queue_size);
private:
  inline int64_t get_total_(const int64_t consumer, const int64_t producer) const;
  inline int64_t get_free_(const int64_t consumer, const int64_t producer) const;
private:
  int64_t queue_size_;
  int64_t queue_mask_;
  QueueNode *queue_;
  volatile int64_t consumer_;
  volatile int64_t producer_;
  
} __attribute__ ((aligned (64)));

template <typename T>
NolockQueue<T>::NolockQueue(const int64_t queue_size):
queue_size_(0), queue_mask_(0), queue_(NULL), consumer_(0), producer_(0)
{
  init(queue_size);
}

template <typename T>
NolockQueue<T>::~NolockQueue()
{
  destroy();
}

template <typename T>
int NolockQueue<T>::init(const int64_t queue_size)
{
  if (queue_size <= 0 || !is2n(queue_size)) {
    fprintf(stderr, "queue size should be 2^x and > 0\n"); exit(EXIT_FAILURE);
  }
  
  if (NULL == (queue_ = (QueueNode*)::valloc(sizeof(QueueNode) * queue_size))) {
    fprintf(stderr, "alloc memory error\n"); exit(EXIT_FAILURE);
  }
  
  memset(queue_, 0, sizeof(QueueNode) * queue_size);
  queue_size_ = queue_size;
  queue_mask_ = queue_size_ - 1;
  consumer_   = 1;
  producer_   = 1;
  
  return NQ_OK;
}

template <typename T>
void NolockQueue<T>::destroy()
{
  ::free(queue_);
  queue_ = NULL;
  queue_size_ = 0;
  queue_mask_ = 0;
  consumer_   = 0;
  producer_   = 0;
}

template <typename T>
inline int64_t NolockQueue<T>::get_total() const
{
  return get_total_(consumer_, producer_);
}

template <typename T>
inline int64_t NolockQueue<T>::get_free() const
{
  return get_free_(consumer_, producer_);
}

template <typename T>
inline int64_t NolockQueue<T>::get_total_(const int64_t consumer, const int64_t producer) const
{
  return (producer - consumer);
}

template <typename T>
inline int64_t NolockQueue<T>::get_free_(const int64_t consumer, const int64_t producer) const
{
  return queue_size_ - get_total_(consumer, producer);
}

template <typename T>
int NolockQueue<T>::push(T *ptr)
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
    if (old_pos == atomic_cas(&producer_, old_pos, new_pos)) {
      break;
    }
  }
  
  if (likely(NQ_OK == ret)) {
    register int64_t index = old_pos & queue_mask_;
    queue_[index].prev = queue_[index].data;
    queue_[index].data = ptr;
    queue_[index].cur_pos = old_pos;
  }

  return ret;
}

template <typename T>
int NolockQueue<T>::pop(T *&ptr)
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
    
    if (old_pos == atomic_cas(&consumer_, old_pos, new_pos)) {
      break;
    }
  }
  
  if (likely(NQ_OK == ret)) {
    
    register int64_t index = old_pos & queue_mask_; // 取余运算
    
    while (true) {
      
      register int64_t cmp_ret = old_pos - queue_[index].cur_pos;
      
      if (likely(0 == cmp_ret)) {
        ptr = queue_[index].data;
        if (likely(old_pos == queue_[index].cur_pos)) { // 二次校验数据是否被生产者覆盖
          break;
        } else {
          continue;
        }
      } else if (0 < cmp_ret) {
        continue; // 生产者还未将数据写入
      } else { //(0 > cmp_ret)
        ptr = queue_[index].prev;
        break; // 消费者cas后还没来得及消费就被生产者再次写入, 因此要读取快照
      }
    }
    
  }
  
  return ret;
}
#endif
