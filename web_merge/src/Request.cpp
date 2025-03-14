/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hben-laz <hben-laz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/04 18:00:58 by hben-laz          #+#    #+#             */
/*   Updated: 2025/03/14 17:30:04 by hben-laz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "../include/web.h"

Request::Request()
{
	statusCode.code = 200; // default status code
	statusCode.message = "OK";
	content_length = 0;
	flag_end_of_headers = false;
	headersParsed = false;
	bodyFlag = false;
	transferEncodingExist = false;
}

Request::~Request()
{
}

Request::Request(const Request& other)
{
	*this = other;
}

Request& Request::operator=(const Request& other)
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

// Getters
std::string Request::getMethod() const 
{ return method; }

std::string Request::getpath() const 
{ return path; }

std::string Request::getVersion() const 
{ return version; }

std::string Request::getExtension() const 
{ return extension; }

std::string Request::getHeader(const std::string& key) const
{
	std::map<std::string, std::string>::const_iterator it = headers.find(key);
	return (it != headers.end()) ? it->second : "";
}
std::string Request::getStatusCodeMessage() const 
{ return statusCode.message; }

std::string Request::getBody() const 
{ return body;}

std::size_t Request::getContentLength() const
{
	return content_length;
}

std::string Request::getBoundary() const
{
	return boundary;
}

bool Request::getFlagEndOfHeaders() const 
{ return flag_end_of_headers; }

bool Request::getBodyFlag() const
{
	return bodyFlag;
}
int Request::getStatusCode() const 
{ return statusCode.code; }

bool Request::getTransferEncodingExist() const
{
	return transferEncodingExist;
}

const std::map<std::string, std::string>& Request::getHeaders() const { 
	return headers; 
}

std::string Request::getContent_type() const
{
	return content_type;
}

/*=========== setters =============*/

void Request::setBody(const std::string& body) 
{ this->body = body; }

void Request::setMethod(const std::string& method) 
{ this->method = method; }

void Request::setPath(const std::string& path) 
{ this->path = path; }

void Request::setVersion(const std::string& version) 
{ this->version = version; }

void Request::setContentLength(int contentLength) 
{ this->content_length = contentLength; }

void Request::setHeader(const std::string& key, const std::string& value) {
    headers[key] = value;
}

void Request::setFlagEndOfHeaders(bool flag) 
{ flag_end_of_headers = flag; }


void Request::setBodyFlag(bool flag)
{
	bodyFlag = flag;
}

void Request::setTransferEncodingExist(bool flag)
{
	transferEncodingExist = flag;
}

void Request::setContent_type(const std::string& content_type)
{
	this->content_type = content_type;
}

void Request::setBoundary(const std::string& boundary)
{
	this->boundary = boundary;
}

/*=========== sendErrorResponse =============*/

void Request::sendErrorResponse(int errorCode) 
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
		case 505: errorMessage = "505  Version Not Supported"; break;
		default: errorMessage = "500 Internal Server Error"; break;
	}
	statusCode.message = errorMessage;
}

/*=========== parseFirstLine =============*/

bool Request::parseFirstLine(const std::string& line)
{
	std::cout << "Parsing first line: == " << line << std::endl;
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
	// if (checkPath())
	// 		return false;
	initializeEncode();
	size_t start = 0;
	while ((start = path.find("%", start)) != path.npos) {
		
		path.replace(start, 3, encode[path.substr(start, 3)]);
	}
	setMethod(method);
	setPath(path);
	setVersion(version);

	//std::cout << "Method: |" << method << "|\nPath: |" << path << "|\nVersion: |" << version << "|" << std::endl;
	return true;
}

void	Request::reset()
{
	method.clear();
	path.clear();
	version.clear();
	extension.clear();
	headers.clear();
	body.clear();
	content_length = 0;
	flag_end_of_headers = false;
	headersParsed = false;
	bodyFlag = false;
	transferEncodingExist = false;
}

void Request::initializeEncode(){
	encode["%20"] = " ";
	encode["%21"] = "!";
	encode["%22"] = "\"";
	encode["%23"] = "#";
	encode["%24"] = "$";
	encode["%25"] = "%";
	encode["%26"] = "&";
	encode["%27"] = "\'";
	encode["%28"] = "(";
	encode["%29"] = ")";
	encode["%2A"] = "*";
	encode["%2B"] = "+";
	encode["%2C"] = ",";
	encode["%2F"] = "/";
	encode["%3A"] = ":";
	encode["%3B"] = ";";
	encode["%3D"] = "=";
	encode["%3F"] = "?";
	encode["%40"] = "@";
	encode["%5B"] = "[";
	encode["%5D"] = "]";
}

bool Request::checkPath(){
	if (path.size() > 2048)
		return false; //status code 414
	for (size_t i = 0; path[i]; ++i){
			if ((path[i] >= 'A' && path[i] <= 'Z') || (path[i] >= 'a' && path[i] <= 'z') || (path[i] >= '0' && path[i] <= '9'))
				continue; //400
			if (path[i] == '~' || path[i] == '!' || (path[i] >= '#' && path[i] <= '/') || path[i] == ':' || path[i] == ';' || path[i] == '=' || path[i] == '?' || path[i] == '@')
				continue;
			if (path[i] == '[' || path[i] == ']' || path[i] == '_')
				continue;
			statusCode.code = 400;
			return false;
	}
	size_t start = path.find("?");
	if (start != path.npos){
		path = path.substr(0, start);
	}
	return true;
}

std::string urlDecode(const std::string& str) {
	std::string decoded;

	for (std::size_t i = 0; i < str.size(); ++i) {
		if (str[i] == '+')
			decoded += ' ';
		else if (str[i] == '%' && i + 2 < str.size()) {
			std::string hex = str.substr(i + 1, 2);
			char c = static_cast<char>(std::strtol(hex.c_str(), NULL, 16));
			decoded += c;
			i += 2;
		}
		else
			decoded += str[i];
	}
	return decoded;
}