#include "Config.h"
#include "ConnectionPool.h"
#include "RegexCache.h"
#include "SMSProcessor.h"
#include "HttpServer.h"
#include <iostream>
#include <csignal>
#include <thread>
#include <chrono>

std::atomic<bool> shutdown_requested(false);

void signalHandler(int signal) {
    std::cout << "\nReceived signal " << signal << ", initiating graceful shutdown..." << std::endl;
    shutdown_requested = true;
}

void setupSignalHandlers() {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
}

bool initializeSystem() {
    std::cout << "  SMS Service High-Performance" << std::endl;
    std::cout << "  Target: 500 TPS" << std::endl;
    
    // Load configuration
    auto& config = Config::getInstance();
    if (!config.loadFromFile("config/config.json")) {
        std::cerr << "Failed to load configuration, using defaults" << std::endl;
    }
    
    // Initialize connection pool
    auto& conn_pool = ConnectionPool::getInstance();
    if (!conn_pool.initialize(
        config.getDbHost(),
        config.getDbPort(),
        config.getDbUser(),
        config.getDbPassword(),
        config.getDbName(),
        config.getDbPoolSize(),
        config.getDbPoolMinSize()
    )) {
        std::cerr << "Failed to initialize connection pool" << std::endl;
        return false;
    }
    
    // Initialize SMS processor
    auto& processor = SMSProcessor::getInstance();
    if (!processor.initialize()) {
        std::cerr << "Failed to initialize SMS processor" << std::endl;
        return false;
    }
    
    // Initialize network 
    auto& server = HttpServer::getInstance();
    if (!server.initialize(config.getServerHost(), config.getServerPort())) {
        std::cerr << "Failed to initialize HTTP server" << std::endl;
        return false;
    }
    
    std::cout << "\nSystem initialization complete!" << std::endl;
    
    return true;
}

void shutdownSystem() {
    std::cout << "  Shutting down system..." << std::endl;
    
    // Stop network server
    auto& server = HttpServer::getInstance();
    server.stop();
    
    // Shutdown connection pool
    auto& conn_pool = ConnectionPool::getInstance();
    conn_pool.shutdown();
    
    std::cout << "Shutdown complete. Goodbye!" << std::endl;
}

int main(int argc, char* argv[]) {
    setupSignalHandlers();
    
    if (!initializeSystem()) {
        std::cerr << "System initialization failed!" << std::endl;
        return 1;
    }
    
    // Start network server 
    auto& server = HttpServer::getInstance();
    std::thread server_thread([&]() {
        server.start();
    });
    
    // Wait for shutdown signal
    std::cout << "\nServer is running. Press Ctrl+C to stop." << std::endl;
    
    while (!shutdown_requested) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // Shutdown
    shutdownSystem();
    
    if (server_thread.joinable()) {
        server_thread.join();
    }
    
    return 0;
}
