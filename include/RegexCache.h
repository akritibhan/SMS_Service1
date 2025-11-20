#pragma once

#include <string>
#include <regex>
#include <unordered_map>
#include <vector>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <list>

struct PatternInfo {
    std::string pattern_str;
    std::shared_ptr<std::regex> compiled_regex;
    std::string entity_id;
    std::string content_id;
    std::string tm_id;
    
    mutable std::chrono::steady_clock::time_point last_used;
    mutable int hit_count = 0;
};

struct MatchResult {
    bool found = false;
    std::string entity_id;
    std::string content_id;
    std::string tm_id;
};

class RegexCache {
public:
    static RegexCache& getInstance();
    
    bool initialize(int max_cache_size);
    bool loadPatternsFromDB(const std::string& sender = "");
    bool refreshCache();
    
    MatchResult findMatch(const std::string& sender, const std::string& sms_content);
    
    void clearCache();
    int getCacheSize() const { return sender_patterns.size(); }
    
   
    struct Stats {
        uint64_t total_lookups = 0;
        uint64_t cache_hits = 0;
        uint64_t cache_misses = 0;
        uint64_t pattern_matches = 0;
        uint64_t pattern_misses = 0;
    };
    
    Stats getStats() const { return stats; }
    void resetStats() { stats = Stats(); }

private:
    RegexCache() = default;
    ~RegexCache() = default;
    RegexCache(const RegexCache&) = delete;
    RegexCache& operator=(const RegexCache&) = delete;
    
    std::shared_ptr<std::regex> compilePattern(const std::string& pattern);
    void updateLRU(const std::string& sender) const;
    void evictLRU();
    
    // Map: sender -> list of pattern info
    std::unordered_map<std::string, std::vector<PatternInfo>> sender_patterns;
    
    // LRU tracking
    mutable std::list<std::string> lru_list;
    mutable std::unordered_map<std::string, std::list<std::string>::iterator> lru_map;
    
    mutable std::shared_mutex cache_mutex;
    
    int max_size = 100000;
    
    mutable Stats stats;
    
    // Regex compilation options
    static constexpr std::regex_constants::syntax_option_type regex_flags = 
        std::regex_constants::ECMAScript | std::regex_constants::optimize;
};
