
#include "../include/web.h"

Request::Request()
{
	code = 200; // default status code
	content_length = 0;
	flag_end_of_headers = false;
	headersParsed = false;
	bodyFlag = false;
	transferEncodingExist = false;

	isCGI = false;
	extension = "";
	query = "";
	method = "";
	path = "";
	version = "";
	fake_path = "";

	//boundary
	formData.flag = 0;
	formData.pos = 0;
	formData.flagData = 0;
	formData.pos_new_line = 0;

	//initialize chunked struct
    chunked.bytesRead = 0;
    chunked.chunkSize = -1;
    chunked.bytesRemaining = 0;
    chunked.flag = 0;
	chunked.fd = -1;
	chunked.isFinished = false;

	// end of request
	endOfRequest = false;
	my_root = "/Users/hben-laz/Desktop/push_web";
}

Request::~Request()
{
}

Request::Request(const Request& other)
{
	*this = other;
}

void Request::set_status_code(int code)
{
	this->code = code;
}

Request& Request::operator=(const Request& other)
{
	if (this != &other)
	{
		method = other.method;
		path = other.path;
		version = other.version;
		extension = other.extension;
		query = other.query;
		body = other.body;
		content_length = other.content_length;
		flag_end_of_headers = other.flag_end_of_headers;
		headersParsed = other.headersParsed;
		bodyFlag = other.bodyFlag;
		transferEncodingExist = other.transferEncodingExist;
		// statusCode = other.statusCode;
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

std::string Request::getExtension() const 
{ return extension; }

std::string Request::getHeader(const std::string& key) const
{
	std::map<std::string, std::string>::const_iterator it = headers.find(key);
	return (it != headers.end()) ? it->second : "";
}
// std::string Request::getStatusCodeMessage() const 
// { return statusCode.message; }

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

bool Request::hasHeader(const std::string& key) const {
	return headers.find(key) != headers.end();
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
	// statusCode.message = errorMessage;
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
	code = 200;
	method.clear();
	path.clear();
	version.clear();
	extension.clear();
	query.clear();
	headers.clear();
	body.clear();
	content_length = 0;
	flag_end_of_headers = false;
	headersParsed = false;
	bodyFlag = false;
	transferEncodingExist = false;
	buffer.clear();
	_requestBuffer.clear();
	endOfRequest = false;

	boundary.clear();
	boundary_end.clear();
	strCGI.clear();
	// transferEncodingExist = false;



    //------------------------
	isCGI = false;
	fake_path.clear();

	
	//boundary
	formData.flag = 0;
	formData.pos = 0;
	formData.flagData = 0;
	formData.pos_new_line = 0;

	//initialize chunked struct
    chunked.bytesRead = 0;
    chunked.chunkSize = -1;
    chunked.bytesRemaining = 0;
    chunked.flag = 0;
	chunked.fd = -1;
	chunked.isFinished = false;                         
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
	{
		this->query  = path.substr(queryPos, path.size());
		path = path.substr(0, queryPos);
	}
	if (path.find("..") != std::string::npos) {
		std::cerr << "Directory traversal attempt: " << path << std::endl;
		return (set_status_code(403), false);
	}
	size_t dotPos = path.find_last_of('.');
		size_t cgiEndPos = std::string::npos;
	if (dotPos != std::string::npos)
	{
		// Check for known CGI extensions
		std::string ext = path.substr(dotPos, 4);  // get up to 4 characters to include .php, .py
		if (ext == ".php" || ext == ".py")
		{
			this->extension = ext;
			this->isCGI = true;
			// Now locate the end of CGI script: ".php" or ".py"
			cgiEndPos = path.find(ext) + ext.length();
			// Split into actual script path and query-like tail
			this->query = path.substr(cgiEndPos); // → /home/nnn
			this->path = path.substr(0, cgiEndPos); // → upload/test.php
			std::cout << "CGI script path: " << this->path << std::endl;
			std::cout << "CGI script query: " << this->query << std::endl;
			std::cout << "CGI script extension: " << this->extension << std::endl;
		}
	}
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



//---------------------------------(chunked data)
void Request::handleChunkedData(Request& request) {
    std::size_t pos;
    while (1) {
        if (request._requestBuffer.empty()) { 
            break;
        }
        pos = request._requestBuffer.find("\r\n");
        if (pos == std::string::npos && !request.chunked.flag)
            return ;
        if (request.chunked.chunkSize == -1)
            openFile(request.getHeaders().at("Content-Type"), request.chunked);

        if (!request.chunked.flag) {
            if (getChunkedSize(request._requestBuffer, request.chunked, pos, request.endOfRequest))
                return;
        }

		request.chunked.bytesRemaining = request.chunked.chunkSize - request.chunked.bytesRead;
        std::size_t chunkDataSize = std::min(request.chunked.bytesRemaining, request._requestBuffer.size());

        if (chunkDataSize > 0)
            writeData(request._requestBuffer, request.chunked, chunkDataSize);
        if (request.chunked.bytesRemaining == 0 && request.chunked.flag)
            endChunkedData(request._requestBuffer, request.chunked);
    }
}
//--------------------boundary-------------------

void Request::handleBoundary(Request& request) {
	request.buffer += request._requestBuffer;
	request._requestBuffer.clear();
	while (1) {
		// pause();
		request.formData.pos = request.buffer.find(request.boundary);
        if (request.formData.pos == std::string::npos && !request.formData.flag)
            break;
		std::cout << "***********>> handleBoundary" << std::endl;
        if (!request.formData.flag && request.formData.pos != std::string::npos)
			getHeaderBody(request);
		
		// At this point, we should have a file open
        if (!request.formData.flag) {
            // Something went wrong, no file is open
            std::cerr << "Error: Trying to write data without opening a file" << std::endl;
            request.buffer.clear();
            break;
        }

		// Find next boundary
        std::size_t nextBoundaryPos = request.buffer.find(request.boundary);
		if (nextBoundaryPos == std::string::npos) {
            if (request.buffer.size() > request.boundary.size())
                writeDataIfNoBoundary(request);
            break;
        }
		
		eraseBoundary(request, nextBoundaryPos);

		checkIfBoundaryEndReached(request);
	}
}


//--------------------boundaryWithChunked-------------------


void Request::handleChunkedBoundary(Request& request) {
	while (true) {
        if (request._requestBuffer.empty())
            break;

        // Handle chunked transfer
        std::size_t pos = request._requestBuffer.find("\r\n");
        if (pos == std::string::npos && !request.chunked.flag)
            return;

        if (!request.chunked.flag) {
            request.chunked.strHex = request._requestBuffer.substr(0, pos);
            request.chunked.chunkSize = hexToDecimal(request.chunked.strHex);

            if (request.chunked.chunkSize == 0) {
                std::cout << "END of Chunked Data\n";
                request.chunked.isFinished = true;
                // Don't close fd here - might close it prematurely
                // Only close when form processing is complete
                request._requestBuffer.erase(0, pos + 2);
                break;
            }
            request._requestBuffer.erase(0, pos + 2);
            request.chunked.flag = 1;
        }

        request.chunked.bytesRemaining = request.chunked.chunkSize - request.chunked.bytesRead;
        std::size_t chunkDataSize = std::min(request.chunked.bytesRemaining, request._requestBuffer.size());

        if (chunkDataSize > 0) {
            request.buffer += request._requestBuffer.substr(0, chunkDataSize);
            request.chunked.bytesRead += chunkDataSize;
            request._requestBuffer.erase(0, chunkDataSize);
        }

        // if (request.chunked.bytesRemaining == 0 && request.chunked.flag) {
        //     // if (request._requestBuffer.size() >= 2)
        //     //     request._requestBuffer.erase(0, 2);
        //     // request.chunked.flag = 0;
        //     // request.chunked.bytesRead = 0;
		// 	endChunkedData(request._requestBuffer, request.chunked);
        // }

		if ((request.chunked.bytesRemaining == 0) && request.chunked.flag)
            endChunkedData(request._requestBuffer, request.chunked);
    }

    if (request.buffer.empty())
        return;

	handleBoundary(request);
}


//set_fake_path(getpath)
std::string Request::get_fake_path() const
{
	return fake_path;
}


void Request::set_fake_path(std::string path)
{
	fake_path = path;
}
