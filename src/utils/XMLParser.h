#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <fstream>
#include <sstream>

namespace RPGEngine {
namespace Utils {

/**
 * XML node class
 * Represents a node in an XML document
 */
class XMLNode {
public:
    /**
     * Constructor
     * @param name Node name
     */
    XMLNode(const std::string& name) : m_name(name) {}
    
    /**
     * Destructor
     */
    ~XMLNode() {}
    
    /**
     * Get the node name
     * @return Node name
     */
    const std::string& getName() const { return m_name; }
    
    /**
     * Get the node value
     * @return Node value
     */
    const std::string& getValue() const { return m_value; }
    
    /**
     * Set the node value
     * @param value Node value
     */
    void setValue(const std::string& value) { m_value = value; }
    
    /**
     * Get an attribute
     * @param name Attribute name
     * @param defaultValue Default value if attribute doesn't exist
     * @return Attribute value
     */
    std::string getAttribute(const std::string& name, const std::string& defaultValue = "") const {
        auto it = m_attributes.find(name);
        if (it != m_attributes.end()) {
            return it->second;
        }
        return defaultValue;
    }
    
    /**
     * Get an attribute as an integer
     * @param name Attribute name
     * @param defaultValue Default value if attribute doesn't exist
     * @return Attribute value as an integer
     */
    int getAttributeInt(const std::string& name, int defaultValue = 0) const {
        auto it = m_attributes.find(name);
        if (it != m_attributes.end()) {
            try {
                return std::stoi(it->second);
            } catch (...) {
                return defaultValue;
            }
        }
        return defaultValue;
    }
    
    /**
     * Get an attribute as a float
     * @param name Attribute name
     * @param defaultValue Default value if attribute doesn't exist
     * @return Attribute value as a float
     */
    float getAttributeFloat(const std::string& name, float defaultValue = 0.0f) const {
        auto it = m_attributes.find(name);
        if (it != m_attributes.end()) {
            try {
                return std::stof(it->second);
            } catch (...) {
                return defaultValue;
            }
        }
        return defaultValue;
    }
    
    /**
     * Get an attribute as a boolean
     * @param name Attribute name
     * @param defaultValue Default value if attribute doesn't exist
     * @return Attribute value as a boolean
     */
    bool getAttributeBool(const std::string& name, bool defaultValue = false) const {
        auto it = m_attributes.find(name);
        if (it != m_attributes.end()) {
            return it->second == "true" || it->second == "1";
        }
        return defaultValue;
    }
    
    /**
     * Set an attribute
     * @param name Attribute name
     * @param value Attribute value
     */
    void setAttribute(const std::string& name, const std::string& value) {
        m_attributes[name] = value;
    }
    
    /**
     * Check if an attribute exists
     * @param name Attribute name
     * @return true if the attribute exists
     */
    bool hasAttribute(const std::string& name) const {
        return m_attributes.find(name) != m_attributes.end();
    }
    
    /**
     * Get all attributes
     * @return Map of attributes
     */
    const std::unordered_map<std::string, std::string>& getAttributes() const {
        return m_attributes;
    }
    
    /**
     * Add a child node
     * @param node Child node
     */
    void addChild(std::shared_ptr<XMLNode> node) {
        m_children.push_back(node);
    }
    
    /**
     * Get a child node by name
     * @param name Child node name
     * @return Child node, or nullptr if not found
     */
    std::shared_ptr<XMLNode> getChild(const std::string& name) const {
        for (const auto& child : m_children) {
            if (child->getName() == name) {
                return child;
            }
        }
        return nullptr;
    }
    
    /**
     * Get all child nodes
     * @return Vector of child nodes
     */
    const std::vector<std::shared_ptr<XMLNode>>& getChildren() const {
        return m_children;
    }
    
    /**
     * Get all child nodes with a specific name
     * @param name Child node name
     * @return Vector of child nodes
     */
    std::vector<std::shared_ptr<XMLNode>> getChildrenByName(const std::string& name) const {
        std::vector<std::shared_ptr<XMLNode>> result;
        for (const auto& child : m_children) {
            if (child->getName() == name) {
                result.push_back(child);
            }
        }
        return result;
    }
    
private:
    std::string m_name;                                        // Node name
    std::string m_value;                                       // Node value
    std::unordered_map<std::string, std::string> m_attributes; // Node attributes
    std::vector<std::shared_ptr<XMLNode>> m_children;          // Child nodes
};

/**
 * XML parser class
 * Simple XML parser for TMX files
 */
class XMLParser {
public:
    /**
     * Constructor
     */
    XMLParser() {}
    
    /**
     * Destructor
     */
    ~XMLParser() {}
    
    /**
     * Parse an XML file
     * @param filename XML file path
     * @return Root node, or nullptr if parsing failed
     */
    std::shared_ptr<XMLNode> parseFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            return nullptr;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        
        return parseString(buffer.str());
    }
    
