#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <regex>
#include <error.h>
#include <mysql/mysql.h>

#include "../buffer/buffer.h"
#include "../log/log.h"
#include "../pool/sqlconnpool.h"

using namespace std;

class HttpRequest
{
public:
    enum PARSE_STATE 
    {
        REQUEST_LINE,
        HEADERS,
        BODY,
        FINISH,
    };

    HttpRequest() { Init(); }
    ~HttpRequest() = default;

    void Init();
    bool parse(Buffer& buff);

    string path() const;
    string& path();
    string method() const;
    string version() const;
    string GetPost(const string& key) const;
    string GetPost(const char* key) const;

    bool IsKeepAlive() const;

private:
    bool ParseRequestLine_(const string& line);     // 处理请求行
    void ParseHeader_(const string& line);          // 处理请求头
    void ParseBody_(const string& line);            // 处理请求体

    void ParsePath_();                              // 处理请求路径
    void ParsePost_();                              // 处理Post事件
    void ParseFromUrlencoded_();                    // 从url中解析编码

    static bool UserVerity(const string& name, const string& pwd, bool isLogin);   // 用户验证

    PARSE_STATE state_;
    string method_, path_, version_, body_;
    unordered_map<string, string> header_;
    unordered_map<string, string> post_;

    static const unordered_set<string> DEFAULT_HTML;
    static const unordered_map<string, int> DEFAULT_HTML_TAG;
    static int ConverHex(char ch);                  // 16进制转换为10进制

};

#endif
