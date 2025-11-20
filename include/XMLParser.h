#pragma once

#include <string>

struct SMSRequest {
    std::string sender;
    std::string receiver;
    std::string sms_content;
};

struct SMSResponse {
    bool success = false;
    std::string entity_id;
    std::string content_id;
    std::string tm_id;
    std::string error_message;
};

class XMLParser {
public:
    static SMSRequest parseRequest(const std::string& xml_data);
    static std::string generateResponse(const SMSResponse& response);
    
private:
    static std::string escapeXML(const std::string& data);
};
