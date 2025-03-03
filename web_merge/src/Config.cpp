#include "Config.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>

Config::Config() {
}

Config::~Config() {
}

bool Config::load(const std::string& configPath) {
    _configPath = configPath;
    return parseFile();
}

bool Config::parseFile() {
    std::ifstream file(_configPath);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open config file: " << _configPath << std::endl;
        return false;
    }
    
    // Read the entire file
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    
    // Tokenize the content
    std::vector<std::string> tokens = tokenize(content);
    
    // Parse tokens
    size_t pos = 0;
    while (pos < tokens.size()) {
        if (tokens[pos] == "server" && pos + 1 < tokens.size() && tokens[pos + 1] == "{") {
            ServerConfig server;
            pos += 2; // Skip "server" and "{"
            
            if (!parseServer(tokens, pos, server)) {
                return false;
            }
            
            _servers.push_back(server);
        } else {
            std::cerr << "Error: Expected 'server {' at position " << pos << std::endl;
            return false;
        }
    }
    
    return !_servers.empty();
}

bool Config::parseServer(std::vector<std::string>& tokens, size_t& pos, ServerConfig& server) {
    while (pos < tokens.size() && tokens[pos] != "}") {
        if (tokens[pos] == "listen" && pos + 2 < tokens.size() && tokens[pos + 2] == ";") {
            // Parse listen directive
            std::string hostPort = tokens[pos + 1];
            size_t colonPos = hostPort.find(':');
            
            if (colonPos != std::string::npos) {
                server.host = hostPort.substr(0, colonPos);
                server.port = std::stoi(hostPort.substr(colonPos + 1));
            } else {
                try {
                    server.port = std::stoi(hostPort);
                    server.host = "0.0.0.0"; // Default to all interfaces
                } catch (const std::exception&) {
                    server.host = hostPort;
                    server.port = 80; // Default port
                }
            }
            
            pos += 3; // Skip "listen", "<host:port>", and ";"
        } else if (tokens[pos] == "server_name" && pos + 1 < tokens.size()) {
            // Parse server_name directive
            pos++; // Skip "server_name"
            
            while (pos < tokens.size() && tokens[pos] != ";") {
                server.serverNames.push_back(tokens[pos]);
                pos++;
            }
            
            if (pos < tokens.size()) {
                pos++; // Skip ";"
            } else {
                std::cerr << "Error: Expected ';' after server_name" << std::endl;
                return false;
            }
        } else if (tokens[pos] == "root" && pos + 2 < tokens.size() && tokens[pos + 2] == ";") {
            // Parse root directive
            server.root = tokens[pos + 1];
            pos += 3; // Skip "root", "<path>", and ";"
        } else if (tokens[pos] == "client_max_body_size" && pos + 2 < tokens.size() && tokens[pos + 2] == ";") {
            // Parse client_max_body_size directive
            std::string sizeStr = tokens[pos + 1];
            size_t multiplier = 1;
            
            if (sizeStr.back() == 'k' || sizeStr.back() == 'K') {
                multiplier = 1024;
                sizeStr.pop_back();
            } else if (sizeStr.back() == 'm' || sizeStr.back() == 'M') {
                multiplier = 1024 * 1024;
                sizeStr.pop_back();
            } else if (sizeStr.back() == 'g' || sizeStr.back() == 'G') {
                multiplier = 1024 * 1024 * 1024;
                sizeStr.pop_back();
            }
            
            server.clientMaxBodySize = std::stoull(sizeStr) * multiplier;
            pos += 3; // Skip "client_max_body_size", "<size>", and ";"
        } else if (tokens[pos] == "error_page" && pos + 3 < tokens.size() && tokens[pos + 3] == ";") {
            // Parse error_page directive
            std::string status = tokens[pos + 1];
            std::string path = tokens[pos + 2];
            
            server.errorPages[status] = path;
            pos += 4; // Skip "error_page", "<status>", "<path>", and ";"
        } else if (tokens[pos] == "location" && pos + 2 < tokens.size() && tokens[pos + 2] == "{") {
            // Parse location block
            Location location;
            location.path = tokens[pos + 1];
            
            pos += 3; // Skip "location", "<path>", and "{"
            
            if (!parseLocation(tokens, pos, location)) {
                return false;
            }
            
            server.locations.push_back(location);
        } else {
            std::cerr << "Error: Unknown directive in server context: " << tokens[pos] << std::endl;
            return false;
        }
    }
    
    // Skip closing bracket
    if (pos < tokens.size()) {
        pos++;
    } else {
        std::cerr << "Error: Unexpected end of file while parsing server block" << std::endl;
        return false;
    }
    
    return true;
}

