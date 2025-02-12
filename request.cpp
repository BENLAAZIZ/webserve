#include "requestPars.hpp"

HTTPRequest::HTTPRequest()
{
	method = "";
	path = "";
	version = "";
}

HTTPRequest::~HTTPRequest()
{
}

bool HTTPRequest::parseRequest(const std::string& rawRequest)
{
	std::istringstream requestStream(rawRequest);
		std::string requestLine;
		
		std::cout << "********************************: " <<  std::endl;
		// Parse Request-Line
		if (!std::getline(requestStream, requestLine))
			return false;
		
		// Remove carriage return if present
		if (!requestLine.empty() && requestLine[requestLine.length()-1] == '\r')
			requestLine = requestLine.substr(0, requestLine.length()-1);
		
		// Parse method, path, and version
		std::istringstream requestLineStream(requestLine);
		if (!(requestLineStream >> method >> path >> version))
			return false;
			
		// Verify it's a GET request
		if (method != "GET")
			return false;
		std::cout << "********************************: " <<  std::endl;
		std::cout << "Method: " << method << std::endl;
		std::cout << "Path: " << path << std::endl;
		std::cout << "Version: " << version << std::endl;

		// Parse headers
		std::string headerLine;
		while (std::getline(requestStream, headerLine) && headerLine != "\r" && headerLine != "") 
		{
			// Remove carriage return if present
			if (!headerLine.empty() && headerLine[headerLine.length()-1] == '\r')
				headerLine = headerLine.substr(0, headerLine.length()-1);
				
			size_t colonPos = headerLine.find(':');
			if (colonPos == std::string::npos)
				continue;
			
			std::string key = trim(headerLine.substr(0, colonPos));
			std::string value = trim(headerLine.substr(colonPos + 1));
			headers[key] = value;
		}
		std::cout << "Headers:\n";
		for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
			std::cout << "|"<< it->first <<  "|" << ": " << "|"<< it->second <<  "|" << std::endl;
		std::cout << "-------------------------- \n" <<  std::endl;
		std::cout << "|\rhamza|" <<  std::endl;
		std::cout << "-------------------------- \n" <<  std::endl;
		
		return true;
}

// Getters
std::string HTTPRequest::getMethod() const 
{ return method; }

std::string HTTPRequest::getpath() const 
{ return path; }

std::string HTTPRequest::getVersion() const 
{ return version; }

std::string HTTPRequest::getHeader(const std::string& key) const
{
	std::map<std::string, std::string>::const_iterator it = headers.find(key);
	return (it != headers.end()) ? it->second : "";
}

bool HTTPRequest::validateGETRequest() const 
{
	// 1. Check if method is GET
	if (method != "GET")
		return false;
		
	// 2. Check HTTP version (must be HTTP/1.1)
	if (version != "HTTP/1.1")
		return false;
		
	// 3. path must start with /
	if (path.empty() || path[0] != '/')
		return false;

	// 4. Check for mandatory headers in HTTP/1.1
	if (version == "HTTP/1.1")
	{
		if (headers.find("Host") == headers.end())
			return false;
	}

	return true;
}