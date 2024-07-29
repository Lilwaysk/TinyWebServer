#include "buffer.h"

// 读写下标初始化，vector<char> 初始化
Buffer::Buffer(int initBuffSize) : buffer_(initBuffSize), readPos_(0), writePos_(0) {}

// 可写的数量：buffer大小 —— 写下标
size_t Buffer::WritableBytes() const
{
    return buffer_.size() - writePos_;
}

// 可读的数量：写下标 —— 读下标
size_t Buffer::ReadableBytes() const
{
    return writePos_ - readPos_;
}

// 可预留空间：已经读过的就没用了，等于读下标
size_t Buffer::PrependableBytes() const
{
    return readPos_;
}

const char* Buffer::Peek() const
{
    return &buffer_[readPos_];
}

// 确保可写的长度
void Buffer::EnsureWriteable(size_t len)
{
    if (len > WritableBytes()) 
        MakeSpace_(len);
    assert(len <= WritableBytes());
}

// 移动写下标，在Append中使用
void Buffer::HasWritten(size_t len)
{
    writePos_ += len;
}

// 读取len长度，移动读下标
void Buffer::Retrieve(size_t len)
{
    readPos_ += len;
}

// 读取buffer到end位置
void Buffer::RetrieveUntil(const char* end)
{
    assert(Peek() <= end);
    Retrieve(end - Peek()); // end指针 - 读指针 长度
}

// 取出所有数据，buffer归零，都写下标归零，在别的函数中会用到
void Buffer::RetrieveAll()
{
    // bzero() 能够将内存块（字符串）的前n个字节清零:void bzero(void *s, int n);
    bzero(&buffer_[0], buffer_.size());  // 覆盖掉原来数据
    readPos_ = writePos_ = 0;
}

// 去除剩余可读的str
string Buffer::RetrieveAllToStr()
{
    string str(Peek(),ReadableBytes());
    RetrieveAll();
    return str;
}

// 写指针的位置
const char* Buffer::BeginWriteConst() const
{
    return &buffer_[writePos_];
}

char* Buffer::BeginWrite()
{
    return &buffer_[writePos_];
}

void Buffer::Append(const string& str)
{
    Append(str.c_str(), str.size());
}

// 添加str到缓冲区
void Buffer::Append(const char* str,size_t len)
{
    assert(str);
    EnsureWriteable(len);  // 确保可写的长度
    copy(str, str + len, BeginWrite());  // 将str放到写下标开始的地方
    HasWritten(len);  // 移动写下标
}

void Buffer::Append(const void* data,size_t len)
{
    Append(static_cast<const char*>(data), len);
}

// 将buffer中的读下标的地方放到该buffer中的写下标位置
void Buffer::Append(const Buffer& buff)
{
    Append(buff.Peek(), buff.ReadableBytes());
}
    
// 将fd的内容读到缓冲区，即writable的位置
ssize_t Buffer::ReadFd(int fd,int* Errno)
{
    char buff[65535];  // 栈区
    /*
        iovec 是一个描述内存缓冲区的数据结构，定义在 <sys/uio.h> 头文件中
        struct iovec {
            void  *iov_base;    // Pointer to data 
            size_t iov_len;     // Length of data 
        };
        iov_base 是指向数据的指针。
        iov_len 是数据的长度。
    */
    struct iovec iov[2];
    size_t writeable = WritableBytes(); // 先记录能写多少
    
    // 分散读，保证数据全部读完
    iov[0].iov_base = BeginWrite();
    iov[0].iov_len = writeable;
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff);

    ssize_t len = readv(fd, iov, 2);
    if(len < 0) {
        *Errno = errno;
    } else if(static_cast<size_t>(len) <= writeable) {   // 若len小于writable，说明写区可以容纳len
        writePos_ += len;   // 直接移动写下标
    } else {    
        writePos_ = buffer_.size(); // 写区写满了,下标移到最后
        Append(buff, static_cast<size_t>(len - writeable)); // 剩余的长度
    }
    return len;
}

// 将buffer中可读的区域写入fd中
ssize_t Buffer::WriteFd(int fd,int* Errno)
{
    ssize_t len = write(fd, Peek(), ReadableBytes());
    if (len < 0)
    {
        *Errno = errno;
        return len;
    }
    Retrieve(len);
    return len;
}

char* Buffer::BeginPtr_()
{
    return &buffer_[0];
}

const char* Buffer::BeginPtr_() const
{
    return &buffer_[0];
}

// 扩展空间
void Buffer::MakeSpace_(size_t len)
{
    if (WritableBytes() + PrependableBytes() < len)
        buffer_.resize(writePos_ + len + 1);
    else
    {
        size_t readable = ReadableBytes();
        copy(BeginPtr_() + readPos_, BeginPtr_() + writePos_, BeginPtr_());
        readPos_ = 0;
        writePos_ = readable;
        assert(readable == ReadableBytes());
    }
}