bool Config::parseLocation(std::vector<std::string>& tokens, size_t& pos, Location& location) {
    while (pos < tokens.size() && tokens[pos] != "}") {
        if (tokens[pos] == "root" && pos + 2 < tokens.size() && tokens[pos + 2] == ";") {
            location.root = tokens[pos + 1];
            pos += 3; // Skip "root", "<path>", and ";"
        } else if (tokens[pos] == "index" && pos + 2 < tokens.size() && tokens[pos + 2] == ";") {
            location.index = tokens[pos + 1];
            pos += 3; // Skip "index", "<file>", and ";"
        } else if (tokens[pos] == "autoindex" && pos + 2 < tokens.size() && tokens[pos + 2] == ";") {
            location.autoindex = (tokens[pos + 1] == "on");
            pos += 3; // Skip "autoindex", "on/off", and ";"
        } else if (tokens[pos] == "client_max_body_size" && pos + 2 < tokens.size() && tokens[pos + 2] == ";") {
            std::string sizeStr = tokens[pos + 1];
            size_t multiplier = 1;
            
            if (sizeStr.back() == 'k' || sizeStr.back() == 'K') {
                multiplier = 1024;
                sizeStr.pop_back();
            } else if (sizeStr.back() == 'm' || sizeStr.back() == 'M') {
                multiplier = 1024 * 1024;
                sizeStr.pop_back();
            } else if (sizeStr.back() == 'g' || sizeStr.back() == 'G') {
                multiplier = 1024 * 1024 * 1024;
                sizeStr.pop_back();
            }
            
            location.clientMaxBodySize = std::stoull(sizeStr) * multiplier;
            pos += 3; // Skip "client_max_body_size", "<size>", and ";"
        } else if (tokens[pos] == "allow_methods" && pos + 1 < tokens.size()) {
            pos++; // Skip "allow_methods"
            
            while (pos < tokens.size() && tokens[pos] != ";") {
                location.allowedMethods.insert(tokens[pos]);
                pos++;
            }
            
            if (pos < tokens.size()) {
                pos++; // Skip ";"
            } else {
                std::cerr << "Error: Expected ';' after allow_methods" << std::endl;
                return false;
            }
        } else if (tokens[pos] == "return" && pos + 3 < tokens.size() && tokens[pos + 3] == ";") {
            int status = std::stoi(tokens[pos + 1]);
            std::string url = tokens[pos + 2];
            
            location.redirects[status] = url;
            pos += 4; // Skip "return", "<status>", "<url>", and ";"
        } else if (tokens[pos] == "error_page" && pos + 3 < tokens.size() && tokens[pos + 3] == ";") {
            std::string status = tokens[pos + 1];
            std::string path = tokens[pos + 2];
            
            location.errorPages[status] = path;
            pos += 4; // Skip "error_page", "<status>", "<path>", and ";"
        } else if (tokens[pos] == "cgi_extension" && pos + 2 < tokens.size() && tokens[pos + 2] == ";") {
            location.cgiExtension = tokens[pos + 1];
            pos += 3; // Skip "cgi_extension", "<ext>", and ";"
        } else if (tokens[pos] == "cgi_handler" && pos + 2 < tokens.size() && tokens[pos + 2] == ";") {
            location.cgiHandler = tokens[pos + 1];
            pos += 3; // Skip "cgi_handler", "<path>", and ";"
        } else {
            std::cerr << "Error: Unknown directive in location context: " << tokens[pos] << std::endl;
            return false;
        }
    }
    
    // Skip closing bracket
    if (pos < tokens.size()) {
        pos++;
    } else {
        std::cerr << "Error: Unexpected end of file while parsing location block" << std::endl;
        return false;
    }
    
    return true;
}

