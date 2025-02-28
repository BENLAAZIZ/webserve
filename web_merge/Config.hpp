/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hben-laz <hben-laz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/28 21:55:55 by hben-laz          #+#    #+#             */
/*   Updated: 2025/02/28 22:34:03 by hben-laz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <vector>
#include <map>
#include <set>

class Location {
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
	std::string host;
	int port;
	std::vector<std::string> serverNames;
	std::string root;
	std::vector<Location> locations;
	std::map<std::string, std::string> errorPages;
	size_t clientMaxBodySize;
	
	ServerConfig() : port(80), clientMaxBodySize(1048576) {} // Default 1MB
};

class Config {
	private:
		std::vector<ServerConfig> _servers;
		std::string _configPath;
		
		// Parsing helpers
		bool v();
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