#include "Config.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

Config& Config::getInstance() {
    static Config instance;
    return instance;
}

bool Config::loadFromFile(const std::string& filename) {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open config file: " << filename << std::endl;
            return false;
        }
        
        json config;
        file >> config;
        
        // Load db configs
        if (config.contains("database")) {
            auto& db = config["database"];
            if (db.contains("host")) db_host = db["host"];
            if (db.contains("port")) db_port = db["port"];
            if (db.contains("user")) db_user = db["user"];
            if (db.contains("password")) db_password = db["password"];
            if (db.contains("database")) db_name = db["database"];
            if (db.contains("pool_size")) db_pool_size = db["pool_size"];
            if (db.contains("pool_min_size")) db_pool_min_size = db["pool_min_size"];
        }
        
        // Load thread pool configs
        if (config.contains("thread_pool")) {
            auto& tp = config["thread_pool"];
            if (tp.contains("size")) thread_pool_size = tp["size"];
        }
        
        // Load cache configs
        if (config.contains("cache")) {
            auto& cache = config["cache"];
            if (cache.contains("size")) cache_size = cache["size"];
            if (cache.contains("refresh_interval_sec")) cache_refresh_interval = cache["refresh_interval_sec"];
        }
        
        // Load network part
        if (config.contains("network")) {
            auto& net = config["network"];
            if (net.contains("host")) server_host = net["host"];
            if (net.contains("port")) server_port = net["port"];
            if (net.contains("max_connections")) max_connections = net["max_connections"];
        }
        
        // Load performance settings
        if (config.contains("performance")) {
            auto& perf = config["performance"];
            if (perf.contains("request_queue_size")) request_queue_size = perf["request_queue_size"];
            if (perf.contains("socket_timeout_ms")) socket_timeout_ms = perf["socket_timeout_ms"];
        }
        
        std::cout << "Configuration loaded successfully from " << filename << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error loading config: " << e.what() << std::endl;
        return false;
    }
}
