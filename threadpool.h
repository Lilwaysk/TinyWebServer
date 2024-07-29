#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <thread>
#include <assert.h>
#include <iostream>

using namespace std;

class ThreadPool
{
public:
    ThreadPool() = default;
    ThreadPool(ThreadPool&&) = default;
    // 尽量用make_shared代替new，如果通过new再传递给shared_ptr，内存是不连续的的，会造成内存碎片化
    explicit ThreadPool(int threadCount = 8) : pool_(make_shared<Pool>()) // make_shared:传递右值，功能是在动态内存中分配一个对象并初始化它。返回指向此对象的shared_ptr
    {
        assert(threadCount > 0);                                          // 判断线程数是否大于零
        for(int i = 0;i < threadCount; ++i)                               // 根据线程数添加线程任务
        {
            thread([this]()                                               
            {
                unique_lock<mutex> locker(pool_->mtx_);                   // 用lambda表达式添加线程任务，上锁
                while(true)
                {
                    if (!pool_->tasks.empty())                            // 当任务队列不为空时
                    {
                        auto task = move(pool_->tasks.front());           // 取任务，左值边右值，资产转移
                        pool_->tasks.pop();                               // 弹出队列
                        locker.unlock();                                  // 因为已经把任务取出来了，所以可以解锁
                        task();                                           // 执行任务
                        locker.lock();                                    // 马上又要取任务了，上锁
                    }
                    else if (pool_->isClosed)
                        break;
                    else
                        pool_->cond_.wait(locker);                        // 等待，如果任务来了就notify
                }
            }).detach();
        }    
    }

    ~ThreadPool() 
    {
        if (pool_)
        {
            unique_lock<mutex> locker(pool_->mtx_);
            pool_->isClosed = true;
        }
        pool_->cond_.notify_all();                                        // 唤醒所有线程
    }

    template<typename T>
    void AddTask(T&& task)
    {
        unique_lock<mutex> locker(pool_->mtx_);
        pool_->tasks.emplace(forward<T>(task));
        pool_->cond_.notify_one();
    }

private:
    // 使用一个结构体封装起来，方便调用
    struct Pool
    {
        mutex mtx_;
        condition_variable cond_;
        bool isClosed;
        queue<function<void()>> tasks; // 任务队列，函数类型为void()
    };
    shared_ptr<Pool> pool_;
};


#endif 

