// #ifndef CLIENT_HPP
// #define CLIENT_HPP

// #include "Request.hpp"
// #include "Response.hpp"
// #include "Config.hpp"
// #include "CGI.hpp"
// #include <string>
// #include <time.h>

// enum ClientState {
//     CLIENT_READING,
//     CLIENT_PROCESSING,
//     CLIENT_WRITING,
//     CLIENT_ERROR,
//     CLIENT_DONE
// };

// class Client {
// private:
//     int _socket;
//     ClientState _state;
//     time_t _lastActive;
	
//     // Request and response
//     Request _request;
//     Response _response;
//     // CGI* _cgi;
	
//     // Configuration
//     ServerConfig* _serverConfig;
//     Config* _config;
	
//     // Buffer for incoming data
//     char _buffer[8192];
	
//     // Process methods
//     void processRequest();
//     void handleStaticFile(const std::string& path);
//     void handleCGI(const std::string& path, const std::string& pathInfo);
//     void handleDirectory(const std::string& path);
//     void handleError(ResponseStatus status);
	
//     // Path resolution
//     std::string resolveUri(const std::string& uri, Location* location);
//     bool checkPath(const std::string& path, bool allowDirectory = false);
//     std::string buildPath(const std::string& uri, Location* location);
	
// public:
//     Client(int socket, ServerConfig* serverConfig, Config* config);
//     ~Client();
	
//     // Socket I/O
//     bool readFromSocket();
//     bool writeToSocket();
	
//     // State management
//     bool wantsToRead() const;
//     bool wantsToWrite() const;
	
//     // Request processing
//     void process();
	
//     // Timeout management
//     bool hasTimedOut(time_t timeout) const;
//     void updateActivity();
// };

// #endif // CLIENT_HPP

// ============================================================================


#pragma once

#include <string>
#include <map>
#include <vector>
#include <netinet/in.h>
// #include "ServerConfig.hpp"
#include "Config.hpp"
#include "../include/Request.hpp"
#include "Response.hpp"

class Client {
	public:
		Request _request;
		Response _response;
		int _socket;
		struct sockaddr_in _address;
		std::string _requestBuffer;
		std::string _responseBuffer;
		bool request_Header_Complete;
		bool _responseSent;
		bool _keepAlive;
		
		// Request information
		// std::string _method;
		// std::string _uri;
		// std::string _httpVersion;
		// std::map<std::string, std::string> _headers;
		// std::string _body;
		
		// Server configuration
		ServerConfig _serverConfig;
		
		// Maximum request size (8MB)
		static const size_t MAX_REQUEST_SIZE = 8388608;

		// Helper methods for handling different HTTP methods
		void handleGetRequest();
		void handlePostRequest();
		void handleDeleteRequest();
		
		// Helper for determining content type
		std::string getContentType(const std::string& path);

	public:
		Client(int socket, struct sockaddr_in address, const ServerConfig& config);
		~Client();
		
		// Read data from client
		// bool readRequest();
		
		// Check if the request is complete
		bool isRequestComplete();
		
		// Parse HTTP request
		bool parse_Header_Request(std::string& line_buf);
		
		// Generate HTTP response
		void generateResponse();
		
		// Send HTTP response
		bool sendResponse();
		
		// Send error response
		void sendErrorResponse(int statusCode, const std::string& statusMessage);
		
		// Check if response is completely sent
		bool isDoneWithResponse() const;
		
		// Check if connection should be kept alive
		bool keepAlive() const;
		
		// Reset client for new request
		void reset();
};