    /**
     * Parse an XML string
     * @param xml XML string
     * @return Root node, or nullptr if parsing failed
     */
    std::shared_ptr<XMLNode> parseString(const std::string& xml) {
        size_t pos = 0;
        return parseNode(xml, pos);
    }
    
private:
    /**
     * Parse an XML node
     * @param xml XML string
     * @param pos Current position in the string
     * @return Node, or nullptr if parsing failed
     */
    std::shared_ptr<XMLNode> parseNode(const std::string& xml, size_t& pos) {
        // Skip whitespace
        skipWhitespace(xml, pos);
        
        // Check for end of string
        if (pos >= xml.length()) {
            return nullptr;
        }
        
        // Check for XML declaration
        if (xml.substr(pos, 5) == "<?xml") {
            // Skip XML declaration
            pos = xml.find("?>", pos);
            if (pos == std::string::npos) {
                return nullptr;
            }
            pos += 2;
            
            // Skip whitespace
            skipWhitespace(xml, pos);
        }
        
        // Check for comment
        if (xml.substr(pos, 4) == "<!--") {
            // Skip comment
            pos = xml.find("-->", pos);
            if (pos == std::string::npos) {
                return nullptr;
            }
            pos += 3;
            
            // Skip whitespace
            skipWhitespace(xml, pos);
        }
        
        // Check for opening tag
        if (xml[pos] != '<') {
            return nullptr;
        }
        pos++;
        
        // Get node name
        size_t nameStart = pos;
        while (pos < xml.length() && xml[pos] != ' ' && xml[pos] != '>' && xml[pos] != '/') {
            pos++;
        }
        
        std::string nodeName = xml.substr(nameStart, pos - nameStart);
        auto node = std::make_shared<XMLNode>(nodeName);
        
        // Parse attributes
        skipWhitespace(xml, pos);
        while (pos < xml.length() && xml[pos] != '>' && xml[pos] != '/') {
            // Get attribute name
            size_t attrNameStart = pos;
            while (pos < xml.length() && xml[pos] != '=') {
                pos++;
            }
            
            std::string attrName = xml.substr(attrNameStart, pos - attrNameStart);
            attrName = trim(attrName);
            
            // Skip '='
            pos++;
            
            // Skip whitespace
            skipWhitespace(xml, pos);
            
            // Check for quote
            if (xml[pos] != '"' && xml[pos] != '\'') {
                return nullptr;
            }
            
            char quote = xml[pos];
            pos++;
            
            // Get attribute value
            size_t attrValueStart = pos;
            while (pos < xml.length() && xml[pos] != quote) {
                pos++;
            }
            
            std::string attrValue = xml.substr(attrValueStart, pos - attrValueStart);
            
            // Skip closing quote
            pos++;
            
            // Add attribute
            node->setAttribute(attrName, attrValue);
            
            // Skip whitespace
            skipWhitespace(xml, pos);
        }
        
        // Check for self-closing tag
        if (xml.substr(pos, 2) == "/>") {
            pos += 2;
            return node;
        }
        
        // Skip '>'
        if (xml[pos] != '>') {
            return nullptr;
        }
        pos++;
        
        // Parse child nodes or text content
        size_t contentStart = pos;
        while (pos < xml.length()) {
            // Check for closing tag
            if (xml.substr(pos, 2 + nodeName.length()) == "</" + nodeName + ">") {
                // Set node value (text content)
                std::string content = xml.substr(contentStart, pos - contentStart);
                content = trim(content);
                if (!content.empty()) {
                    node->setValue(content);
                }
                
                // Skip closing tag
                pos += 2 + nodeName.length() + 1;
                
                return node;
            }
            
            // Check for child node
            if (xml[pos] == '<' && xml[pos + 1] != '/') {
                // Parse child node
                auto childNode = parseNode(xml, pos);
                if (childNode) {
                    node->addChild(childNode);
                }
                
                // Update content start
                contentStart = pos;
            } else {
                pos++;
            }
        }
        
        return nullptr;
    }
    
    /**
     * Skip whitespace characters
     * @param xml XML string
     * @param pos Current position in the string
     */
    void skipWhitespace(const std::string& xml, size_t& pos) {
        while (pos < xml.length() && (xml[pos] == ' ' || xml[pos] == '\t' || xml[pos] == '\n' || xml[pos] == '\r')) {
            pos++;
        }
    }
    
    /**
     * Trim whitespace from a string
     * @param str String to trim
     * @return Trimmed string
     */
    std::string trim(const std::string& str) {
        size_t start = 0;
        while (start < str.length() && (str[start] == ' ' || str[start] == '\t' || str[start] == '\n' || str[start] == '\r')) {
            start++;
        }
        
        size_t end = str.length();
        while (end > start && (str[end - 1] == ' ' || str[end - 1] == '\t' || str[end - 1] == '\n' || str[end - 1] == '\r')) {
            end--;
        }
        
        return str.substr(start, end - start);
    }
};

} // namespace Utils
} // namespace RPGEngine