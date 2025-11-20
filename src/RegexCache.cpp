#include "RegexCache.h"
#include "ConnectionPool.h"
#include <iostream>
#include <mysql.h>

RegexCache& RegexCache::getInstance() {
    static RegexCache instance;
    return instance;
}

bool RegexCache::initialize(int max_cache_size) {
    max_size = max_cache_size;
    std::cout << "RegexCache initialized with max size: " << max_size << std::endl;
    return true;
}

std::shared_ptr<std::regex> RegexCache::compilePattern(const std::string& pattern) {
    try {
        return std::make_shared<std::regex>(pattern, regex_flags);
    } catch (const std::regex_error& e) {
        std::cerr << "Regex compilation error for pattern '" << pattern << "': " << e.what() << std::endl;
        return nullptr;
    }
}

bool RegexCache::loadPatternsFromDB(const std::string& sender_filter) {
    auto conn = ConnectionPool::getInstance().acquireConnection();
    if (!conn) {
        std::cerr << "Failed to acquire database connection" << std::endl;
        return false;
    }
    
    ConnectionGuard guard(conn);
    MYSQL* mysql = guard.get();
    
    // Build query
    std::string query = "SELECT Sender, SMS_Content, Entity_ID, Content_ID, TM_ID FROM sms_patterns";
    if (!sender_filter.empty()) {
        query += " WHERE Sender = '" + sender_filter + "'";
    }
    
    if (mysql_query(mysql, query.c_str())) {
        std::cerr << "Query failed: " << mysql_error(mysql) << std::endl;
        return false;
    }
    
    MYSQL_RES* result = mysql_store_result(mysql);
    if (!result) {
        std::cerr << "Failed to store result: " << mysql_error(mysql) << std::endl;
        return false;
    }
    
    std::unique_lock<std::shared_mutex> lock(cache_mutex);
    
    // Clear existing cache if loading all
    if (sender_filter.empty()) {
        sender_patterns.clear();
        lru_list.clear();
        lru_map.clear();
    }
    
    MYSQL_ROW row;
    int loaded_count = 0;
    
    while ((row = mysql_fetch_row(result))) {
        std::string sender = row[0] ? row[0] : "";
        std::string pattern = row[1] ? row[1] : "";
        std::string entity_id = row[2] ? row[2] : "";
        std::string content_id = row[3] ? row[3] : "";
        std::string tm_id = row[4] ? row[4] : "";
        
        if (sender.empty() || pattern.empty()) {
            continue;
        }
        
        // Compile regex
        auto compiled = compilePattern(pattern);
        if (!compiled) {
            std::cerr << "Skipping invalid pattern for sender: " << sender << std::endl;
            continue;
        }
        
        PatternInfo info;
        info.pattern_str = pattern;
        info.compiled_regex = compiled;
        info.entity_id = entity_id;
        info.content_id = content_id;
        info.tm_id = tm_id;
        info.last_used = std::chrono::steady_clock::now();
        
        sender_patterns[sender].push_back(std::move(info));
        ++loaded_count;
        
        // Update LRU
        if (lru_map.find(sender) == lru_map.end()) {
            lru_list.push_front(sender);
            lru_map[sender] = lru_list.begin();
        }
    }
    
    mysql_free_result(result);
    
    std::cout << "Loaded " << loaded_count << " patterns for " 
              << sender_patterns.size() << " senders" << std::endl;
    
    return true;
}

bool RegexCache::refreshCache() {
    std::cout << "Refreshing regex cache..." << std::endl;
    return loadPatternsFromDB();
}

void RegexCache::updateLRU(const std::string& sender) const {
    auto it = lru_map.find(sender);
    if (it != lru_map.end()) {
        // Move to front
        lru_list.erase(it->second);
        lru_list.push_front(sender);
        lru_map[sender] = lru_list.begin();
    }
}

void RegexCache::evictLRU() {
    if (lru_list.empty()) return;
    
    std::string victim = lru_list.back();
    lru_list.pop_back();
    lru_map.erase(victim);
    sender_patterns.erase(victim);
    
    std::cout << "Evicted sender from cache: " << victim << std::endl;
}

