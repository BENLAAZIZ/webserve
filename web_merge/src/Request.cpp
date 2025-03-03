#include "Request.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>

Request::Request() : 
    _parseState(PARSE_START_LINE), 
    _parsePosition(0),
    _method(UNKNOWN),
    _chunkSize(0),
    _lastChunk(false) {
}

Request::~Request() {
}

void Request::appendData(const char* data, size_t length) {
    _buffer.append(data, length);
}

void Request::clear() {
    _parseState = PARSE_START_LINE;
    _parsePosition = 0;
    _method = UNKNOWN;
    _uri.clear();
    _httpVersion.clear();
    _headers.clear();
    _body.clear();
    _buffer.clear();
    _chunkSize = 0;
    _lastChunk = false;
}

bool Request::parse() {
    bool continueParsingLoop = true;
    
    while (continueParsingLoop) {
        switch (_parseState) {
            case PARSE_START_LINE:
                continueParsingLoop = parseStartLine();
                break;
            case PARSE_HEADERS:
                continueParsingLoop = parseHeaders();
                break;
            case PARSE_BODY:
                continueParsingLoop = parseBody();
                break;
            case PARSE_CHUNKED_BODY:
                continueParsingLoop = parseChunkedBody();
                break;
            case PARSE_COMPLETE:
                return true;
            case PARSE_ERROR:
                return false;
        }
    }
    
    return _parseState == PARSE_COMPLETE;
}

bool Request::parseStartLine() {
    // Find the end of the start line
    size_t endOfLine = _buffer.find("\r\n", _parsePosition);
    if (endOfLine == std::string::npos) {
        return false; // Need more data
    }
    
    // Extract the line
    std::string startLine = _buffer.substr(_parsePosition, endOfLine - _parsePosition);
    _parsePosition = endOfLine + 2; // Skip CRLF
    
    // Parse method, URI, and HTTP version
    std::istringstream iss(startLine);
    std::string methodStr;
    
    if (!(iss >> methodStr >> _uri >> _httpVersion)) {
        _parseState = PARSE_ERROR;
        return false;
    }
    
    // Convert method string to enum
    _method = stringToMethod(methodStr);
    if (_method == UNKNOWN) {
        _parseState = PARSE_ERROR;
        return false;
    }
    
    // Move to headers parsing
    _parseState = PARSE_HEADERS;
    return true;
}

bool Request::parseHeaders() {
    // Parse headers until we find an empty line (CRLFCRLF)
    while (true) {
        size_t endOfLine = _buffer.find("\r\n", _parsePosition);
        if (endOfLine == std::string::npos) {
            return false; // Need more data
        }
        
        // Check if this is the empty line indicating the end of headers
        if (endOfLine == _parsePosition) {
            _parsePosition += 2; // Skip the empty line
            
            // Check if we expect a body
            if (_headers.count("Content-Length") > 0) {
                _parseState = PARSE_BODY;
            } else if (_headers.count("Transfer-Encoding") > 0 && 
                      _headers["Transfer-Encoding"].find("chunked") != std::string::npos) {
                _parseState = PARSE_CHUNKED_BODY;
            } else {
                // No body expected, request is complete
                _parseState = PARSE_COMPLETE;
            }
            return true;
        }
        
        // Extract the header line
        std::string headerLine = _buffer.substr(_parsePosition, endOfLine - _parsePosition);
        _parsePosition = endOfLine + 2; // Skip CRLF
        
        // Find the colon separator
        size_t colon = headerLine.find(':');
        if (colon == std::string::npos) {
            _parseState = PARSE_ERROR;
            return false;
        }
        
        // Extract key and value
        std::string key = headerLine.substr(0, colon);
        std::string value = headerLine.substr(colon + 1);
        
        // Trim whitespace
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);
        
        // Store header
        _headers[key] = value;
    }
    
    return true;
}

bool Request::parseBody() {
    // Get the expected content length
    size_t contentLength = std::stoul(_headers["Content-Length"]);
    
    // Check if we have enough data
    size_t availableData = _buffer.size() - _parsePosition;
    if (availableData < contentLength) {
        return false; // Need more data
    }
    
    // Extract the body
    _body = _buffer.substr(_parsePosition, contentLength);
    _parsePosition += contentLength;
    
    // Request is complete
    _parseState = PARSE_COMPLETE;
    return true;
}

bool Request::parseChunkedBody() {
    while (true) {
        // If we're starting a new chunk
        if (_chunkSize == 0 && !_lastChunk) {
            // Find the chunk size line
            size_t endOfLine = _buffer.find("\r\n", _parsePosition);
            if (endOfLine == std::string::npos) {
                return false; // Need more data
            }
            
            // Extract the chunk size in hex
            std::string chunkSizeHex = _buffer.substr(_parsePosition, endOfLine - _parsePosition);
            _parsePosition = endOfLine + 2; // Skip CRLF
            
            // Convert hex to decimal
            std::istringstream iss(chunkSizeHex);
            iss >> std::hex >> _chunkSize;
            
            // Check if this is the last chunk
            if (_chunkSize == 0) {
                _lastChunk = true;
                // Skip the final CRLF
                if (_buffer.size() - _parsePosition < 2) {
                    return false; // Need more data
                }
                _parsePosition += 2;
                _parseState = PARSE_COMPLETE;
                return true;
            }
        }
        
        // Check if we have enough data for the current chunk
        if (_buffer.size() - _parsePosition < _chunkSize + 2) {
            return false; // Need more data
        }
        
        // Extract the chunk data
        _body.append(_buffer.substr(_parsePosition, _chunkSize));
        _parsePosition += _chunkSize + 2; // Skip chunk data and CRLF
        
        // Reset for next chunk
        _chunkSize = 0;
    }
    
    return true;
}

RequestMethod Request::stringToMethod(const std::string& method) {
    if (method == "GET") return GET;
    if (method == "POST") return POST;
    if (method == "DELETE") return DELETE;
    return UNKNOWN;
}

bool Request::isComplete() const {
    return _parseState == PARSE_COMPLETE;
}

bool Request::hasError() const {
    return _parseState == PARSE_ERROR;
}

RequestMethod Request::getMethod() const {
    return _method;
}

const std::string& Request::getUri() const {
    return _uri;
}

const std::string& Request::getHttpVersion() const {
    return _httpVersion;
}

const std::map<std::string, std::string>& Request::getHeaders() const {
    return _headers;
}

std::string Request::getHeader(const std::string& key) const {
    auto it = _headers.find(key);
    if (it != _headers.end()) {
        return it->second;
    }
    return "";
}

const std::string& Request::getBody() const {
    return _body;
}

void Request::dump() const {
    std::cout << "=== Request Dump ===" << std::endl;
    std::cout << "Method: " << (_method == GET ? "GET" : (_method == POST ? "POST" : (_method == DELETE ? "DELETE" : "UNKNOWN"))) << std::endl;
    std::cout << "URI: " << _uri << std::endl;
    std::cout << "HTTP Version: " << _httpVersion << std::endl;
    
    std::cout << "Headers:" << std::endl;
    for (const auto& header : _headers) {
        std::cout << " - " << header.first << ": " << header.second << std::endl;
    }
    
    std::cout << "Body Length: " << _body.size() << " bytes" << std::endl;
    std::cout << "Parse State: " << _parseState << std::endl;
    std::cout << "===================" << std::endl;
}