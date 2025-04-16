

#include "../include/web.h"

Response::Response() : _responseSent(false), 
                        _keepAlive(false),
                        _header_falg(false),
                        _isopen(false),
                        _fileOffset(0),
                        flag_p(0),
                        is_file(0),
                        is_dir (0),
                        bytes_sent(0)
						{
							CHUNK_SIZE = 1024;
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
	is_file = 0;
	is_dir = 0;
}

// ========== send header response ==========

void Response::send_header_response(size_t CHUNK_SIZE, std::string path, Request &request, int flag_autoindex) 
{
	std::string content_type;
	_isopen = true;
	_fileOffset = 0; 
	bool partial = false;
	size_t start = 0;
	size_t end = 0;
	size_t content_length;
	size_t file_size = 0;
    
	if (flag_autoindex  == 0)
	{
		content_type = get_MimeType(path);
		file.seekg(0, std::ios::end);
		file_size = file.tellg();
		file.seekg(0, std::ios::beg);  // Reset to beginning
		end = file_size - 1;
		std::cout << "conte : " << request.getContentLength() << std::endl;
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
		content_length = end - start + 1;
		_fileOffset = start;
		file.seekg(_fileOffset, std::ios::beg);
	}
	else
	{
		content_length = request.getContentLength();
		content_type = request.getContent_type();
	}
	if (content_length > CHUNK_SIZE)
		_keepAlive = true;
	// Generate headers
	std::ostringstream headers;
	if (partial)
		headers << "HTTP/1.1 206 Partial Content\r\n";
	else
		headers << "HTTP/1.1 "  << get_status_missage(request.getStatusCode()) << "\r\n";
	// else
	// 	headers << "HTTP/1.1 200 OK\r\n";

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
			// reset();
			std::cout << " end of file : File sent successfully" << std::endl;
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

std::string generateAutoIndex(const std::string& dirPath, const std::string& uriPath = "/") {
    DIR* dir = opendir(dirPath.c_str());
    if (!dir)
        return "<html><body><h1>Unable to open directory</h1></body></html>";

    std::ostringstream html;
    html << "<!DOCTYPE html>\n";
    html << "<html lang=\"en\" style=\"font-family:sans-serif;text-align:center;margin-top:50px;background-color:#f9f9f9;\">\n";
    html << "<head><meta charset=\"UTF-8\">\n";
    html << "<title>Index of " << uriPath << "</title>\n";
    html << "<style>\n"
         << "  body { max-width: 800px; margin: auto; background-color: #fff; padding: 20px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }\n"
         << "  h1 { color: #444; }\n"
         << "  ul { list-style-type: none; padding: 0; text-align: left; }\n"
         << "  li { padding: 8px 0; border-bottom: 1px solid #eee; }\n"
         << "  a { text-decoration: none; color: #3498db; font-weight: bold; }\n"
         << "  a:hover { text-decoration: underline; }\n"
         << "</style>\n";
    html << "</head>\n<body>\n";
    html << "<h1>Index of " << uriPath << "</h1>\n<ul>\n";

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string name(entry->d_name);
        if (name == "." || name == "..") continue;

        std::string fullPath = dirPath + "/" + name;
        struct stat st;
        bool isDir = (stat(fullPath.c_str(), &st) == 0 && S_ISDIR(st.st_mode));

        html << "<li><a href=\"" << uriPath;
        if (uriPath.back() != '/')
            html << "/";
        html << name;
        if (isDir) html << "/";
        html << "\">" << name << (isDir ? "/" : "") << "</a></li>\n";
    }
    closedir(dir);
    html << "</ul>\n</body></html>\n";
    return html.str();
}


