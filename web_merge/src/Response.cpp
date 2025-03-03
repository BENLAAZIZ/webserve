#include "Response.hpp"
#include <sstream>
#include <fstream>
#include <iostream>
#include <sys/stat.h>

Response::Response() : 
    _status(OK_200),
    _httpVersion("HTTP/1.1"),
    _isServingFile(false),
    _fileSize(0),
    _bytesSent(0),
    _state(RESPONSE_BUILDING),
    _bufferPosition(0) {
}

Response::~Response() {
    if (_fileStream.is_open()) {
        _fileStream.close();
    }
}

void Response::setStatus(ResponseStatus status) {
    _status = status;
    _statusMessage = getStatusMessage(status);
}

void Response::setHeader(const std::string& key, const std::string& value) {
    _headers[key] = value;
}

void Response::setBody(const std::string& body) {
    _body = body;
    _headers["Content-Length"] = std::to_string(body.size());
}

bool Response::setFileToServe(const std::string& path) {
    // Check if file exists and is readable
    struct stat fileStat;
    if (stat(path.c_str(), &fileStat) != 0) {
        return false;
    }
    
    // Open the file
    _fileStream.open(path, std::ios::binary);
    if (!_fileStream.is_open()) {
        return false;
    }
    
    _filePath = path;
    _isServingFile = true;
    _fileSize = fileStat.st_size;
    _headers["Content-Length"] = std::to_string(_fileSize);
    _headers["Content-Type"] = getMimeType(path);
    
    return true;
}

void Response::buildResponse() {
    // First, add any missing headers
    buildHeaders();
    
    // Create the response header section
    std::stringstream ss;
    ss << _httpVersion << " " << _status << " " << _statusMessage << "\r\n";
    
    // Add headers
    for (const auto& header : _headers) {
        ss << header.first << ": " << header.second << "\r\n";
    }
    
    // End of headers
    ss << "\r\n";
    
    // Add body if not serving a file
    if (!_isServingFile) {
        ss << _body;
    }
    
    // Store in buffer
    _buffer = ss.str();
    _bufferPosition = 0;
    _state = RESPONSE_READY;
}

void Response::buildHeaders() {
    // Set date header
    // This would be better with a proper date/time library
    time_t now = time(0);
    struct tm* gmtm = gmtime(&now);
    char dateStr[100];
    strftime(dateStr, sizeof(dateStr), "%a, %d %b %Y %H:%M:%S GMT", gmtm);
    _headers["Date"] = dateStr;
    
    // Set server header
    _headers["Server"] = "webserv/1.0";
    
    // Set connection header
    if (_headers.find("Connection") == _headers.end()) {
        _headers["Connection"] = "keep-alive";
    }
    
    // Set content-type if not already set
    if (_headers.find("Content-Type") == _headers.end() && !_body.empty()) {
        _headers["Content-Type"] = "text/html";
    }
}

bool Response::hasMoreData() const {
    if (_isServingFile) {
        return _bufferPosition < _buffer.size() || _bytesSent < _fileSize;
    } else {
        return _bufferPosition < _buffer.size();
    }
}

const char* Response::getNextDataChunk(size_t& length) {
    // If we still have header data to send
    if (_bufferPosition < _buffer.size()) {
        length = _buffer.size() - _bufferPosition;
        return _buffer.c_str() + _bufferPosition;
    }
    
    // If we're serving a file and have file data to send
    if (_isServingFile && _bytesSent < _fileSize) {
        // Buffer size for file reading
        const size_t BUFFER_SIZE = 8192;
        static char fileBuffer[BUFFER_SIZE];
        
        // Read from file
        _fileStream.read(fileBuffer, BUFFER_SIZE);
        length = _fileStream.gcount();
        
        return fileBuffer;
    }
    
    // No more data
    length = 0;
    return nullptr;
}