MatchResult RegexCache::findMatch(const std::string& sender, const std::string& sms_content) {
    MatchResult result;
    ++stats.total_lookups;
    
    // Try read lock first for pattern matching
    {
        std::shared_lock<std::shared_mutex> lock(cache_mutex);
        
        auto it = sender_patterns.find(sender);
        if (it != sender_patterns.end()) {
            
            // Fast pattern matching - try substring first, then regex
            for (const auto& pattern_info : it->second) {
                try {
                    bool matched = false;
                    
                    // Check if pattern is simple (no regex metacharacters)
                    if (pattern_info.pattern_str.find_first_of(".*+?[]{}()^$|\\") == std::string::npos) {
                        // Fast case-insensitive substring search
                        auto it_content = std::search(
                            sms_content.begin(), sms_content.end(),
                            pattern_info.pattern_str.begin(), pattern_info.pattern_str.end(),
                            [](char ch1, char ch2) { return std::tolower(ch1) == std::tolower(ch2); }
                        );
                        matched = (it_content != sms_content.end());
                    } else {
                        // Use precompiled regex for complex patterns
                        matched = std::regex_search(sms_content, *pattern_info.compiled_regex);
                    }
                    
                    if (matched) {
                        result.found = true;
                        result.entity_id = pattern_info.entity_id;
                        result.content_id = pattern_info.content_id;
                        result.tm_id = pattern_info.tm_id;
                        
                        ++stats.pattern_matches;
                        
                        return result;
                    }
                } catch (const std::regex_error& e) {
                    // Silent continue on regex error
                }
            }
            
            ++stats.pattern_misses;
            return result;
        }
        
        ++stats.cache_misses;
    }
    
    // Cache miss - try to load from DB
    std::cout << "Cache miss for sender: " << sender << ", loading from DB" << std::endl;
    
    // Load patterns for this sender
    auto conn = ConnectionPool::getInstance().acquireConnection();
    if (!conn) {
        return result;
    }

    // Escape sender for SQL
    std::vector<char> escaped_sender(sender.length() * 2 + 1);
    mysql_real_escape_string(conn->get(), escaped_sender.data(), sender.c_str(), sender.length());

    std::string query = "SELECT SMS_Content, Entity_ID, Content_ID, TM_ID FROM sms_patterns WHERE Sender = '";
    query += escaped_sender.data();
    query += "'";
    
    if (mysql_query(conn->get(), query.c_str())) {
        std::cerr << "Query failed: " << mysql_error(conn->get()) << std::endl;
        ConnectionPool::getInstance().releaseConnection(conn);
        return result;
    }
    
    MYSQL_RES* db_result = mysql_store_result(conn->get());
    if (!db_result) {
        ConnectionPool::getInstance().releaseConnection(conn);
        return result;
    }
    
    // Read all data from result before releasing connection
    std::vector<std::tuple<std::string, std::string, std::string, std::string>> rows_data;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(db_result))) {
        std::string pattern = row[0] ? row[0] : "";
        std::string entity_id = row[1] ? row[1] : "";
        std::string content_id = row[2] ? row[2] : "";
        std::string tm_id = row[3] ? row[3] : "";
        rows_data.emplace_back(pattern, entity_id, content_id, tm_id);
    }
    
    mysql_free_result(db_result);
    ConnectionPool::getInstance().releaseConnection(conn);
    
    // Now safely update cache with write lock
    std::unique_lock<std::shared_mutex> lock(cache_mutex);
    
    // Check if another thread already loaded it
    if (sender_patterns.find(sender) != sender_patterns.end()) {
        return findMatch(sender, sms_content); // Retry
    }
    
    std::vector<PatternInfo> patterns;
    
    for (const auto& [pattern, entity_id, content_id, tm_id] : rows_data) {
        
        if (pattern.empty()) continue;
        
        auto compiled = compilePattern(pattern);
        if (!compiled) continue;
        
        PatternInfo info;
        info.pattern_str = pattern;
        info.compiled_regex = compiled;
        info.entity_id = entity_id;
        info.content_id = content_id;
        info.tm_id = tm_id;
        info.last_used = std::chrono::steady_clock::now();
        
        // Check if this pattern matches
        try {
            if (std::regex_search(sms_content, *compiled)) {
                result.found = true;
                result.entity_id = entity_id;
                result.content_id = content_id;
                result.tm_id = tm_id;
                ++info.hit_count;
            }
        } catch (const std::regex_error& e) {
            std::cerr << "Regex match error: " << e.what() << std::endl;
        }
        
        patterns.push_back(std::move(info));
    }
    
    // Add to cache
    if (!patterns.empty()) {
        sender_patterns[sender] = std::move(patterns);
        lru_list.push_front(sender);
        lru_map[sender] = lru_list.begin();
        
        // Check cache size and evict if needed
        if (sender_patterns.size() > static_cast<size_t>(max_size)) {
            evictLRU();
        }
    }
    
    return result;
}

void RegexCache::clearCache() {
    std::unique_lock<std::shared_mutex> lock(cache_mutex);
    sender_patterns.clear();
    lru_list.clear();
    lru_map.clear();
    std::cout << "Cache cleared" << std::endl;
}
