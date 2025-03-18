

#include "../include/web.h"

Client::Client() : request_Header_Complete(false), 
							_responseSent(false), 
								_keepAlive(false),
								_header_falg(false),
								  _isopen(false),
								  _fileOffset(0) {
}

Client::~Client() {
}

Client::Client(const Client& other)
{
	*this = other;
}

Client& Client::operator=(const Client& other)
{
	if (this != &other)
	{
		_request = other._request;
		_requestBuffer = other._requestBuffer;
		_responseBuffer = other._responseBuffer;
		request_Header_Complete = other.request_Header_Complete;
		_responseSent = other._responseSent;
		_keepAlive = other._keepAlive;
	}
	return *this;
}

bool Client::is_Header_Complete() {
	return request_Header_Complete;
}

bool Client::parse_Header_Request(std::string& line_buf) 
{
	size_t headerEndPos = line_buf.find("\r\n\r\n");
	if (headerEndPos != std::string::npos) {
		this->request_Header_Complete = true;
	}
	while (1) {
		size_t lineEnd = line_buf.find("\r\n");
		if (lineEnd == std::string::npos)
			break;
		std::string line = line_buf.substr(0, lineEnd);
		line_buf.erase(0, lineEnd + 2); // Remove processed line
		if (_request.getMethod().empty() || _request.getpath().empty() || _request.getVersion().empty()) {
			if (!_request.parseFirstLine(line)) {
				this->request_Header_Complete = true;
				return false;
			}
		}
		else 
		{
			int flag = 0;
			end_of_headers(line, &flag);
			if (flag == 0)
			{
				this->request_Header_Complete = true;
				return false;
			}
			else if (flag == 1)
				break;
			if (generate_header_map(line) == false)
			{
				this->request_Header_Complete = true;
				return false;
			}
		}	
	}
	return true;
}

bool Client::generate_header_map(std::string& line)
{
	size_t colonPos = line.find(":");
	if (colonPos == std::string::npos || colonPos == 0 || line[colonPos - 1] == ' ') {
		_request.set_status_code(400);
		return false;
	}
	std::string hostHeader = line.substr(0, colonPos);
	std::string key;
	for (size_t i = 0; i < hostHeader.length(); i++) {
		hostHeader[i] = std::tolower(hostHeader[i]);
	}
	if (hostHeader == "host")
		key = hostHeader.substr(0, colonPos);
	else
		key = line.substr(0, colonPos);
	std::string value = line.substr(colonPos + 1);
	value.erase(0, value.find_first_not_of(" "));
	_request.setHeader(key, value);
	// std::cout << "Header: ||" << key << "|| = ||" << value << "||" << std::endl;
	return true;
}

void Client::end_of_headers(std::string& line, int *flag)
{
	if (line.empty()) {
		if (_request.getHeaders().find("host") == _request.getHeaders().end()) {
			_request.set_status_code(400);
			return ;
		}
		if (_request.getMethod() == "POST")
		{
			if (_request.getHeaders().find("Content-Length") == _request.getHeaders().end())
			{
				_request.set_status_code(411);
				return ;
			}
			else
				_request.setContentLength(atoi(_request.getHeader("Content-Length").c_str()));
			if (_request.getHeaders().find("Transfer-Encoding") != _request.getHeaders().end())
				_request.setTransferEncodingExist(true);
			if (_request.getHeaders().find("Content-Type") != _request.getHeaders().end())
			{
				if (_request.getHeader("Content-Type").find("boundary=") != std::string::npos)
				{
					size_t boundary_pos = _request.getHeader("Content-Type").find("boundary=");
					_request.boundary = _request.getHeader("Content-Type").substr(boundary_pos + 9);
					_request.setBoundary(_request.boundary);
					_request.setContent_type("multipart/form-data");
				}
				else
				{
					_request.setContent_type(_request.getHeader("Content-Type"));
				}
			}
		}
		*flag = 1;
		return ;
	}
	*flag = 2;
	return ;
}

int Client::generateResponse_GET_DELETE() {
	if (_request.getMethod() == "GET")
	{
		if (handleGetRequest())
			return 1;
	}
	else
		handleDeleteRequest();
	return 0;
}

