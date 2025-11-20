#pragma once

#include <string>

class Config {
public:
    static Config& getInstance();
    
    bool loadFromFile(const std::string& filename);
    
    // Database configuration
    std::string getDbHost() const { return db_host; }
    int getDbPort() const { return db_port; }
    std::string getDbUser() const { return db_user; }
    std::string getDbPassword() const { return db_password; }
    std::string getDbName() const { return db_name; }
    
    // Connection pool settings
    int getDbPoolSize() const { return db_pool_size; }
    int getDbPoolMinSize() const { return db_pool_min_size; }
    
    // Thread pool settings
    int getThreadPoolSize() const { return thread_pool_size; }
    
    // Cache settings
    int getCacheSize() const { return cache_size; }
    int getCacheRefreshIntervalSec() const { return cache_refresh_interval; }
    
    // Network settings
    int getServerPort() const { return server_port; }
    std::string getServerHost() const { return server_host; }
    int getMaxConnections() const { return max_connections; }
    
    // Performance settings
    int getRequestQueueSize() const { return request_queue_size; }
    int getSocketTimeoutMs() const { return socket_timeout_ms; }

private:
    Config() = default;
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    
    // Database
    std::string db_host = "localhost";
    int db_port = 3306;
    std::string db_user = "root";
    std::string db_password = "";
    std::string db_name = "sms_service";
    int db_pool_size = 20;
    int db_pool_min_size = 5;
    
    // Thread pool
    int thread_pool_size = 16;
    
    // Cache
    int cache_size = 100000;
    int cache_refresh_interval = 300;
    
    // Network
    std::string server_host = "0.0.0.0";
    int server_port = 8080;
    int max_connections = 1000;
    
    // Performance
    int request_queue_size = 10000;
    int socket_timeout_ms = 5000;
};
