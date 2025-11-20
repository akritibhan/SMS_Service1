#include "HttpServer.h"
#include "SMSProcessor.h"
#include <iostream>

HttpServer& HttpServer::getInstance() {
    static HttpServer instance;
    return instance;
}

bool HttpServer::initialize(const std::string& host, int port) {
    bind_host = host;
    bind_port = port;
    
    std::cout << "HTTP server initialized on " << host << ":" << port << std::endl;
    return true;
}

void HttpServer::start() {
    if (running) {
        std::cout << "Server already running" << std::endl;
        return;
    }
    
    running = true;
    
    // Create HTTP server
    server = std::make_unique<httplib::Server>();
    
    // Set thread pool size for handling requests
    auto& config = Config::getInstance();
    server->new_task_queue = [&config] { 
        return new httplib::ThreadPool(config.getThreadPoolSize()); 
    };
    
    // POST endpoint for SMS requests
    server->Post("/sms", [this](const httplib::Request& req, httplib::Response& res) {
        ++total_requests;
        ++active_requests;
        
        static bool first_logged = false;
        
        try {
            // Get XML from request body
            std::string request_xml = req.body;
            
            if (request_xml.empty()) {
                res.status = 400;
                res.set_content("Empty request body", "text/plain");
                --active_requests;
                return;
            }
            
            // Log first request
            if (!first_logged) {
                std::cout << "\nSAMPLE REQUEST\n";
                std::cout << request_xml << std::endl;
                first_logged = true;
            }
            
            // Process request
            auto& processor = SMSProcessor::getInstance();
            std::string response_xml = processor.processXMLRequest(request_xml);
            
            // Log first response
            static bool first_response_logged = false;
            if (!first_response_logged) {
                std::cout << "\nSAMPLE RESPONSE\n";
                std::cout << response_xml << std::endl;
                first_response_logged = true;
            }
            
            // Send response to the client
            res.set_content(response_xml, "application/xml");
            
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(std::string("Error: ") + e.what(), "text/plain");
        }
        
        --active_requests;
    });
    
    std::cout << "HTTP server listening on " << bind_host << ":" << bind_port << std::endl;
    
    // Start listening 
    server->listen(bind_host.c_str(), bind_port);
}

void HttpServer::stop() {
    if (!running) return;
    
    std::cout << "Stopping HTTP server..." << std::endl;
    running = false;
    
    if (server) {
        server->stop();
    }
    
    std::cout << "HTTP server stopped" << std::endl;
}

bool HttpServer::isRunning() const {
    return running;
}

uint64_t HttpServer::getTotalRequests() const {
    return total_requests;
}

uint32_t HttpServer::getActiveRequests() const {
    return active_requests;
}
