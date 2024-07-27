# ifndef BLOCKQUEUE_H
# define BLOCKQUEUE_H

#include <deque>
#include <condition_variable>
#include <mutex>
#include <sys/time.h>

using namespace std;

template<typename T>
class BlockQueue 
{
public:
    explicit BlockQueue(size_t maxsize = 1000);
    ~BlockQueue();
    bool empty();
    bool full();
    void push_back(const T& item);
    void push_front(const T& item);
    bool pop(T& item);                      // 弹出的任务放入item
    bool pop(T& item, int timeout);         // 等待时间
    void clear();
    T front();
    T back();
    size_t capacity();
    size_t size();

    void flush();
    void Close();

private:
    deque<T> deq_;                          // 底层数据结构
    mutex mtx_;                             // 锁
    bool isClose_;                          // 关闭标志
    size_t capacity_;                       // 容量
    condition_variable condConsumer_;       // 消费者条件变量
    condition_variable condProdicer_;       // 生产者条件变量
};


template<typename T>
BlockQueue<T>::BlockQueue(size_t maxsize) : capacity_(maxsize) 
{
    assert(maxsize > 0);
    isClose_ = false;
}

template<typename T>
BlockQueue<T>::~BlockQueue() 
{
    Close();
}

template<typename T>
void BlockQueue<T>::Close()
{
    // lock_guard<mutex> locker(mtx_);  // 操控队列之前，都需要上锁
    // deq_.clear()                     // 清空队列
    clear();
    isClose_ = true;
    condConsumer_.notify_all();
    condProdicer_.notify_all();
}

template<typename T>
void BlockQueue<T>::clear()
{
    lock_guard<mutex> locker(mtx_);  // 操控队列之前，都需要上锁
    deq_.clear()                     // 清空队列
}

template<typename T>
bool BlockQueue<T>::full()
{
    lock_guard<mutex> locker(mtx_);
    return deq_.size() >= capacity_;
}

template<typename T>
void BlockQueue<T>::push_back(const T& item)
{
    unique_lock<mutex> locker(mtx_);            // 注意，条件变量需要搭配unique_lock
    while(deq_.size() >= capacity_)             // 队列满了，需要等待
        condProdicer__.wait(locker);            // 停止生产，等待消费者唤醒生产条件变量
    
    deq_.push_front(item);
    condConsumer_.notify_one();                 // 唤醒消费者
}

template<typename T>
void BlockQueue<T>::push_front(const T& item)
{
    unique_lock<mutex> locker(mtx_);            // lock
    while(deq_.size() >= capacity_)             // 队列满了，需要等待
        condProdicer_.wait(locker);             // 停止生产，等待消费者唤醒生产条件变量

    deq_.push_front(item);                      
    condConsumer_.notify_one();                 // 唤醒消费者
}

template<typename T>
bool BlockQueue<T>::pop(T& item)
{   
    unique_lock<mutex> locker(mtx_);            // lock
    while(deq_.empty())                         // 当队列为空，阻塞等待
        condConsumer_.wait(locker);             // 停止消费
    
    item = deq_.front();                        // 取出任务
    deq_.pop_front();                           // 弹出队列
    condProdicer_.notify_one;                   // 通知线程
    return true;
}

template<typename T>
bool BlockQueue<T>::pop(T& item,int timeout)
{
    unique_lock<mutex> locker(mtx_);
    while(deq_.empty())
    {
        if (condConsumer_.wait_for(locker,chrono::seconds(timeout)) == cv_status::timeout)
            return false;
        if (isClose_)
            return false;
    }
    item = deq_.front();
    deq_.pop_front();
    condProdicer_.notify_one();
    return true;
}

template<typename T>
T BlockQueue<T>::front()
{
    lock_guard<mutex> locker(mtx_);
    return deq_.front();
}

template<typename T>
T BlockQueue<T>::back()
{
    lock_guard<mutex> locker(mtx_);
    return deq_.back();
}

template<typename T>
size_t BlockQueue<T>::capacity()
{
    lock_guard<mutex> locker(mtx_);
    return capacity_;
}

template<typename T>
size_t BlockQueue<T>::size()
{
    lock_guard<mutex> locker(mtx_);
    return deq_.size();
}

template<typename T>
void BlockQueue<T>::flush()
{
    condConsumer_.notify_one();
}

#endif
