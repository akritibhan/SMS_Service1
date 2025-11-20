#pragma once

#include "XMLParser.h"
#include <memory>
#include <atomic>
#include <chrono>

class SMSProcessor {
public:
    static SMSProcessor& getInstance();
    
    bool initialize();
    SMSResponse processRequest(const SMSRequest& request);
    std::string processXMLRequest(const std::string& xml_request);
    
    void shutdown();
    
    // Statistics
    struct Statistics {
        std::atomic<uint64_t> total_requests{0};
        std::atomic<uint64_t> successful_requests{0};
        std::atomic<uint64_t> failed_requests{0};
        std::atomic<uint64_t> avg_processing_time_us{0};
        std::atomic<uint64_t> total_processing_time_us{0};
    };
    
    Statistics& getStats() { return stats; }
    void resetStats();
    void printStats() const;

private:
    SMSProcessor() = default;
    ~SMSProcessor() { shutdown(); }
    SMSProcessor(const SMSProcessor&) = delete;
    SMSProcessor& operator=(const SMSProcessor&) = delete;
    
    std::atomic<bool> initialized{false};
    Statistics stats;
};
