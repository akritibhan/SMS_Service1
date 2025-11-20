#include "XMLParser.h"
#include "../third_party/pugixml/src/pugixml.hpp"
#include <sstream>
#include <iostream>

SMSRequest XMLParser::parseRequest(const std::string& xml_data) {
    SMSRequest request;
    
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_string(xml_data.c_str());
    
    if (!result) {
        std::cerr << "XML parse error: " << result.description() << std::endl;
        return request;
    }
    
    // Parse XML structure - expecting:
    // <SMSRequest>
    //   <Sender>...</Sender>
    //   <Receiver>...</Receiver>
    //   <SMSContent>...</SMSContent>
    // </SMSRequest>
    
    pugi::xml_node root = doc.child("SMSRequest");
    if (!root) {
        std::cerr << "Invalid XML: Missing SMSRequest root element" << std::endl;
        return request;
    }
    
    request.sender = root.child_value("Sender");
    request.receiver = root.child_value("Receiver");
    request.sms_content = root.child_value("SMSContent");
    
    return request;
}

std::string XMLParser::generateResponse(const SMSResponse& response) {
    pugi::xml_document doc;
    
    // Create XML structure:
    // <SMSResponse>
    //   <Success>true/false</Success>
    //   <EntityID>...</EntityID>
    //   <ContentID>...</ContentID>
    //   <TMID>...</TMID>
    //   <ErrorMessage>...</ErrorMessage> (if error)
    // </SMSResponse>
    
    pugi::xml_node root = doc.append_child("SMSResponse");
    
    root.append_child("Success").text().set(response.success ? "true" : "false");
    
    if (response.success) {
        root.append_child("EntityID").text().set(response.entity_id.c_str());
        root.append_child("ContentID").text().set(response.content_id.c_str());
        root.append_child("TMID").text().set(response.tm_id.c_str());
    } else {
        root.append_child("ErrorMessage").text().set(response.error_message.c_str());
    }
    
    // Convert to string
    std::ostringstream oss;
    doc.save(oss, "  ", pugi::format_default, pugi::encoding_utf8);
    
    return oss.str();
}

std::string XMLParser::escapeXML(const std::string& data) {
    std::string buffer;
    buffer.reserve(data.size());
    
    for (char c : data) {
        switch (c) {
            case '&':  buffer.append("&amp;");  break;
            case '<':  buffer.append("&lt;");   break;
            case '>':  buffer.append("&gt;");   break;
            case '"':  buffer.append("&quot;"); break;
            case '\'': buffer.append("&apos;"); break;
            default:   buffer.push_back(c);     break;
        }
    }
    
    return buffer;
}
