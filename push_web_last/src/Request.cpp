
#include "../include/web.h"

Request::Request()
{
	// statusCode.code = 200; // default status code
	code = 200; // default status code
	// statusCode.message = "OK";
	content_length = 0;
	flag_end_of_headers = false;
	headersParsed = false;
	bodyFlag = false;
	transferEncodingExist = false;

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

// void Request::initializeEncode(){
// 	encode["%20"] = " ";
// 	encode["%21"] = "!";
// 	encode["%22"] = "\"";
// 	encode["%23"] = "#";
// 	encode["%24"] = "$";
// 	encode["%25"] = "%";
// 	encode["%26"] = "&";
// 	encode["%27"] = "\'";
// 	encode["%28"] = "(";
// 	encode["%29"] = ")";
// 	encode["%2A"] = "*";
// 	encode["%2B"] = "+";
// 	encode["%2C"] = ",";
// 	encode["%2F"] = "/";
// 	encode["%3A"] = ":";
// 	encode["%3B"] = ";";
// 	encode["%3D"] = "=";
// 	encode["%3F"] = "?";
// 	encode["%40"] = "@";
// 	encode["%5B"] = "[";
// 	encode["%5D"] = "]";
// }

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
	std::string pathCopy = path;
	size_t queryPos = path.find('?');
	if (queryPos != std::string::npos)
	{
		pathCopy = path.substr(queryPos, path.size());
		// std::cout << "pathCopy: " << pathCopy << std::endl;
		path = path.substr(0, queryPos);
		// std::cout << "path: " << path << std::endl;
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
            if (getChunkedSize(request._requestBuffer, request.chunked, pos))
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

	// std::cout << "Buffer: " << request.buffer << std::endl;
	request._requestBuffer.clear();
	while (1) {
		request.formData.pos = request.buffer.find(request.boundary);
        if (request.formData.pos == std::string::npos && !request.formData.flag)
            break;
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