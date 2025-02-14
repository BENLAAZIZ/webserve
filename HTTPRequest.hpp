

#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <map>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cerrno>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <exception>

typedef struct StatusCode {
	std::size_t code;
	std::string message;
} StatusCode;

class HTTPRequest 
{
	private:
		std::string method;
		std::string path;
		std::string version;
		std::string extension;
		// int statusCode;

		StatusCode statusCode;

		std::map<std::string, std::string> headers;
		

	public:
		HTTPRequest();
		~HTTPRequest();
		bool parseRequest(const std::string& rawRequest) ;
		std::string trim(const std::string& str);
		
		// Getters
		std::string getMethod() const ;
		std::string getpath() const ;
		std::string getVersion() const ;
		std::string getExtension() const ;
		std::string getHeader(const std::string& key) const ;
		int getStatusCode() const ;
		const std::map<std::string, std::string>& getHeaders() const {
        	return headers;
    	}
	void sendErrorResponse(int errorCode) {
        std::string errorMessage = "";
    
    	switch (errorCode) {
			case 400: errorMessage = "400 Bad Request"; break;
			case 403: errorMessage = "403 Forbidden"; break;
			case 404: errorMessage = "404 Not Found"; break;
			case 405: errorMessage = "405 Method Not Allowed"; break;
			case 411: errorMessage = "411 Length Required"; break;
			case 413: errorMessage = "413 Payload Too Large"; break;
			case 414: errorMessage = "414 URI Too Long"; break;
			case 500: errorMessage = "500 Internal Server Error"; break;
			case 505: errorMessage = "505 HTTP Version Not Supported"; break;
			default: errorMessage = "500 Internal Server Error"; break;
    	}
    		// message = errorMessage;
			// // statusCode = errorCode;
			statusCode.message = errorMessage;
		}
		// Setters
		// void setStatusCode(int code) ;

		// void setStatusCodeMessage(const std::string& message) {
		// 	statusCode.message = message;
		// }

		std::string getStatusCodeMessage() const;
		
		
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