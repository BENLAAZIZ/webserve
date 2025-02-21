/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hben-laz <hben-laz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 10:30:42 by hben-laz          #+#    #+#             */
/*   Updated: 2025/02/21 16:10:52 by hben-laz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPRequest.hpp"

HTTPRequest::HTTPRequest()
{
	// method = "";
	// path = "";
	// version = "";
	// extension = ""; 
	statusCode.code = 200; // default status code
	statusCode.message = "OK";
	contentLength = 0;
	flag_end_of_headers = false;
	headersParsed = false;
	bodyFlag = false;
}

HTTPRequest::~HTTPRequest()
{
}

HTTPRequest::HTTPRequest(const HTTPRequest& other)
{
	*this = other;
}

HTTPRequest& HTTPRequest::operator=(const HTTPRequest& other)
{
	if (this != &other)
	{
		method = other.method;
		path = other.path;
		version = other.version;
		extension = other.extension;
		statusCode = other.statusCode;
		headers = other.headers;
	}
	return *this;
}

// bool HTTPRequest::parseRequest(const std::string& rawRequest)
// {
// 	std::istringstream requestStream(rawRequest);
// 		std::string requestLine;
		
// 		// Parse Request-Line
// 		if (!std::getline(requestStream, requestLine))
// 		{
// 			statusCode.code = 400; // bad Request
// 			return false;
// 		}
// 		// Remove carriage return if present
// 		if (!requestLine.empty() && requestLine[requestLine.length()-1] == '\r')
// 			requestLine = requestLine.substr(0, requestLine.length()-1);
// 		// Parse method, path, and version
// 		std::istringstream requestLineStream(requestLine);
// 		if (!(requestLineStream >> method >> path >> version))
// 		{
// 			statusCode.code = 400;
// 			return false;
// 		}
// 		//============check first line of request================
// 		if (method != "GET" && method != "POST" && method != "DELETE") 
// 		{
// 			statusCode.code = 405; // Method Not Allowed
// 			return false;
// 		}
// 		// 2. Check HTTP version (must be HTTP/1.1)
// 		if (path.empty() || path[0] != '/')
// 		{
// 			statusCode.code = 400;
// 			return false;
// 		}
// 		if (version != "HTTP/1.1")
// 		{
// 			statusCode.code = 505; // HTTP Version Not Supported
// 			return false;
// 		}
// 		std::size_t pos = path.rfind('.');
// 		if (pos != std::string::npos)
// 			extension = path.substr(pos + 1);
// 		std::string headerLine;
// 		while (std::getline(requestStream, headerLine) && headerLine != "\r" && headerLine != "") 
// 		{
// 			// Remove carriage return if present
// 			if (!headerLine.empty() && headerLine[headerLine.length()-1] == '\r')
// 				headerLine = headerLine.substr(0, headerLine.length()-1);
// 			// Parse header
// 			size_t colonPos = headerLine.find(':');
// 			if (colonPos == std::string::npos)
// 			{
// 				statusCode.code = 400;
// 				return false;
// 			}
// 			std::string key = trim(headerLine.substr(0, colonPos));
// 			std::string value = trim(headerLine.substr(colonPos + 1));
// 			headers[key] = value;
// 		}
// 		// Check for mandatory headers in HTTP/1.1
// 		if (headers.find("Host") == headers.end())
// 		{
// 			statusCode.code = 400;
// 			return false;
// 		}
// 		return true;
// }




// Getters
std::string HTTPRequest::getMethod() const 
{ return method; }

std::string HTTPRequest::getpath() const 
{ return path; }

std::string HTTPRequest::getVersion() const 
{ return version; }

std::string HTTPRequest::getExtension() const 
{ return extension; }

std::string HTTPRequest::getHeader(const std::string& key) const
{
	std::map<std::string, std::string>::const_iterator it = headers.find(key);
	return (it != headers.end()) ? it->second : "";
}
std::string HTTPRequest::getStatusCodeMessage() const 
{ return statusCode.message; }

std::string HTTPRequest::getBody() const 
{ return body;}

std::size_t HTTPRequest::getContentLength() const
{
	return contentLength;
}

		
bool HTTPRequest::getFlagEndOfHeaders() const 
{ return flag_end_of_headers; }

bool HTTPRequest::getBodyFlag() const
{
	return bodyFlag;
}
int HTTPRequest::getStatusCode() const 
{ return statusCode.code; }

const std::map<std::string, std::string>& HTTPRequest::getHeaders() const { 
	return headers; 
}

/*=========== setters =============*/

void HTTPRequest::setBody(const std::string& body) 
{ this->body = body; }

void HTTPRequest::setMethod(const std::string& method) 
{ this->method = method; }

void HTTPRequest::setPath(const std::string& path) 
{ this->path = path; }

void HTTPRequest::setVersion(const std::string& version) 
{ this->version = version; }

void HTTPRequest::setContentLength(int contentLength) 
{ this->contentLength = contentLength; }

void HTTPRequest::setHeader(const std::string& key, const std::string& value) {
    headers[key] = value;
}

void HTTPRequest::setFlagEndOfHeaders(bool flag) 
{ flag_end_of_headers = flag; }


void HTTPRequest::setBodyFlag(bool flag)
{
	bodyFlag = flag;
}


/*=========== sendErrorResponse =============*/

void HTTPRequest::sendErrorResponse(int errorCode) 
{
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
	statusCode.message = errorMessage;
}

/*=========== parseFirstLine =============*/


bool HTTPRequest::parseFirstLine(const std::string& line)
{
	std::istringstream iss(line);
	std::string method, path, version;
	if (!(iss >> method >> path >> version)) {
		this->statusCode.code = 400;
		return false;
	}
	if (method != "GET" && method != "POST" && method != "DELETE") {
		this->statusCode.code = 405;
		return false;
	}
	if (path.empty() || path[0] != '/' || version != "HTTP/1.1") {
		this->statusCode.code = 400;
		return false;
	}

	// Store method, path, version
	setMethod(method);
	setPath(path);
	setVersion(version);

	std::cout << "Method: |" << method << "|\nPath: |" << path << "|\nVersion: |" << version << "|" << std::endl;
	return true;
}