void Response::advancePosition(size_t sent) {
    // If we're still sending the header/non-file body
    if (_bufferPosition < _buffer.size()) {
        size_t headerRemaining = _buffer.size() - _bufferPosition;
        if (sent <= headerRemaining) {
            _bufferPosition += sent;
            return;
        }
        // We've sent all the header and some file data
        _bufferPosition = _buffer.size();
        sent -= headerRemaining;
    }
    
    // Update file position if we're serving a file
    if (_isServingFile) {
        _bytesSent += sent;
    }
    
    // Check if we're done
    if (!hasMoreData()) {
        _state = RESPONSE_COMPLETE;
    }
}

ResponseState Response::getState() const {
    return _state;
}

void Response::setState(ResponseState state) {
    _state = state;
}

void Response::reset() {
    _status = OK_200;
    _statusMessage = getStatusMessage(_status);
    _headers.clear();
    _body.clear();
    _buffer.clear();
    _bufferPosition = 0;
    
    if (_fileStream.is_open()) {
        _fileStream.close();
    }
    
    _filePath.clear();
    _isServingFile = false;
    _fileSize = 0;
    _bytesSent = 0;
    
    _state = RESPONSE_BUILDING;
}

void Response::generateErrorPage(ResponseStatus status) {
    setStatus(status);
    
    std::stringstream ss;
    ss << "<!DOCTYPE html>\n";
    ss << "<html>\n";
    ss << "<head><title>" << _status << " " << _statusMessage << "</title></head>\n";
    ss << "<body>\n";
    ss << "<h1>" << _status << " " << _statusMessage << "</h1>\n";
    ss << "<hr>\n";
    ss << "<p>webserv/1.0</p>\n";
    ss << "</body>\n";
    ss << "</html>";
    
    setBody(ss.str());
    setHeader("Content-Type", "text/html");
}

std::string Response::getStatusMessage(ResponseStatus status) {
    switch (status) {
        case OK_200: return "OK";
        case CREATED_201: return "Created";
        case NO_CONTENT_204: return "No Content";
        case MOVED_PERMANENTLY_301: return "Moved Permanently";
        case FOUND_302: return "Found";
        case BAD_REQUEST_400: return "Bad Request";
        case UNAUTHORIZED_401: return "Unauthorized";
        case FORBIDDEN_403: return "Forbidden";
        case NOT_FOUND_404: return "Not Found";
        case METHOD_NOT_ALLOWED_405: return "Method Not Allowed";
        case REQUEST_TIMEOUT_408: return "Request Timeout";
        case LENGTH_REQUIRED_411: return "Length Required";
        case PAYLOAD_TOO_LARGE_413: return "Payload Too Large";
        case URI_TOO_LONG_414: return "URI Too Long";
        case UNSUPPORTED_MEDIA_TYPE_415: return "Unsupported Media Type";
        case INTERNAL_SERVER_ERROR_500: return "Internal Server Error";
        case NOT_IMPLEMENTED_501: return "Not Implemented";
        case BAD_GATEWAY_502: return "Bad Gateway";
        case SERVICE_UNAVAILABLE_503: return "Service Unavailable";
        case GATEWAY_TIMEOUT_504: return "Gateway Timeout";
        case HTTP_VERSION_NOT_SUPPORTED_505: return "HTTP Version Not Supported";
        default: return "Unknown Status";
    }
}

std::string Response::getMimeType(const std::string& path) {
    // Extract file extension
    size_t dotPos = path.find_last_of('.');
    if (dotPos == std::string::npos) {
        return "application/octet-stream";
    }
    
    std::string ext = path.substr(dotPos + 1);
    
    // Convert to lowercase
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    // Common MIME types
    if (ext == "html" || ext == "htm") return "text/html";
    if (ext == "css") return "text/css";
    if (ext == "js") return "application/javascript";
    if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
    if (ext == "png") return "image/png";
    if (ext == "gif") return "image/gif";
    if (ext == "svg") return "image/svg+xml";
    if (ext == "ico") return "image/x-icon";
    if (ext == "txt") return "text/plain";
    if (ext == "pdf") return "application/pdf";
    if (ext == "xml") return "application/xml";
    if (ext == "json") return "application/json";
    
    // Default
    return "application/octet-stream";
}