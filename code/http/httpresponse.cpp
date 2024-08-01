#include "httpresponse.h"

using namespace std;

const unordered_map<string,string> HttpResponse::SUFFIX_TYPE = 
{
    {".html", "text/html"},
    {".xml", "text/xml"},
    { ".xhtml", "application/xhtml+xml" },
    { ".txt",   "text/plain" },
    { ".rtf",   "application/rtf" },
    { ".pdf",   "application/pdf" },
    { ".word",  "application/nsword" },
    { ".png",   "image/png" },
    { ".gif",   "image/gif" },
    { ".jpg",   "image/jpeg" },
    { ".jpeg",  "image/jpeg" },
    { ".au",    "audio/basic" },
    { ".mpeg",  "video/mpeg" },
    { ".mpg",   "video/mpeg" },
    { ".avi",   "video/x-msvideo" },
    { ".gz",    "application/x-gzip" },
    { ".tar",   "application/x-tar" },
    { ".css",   "text/css "},
    { ".js",    "text/javascript "},
};

const unordered_map<int, string> HttpResponse::CODE_STATUS = {
    { 200, "OK" },
    { 400, "Bad Request" },
    { 403, "Forbidden" },
    { 404, "Not Found" },
};

const unordered_map<int, string> HttpResponse::CODE_PATH = {
    { 400, "/400.html" },
    { 403, "/403.html" },
    { 404, "/404.html" },
};

HttpResponse::HttpResponse()
{
    code_ = -1;
    path_ = srcDir_ = "";
    isKeepAlive_ = false;
    mmFile_ = nullptr;
    mmFileStat_ = {0};
};

HttpResponse::~HttpResponse()
{
    UnmapFile();
}

void HttpResponse::Init(const string& srcDir, string& path, bool isKeepAlive, int code)
{
    assert(srcDir != "");
    if(mmFile_) { UnmapFile(); }
    code_ = code;
    isKeepAlive_ = isKeepAlive;
    path_ = path;
    srcDir_ = srcDir;
    mmFile_ = nullptr;
    mmFileStat_ = {0};
}

void HttpResponse::MakeResponse(Buffer& buff)
{
    // 判断请求的资源文件
    if(stat((srcDir_ + path_).data(), &mmFileStat_) < 0 || S_ISDIR(mmFileStat_.st_mode))
        code_ = 404;
    else if(!(mmFileStat_.st_mode & S_IROTH))
        code_ = 403;
    else if(code_ == -1)
        code_ = 200;
    
    ErrorHtml_();
    AddStateLine_(buff);
    AddHeader_(buff);
    AddContent_(buff);
}

char* HttpResponse::File()
{
    return mmFile_;
}

size_t HttpResponse::FileLen() const
{
    return mmFileStat_.st_size;
}