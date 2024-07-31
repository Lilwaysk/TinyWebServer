#ifndef BUFFER_H
#define BUFFER_H
#include <cstring>       // perror
#include <iostream>
#include <unistd.h>      // write
#include <sys/uio.h>     // readv
#include <vector>        // readv
#include <atomic>
#include <assert.h>

using namespace std;

class Buffer 
{
public:
    Buffer(int initBuffSize = 1024);
    ~Buffer() = default;

    size_t WritableBytes() const;
    size_t ReadableBytes() const;
    size_t PrependableBytes() const;

    const char* Peek() const;
    void EnsureWriteable(size_t len);
    void HasWritten(size_t len);

    void Retrieve(size_t len);
    void RetrieveUntil(const char* end);

    void RetrieveAll();
    string RetrieveAllToStr();

    const char* BeginWriteConst() const;
    char* BeginWrite();

    void Append(const string& str);
    void Append(const char* str,size_t len);
    void Append(const void* data,size_t len);
    void Append(const Buffer& buff);
    
    ssize_t ReadFd(int fd,int* Errno);
    ssize_t WriteFd(int fd,int* Errno);
    
private:
    char* BeginPtr_();    // buffer 开头
    const char* BeginPtr_() const;
    void MakeSpace_(size_t len);

    vector<char> buffer_;
    atomic<size_t> readPos_;     // 读操作下标
    atomic<size_t> writePos_;    // 写操作下标
};

#endif