std::vector<std::string> Config::tokenize(const std::string& content) {
    std::vector<std::string> tokens;
    std::string token;
    bool inQuotes = false;
    bool inComment = false;
    
    for (size_t i = 0; i < content.size(); i++) {
        char c = content[i];
        
        // Handle comments
        if (c == '#') {
            inComment = true;
            continue;
        }
        
        if (inComment) {
            if (c == '\n') {
                inComment = false;
            }
            continue;
        }
        
        // Handle quotes
        if (c == '"') {
            if (inQuotes) {
                if (!token.empty()) {
                    tokens.push_back(token);
                    token.clear();
                }
                inQuotes = false;
            } else {
                inQuotes = true;
            }
            continue;
        }
        
        if (inQuotes) {
            token += c;
            continue;
        }
        
        // Handle whitespace
        if (std::isspace(c)) {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
            continue;
        }
        
        // Handle special characters
        if (c == '{' || c == '}' || c == ';') {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
            tokens.push_back(std::string(1, c));
            continue;
        }
        
        // Normal character
        token += c;
    }
    
    // Add the last token if it exists
    if (!token.empty()) {
        tokens.push_back(token);
    }
    
    return tokens;
}

const std::vector<ServerConfig>& Config::getServers() const {
    return _servers;
}

ServerConfig* Config::getServerByHostPort(const std::string& host, int port) {
    for (auto& server : _servers) {
        if (server.host == host && server.port == port) {
            return &server;
        }
    }
    return nullptr;
}

ServerConfig* Config::getServerByName(const std::string& serverName, int port) {
    for (auto& server : _servers) {
        if (server.port == port) {
            for (const auto& name : server.serverNames) {
                if (name == serverName) {
                    return &server;
                }
            }
        }
    }
    return nullptr;
}

Location* Config::matchLocation(ServerConfig* server, const std::string& uri) {
    if (!server) {
        return nullptr;
    }
    
    // Find longest matching prefix
    Location* bestMatch = nullptr;
    size_t bestMatchLength = 0;
    
    for (auto& location : server->locations) {
        if (uri.find(location.path) == 0) {
            size_t matchLength = location.path.length();
            if (matchLength > bestMatchLength) {
                bestMatch = &location;
                bestMatchLength = matchLength;
            }
        }
    }
    
    return bestMatch;
}

void Config::dump() const {
    std::cout << "=== Configuration Dump ===" << std::endl;
    std::cout << "Number of server blocks: " << _servers.size() << std::endl;
    
    for (size_t i = 0; i < _servers.size(); i++) {
        const auto& server = _servers[i];
        std::cout << "Server " << i + 1 << ":" << std::endl;
        std::cout << " - Host: " << server.host << std::endl;
        std::cout << " - Port: " << server.port << std::endl;
        std::cout << " - Server Names: ";
        for (const auto& name : server.serverNames) {
            std::cout << name << " ";
        }
        std::cout << std::endl;
        std::cout << " - Root: " << server.root << std::endl;
        std::cout << " - Client Max Body Size: " << server.clientMaxBodySize << " bytes" << std::endl;
        
        std::cout << " - Error Pages:" << std::endl;
        for (const auto& page : server.errorPages) {
            std::cout << "   - " << page.first << ": " << page.second << std::endl;
        }
        
        std::cout << " - Locations:" << std::endl;
        for (const auto& location : server.locations) {
            std::cout << "   - Path: " << location.path << std::endl;
            std::cout << "     Root: " << location.root << std::endl;
            std::cout << "     Index: " << location.index << std::endl;
            std::cout << "     Autoindex: " << (location.autoindex ? "on" : "off") << std::endl;
            std::cout << "     Allowed Methods: ";
            for (const auto& method : location.allowedMethods) {
                std::cout << method << " ";
            }
            std::cout << std::endl;
            std::cout << "     CGI Extension: " << location.cgiExtension << std::endl;
            std::cout << "     CGI Handler: " << location.cgiHandler << std::endl;
        }
    }
    
    std::cout << "=========================" << std::endl;
}

//========================================================

	ServerConfig::ServerConfig() : port(80), clientMaxBodySize(1048576) {} // Default 1MB
		// get server name
		std::string ServerConfig::getServerName() const{
            return serverNames[0];
        }
		// get host
		std::string ServerConfig::getHost() const{
            return host;
        }
		// get port
		int ServerConfig::getPort() const{
            return port;
        }
		// get root
		std::string ServerConfig::getRoot() const{
            return root;
        }
		// get locations
		const std::vector<Location>& ServerConfig::getLocations() const{
            return locations;
        }
		// get error pages
		const std::map<std::string, std::string>& ServerConfig::getErrorPages() const{
            return errorPages;
        }
		// get client max body size
		size_t ServerConfig::getClientMaxBodySize() const{
            return clientMaxBodySize;
        }