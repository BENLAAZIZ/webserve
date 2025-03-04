



// #include "Client.hpp"
// #include <unistd.h>
// #include <sys/stat.h>
// #include <iostream>
// #include <fstream>
// #include <sstream>
// #include <dirent.h>
// #include <algorithm>

// Client::Client(int socket, ServerConfig* serverConfig, Config* config) : 
//     _socket(socket),
//     _state(CLIENT_READING),
//     // _cgi(nullptr),
//     _serverConfig(serverConfig),
//     _config(config) {
//     updateActivity();
// }

// Client::~Client() {
//     // if (_cgi) {
//     //     delete _cgi;
//     // }
// }

// bool Client::readFromSocket() {
//     // Only read if in reading state
//     if (_state != CLIENT_READING) {
//         return true;
//     }
    
//     // Read from socket
//     ssize_t bytesRead = read(_socket, _buffer, sizeof(_buffer));
    
//     if (bytesRead <= 0)
//     {
//         if (bytesRead == 0) {
//             // Connection closed by client
//             return false;
//         } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
//             // Read error
//             std::cerr << "Error reading from client: " << strerror(errno) << std::endl;
//             return false;
//         }
        
//         // Would block, so just return
//         return true;
//     }
    
//     // Update activity timestamp
//     updateActivity();
    
//     // Append data to request parser
//     _request.appendData(_buffer, bytesRead);
    
//     // Attempt to parse the request
//     bool parseResult = _request.parse();
    
//     // Check for parse errors
//     if (_request.hasError()) {
//         _state = CLIENT_ERROR;
//         handleError(BAD_REQUEST_400);
//         return true;
//     }
    
//     // Check if request is complete
//     if (_request.isComplete()) {
//         _state = CLIENT_PROCESSING;
//     }
    
// }

//==================================

#include "Client.hpp"
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <cstring>
#include <fstream>
#include <sys/stat.h>

Client::Client(int socket, struct sockaddr_in address, const ServerConfig& config)
    : _socket(socket), _address(address), _requestComplete(false), 
      _responseSent(false), _keepAlive(false), _serverConfig(config) {
}

Client::~Client() {
    // Socket is closed by the Server class
}

bool Client::readRequest() {
    char buffer[4096];
    ssize_t bytesRead = recv(_socket, buffer, sizeof(buffer) - 1, 0);
    
    if (bytesRead <= 0) {
        if (bytesRead == 0) {
            // Client closed connection
            return false;
        }
        
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            std::cerr << "Error reading from client: " << strerror(errno) << std::endl;
            return false;
        }
        
        // No data available at the moment
        return true;
    }
    
    // Add received data to buffer
    buffer[bytesRead] = '\0';
    _requestBuffer.append(buffer, bytesRead);
    
    // Check if the request is too large
    if (_requestBuffer.size() > MAX_REQUEST_SIZE) {
        std::cerr << "Request too large!" << std::endl;
        sendErrorResponse(413, "Request Entity Too Large");
        return false;
    }
    
    // Check if we have a complete request
    if (_requestBuffer.find("\r\n\r\n") != std::string::npos) {
        _requestComplete = true;
    }
    
    return true;
}

bool Client::isRequestComplete() {
    return _requestComplete;
}

bool Client::parseRequest(std::string& line_buf) 
{
	// Check for end of headers ("\r\n\r\n")
	size_t headerEndPos = line_buf.find("\r\n\r\n");
		if (headerEndPos != std::string::npos) {
			setFlagEndOfHeaders(true);
			// //std::cout << "End of headers" << std::endl;
		}
		// Process data line by line
}


// bool Client::parseRequest() {
    // std::istringstream requestStream(_requestBuffer);
    // std::string line;
    
    // // Parse request line
    // if (!std::getline(requestStream, line, '\r')) {
    //     return false;
    // }
    
    // // Skip the \n after \r
    // requestStream.get(); 
    
    // std::istringstream requestLineStream(line);
    // if (!(requestLineStream >> _method >> _uri >> _httpVersion)) {
    //     return false;
    // }
    
    // // Parse headers
    // while (std::getline(requestStream, line, '\r')) {
    //     // Skip the \n after \r
    //     requestStream.get();
        
    //     if (line.empty()) {
    //         break; // Empty line indicates end of headers
    //     }
        
    //     size_t colonPos = line.find(':');
    //     if (colonPos != std::string::npos) {
    //         std::string name = line.substr(0, colonPos);
    //         std::string value = line.substr(colonPos + 1);
            
    //         // Trim leading/trailing whitespace
    //         size_t firstNonSpace = value.find_first_not_of(" \t");
    //         if (firstNonSpace != std::string::npos) {
    //             value = value.substr(firstNonSpace);
    //         }
            
    //         _headers[name] = value;
    //     }
    // }
    
    // // Check for Connection: keep-alive header
    // auto it = _headers.find("Connection");
    // if (it != _headers.end() && it->second == "keep-alive") {
    //     _keepAlive = true;
    // }
    
    // // Read message body if Content-Length is provided
    // auto contentLengthIt = _headers.find("Content-Length");
    // if (contentLengthIt != _headers.end()) {
    //     size_t contentLength = std::stoul(contentLengthIt->second);
        
    //     // Get the current position in the stream
    //     std::streampos bodyPos = requestStream.tellg();
    //     if (bodyPos != -1) {
    //         // Extract body
    //         _body = _requestBuffer.substr(bodyPos, contentLength);
    //     }
    // }
    
    // return true;
// }

void Client::generateResponse() {
    // Route request based on method and URI
    if (_method == "GET") {
        handleGetRequest();
    } else if (_method == "POST") {
        handlePostRequest();
    } else if (_method == "DELETE") {
        handleDeleteRequest();
    } else {
        // Method not supported
        sendErrorResponse(501, "Method Not Implemented");
    }
}

