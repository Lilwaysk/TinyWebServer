#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <unordered_map>
#include <fcntl.h>              // open
#include <unistd.h>             // 
#include <sys/stat.h>
#include <sys/mman.h>

#include "../buffer/buffer.h"
#include "../log/log.h"

class HttpResponse
{
public:
    HttpResponse();
    ~HttpResponse();

    void Init(const string& srcDir_, string& path_, bool isKeepAlive_ = false, int code = -1);
    void NakeResponse(Buffer& buff);
    void UnmapFile();
    char* File();
    size_t FileLen() const;
    void ErrorContent(Buffer& buff, string message);
    int Code() const { return code_; }
    
private:
    void AddStateLine_(Buffer &buff);
    void AddHeader_(Buffer &buff);
    void AddContent_(Buffer &buff);

    void ErrorHtml_();
    string GetFileType_();

    int code_;
    bool isKeepAlive_;

    string path_;
    string srcDir_;

    char* mmFile_;
    struct stat mmFileStat_;

    static const unordered_map<string, string> SUFFIX_TYPE;  // 后缀类型集
    static const unordered_map<int, string> CODE_STATUS;  // 编码状态集
    static const unordered_map<int, string> CODE_PATH;  // 编码路径集
};

#endif