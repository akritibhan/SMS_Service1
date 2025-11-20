#include "SMSProcessor.h"
#include "RegexCache.h"
#include "Config.h"
#include <iostream>
#include <iomanip>

SMSProcessor& SMSProcessor::getInstance() {
    static SMSProcessor instance;
    return instance;
}

bool SMSProcessor::initialize() {
    if (initialized) {
        std::cout << "SMSProcessor already initialized" << std::endl;
        return true;
    }
    
    std::cout << "Initializing SMSProcessor..." << std::endl;
    
    // Initialize regex cache
    auto& cache = RegexCache::getInstance();
    auto& config = Config::getInstance();
    
    if (!cache.initialize(config.getCacheSize())) {
        std::cerr << "Failed to initialize regex cache" << std::endl;
        return false;
    }
    
    // Preload patterns from database
    std::cout << "Preloading patterns from database..." << std::endl;
    if (!cache.loadPatternsFromDB()) {
        std::cerr << "Warning: Failed to preload patterns" << std::endl;
        // Continue anyway - patterns will be loaded on-demand
    }
    
    initialized = true;
    std::cout << "SMSProcessor initialized successfully" << std::endl;
    return true;
}

SMSResponse SMSProcessor::processRequest(const SMSRequest& request) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    SMSResponse response;
    ++stats.total_requests;
    
    // Validate request
    if (request.sender.empty() || request.sms_content.empty()) {
        response.success = false;
        response.error_message = "Invalid request: sender and SMS content are required";
        ++stats.failed_requests;
        return response;
    }
    
    // Search for matching pattern
    auto& cache = RegexCache::getInstance();
    auto match_result = cache.findMatch(request.sender, request.sms_content);
    
    if (match_result.found) {
        response.success = true;
        response.entity_id = match_result.entity_id;
        response.content_id = match_result.content_id;
        response.tm_id = match_result.tm_id;
        ++stats.successful_requests;
    } else {
        response.success = false;
        response.error_message = "No matching pattern found for sender: " + request.sender;
        ++stats.failed_requests;
    }
    
    // Calculate processing time
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    stats.total_processing_time_us += duration.count();
    stats.avg_processing_time_us = stats.total_processing_time_us / stats.total_requests;
    
    return response;
}

std::string SMSProcessor::processXMLRequest(const std::string& xml_request) {
    // Parse XML request
    SMSRequest request = XMLParser::parseRequest(xml_request);
    
    // Process request
    SMSResponse response = processRequest(request);
    
    // Generate XML response
    return XMLParser::generateResponse(response);
}

void SMSProcessor::shutdown() {
    if (!initialized) return;
    
    std::cout << "Shutting down SMSProcessor..." << std::endl;
    printStats();
    
    initialized = false;
}

void SMSProcessor::resetStats() {
    stats.total_requests = 0;
    stats.successful_requests = 0;
    stats.failed_requests = 0;
    stats.avg_processing_time_us = 0;
    stats.total_processing_time_us = 0;
}

void SMSProcessor::printStats() const {
    std::cout << "Total Requests:       " << stats.total_requests << std::endl;
    std::cout << "Successful:           " << stats.successful_requests << std::endl;
    std::cout << "Failed:               " << stats.failed_requests << std::endl;
    
    if (stats.total_requests > 0) {
        double success_rate = (static_cast<double>(stats.successful_requests) / stats.total_requests) * 100.0;
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Success Rate:         " << success_rate << "%" << std::endl;
        std::cout << "Avg Processing Time:  " << stats.avg_processing_time_us << " μs" << std::endl;
        std::cout << "                      " << (stats.avg_processing_time_us / 1000.0) << " ms" << std::endl;
    }
    
    // Print regex cache stats
    auto cache_stats = RegexCache::getInstance().getStats();
    std::cout << "Total Lookups:        " << cache_stats.total_lookups << std::endl;
    std::cout << "Cache Hits:           " << cache_stats.cache_hits << std::endl;
    std::cout << "Cache Misses:         " << cache_stats.cache_misses << std::endl;
    std::cout << "Pattern Matches:      " << cache_stats.pattern_matches << std::endl;
    std::cout << "Pattern Misses:       " << cache_stats.pattern_misses << std::endl;
    
    if (cache_stats.total_lookups > 0) {
        double hit_rate = (static_cast<double>(cache_stats.cache_hits) / cache_stats.total_lookups) * 100.0;
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Cache Hit Rate:       " << hit_rate << "%" << std::endl;
    }
}