void Client::handleGetRequest() {
    // Remove query parameters
    std::string path = _uri;
    size_t queryPos = path.find('?');
    if (queryPos != std::string::npos) {
        path = path.substr(0, queryPos);
    }
    
    // Sanitize path
    if (path == "/") {
        path = "/index.html"; // Default page
    }
    
    // Check for directory traversal attempts
    if (path.find("..") != std::string::npos) {
        sendErrorResponse(403, "Forbidden");
        return;
    }
    
    // Prepend document root from config
    std::string fullPath = _serverConfig.getDocumentRoot() + path;
    
    // Check if file exists
    struct stat fileStat;
    if (stat(fullPath.c_str(), &fileStat) == 0 && S_ISREG(fileStat.st_mode)) {
        // File exists, serve it
        std::ifstream file(fullPath, std::ios::binary);
        if (file) {
            // Determine content type based on file extension
            std::string contentType = getContentType(path);
            
            // Read file content
            std::string fileContent((std::istreambuf_iterator<char>(file)),
                                     std::istreambuf_iterator<char>());
            
            // Generate HTTP response
            std::ostringstream response;
            response << "HTTP/1.1 200 OK\r\n";
            response << "Content-Type: " << contentType << "\r\n";
            response << "Content-Length: " << fileContent.size() << "\r\n";
            if (_keepAlive) {
                response << "Connection: keep-alive\r\n";
            } else {
                response << "Connection: close\r\n";
            }
            response << "\r\n";
            response << fileContent;
            
            _responseBuffer = response.str();
        } else {
            // File exists but couldn't be opened
            sendErrorResponse(500, "Internal Server Error");
        }
    } else if (stat(fullPath.c_str(), &fileStat) == 0 && S_ISDIR(fileStat.st_mode)) {
        // Directory listing (optional, could redirect to index or show listing)
        sendErrorResponse(403, "Forbidden");
    } else {
        // File not found
        sendErrorResponse(404, "Not Found");
    }
}

void Client::handlePostRequest() {
    // Simple POST handler
    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n";
    response << "Content-Type: text/plain\r\n";
    
    std::string responseBody = "Received POST request to " + _uri + " with " + 
                              std::to_string(_body.length()) + " bytes of data\r\n";
    
    response << "Content-Length: " << responseBody.size() << "\r\n";
    if (_keepAlive) {
        response << "Connection: keep-alive\r\n";
    } else {
        response << "Connection: close\r\n";
    }
    response << "\r\n";
    response << responseBody;
    
    _responseBuffer = response.str();
}



void Client::handleDeleteRequest() {
    // Simple DELETE handler
    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n";
    response << "Content-Type: text/plain\r\n";

    sendErrorResponse(501, "Method Not Implemented");
}

std::string Client::getContentType(const std::string& path) {
    // Determine content type based on file extension
    std::string contentType = "text/plain";
    size_t dotPos = path.find_last_of('.');
    if (dotPos != std::string::npos) {
        std::string ext = path.substr(dotPos);
        if (ext == ".html" || ext == ".htm") contentType = "text/html";
        else if (ext == ".css") contentType = "text/css";
        else if (ext == ".js") contentType = "application/javascript";
        else if (ext == ".json") contentType = "application/json";
        else if (ext == ".jpg" || ext == ".jpeg") contentType = "image/jpeg";
        else if (ext == ".png") contentType = "image/png";
        else if (ext == ".gif") contentType = "image/gif";
        else if (ext == ".svg") contentType = "image/svg+xml";
        else if (ext == ".pdf") contentType = "application/pdf";
        else if (ext == ".txt") contentType = "text/plain";
        else if (ext == ".xml") contentType = "application/xml";
    }
    return contentType;
}

void Client::sendErrorResponse(int statusCode, const std::string& statusMessage) {
    std::ostringstream response;
    response << "HTTP/1.1 " << statusCode << " " << statusMessage << "\r\n";
    response << "Content-Type: text/html\r\n";
    
    std::string responseBody = "<html><head><title>" + std::to_string(statusCode) + " " + 
                              statusMessage + "</title></head><body><h1>" + 
                              std::to_string(statusCode) + " " + statusMessage + 
                              "</h1></body></html>";
    
    response << "Content-Length: " << responseBody.size() << "\r\n";
    response << "Connection: close\r\n";  // Don't keep alive on errors
    response << "\r\n";
    response << responseBody;
    
    _responseBuffer = response.str();
    _keepAlive = false;  // Don't keep alive on errors
}

bool Client::sendResponse() {
    if (_responseBuffer.empty()) {
        _responseSent = true;
        return false;
    }
    
    ssize_t bytesSent = send(_socket, _responseBuffer.c_str(), _responseBuffer.size(), 0);
    
    if (bytesSent < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // Would block, try again later
            return true;
        }
        
        std::cerr << "Error sending response: " << strerror(errno) << std::endl;
        return false;
    }
    
    if (bytesSent > 0) {
        // Remove sent data from buffer
        _responseBuffer.erase(0, bytesSent);
    }
    
    // Check if we're done sending
    if (_responseBuffer.empty()) {
        _responseSent = true;
        return false;
    }
    
    return true;
}

bool Client::isDoneWithResponse() const {
    return _responseSent;
}

bool Client::keepAlive() const {
    return _keepAlive;
}

void Client::reset() {
    _requestBuffer.clear();
    _responseBuffer.clear();
    _requestComplete = false;
    _responseSent = false;
    _method.clear();
    _uri.clear();
    _httpVersion.clear();
    _headers.clear();
    _body.clear();
    // Keep the socket and address intact
}