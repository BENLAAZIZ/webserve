

// =================================================================================




#include "../include/web.h"

Response::Response() : _responseSent(false), 
                        _keepAlive(false),
                        _header_falg(false),
                        _isopen(false),
                        _fileOffset(0),
                        flag_p(0),
                        is_file(0),
                        is_dir (0),
                        bytes_sent(0){
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
        _responseSent = other._responseSent;
        _keepAlive = other._keepAlive;
        _header_falg = other._header_falg;
        _isopen = other._isopen;
        _fileOffset = other._fileOffset;
	}
	return *this;
}

void Response::reset() {
    _responseSent = false;
    _keepAlive = false;
    _header_falg = false;
    _isopen = false;
    _fileOffset = 0;
    _responseBuffer.clear();
    file.close();
	fullPath.clear();
	flag_p = 0;
}

// ========== send header response ==========

void Response::send_header_response(size_t CHUNK_SIZE, std::string path, Request &request) 
{
	std::string content_type = get_MimeType(path);
	_isopen = true;
	_fileOffset = 0; 

	file.seekg(0, std::ios::end);
	size_t file_size = file.tellg();
	file.seekg(0, std::ios::beg);  // Reset to beginning

	size_t start = 0;
	size_t end = file_size - 1;
	bool partial = false;

	// Parse Range Header (example: Range: bytes=500-1000)
	if (request.hasHeader("Range")) {
		std::string range = request.getHeader("Range"); // ex: "bytes=500-1000"
		size_t pos = range.find("bytes=");
		if (pos != std::string::npos) {
			range = range.substr(pos + 6); // skip "bytes="
			size_t dash = range.find("-");
			if (dash != std::string::npos) {
				std::string startStr = range.substr(0, dash);
				std::string endStr = range.substr(dash + 1);
				start = std::stoul(startStr);
				if (!endStr.empty())
					end = std::stoul(endStr);
				if (end >= file_size)
					end = file_size - 1;
				if (start < file_size && start <= end)
					partial = true;
			}
		}
	}

	size_t content_length = end - start + 1;
	_fileOffset = start;
	file.seekg(_fileOffset, std::ios::beg);

	if (content_length > CHUNK_SIZE)
		_keepAlive = true;

	// Generate headers
	std::ostringstream headers;
	if (partial)
		headers << "HTTP/1.1 206 Partial Content\r\n";
	else
		headers << "HTTP/1.1 200 OK\r\n";

	headers << "Content-Type: " << content_type << "\r\n";
	headers << "Content-Length: " << content_length << "\r\n";
	headers << "Accept-Ranges: bytes\r\n";

	if (partial)
		headers << "Content-Range: bytes " << start << "-" << end << "/" << file_size << "\r\n";

	if (_keepAlive)
		headers << "Connection: keep-alive\r\n";
	else
		headers << "Connection: close\r\n";

	headers << "\r\n";

	_header_falg = true;
	_responseBuffer = headers.str();
	send(_clientFd, _responseBuffer.c_str(), _responseBuffer.size(), 0);

	std::cout << "\n*********************** Headers response *********************" <<  std::endl;
	std::cout  << _responseBuffer ;
	std::cout << "*********************** end of header response *********************\n" <<  std::endl;

	_responseBuffer.clear();
}

int	Response::send_file_response(char *buffer, int bytes_read)
{
		ssize_t sent = send(_clientFd, buffer, bytes_read, 0);
		buffer[bytes_read] = '\0'; // Null-terminate the buffer for safety
	if (sent >= 0) {
		_fileOffset += sent;
		if (file.eof()) 
		{
			_responseSent = true;
			_header_falg = false;
			_fileOffset = 0;
			file.close();
			_isopen = false;
				fullPath.clear();
				flag_p = 0;
			std::cout << " end of file : File sent successfully" << std::endl;
			// reset();
			return 2;
		}
		// Not done yet, return  to continue processing
		return 0;
	} 
	else 
	{
		// ============ Error handling ================
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			// Socket not ready yet, try again later
			std::cout << "Socket not ready yet, try again later" << std::endl;
			return 0;
		}
		// ==========================================
		std::cerr << "Error sending file data: " << strerror(errno) << std::endl;
		_responseBuffer.clear();
		_responseSent = true;
		_header_falg = false;
		_fileOffset = 0;
		file.close();
		_isopen = false;
		fullPath.clear();
		flag_p = 0;
		// reset();
		return 1;
	}
}


