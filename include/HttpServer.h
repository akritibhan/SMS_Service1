#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#define CPPHTTPLIB_THREAD_POOL_COUNT 128 //use 128 worker threads
//
#include "../third_party/cpp-httplib/httplib.h"
//after because intrnally defaults to 8 
#include "Config.h"
#include <string>
#include <memory>
#include <atomic>

class HttpServer {
public:
    static HttpServer& getInstance();
    
    bool initialize(const std::string& host, int port);
    void start(); //blocking call
    void stop();
    
    bool isRunning() const;
    uint64_t getTotalRequests() const;
    uint32_t getActiveRequests() const;
    
private:
    HttpServer() = default;
    ~HttpServer() = default;
    HttpServer(const HttpServer&) = delete;
    HttpServer& operator=(const HttpServer&) = delete;
    
    std::unique_ptr<httplib::Server> server;
    std::string bind_host;
    int bind_port = 8080;
    std::atomic<bool> running{false};
    std::atomic<uint64_t> total_requests{0};
    std::atomic<uint32_t> active_requests{0};
};

#endif // HTTP_SERVER_H