void Response::handleGetResponse(int *flag, Request &request) {

    *flag = 0;

    std::string path = request.getpath();

    std::cout << "==   path  = : " << path << std::endl;
	if (flag_p == 0)
	{
		type_of_path(path);
		flag_p = 1;
	}
    // Check if file exists
	if (request.isCGI)
	{
		std::cout << "CGI" << std::endl;
		pause()	;
	}
    if (this->is_file) 
	{
        if (_header_falg == false) {
			if (open_file(flag, path, &request.code) == 1)
			{
				std::cout << "Error opening file ----------------------: " << path << std::endl;
				std::cout << "flag = " << *flag << std::endl;
			    return ;
			}
			send_header_response(CHUNK_SIZE, path, request, 0);
        }
        if (_isopen) {
            file.seekg(_fileOffset, std::ios::beg);
            char buffer[CHUNK_SIZE];
            file.read(buffer, CHUNK_SIZE);
            int bytes_read = file.gcount();
            if (bytes_read > 0)
				*flag = send_file_response(buffer, bytes_read);
			else 
			{
				std::cerr << "Error reading file: " << strerror(errno) << std::endl;
				reset();
                *flag = 1;
                return ;
            }
        }
    } 
	else if (this->is_dir) 
	{
		std::cout << "-------- autoindex: --------" << std::endl;
		std::string html = generateAutoIndex(request.getpath(), request.get_fake_path()); // real path, URI path
		request.setContentLength(html.length());
		request.setContent_type("text/html");
        send_header_response(CHUNK_SIZE, path, request, 1);
		send(_clientFd, html.c_str(), html.length(), 0);
		reset();
        request.set_status_code(200);
        *flag = 2;
        return ;
    } 
	else 
	{
		std::cout << "== File not found -->   fullPath : " << path << std::endl;
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

// ======================== generate error response ========================
void  Response::there_is_error_file(std::string fullPath, int statusCode)
{
	std::ifstream file(fullPath, std::ios::binary);
	if (!file.is_open()) {
		std::cerr << "Error: Could not open success.html" << std::endl;
		return;
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string responseBody = buffer.str();
	std::ostringstream response;
	struct stat fileStat;
	if (stat(fullPath.c_str(), &fileStat) == 0 && S_ISREG(fileStat.st_mode)) {
		if (file) {
			std::ostringstream response;
			response << "HTTP/1.1 "  << get_status_missage(statusCode) << "\r\n";
			response << "Content-Type: text/html\r\n";
			response << "Content-Length: " << responseBody.size() << "\r\n";

			std::cout << "Content-Length: " << responseBody.size() << std::endl;
			std::cout << "in fullPath: " << fullPath << std::endl;
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
}



void Response::generate_default_error_response(int statusCode) {
	std::ostringstream body;
	std::string statusMessage = get_status_missage(statusCode);
	// Basic HTML content
	body << "<!DOCTYPE html>\n"
		 << "<html lang=\"en\">\n"
		 << "<head><meta charset=\"UTF-8\"><title>Error "
		 << statusCode << "</title></head>\n"
		 << "<body style=\"font-family:sans-serif;text-align:center;margin-top:50px;\">\n"
		 << "<h1>" << statusCode << " - " << statusMessage << "</h1>\n"
		 << "<p>Sorry, something went wrong.</p>\n"
		 << "<hr><p>WebServ</p>\n"
		 << "</body></html>";

	std::string responseBody = body.str();
	// Construct full HTTP response headers
	std::ostringstream response;
	response << "HTTP/1.1 " << statusCode << " " << statusMessage << "\r\n";
	response << "Content-Type: text/html\r\n";
	response << "Content-Length: " << responseBody.size() << "\r\n";
	if (_keepAlive) {
		response << "Connection: keep-alive\r\n";
	} else {
		response << "Connection: close\r\n";
	}
	response << "\r\n";
	// Append body
	response << responseBody;
	// Store full response
	_responseBuffer = response.str();
}

bool Response::send_Error_Response(int client_fd) {
	if (_responseBuffer.empty())
	{
		_responseSent = true;
		return false;
	}
	ssize_t bytesSent = send(client_fd, _responseBuffer.c_str(), _responseBuffer.size(), 0);
	if (bytesSent < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return true;
		std::cerr << "Error sending response: " << strerror(errno) << std::endl;
		return false;
	}
	if (bytesSent > 0)
		_responseBuffer.erase(0, bytesSent);
	if (_responseBuffer.empty())
	{
		_responseSent = true;
		return false;
	}
	return true;
}

void Response::generate_error_response(int statusCode,  int client_fd, Server_holder& serv_hldr) {
	std::map<int, std::string>::iterator it = serv_hldr.error_pages.find(statusCode);
	if (it != serv_hldr.error_pages.end()) {
		std::string error_page_path = it->second;
		std::cout << "Error page path: " << error_page_path << std::endl;
		fullPath = "/Users/hben-laz/Desktop/push_web/docs" + error_page_path;
		there_is_error_file(fullPath, statusCode);
	} else {
		std::cout << "No custom error page found for status code: " << statusCode << std::endl;
		generate_default_error_response(statusCode);
	}
	_keepAlive = false;  // Don't keep alive on errors
	send_Error_Response(client_fd);
	_responseBuffer.clear();
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

std::string	Response::get_status_missage(int errorCode) const
{
	std::string errorMessage = "";
	switch (errorCode) {
		case 200: errorMessage = "200 OK"; break;
		case 201: errorMessage = "201 Created"; break;
		case 204: errorMessage = "204 No Content"; break;
		case 206: errorMessage = "206 Partial Content"; break;
		case 301: errorMessage = "301 Moved Permanently"; break;
		case 302: errorMessage = "302 Found"; break;
		case 304: errorMessage = "304 Not Modified"; break;
		case 307: errorMessage = "307 Temporary Redirect"; break;
		case 308: errorMessage = "308 Permanent Redirect"; break;
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
void Response::type_of_path(std::string& path)
{
	if (file_exists(path))
		is_file = 1;
	else if (is_directory(path))
	   is_dir = 1;
	return ;
}