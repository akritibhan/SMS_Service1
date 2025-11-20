#include "ConnectionPool.h"
#include <iostream>
#include <thread>
#include <chrono>

ConnectionPool& ConnectionPool::getInstance() {
    static ConnectionPool instance;
    return instance;
}

bool ConnectionPool::initialize(const std::string& host, int port, const std::string& user,
                               const std::string& password, const std::string& database,
                               int pool_size, int min_size) {
    db_host = host;
    db_port = port;
    db_user = user;
    db_password = password;
    db_name = database;
    max_pool_size = pool_size;
    min_pool_size = min_size;
    
    // minimum number of connections
    for (int i = 0; i < min_size; ++i) {
        MYSQL* conn = createConnection();
        if (!conn) {
            std::cerr << "Failed to create initial connection " << i << std::endl;
            return false;
        }
        available_pool.push(std::make_shared<MySQLConnection>(conn));
    }
    
    std::cout << "Connection pool initialized with " << min_size << " connections" << std::endl;
    return true;
}

MYSQL* ConnectionPool::createConnection() {
    MYSQL* conn = mysql_init(nullptr);
    if (!conn) {
        std::cerr << "mysql_init failed" << std::endl;
        return nullptr;
    }
    
    unsigned int timeout = 5;
    mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
    mysql_options(conn, MYSQL_OPT_READ_TIMEOUT, &timeout);
    mysql_options(conn, MYSQL_OPT_WRITE_TIMEOUT, &timeout);
    
    // Enable auto-reconnect
    bool reconnect = true;
    mysql_options(conn, MYSQL_OPT_RECONNECT, &reconnect);
    
    // Set character set
    mysql_options(conn, MYSQL_SET_CHARSET_NAME, "utf8mb4");
    
    if (!mysql_real_connect(conn, db_host.c_str(), db_user.c_str(), 
                           db_password.c_str(), db_name.c_str(), 
                           db_port, nullptr, CLIENT_MULTI_STATEMENTS)) {
        std::cerr << "MySQL connection failed: " << mysql_error(conn) << std::endl;
        mysql_close(conn);
        return nullptr;
    }
    
    return conn;
}

std::shared_ptr<MySQLConnection> ConnectionPool::acquireConnection() {
    std::unique_lock<std::mutex> lock(pool_mutex);
    
    // Wait for available connection or create new one
    while (available_pool.empty() && active_connections >= max_pool_size && !shutdown_flag) {
        pool_cv.wait_for(lock, std::chrono::milliseconds(100));
    }
    
    if (shutdown_flag) {
        return nullptr;
    }
    
    std::shared_ptr<MySQLConnection> conn;
    
    if (!available_pool.empty()) {
        conn = available_pool.front();
        available_pool.pop();
        
        // Validate connection
        if (!conn->isValid()) {
            std::cerr << "Invalid connection found, creating new one" << std::endl;
            MYSQL* new_conn = createConnection();
            if (new_conn) {
                conn = std::make_shared<MySQLConnection>(new_conn);
            } else {
                return nullptr;
            }
        }
    } else if (active_connections < max_pool_size) {
        // Create new connection
        MYSQL* new_conn = createConnection();
        if (new_conn) {
            conn = std::make_shared<MySQLConnection>(new_conn);
            ++active_connections;
        }
    }
    
    return conn;
}

void ConnectionPool::releaseConnection(std::shared_ptr<MySQLConnection> conn) {
    if (!conn || shutdown_flag) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(pool_mutex);
    
    // Validate before returning to pool
    if (conn->isValid()) {
        available_pool.push(conn);
    } else {
        --active_connections;
    }
    
    pool_cv.notify_one();
}

void ConnectionPool::shutdown() {
    shutdown_flag = true;
    
    std::lock_guard<std::mutex> lock(pool_mutex);
    
    // Clear all connections
    while (!available_pool.empty()) {
        available_pool.pop();
    }
    
    active_connections = 0;
    pool_cv.notify_all();
    
    std::cout << "Connection pool shutdown complete" << std::endl;
}
