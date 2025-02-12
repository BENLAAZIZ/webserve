
#ifndef REQUESTPARS_HPP
#define REQUESTPARS_HPP

#include <string>
#include <map>
#include <sstream>
#include <iostream>

class HTTPRequest 
{
	private:
		std::string method;
		std::string path;
		std::string version;
		std::map<std::string, std::string> headers;
		
		// Helper function to trim whitespace
		std::string trim(const std::string& str)
		{
			size_t first = str.find_first_not_of(" \t\n\r");
			size_t last = str.find_last_not_of(" \t\n\r");
			if (first == std::string::npos || last == std::string::npos)
				return "";
			return str.substr(first, last - first + 1);
		}

	public:
		HTTPRequest();
		~HTTPRequest();
		bool parseRequest(const std::string& rawRequest) ;
		
		// Getters
		std::string getMethod() const ;
		std::string getpath() const ;
		std::string getVersion() const ;
		std::string getHeader(const std::string& key) const ;
		
		bool validateGETRequest() const ;
};
// GET /index.html HTTP/1.1\r\n
// Host: localhost:8080\r\n
// User-Agent: curl/7.64.1\r\n
// Accept: */*
// class GETRequestHandler {
// public:
//     std::string handleRequest(const std::string& rawRequest) {
//         HTTPRequest request;
		
//         // Parse the request
//         if (!request.parseRequest(rawRequest)) {
//             return generateErrorResponse(400, "Bad Request");
//         }
		
//         // Validate the request
//         if (!request.validateGETRequest()) {
//             return generateErrorResponse(400, "Bad Request");
//         }
		
//         // Handle the request based on path
//         std::string path = request.getpath();
		
//         // Here you would typically:
//         // 1. Check if the requested resource exists
//         // 2. Check permissions
//         // 3. Read the file or generate content
//         // 4. Generate appropriate response
		
//         // For now, we'll just return a success response
//         return generateSuccessResponse("Resource found!");
//     }
	
// private:
//     std::string generateErrorResponse(int statusCode, const std::string& message) {
//         std::stringstream response;
//         response << "HTTP/1.1 " << statusCode << " " << message << "\r\n";
//         response << "Content-Type: text/plain\r\n";
//         response << "Content-Length: " << message.length() << "\r\n";
//         response << "\r\n";
//         response << message;
//         return response.str();
//     }
	
//     std::string generateSuccessResponse(const std::string& content) {
//         std::stringstream response;
//         response << "HTTP/1.1 200 OK\r\n";
//         response << "Content-Type: text/plain\r\n";
//         response << "Content-Length: " << content.length() << "\r\n";
//         response << "\r\n";
//         response << content;
//         return response.str();
//     }
// };
	

#endif