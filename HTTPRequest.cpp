#include "HTTPRequest.hpp"

HTTPRequest::HTTPRequest()
{
	method = "";
	path = "";
	version = "";
	extension = ""; // should inisialize to empty string or not
	statusCode.code = 200; // default status code
	statusCode.message = "OK"; //
}

HTTPRequest::~HTTPRequest()
{
}

bool HTTPRequest::parseRequest(const std::string& rawRequest)
{
	std::istringstream requestStream(rawRequest);
		std::string requestLine;
		
		// Parse Request-Line
		if (!std::getline(requestStream, requestLine))
		{
			statusCode.code = 400; // bad Request
			return false;
		}
		
		// Remove carriage return if present
		if (!requestLine.empty() && requestLine[requestLine.length()-1] == '\r')
			requestLine = requestLine.substr(0, requestLine.length()-1);
		
		// Parse method, path, and version
		std::istringstream requestLineStream(requestLine);
		if (!(requestLineStream >> method >> path >> version))
		{
			statusCode.code = 400; // bad Request
			return false;
		}
			


		//============check first line
		// 1. Check if method is GET
		if (method != "GET" && method != "POST" && method != "DELETE") 
		{
			statusCode.code = 405; // Method Not Allowed
			return false;
		}

		if (method == "POST") {
			statusCode.code = 501; // Not Implemented
			return false;
		}


		// 2. Check HTTP version (must be HTTP/1.1)
		if (path.empty() || path[0] != '/')
		{
			statusCode.code = 400; // bad Request
			return false;
		}


		if (version != "HTTP/1.1")
		{
			statusCode.code = 505; // HTTP Version Not Supported
			return false;
		}

		
		std::size_t pos = path.rfind('.');
		if (pos != std::string::npos)
			extension = path.substr(pos + 1);

		std::string headerLine;
		while (std::getline(requestStream, headerLine) && headerLine != "\r" && headerLine != "") 
		{
			// Remove carriage return if present
			if (!headerLine.empty() && headerLine[headerLine.length()-1] == '\r')
				headerLine = headerLine.substr(0, headerLine.length()-1);
				
			size_t colonPos = headerLine.find(':');
			if (colonPos == std::string::npos)
			{
				statusCode.code = 400; // bad Request
				return false;
			}
			
			std::string key = trim(headerLine.substr(0, colonPos));
			std::string value = trim(headerLine.substr(colonPos + 1));
			headers[key] = value;
		}
		// Check for mandatory headers in HTTP/1.1
		if (headers.find("Host") == headers.end())
		{
			statusCode.code = 400; // bad Request
			return false;
		}

		return true;
}

std::string HTTPRequest::trim(const std::string& str)
{
	size_t first = str.find_first_not_of(" \t\n\r");
	size_t last = str.find_last_not_of(" \t\n\r");
	if (first == std::string::npos || last == std::string::npos)
		return "";
	return str.substr(first, last - first + 1);
}

// Getters
std::string HTTPRequest::getMethod() const 
{ return method; }

std::string HTTPRequest::getpath() const 
{ return path; }

std::string HTTPRequest::getVersion() const 
{ return version; }

std::string HTTPRequest::getExtension() const 
{ return extension; }

int HTTPRequest::getStatusCode() const 
{ return statusCode.code; }

// void HTTPRequest::setStatusCode(int code) 
// { statusCode = code; }
std::string HTTPRequest::getStatusCodeMessage() const {
			return statusCode.message;
		}
		
std::string HTTPRequest::getHeader(const std::string& key) const
{
	std::map<std::string, std::string>::const_iterator it = headers.find(key);
	return (it != headers.end()) ? it->second : "";
}