int Client::handleGetRequest() {
    std::string path = _request.getpath();
    if (path == "/") {
        path = "/index.html"; // Default page
    }
	// Check for directory traversal attempts
	if (path.find("..") != std::string::npos) {
		std::cerr << "Directory traversal attempt: " << path << std::endl;
		_request.set_status_code(403);
		return 1;
	}
    // Prepend document root from config
    std::string fullPath = "/Users/hben-laz/Desktop/webserve/web_merge/www" + path;
    // Check if file exists
    struct stat fileStat;
    if (stat(fullPath.c_str(), &fileStat) == 0 && S_ISREG(fileStat.st_mode)) {
        // File exists, serve it
        std::string content_type = get_MimeType(path);
        const size_t CHUNK_SIZE = 1024; // Increased chunk size for better performance
        if (_header_falg == false) {
            file.open(fullPath, std::ios::binary);
            if (!file) {
                std::cerr << "Failed to open file: " << fullPath << std::endl;
                _request.set_status_code(500);
                return 1;
            }
            _isopen = true;
            _fileOffset = 0; 
            file.seekg(_fileOffset, std::ios::end);
            size_t file_size = file.tellg();
            file.seekg(_fileOffset, std::ios::beg);  // Reset to beginning for reading
			if (file_size > CHUNK_SIZE)
				_keepAlive = true;            
            // Generate headers
            std::ostringstream headers;
            headers << "HTTP/1.1 200 OK\r\n";
            headers << "Content-Type: " << content_type << "\r\n";
            headers << "Content-Length: " << file_size << "\r\n";
            headers << "Accept-Ranges: bytes\r\n";
            if (_keepAlive)
                headers << "Connection: keep-alive\r\n";
            else
                headers << "Connection: close\r\n";
            headers << "\r\n";
            
            _header_falg = true;
            _responseBuffer = headers.str();
            std::cout << "Headers sent, file size: " << file_size << " bytes" << std::endl;
			std::cout << "getClientFd: " << getClientFd() << std::endl;
            send(_clientFd, _responseBuffer.c_str(), _responseBuffer.size(), 0);
			// std::cout << "headers: " << _responseBuffer << std::endl;
            _responseBuffer.clear();
        }
        if (_isopen) {
            file.seekg(_fileOffset, std::ios::beg);
            char buffer[CHUNK_SIZE];
            file.read(buffer, CHUNK_SIZE);
            int bytes_read = file.gcount();
            if (bytes_read > 0) {
                ssize_t sent = send(_clientFd, buffer, bytes_read, 0);
                if (sent > 0) {
                    _fileOffset += sent;
                    if (file.eof()) 
					{
                        _responseSent = true;
                        _header_falg = false;
                        _fileOffset = 0;
                        file.close();
                        _isopen = false;
                        return 1;
                    }
                    // Not done yet, return 0 to continue processing
                    return 0;
                } 
				else 
				{
                    std::cerr << "Error sending file data: " << strerror(errno) << std::endl;
                    _responseSent = true;
                    _header_falg = false;
                    _fileOffset = 0;
                    file.close();
                    _isopen = false;
                    return 1;
                }
            } 
			else 
			{
                // File is complete
                std::cout << "File transfer complete" << std::endl;
                _responseSent = true;
                _header_falg = false;
                _fileOffset = 0;
                file.close();
                _isopen = false;
                return 1;
            }
        }
    } else if (stat(fullPath.c_str(), &fileStat) == 0 && S_ISDIR(fileStat.st_mode)) {
        // Directory listing (optional, could redirect to index or show listing)
        std::cout << "Directory listing not implemented" << std::endl;
        _request.set_status_code(403);
        return 1;
    } else {
		std::cout << "fullPath: " << fullPath << std::endl;
        // File not found
        _request.set_status_code(404);
        return 1;
    }
    return 0;
}

void Client::handleDeleteRequest() {
	// Simple DELETE handler
	// Generate response with method and URI
	std::ostringstream response;
	response << "HTTP/1.1 200 OK\r\n";
	response << "Content-Type: text/plain\r\n";
}

