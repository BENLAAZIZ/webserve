/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hben-laz <hben-laz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/04 18:00:58 by hben-laz          #+#    #+#             */
/*   Updated: 2025/03/16 23:37:42 by hben-laz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "../include/web.h"

Request::Request()
{
	code = 200; // default status code
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
		code = other.code;
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

std::string Request::get_MimeType () const 
{ return extension; }

std::string Request::getHeader(const std::string& key) const
{
	std::map<std::string, std::string>::const_iterator it = headers.find(key);
	return (it != headers.end()) ? it->second : "";
}

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
{ return code; }

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

std::string	Request::get_error_missage(int errorCode) const
{
	std::string errorMessage = "";
	switch (errorCode) {
		case 400: errorMessage = "400 Bad Request"; break;
		case 403: errorMessage = "403 Forbidden"; break;
		case 404: errorMessage = "404 Not Found"; break;
		case 405: errorMessage = "405 Method Not Allowed"; break;
		case 411: errorMessage = "411 Length Required"; break;
		case 413: errorMessage = "413 Payload Too Large"; break;
		case 414: errorMessage = "414 Request-URI Too Long"; break;
		case 500: errorMessage = "500 Internal Server Error"; break;
		case 505: errorMessage = "505  Version Not Supported"; break;
		default: errorMessage = "500 Internal Server Error"; break;
	}
	return errorMessage;
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

void Request::set_status_code(int code)
{
	this->code = code;
}

/*=========== parseFirstLine =============*/

bool Request::parseFirstLine(const std::string& line)
{
	std::istringstream iss(line);
	std::string method, path, version;
	if (!(iss >> method >> path >> version))
		return (set_status_code(400), false);
	if (method != "GET" && method != "POST" && method != "DELETE")
		return (set_status_code(405), false);
	if (version != "HTTP/1.1")
		return (set_status_code(505), false);
	if (checkPath(path))
	{
		setMethod(method);
		setPath(path);
		setVersion(version);
	}
	else
		return false;
	return true;
}

void	Request::reset()
{
	method.clear();
	path.clear();
	version.clear();
	extension.clear();
	headers.clear();
	// body.clear();
	content_length = 0;
	flag_end_of_headers = false;
	headersParsed = false;
	bodyFlag = false;
	transferEncodingExist = false;
}


bool Request::checkPath(std::string& path){
	if (path.size() > 2048)
		return (set_status_code(414), false);
	if (path.empty() || path[0] != '/') 
		return (set_status_code(400), false);
	for (size_t i = 0; path[i]; ++i){
			if ((path[i] >= 'A' && path[i] <= 'Z') || (path[i] >= 'a' && path[i] <= 'z') || (path[i] >= '0' && path[i] <= '9'))
				continue;
			if (path[i] == '~' || path[i] == '!' || (path[i] >= '#' && path[i] <= '/') || path[i] == ':' || path[i] == ';' || path[i] == '=' || path[i] == '?' || path[i] == '@')
				continue;
			if (path[i] == '[' || path[i] == ']' || path[i] == '_')
				continue;
			return (set_status_code(400), false);
	}
	path = urlDecode(path);
	size_t queryPos = path.find('?');
	if (queryPos != std::string::npos)
		path = path.substr(0, queryPos);
	return true;
}

std::string Request::urlDecode(const std::string& str) {
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