int Response::open_file(int *flag, std::string fullPath, int *code)
{
	file.open(fullPath, std::ios::binary);
	if (!file) {
		std::cerr << "Failed to open file: " << fullPath << std::endl;
		*code = 500;
		*flag = 1;
		return 1;
	}
	std::cout << "File opened successfully: " << fullPath << std::endl;
	return 0;
}

void Response::handleGetResponse(int *flag, Request &request) {

    *flag = 0;

    std::string path = request.getpath();

    std::cout << "==   path  = : " << path << std::endl;
	if (flag_p == 0)
	{

		resolverequest_path22(path);
		  flag_p = 1;
	}
    // Check if file exists
	if (request.isCGI)
	{
			std::cout << "CGI" << std::endl;
			pause();
	}
//    pause();
    if (this->is_file) {
        // File exists, serve it
		std::cout << "/////   fullPath: " << path << std::endl;
        const size_t CHUNK_SIZE = 1024; // Increased chunk size for better performance
        if (_header_falg == false) {
			std::cout << "_header_flag: " << _header_falg << std::endl;
			if (open_file(flag, path, &request.code) == 1)
			    return ;
			send_header_response(CHUNK_SIZE, path, request);
        }
        if (_isopen) {
            file.seekg(_fileOffset, std::ios::beg);
            char buffer[CHUNK_SIZE];
            file.read(buffer, CHUNK_SIZE);
            int bytes_read = file.gcount();
			// write(1, "88888\n", 8);
            if (bytes_read > 0)
			{
				*flag = send_file_response(buffer, bytes_read);
				// bytes_sent += bytes_read;
				// if (*flag == 2)
				// {
				// 	std::cout << "bytes_sent: " << bytes_sent << std::endl;

				// }
				
			}
			else 
			{
				std::cerr << "Error reading file: " << strerror(errno) << std::endl;
				reset();
                *flag = 1;
                return ;
            }
        }
    } else if (this->is_dir) {
        // Directory listing (optional, could redirect to index or show listing)
        std::cout << "Directory listing not implemented" << std::endl;
        request.set_status_code(403);
        *flag = 1;
        return ;
    } else {
		std::cout << "==   fullPath: " << path << std::endl;
        // File not found
        request.set_status_code(404);
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
		else if (ext == ".mkv") contentType = "video/x-matroska";
		else if (ext == ".flv") contentType = "video/x-flv";
		else if (ext == ".ico") contentType = "image/vnd.microsoft.icon";
		else if (ext == ".bmp") contentType = "image/bmp";
		else if (ext == ".tiff") contentType = "image/tiff";
		else if (ext == ".svg") contentType = "image/svg+xml";
		else if (ext == ".woff") contentType = "font/woff";
		else if (ext == ".woff2") contentType = "font/woff2";
		else if (ext == ".eot") contentType = "application/vnd.ms-fontobject";
		else if (ext == ".otf") contentType = "font/otf";
		else if (ext == ".ttf") contentType = "font/ttf";

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
	std::cout << "\n*********************** Headers response *********************" <<  std::endl;
	std::string resss;
	resss = response.str();
	std::cout  << resss ;
	std::cout << "*********************** end of header response *********************\n" <<  std::endl;
			response << responseBody;
			_responseBuffer = response.str();
		}
	}

	_keepAlive = false;  // Don't keep alive on errors
	sendResponse(client_fd);
	_responseBuffer.clear();
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


// ====== location ======

// ========== is directory ==========

bool is_directory(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0)
        return false;
    return S_ISDIR(info.st_mode);
}

// ========= file exist ==========

bool file_exists(const std::string& path) {
    struct stat info;
    return (stat(path.c_str(), &info) == 0 && S_ISREG(info.st_mode));
}

// ========== end location ==========



// state of path
void Response::resolverequest_path22(std::string& path)
{
	if (file_exists(path))
		is_file = 1;
	else if (is_directory(path))
	   is_dir = 1;
	return ;
}


