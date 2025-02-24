/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parseConfig.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aben-cha <aben-cha@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 16:10:57 by aben-cha          #+#    #+#             */
/*   Updated: 2025/02/19 11:49:58 by aben-cha         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>

// Class Location;
// Class Server;
// class ConfigParser;

std::string trim(const std::string& str, const std::string& tokens) {
    size_t first = str.find_first_not_of(tokens);
    if (first == std::string::npos) 
        return "";
    size_t last = str.find_last_not_of(tokens);
    return str.substr(first, last - first + 1);
}

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        token = trim(token, " \t");
        if (!token.empty())
            tokens.push_back(token);
    }
    return tokens;
}

class Location {
    private:
        std::string path;
        std::string root;
        std::string index;
        std::string upload_store;
        std::vector<std::string> allowed_methods;
        std::string cgi_extension;
        std::string cgi_handler;
        bool autoindex;
        std::size_t client_max_body_size;
        int redirect_code;
        std::string redirect_url;
    public:
        Location() : autoindex(false), client_max_body_size(0), redirect_code(0) {}

        // Getters
        std::string getPath() const { return path; }
        std::string getRoot() const { return root; }
        std::string getIndex() const { return index; }
        std::string getUploadStore() const { return upload_store; }
        std::vector<std::string> getAllowedMethods() const { return allowed_methods; }
        std::string getCgiExtension() const { return cgi_extension; }
        std::string getCgiHandler() const { return cgi_handler; }
        bool isAutoindex() const { return autoindex; }
        std::size_t getClientMaxBodySize() const { return client_max_body_size; }
        int getRedirectCode() const { return redirect_code; }
        std::string getRedirectUrl() const { return redirect_url; }
        
        // Setters
        void setPath(const std::string& p) { path = p; }
        void setRoot(const std::string& r) { root = r; }
        void setIndex(const std::string& i) { index = i; }
        void setUploadStore(const std::string& u) { upload_store = u; }
        void setAllowedMethods(const std::vector<std::string>& methods) { allowed_methods = methods; }
        void setCgiExtension(const std::string& ext) { cgi_extension = ext; }
        void setCgiHandler(const std::string& handler) { cgi_handler = handler; }
        void setAutoindex(bool a) { autoindex = a; }
        void setClientMaxBodySize(std::size_t size) { client_max_body_size = size; }
        void setRedirectCode(int code) { redirect_code = code; }
        void setRedirectUrl(const std::string& url) { redirect_url = url; }

        void parseLocation(std::ifstream& file, Location& loc, const std::string& line) {
                // Extract location path
            // ConfigParser conf;
            size_t pathStart = line.find("/");
            size_t pathEnd = line.find("{");
            std::cout << "pathStart: " << pathStart << std::endl;
            std::cout << "pathEnd: " << pathEnd << std::endl;
            if (pathStart != std::string::npos && pathEnd != std::string::npos) {
                loc.path = trim(line.substr(pathStart, pathEnd - pathStart), " \t");
                std::cout << "path: '" << loc.path << "'" << std::endl;
            }

            std::string configLine;
            while (std::getline(file, configLine)) {
                configLine = trim(configLine, " \t");

                // Skip empty lines and comments
                if (configLine.empty() || configLine[0] == '#')
                    continue;
                // Check for end of location block
                if (configLine == "}")
                    break;
                
                
                std::vector<std::string> tokens = split(configLine, ' ');
                if (tokens.empty())
                    continue;

                if (tokens[0] == "root")
                    loc.setRoot(tokens[1]);
                else if (tokens[0] == "index")
                    loc.setIndex(tokens[1]);
                else if (tokens[0] == "allowed_methods") {
                    for (size_t i = 1; i < tokens.size(); ++i){}
                        // loc.setAllowedMethods(&tokens[i]);
                        // loc.getAllowedMethods().push_back(tokens[i]);
                }
                else if (tokens[0] == "autoindex")
                    loc.setAutoindex((tokens[1] == "on"));
                else if (tokens[0] == "upload_store")
                    loc.setUploadStore(tokens[1]);
                else if (tokens[0] == "client_max_body_size")
                    loc.setClientMaxBodySize(std::atol(tokens[1].c_str()));
                else if (tokens[0] == "cgi_extension")
                    loc.setCgiExtension(tokens[1]);
                else if (tokens[0] == "cgi_handler")
                    loc.setCgiHandler(tokens[1]);
                else if (tokens[0] == "redirect") {
                    loc.setRedirectCode(std::atoi(tokens[1].c_str()));
                    loc.setRedirectUrl(tokens[2]);
                }
            }
        }
};

class Server {
    private:
        int port;
        std::string host;
        std::size_t client_max_body_size;
        std::map<int, std::string> error_pages;
        std::vector<Location> locations;
    
    public:
        Server() : port(80), client_max_body_size(1024 * 1024) {} // Default 1MB

        // Getters
        int getPort() const { return port; }
        std::string getHost() const { return host; }
        std::size_t getClientMaxBodySize() const { return client_max_body_size; }
        std::map<int, std::string> getErrorPages() const { return error_pages; }
        std::vector<Location> getLocations() const { return locations; }
        
        // Setters
        void setPort(int p) { port = p; }
        void setHost(const std::string& h) { host = h; }
        void setClientMaxBodySize(std::size_t size) { client_max_body_size = size; }

        // errors
        // void setErrorPages(const std::map<int, std::string>& pages) { error_pages = pages; }
        void addErrorPage(int code, const std::string& page) { error_pages[code] = page; }
        
        // locations
        // void setLocations(const std::vector<Location>& locs) { locations = locs; }
        void addLocation(const Location& loc) { locations.push_back(loc); }

};


class ConfigParser {
    private:
        std::stack<int> nbrs;
    public:


        void parseConfig(const std::string& filename) {
            Server server;
            
            std::ifstream file(filename);
            int i = 0;
            std::string line;
            if (!file.is_open())
                throw std::runtime_error("Error opening file for reading");

            while (std::getline(file, line)) {
                line = trim(line, " \t");
                if (line.empty() || line.at(0) == '#')
                    continue ;

                std::vector<std::string> tokens = split(line, ' ');
                if (tokens[0] == "server")
                    i++;
                // if (i != 0)
                    // throw std::runtime_error("")

                if (tokens[0] == "listen")
                    server.setPort(std::atoi(tokens[1].c_str()));
                else if (tokens[0] == "host")
                    server.setHost(tokens[1]);
                else if (tokens[0] == "client_max_body_size")
                    server.setClientMaxBodySize(std::atol(tokens[1].c_str()));
                else if (tokens[0] == "error_page" && tokens.size() >= 3)
                    server.addErrorPage(std::atoi(tokens[1].c_str()), tokens[2]);
                else if (tokens[0] == "location") {
                    Location loc;
                    loc.parseLocation(file, loc, line);
                    server.getLocations().push_back(loc);
                    throw std::runtime_error("Error: location block not closed");
                }
            }
        }
};

int main(int ac, char **av) {
    try {
        std::string filename;
        if (ac != 2) // Default config
            throw std::runtime_error("Invalid Argument.");
        filename = av[1];
        ConfigParser conf;
        conf.parseConfig(filename);

                // Print parsed configuration
        std::cout << "Server Configuration:\n";
        // std::cout << "Port: " << conf.getS << "\n";
        // std::cout << "Host: " << server.host << "\n";
        // std::cout << "Server Name: " << server.server_name << "\n";
        // std::cout << "Max Body Size: " << server.client_max_body_size << "\n";
        
        // Print locations
        for (size_t i = 0; i < server.locations.size(); ++i) {
            const Location& loc = server.locations[i];
            std::cout << "\nLocation " << loc.getPath() << ":\n";
            std::cout << "  Root: " << loc.getRoot() << "\n";
            std::cout << "  Index: " << loc.getIndex() << "\n";
            std::cout << "  Allowed Methods:";
            for (size_t j = 0; j < loc.getAllowedMethods().size(); ++j)
                std::cout << " " << loc.getAllowedMethods()[j];
            std::cout << "\n";
        }
    } catch(const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}


int main() {
    ConfigParser parser;
    Server server = parser.parseConfig("webserver.conf");
    

    
    return 0;
}
