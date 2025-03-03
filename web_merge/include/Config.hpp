#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <vector>
#include <map>
#include <set>

struct Location {
	std::string path;
	std::string root;
	std::string index;
	bool autoindex;
	std::set<std::string> allowedMethods;
	std::map<std::string, std::string> errorPages;
	std::map<int, std::string> redirects;
	std::string cgiExtension;
	std::string cgiHandler;
	size_t clientMaxBodySize;
	
	Location() : autoindex(false), clientMaxBodySize(1048576) {} // Default 1MB
};

class ServerConfig {
	public:
		std::string host;
		int port;
		std::vector<std::string> serverNames;
		std::string root;
		std::vector<Location> locations;
		std::map<std::string, std::string> errorPages;
		size_t clientMaxBodySize;
		
		ServerConfig();
		// get server name
		std::string getServerName() const;
		// get host
		std::string getHost() const;
		// get port
		int getPort() const;
		// get root
		std::string getRoot() const;
		// get locations
		const std::vector<Location>& getLocations() const;
		// get error pages
		const std::map<std::string, std::string>& getErrorPages() const;
		// get client max body size
		size_t getClientMaxBodySize() const;


};

class Config {
	private:
		std::vector<ServerConfig> _servers;
		std::string _configPath;
		
		// Parsing helpers
		bool parseFile();
		bool parseServer(std::vector<std::string>& tokens, size_t& pos, ServerConfig& server);
		bool parseLocation(std::vector<std::string>& tokens, size_t& pos, Location& location);
		
		// Tokenization
		std::vector<std::string> tokenize(const std::string& content);
		
	public:
		Config();
		~Config();
		
		bool load(const std::string& configPath);
		
		// Config getters
		const std::vector<ServerConfig>& getServers() const;
		ServerConfig* getServerByHostPort(const std::string& host, int port);
		ServerConfig* getServerByName(const std::string& serverName, int port);
		
		// Location matching
		Location* matchLocation(ServerConfig* server, const std::string& uri);
		
		// Debug
		void dump() const;
};

#endif // CONFIG_HPP