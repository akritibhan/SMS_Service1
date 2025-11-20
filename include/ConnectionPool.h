#pragma once

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <winsock2.h>
    #include <windows.h>
#endif

#include <mysql.h>
#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>

class MySQLConnection {
public:
    explicit MySQLConnection(MYSQL* conn) : connection(conn) {}
    ~MySQLConnection() {
        if (connection) {
            mysql_close(connection);
        }
    }
    
    MYSQL* get() { return connection; }
    bool isValid() const { return connection != nullptr && mysql_ping(connection) == 0; }

private:
    MYSQL* connection;
};

class ConnectionPool {
public:
    static ConnectionPool& getInstance();
    
    bool initialize(const std::string& host, int port, const std::string& user,
                   const std::string& password, const std::string& database,
                   int pool_size, int min_size);
    
    std::shared_ptr<MySQLConnection> acquireConnection();
    void releaseConnection(std::shared_ptr<MySQLConnection> conn);
    
    void shutdown();
    
    int getActiveConnections() const { return active_connections; }
    int getAvailableConnections() const { return available_pool.size(); }

private:
    ConnectionPool() = default;
    ~ConnectionPool() { shutdown(); }
    ConnectionPool(const ConnectionPool&) = delete;
    ConnectionPool& operator=(const ConnectionPool&) = delete;
    
    MYSQL* createConnection();
    
    std::string db_host;
    int db_port;
    std::string db_user;
    std::string db_password;
    std::string db_name;
    int max_pool_size;
    int min_pool_size;
    
    std::queue<std::shared_ptr<MySQLConnection>> available_pool;
    std::mutex pool_mutex;
    std::condition_variable pool_cv;
    
    std::atomic<int> active_connections{0};
    std::atomic<bool> shutdown_flag{false};
};

// RAII wrapper for automatic connection release
class ConnectionGuard {
public:
    explicit ConnectionGuard(std::shared_ptr<MySQLConnection> conn)
        : connection(conn) {}
    
    ~ConnectionGuard() {
        if (connection) {
            ConnectionPool::getInstance().releaseConnection(connection);
        }
    }
    
    MYSQL* get() { return connection ? connection->get() : nullptr; }
    
    ConnectionGuard(const ConnectionGuard&) = delete;
    ConnectionGuard& operator=(const ConnectionGuard&) = delete;

private:
    std::shared_ptr<MySQLConnection> connection;
};
