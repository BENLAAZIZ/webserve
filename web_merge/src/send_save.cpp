


int Client::generateResponse_GET_DELETE() {
	if (_request.getMethod() == "GET")
		if (handleGetRequest())
			return 1;
	else
		handleDeleteRequest();
	return 0;
}

int  Client::handleGetRequest() {
    std::string path = _request.getpath();
    if (path == "/") {
        path = "/index.html"; // Default page
    }
    // Check for directory traversal attempts
    if (path.find("..") != std::string::npos) {
        std::cerr << "Directory traversal attempt: " << path << std::endl;
        return (_request.set_status_code(403), 1);
    }
    
    // Prepend document root from config
    std::string fullPath = "/Users/hben-laz/Desktop/webserve/web_merge/www" + path;
	std::cout << "fullPath: " << fullPath << std::endl;

    // Check if file exists
    struct stat fileStat;
    if (stat(fullPath.c_str(), &fileStat) == 0 && S_ISREG(fileStat.st_mode)) {
        // File exists, serve it
        std::string content_type = get_MimeType(path);
        // const size_t LARGE_FILE_THRESHOLD = 100000; // 100KB threshold
        const size_t CHUNK_SIZE = 1024 * 1024; // 1MB chunks

        if (fileStat.st_size > 10000) {
            // Handle large file
            std::ostringstream headers;
            headers << "HTTP/1.1 200 OK\r\n";
            headers << "Content-Type: " << content_type << "\r\n";
            headers << "Content-Length: " << fileStat.st_size << "\r\n";
            headers << "Accept-Ranges: bytes\r\n";
            
            if (_keepAlive)
                headers << "Connection: keep-alive\r\n";
            else
                headers << "Connection: close\r\n";
            headers << "\r\n";
            
            _responseBuffer = headers.str();
            
            // Setup for chunked file sending
            _isChunkedFile = true;
            _fileToSend.open(fullPath, std::ios::binary);
            _fileBytesRemaining = fileStat.st_size;
            _chunkSize = CHUNK_SIZE;
            
            // Don't read any file data yet - just send headers
            // The file content will be read and sent by the event loop
        }
        else {
            // Handle small file (read entire file at once)
            std::ifstream file(fullPath, std::ios::binary);
            if (file) {
                // Read file content
                std::string fileContent((std::istreambuf_iterator<char>(file)),
                                       std::istreambuf_iterator<char>());
                
                // Generate HTTP response
                std::ostringstream response;
                response << "HTTP/1.1 200 OK\r\n";
                response << "Content-Type: " << content_type << "\r\n";
                response << "Content-Length: " << fileContent.size() << "\r\n";
                if (_keepAlive)
                    response << "Connection: keep-alive\r\n";
                else
                    response << "Connection: close\r\n";
                response << "\r\n";
                response << fileContent;
                
                _responseBuffer = response.str();
                _isChunkedFile = false;
            }
            else
                _request.set_status_code(500);
        }
    }
    else if (stat(fullPath.c_str(), &fileStat) == 0 && S_ISDIR(fileStat.st_mode)) {
        // Directory listing (optional, could redirect to index or show listing)
		std::cout << "Directory listing not implemented" << std::endl;
        _request.set_status_code(403);
    } 
    else {
        // File not found
        _request.set_status_code(404);
    }
}


// ========================




bool Client::sendResponse(int client_fd) {
	if (_responseBuffer.empty()) {
		_responseSent = true;
		return false;
	}
	
	ssize_t bytesSent = send(client_fd, _responseBuffer.c_str(), _responseBuffer.size(), 0);
	
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