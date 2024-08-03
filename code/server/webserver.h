#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <unordered_map>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "epoller.h"
#include "../timer/heaptimer.h"

#include "../log/log.h"
#include "../pool/sqlconnpool.h"
#include "../pool/threadpool.h"

#include "../http/httpconn.h"

class WebServer
{
public:
    WebServer
    {
        int port, int trigMode, int timeoutMS, bool OptLinger,
        int sqlPort, const char* sqlUser, const char* sqlPwd,
        const char* dbName, int connPoolNum, int threadNum,
        bool openLog, int logLevel, int logQueSize
    };

    ~WebServer();
    void Start();

private:
    bool InitSocket_();
    void InitEventMode_(int trigMode);
    void AddClient_(int fd, sockaddr_in addr);

    void DealListen_();
    void DealWrite_(HttpConn* client);
    void DealRead_(HttpConn* client);

    void SendError_(int fd, const char* info);
    void ExtentTime_(HttpConn* client);
    void CloseConn_(HttpConn* client);

    void OnRead_(HttpConn* client);
    void OnWrite_(HttpConn* client);
    void OnProcess(HttpConn* client);

    static const int MAX_FD = 65536;

    static int SetFdNonblock(int fd);

    int port_;
    bool openLinger_;
    int timeoutMS_;
    bool isClose_;
    int listenFd_;
    char* srcDir_;

    uint32_t listenEvent_;
    uint32_t connEvent_;

    unique_ptr<HeapTimer> timer_;
    unique_ptr<ThreadPool> threadpool_;
    unique_ptr<Epoller> epoller_;
    unordered_map<int, HttpConn> users_;
};


#endif