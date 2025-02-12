
#include "GETRequestHandler.hpp"


std::string GETRequestHandler::generateErrorResponse(int statusCode, const std::string& message) {
	std::stringstream response;
	response << "HTTP/1.1 " << statusCode << " " << message << "\r\n";
	response << "Content-Type: text/plain\r\n";
	response << "Content-Length: " << message.length() << "\r\n";
	response << "\r\n";
	response << message;
	return response.str();
}

std::string GETRequestHandler::generateSuccessResponse(const std::string& content) {
	std::stringstream response;
	response << "HTTP/1.1 200 OK\r\n";
	response << "Content-Type: text/plain\r\n";
	response << "Content-Length: " << content.length() << "\r\n";
	response << "\r\n";
	response << content;
	return response.str();
}



std::string GETRequestHandler::handleRequest(const std::string& rawRequest)
{
	HTTPRequest request;
	
	// Parse the request
	if (!request.parseRequest(rawRequest)) {
		return generateErrorResponse(400, "Bad Request");
	}
	
	// Validate the request
	if (!request.validateGETRequest()) {
		return generateErrorResponse(400, "Bad Request");
	}
	
	// Handle the request based on path
	std::string path = request.getpath();
	
	// Here you would typically:
	// 1. Check if the requested resource exists
	// 2. Check permissions
	// 3. Read the file or generate content
	// 4. Generate appropriate response
	
	// For now, we'll just return a success response
	return generateSuccessResponse("Resource found!");
}