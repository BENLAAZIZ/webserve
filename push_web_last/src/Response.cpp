#include "../include/web.h"

Response::Response() : _responseSent(false), 
                _keepAlive(false),
                _header_falg(false),
                    _isopen(false),
                    _fileOffset(0) {
}

Response::~Response() {
 
}

Response::Response(const Response& other)
{
	*this = other;
}


Response& Response::operator=(const Response& other)
{
	if (this != &other)
	{
		_responseBuffer = other._responseBuffer;
        _request = other._request;
        _responseSent = other._responseSent;
        _keepAlive = other._keepAlive;
        _header_falg = other._header_falg;
        _isopen = other._isopen;
        _fileOffset = other._fileOffset;
        // file = other.file;
        // filetest = other.filetest;
	}
	return *this;
}

int Response::getStatus() const {
    return _request.getStatusCode();
}



void Response::reset() {
    _responseSent = false;
    _keepAlive = false;
    _header_falg = false;
    _isopen = false;
    _fileOffset = 0;
    _responseBuffer.clear();
    file.close();
    _request.reset();
}




void Response::send_header_response(size_t CHUNK_SIZE, std::string path) 
{
			std::string content_type = get_MimeType(path);
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
            send(_clientFd, _responseBuffer.c_str(), _responseBuffer.size(), 0);
            _responseBuffer.clear();
}


int	Response::send_file_response(char *buffer, int bytes_read)
{
	 			 ssize_t sent = send(_clientFd, buffer, bytes_read, 0);
                if (sent > 0) {
                    _fileOffset += sent;
                    if (file.eof()) 
					{
                        // _responseSent = true;
                        // _header_falg = false;
                        // _fileOffset = 0;
                        // file.close();
                        // _isopen = false;
						reset();
                        return 2;
                    }
                    // Not done yet, return  to continue processing
                    return 0;
                } 
				else 
				{
                    std::cerr << "Error sending file data: " << strerror(errno) << std::endl;
					// _responseBuffer.clear();
                    // _responseSent = true;
                    // _header_falg = false;
                    // _fileOffset = 0;
                    // file.close();
                    // _isopen = false;
					reset();
                    return 1;
                }
}



int Response::open_file(int *flag, std::string fullPath)
{
	file.open(fullPath, std::ios::binary);
	if (!file) {
		std::cerr << "Failed to open file: " << fullPath << std::endl;
		_request.set_status_code(500);
		*flag = 1;
		return 1;
	}
	return 0;
}



void Response::handleGetResponse(int *flag) {

    *flag = 0;
    std::string path = _request.getpath();
    if (path == "/") {
        path = "/index.html"; // Default page
    }
	// Check for directory traversal attempts
	if (path.find("..") != std::string::npos) {
		std::cerr << "Directory traversal attempt: " << path << std::endl;
		_request.set_status_code(403);
        *flag = 1;
		return ;
	}

	// ----------------------------------------

	// find location
		// path = find_location(path);
	// ----------------------------------------
    // Prepend document root from config
    std::string fullPath = "/Users/hben-laz/Desktop/webserve/web_merge/www" + path;
    // Check if file exists
	if (is_CGI())
	{
			std::cout << "CGI" << std::endl;
			pause();
	}
    struct stat fileStat;
    if (stat(fullPath.c_str(), &fileStat) == 0 && S_ISREG(fileStat.st_mode)) {
        // File exists, serve it
        const size_t CHUNK_SIZE = 8000; // Increased chunk size for better performance
        if (_header_falg == false) {
			if (open_file(flag, fullPath) == 1)
			    return ;
			send_header_response(CHUNK_SIZE, path);
        }
        if (_isopen) {
            file.seekg(_fileOffset, std::ios::beg);
            char buffer[CHUNK_SIZE];
            file.read(buffer, CHUNK_SIZE);
            int bytes_read = file.gcount();
			// write(1, buffer, bytes_read);
            if (bytes_read > 0)
				*flag =  send_file_response(buffer, bytes_read);
			else 
			{
				reset();
                *flag = 1;
                return ;
            }
        }
    } else if (stat(fullPath.c_str(), &fileStat) == 0 && S_ISDIR(fileStat.st_mode)) {
        // Directory listing (optional, could redirect to index or show listing)
        std::cout << "Directory listing not implemented" << std::endl;
        _request.set_status_code(403);
        *flag = 1;
        return ;
    } else {
		std::cout << "==   fullPath: " << fullPath << std::endl;
        // File not found
        _request.set_status_code(404);
        *flag = 1;
        return ;
    }
    return ;
}

// void Response::handleDeleteRequest() {
// 	// Simple DELETE handler
// 	// Generate response with method and URI
// 	std::ostringstream response;
// 	response << "HTTP/1.1 200 OK\r\n";
// 	response << "Content-Type: text/plain\r\n";
// }

std::string Response::get_MimeType (const std::string& path) {
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

void Response::generate_error_response(int statusCode,  int client_fd) {

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
			response << "HTTP/1.1 "  << get_error_missage(statusCode) << "\r\n";
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

bool Response::sendResponse(int client_fd) {
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

bool Response::keepAlive() const {
	return _keepAlive;
}

std::string	Response::get_code_error_path(int errorCode) const {
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

std::string	Response::get_error_missage(int errorCode) const
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

bool        Response::is_CGI()
{
	if (_request.getMethod() == "POST" || _request.getMethod() == "GET")
	{
		if (_request.getpath().find(".php") != std::string::npos || _request.getpath().find(".js") != std::string::npos)
			return true;
	}
	return false;
}