std::string Client::get_MimeType (const std::string& path) {
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
		else if (ext == ".mp4") contentType = "video/mp4";
		else if (ext == ".webm") contentType = "video/webm";
		else if (ext == ".ogg") contentType = "video/ogg";
		else if (ext == ".mp3") contentType = "audio/mpeg";
		else if (ext == ".wav") contentType = "audio/wav";
		else if (ext == ".flac") contentType = "audio/flac";
		else if (ext == ".zip") contentType = "application/zip";
		else if (ext == ".tar") contentType = "application/x-tar";
		else if (ext == ".gz") contentType = "application/gzip";
		else if (ext == ".bz2") contentType = "application/x-bzip2";
		else if (ext == ".7z") contentType = "application/x-7z-compressed";
		else if (ext == ".rar") contentType = "application/x-rar-compressed";
		else if (ext == ".exe") contentType = "application/x-msdownload";
		else if (ext == ".doc") contentType = "application/msword";
		else if (ext == ".docx") contentType = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
		else if (ext == ".xls") contentType = "application/vnd.ms-excel";
		else if (ext == ".xlsx") contentType = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
		else if (ext == ".ppt") contentType = "application/vnd.ms-powerpoint";
		else if (ext == ".pptx") contentType = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
		else if (ext == ".csv") contentType = "text/csv";
		else if (ext == ".rtf") contentType = "application/rtf";
		else if (ext == ".wav") contentType = "audio/wav";
		else if (ext == ".avi") contentType = "video/x-msvideo";
		else if (ext == ".mov") contentType = "video/quicktime";
		else if (ext == ".wmv") contentType = "video/x-ms-wmv";
		else if (ext == ".mpg") contentType = "video/mpeg";
		else if (ext == ".mpeg") contentType = "video/mpeg";
		else if (ext == ".webp") contentType = "image/webp";
	}
	return contentType;
}

void Client::genetate_error_response(int statusCode,  int client_fd) {

    std::string code_path = "";
	code_path = get_code_error_path(statusCode);
	std::string fullPath = "/Users/hben-laz/Desktop/webserve/web_merge/docs/errors" +  code_path;
	std::ostringstream response;
	// Check if file exists
	struct stat fileStat;
	if (stat(fullPath.c_str(), &fileStat) == 0 && S_ISREG(fileStat.st_mode)) {
		std::ifstream file(fullPath, std::ios::binary);
		if (file) {
			// Read file content
			std::string responseBody((std::istreambuf_iterator<char>(file)),
									 std::istreambuf_iterator<char>());
			// Generate HTTP response
			std::ostringstream response;
			response << "HTTP/1.1 "  << _request.get_error_missage(statusCode) << "\r\n";
			response << "Content-Type: text/html\r\n";
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
	}
	_keepAlive = false;  // Don't keep alive on errors
	sendResponse(client_fd);
}

bool Client::sendResponse(int client_fd) {
	if (_responseBuffer.empty())
	{
		_responseSent = true;
		return false;
	}
	ssize_t bytesSent = send(client_fd, _responseBuffer.c_str(), _responseBuffer.size(), 0);
	if (bytesSent < 0)
	{
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
	if (_responseBuffer.empty())
	{
		_responseSent = true;
		return false;
	}
	return true;
}


bool Client::keepAlive() const {
	return _keepAlive;
}

void Client::reset() {
	_requestBuffer.clear();
	_responseBuffer.clear();
	request_Header_Complete = false;
	_responseSent = false;
	_request.reset();
}

// set client_fd
void Client::setClientFd(int client_fd){
	_clientFd = client_fd;
}

// get client_fd
int Client::getClientFd() const {
	return _clientFd;
}

std::string	Client::get_code_error_path(int errorCode) const {
	std::string code_path = "";
	switch (errorCode) {
		case 400: code_path = "/400.html"; break;
		case 403: code_path = "/403.html"; break;
		case 404: code_path = "/404.html"; break;
		case 405: code_path = "/405.html"; break;
		case 411: code_path = "/411.html"; break;
		case 413: code_path = "/413.html"; break;
		case 414: code_path = "/414.html"; break;
		case 500: code_path = "/500.html"; break;
		case 505: code_path = "/505.html"; break;
		default: code_path = "/500.html"; break;
	}
	return code_path;
}