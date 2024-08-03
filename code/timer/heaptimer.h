#ifndef HEAP_TIMER_H
#define HEAP_TIMER_H

#include <queue>
#include <unordered_map>
#include <time.h>
#include <algorithm>
#include <arpa/inet.h>
#include <functional>
#include <assert.h>
#include <chrono>
#include "../log/log.h"

using namespace std;

typedef function<void()> TimeoutCallBack;
typedef chrono::high_resolution_clock Clock;
typedef chrono::milliseconds MS;
typedef Clock::time_point TimeStamp;

struct TimerNode
{
    int id;
    TimeStamp expires;                  // 超时时间点
    TimeoutCallBack cb;                 // 回调function<void()>
    bool operator<(const TimerNode& t)  // 重载比较运算符
    {
        return expires < t.expires;
    }
    bool operator>(const TimerNode& t)  // 重载比较运算符
    {
        return expires > t.expires;
    }
};

class HeapTimer
{
public:
    HeapTimer() {heap_.reserve(64);}
    ~HeapTimer() {clear();}

    void adjust(int id, int newExpires);
    void add(int id, int timeOut, const TimeoutCallBack& cb);
    void doWork(int id);
    void clear();
    void tick();
    void pop();
    int GetNextTick();

private:
    void del_(size_t i);
    void siftup_(size_t i);
    bool siftdown_(size_t i, size_t n);
    void SwapNode_(size_t i, size_t j);

    vector<TimerNode> heap_;
    // key:id value:vector的下标
    unordered_map<int, size_t> ref_;  // id对应的在heap_中的下标
};

#endif