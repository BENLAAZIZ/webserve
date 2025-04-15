

#include "../include/web.h"

Client::Client() : request_Header_Complete(false), 
							_responseSent(false), 
								_keepAlive(false),
								_header_falg(false),
								  _isopen(false),
								  _fileOffset(0) {
									full_path = "";
}

Client::~Client() {
	// Socket is closed by the Server class
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
		_response = other._response;
		request_Header_Complete = other.request_Header_Complete;
		_responseSent = other._responseSent;
		_keepAlive = other._keepAlive;
	}
	return *this;
}

bool Client::is_Header_Complete() {
	return request_Header_Complete;
}

bool Client::parse_Header_Request() 
{
	size_t headerEndPos = _request._requestBuffer.find("\r\n\r\n");
	if (headerEndPos != std::string::npos) {
		this->request_Header_Complete = true;

	}
	while (1) {
		size_t lineEnd = _request._requestBuffer.find("\r\n");
		if (lineEnd == std::string::npos)
			break;
		std::string line = _request._requestBuffer.substr(0, lineEnd);
		_request._requestBuffer.erase(0, lineEnd + 2); // Remove processed line
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
					// size_t boundary_pos = _request.getHeader("Content-Type").find("boundary=");
					// _request.boundary += "--";
					// _request.boundary = _request.getHeader("Content-Type").substr(boundary_pos + 9);
					// _request.setBoundary(_request.boundary);
					// _request.boundary_end = _request.boundary + "--";
					// _request.setContent_type("multipart/form-data");
						size_t boundary_pos = _request.getHeader("Content-Type").find("boundary=");
						_request.boundary += "--";
						_request.boundary += _request.getHeader("Content-Type").substr(boundary_pos + 9);
						_request.setBoundary(_request.boundary);
						_request.boundary_end = _request.boundary + "--";
						//std::cout << "boundary:* " << getBoundary() << std::endl;
						_request.setContent_type("multipart/form-data");
				}
				else
					_request.setContent_type(_request.getHeader("Content-Type"));
			}
		}
		if (_request.getHeaders().find("Connection") != _request.getHeaders().end())
		{
			if (_request.getHeader("Connection") == "keep-alive")
				_keepAlive = true;
		}
		*flag = 1;
		return ;
	}
	*flag = 2;
	return ;
}

bool Client::keepAlive() const {
	return _keepAlive;
}

void Client::reset() {
	request_Header_Complete = false;
	_responseSent = false;
	_keepAlive = false;
	_header_falg = false;
	_isopen = false;
	_fileOffset = 0;
	full_path.clear();
	_request.reset();
	_response.reset();
	_isResolved = false;
}


// set client_fd
void Client::setClientFd(int client_fd)
{
	_clientFd = client_fd;
}

// get client_fd
int Client::getClientFd() const
{
	return _clientFd;
}

void Client::handlePostRequest() {

    bool isChunked = (_request.getHeaders().find("Transfer-Encoding") != _request.getHeaders().end() && 
                      _request.getHeader("Transfer-Encoding") == "chunked");
    
    std::string contentType = _request.getContent_type();

	// std::cout << "contentType: " << contentType << std::endl;

	// std::cout << "isChunked: " << isChunked << std::endl;

	// pause();
    
    if (contentType == "application/x-www-form-urlencoded") {
    	//     handleURLEncoded();
		std::cout << "handleURLEncoded" << std::endl;
    }
    if (contentType == "multipart/form-data") {
        if (isChunked) {
			// std::cout << "handleChunkedBoundary" << std::endl;
			_request.handleChunkedBoundary(_request);
        } else {
			// std::cout << "handleBoundary" << std::endl;
			_request.handleBoundary(_request);
        }
    }
    else if (isChunked) {
 		// std::cout << "handleChunkedData" << std::endl;	
		_request.handleChunkedData(_request);
    }
    // else {
    //     // Handle any other content types
    // }
}

// int Client::read_data(int client_fd)
int Client::read_data()
{
	// std::cout << "client_fd: " << _clientFd << std::endl;
	char buffer[BUFFER_SIZE];
	ssize_t bytes_read = recv(_clientFd, buffer, BUFFER_SIZE, 0);
	if (bytes_read <= 0) {
		if (bytes_read < 0 && (errno == EWOULDBLOCK || errno == EAGAIN))
			return -1; // No data available yet
		return -1;
	}
	std::string data(buffer, bytes_read);
	_request._requestBuffer += data; // Append new data to client's buffer
	return 0;
}

void Client::sendSuccessResponse(int clientSocket, const std::string& path) {
	// Open the success.html file
	std::string root = "/Users/hben-laz/Desktop/webserve/push_web_last1/docs/upload";
	std::string filePath = root + path;
	std::ifstream file(filePath);
				
	if (!file.is_open()) {
		std::cerr << "Error: Could not open success.html" << std::endl;
		return;
	}
 
	// Read the file contents into a stringstream
	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string htmlContent = buffer.str();

	// Close the file
	file.close();

	// Prepare the HTTP response
	std::string response = "HTTP/1.1 200 OK\r\n"
						"Content-Type: text/html\r\n"
						"Connection: close\r\n"
						"Content-Length: " + std::to_string(htmlContent.length()) + "\r\n"
						"\r\n" +
						htmlContent;
 
	// Send the response to the client
	ssize_t bytesSent = send(clientSocket, response.c_str(), response.length(), 0);
	if (bytesSent == -1) {
		std::cerr << "Error sending response to client." << std::endl;
		// Handle the error appropriately
    }
}

// void Client::handlePostResponse() {
// 	// std::string root = "/Users/hben-laz/Desktop/db/push_web_last/docs"; //_request.getRoot();
// 	std::string filePath =  _request.getpath();
// 	// std::cout << "my_root: " << _request.my_root << std::endl;
// 	std::string root = _request.my_root + "/docs/upload/success.html";
// 	// std::cout << "filePath: " << filePath << std::endl;
// 	// handleG();
// 	_response.handleGetResponse(&_fileOffset, _request);
// 	pause();
// 	sendSuccessResponse(_clientFd, "/success.html");
// 	// pause();
// }


// ====== location ======

int  Client::resolve_request_path(Server_holder & serv_hldr) {
	Location* loc = find_matching_location(_request, serv_hldr.locations);
	std::string root = loc ? loc->root : serv_hldr.root;
	std::string location_path = loc ? loc->path : "/";
	std::string getpath = _request.getpath();

	_request.set_fake_path(getpath);
	std::string relative_path = getpath;
	if (getpath.find(location_path) == 0) {
		relative_path = getpath.substr(location_path.length()); // strip location path
	}
	// Remove leading '/' to avoid double slashes when joining
	if (!relative_path.empty() && relative_path[0] == '/')
		relative_path = relative_path.substr(1);
	// Compose full_path
	full_path = join_paths(_request.my_root, join_paths(root, relative_path));
	if (_request.isCGI)
	{
		std::cout << "CGI" << std::endl;
		return 200;
	}
	if (is_directory(full_path)) {
		if (loc && !loc->index.empty()) {
			std::string index_path = join_paths(full_path, loc->index[0]);
			if (file_exists(index_path)) {
				_request.setPath(index_path);
				full_path.clear();
				_request.set_status_code(200);
				return 200;
			}
			else {
				_request.set_status_code(404);
				return 404;
			}
		}
		else if (loc && loc->autoindex) {
			_request.set_status_code(200);
			_request.setPath(full_path);
			full_path.clear();
			return 200;
		}
		_request.set_status_code(403);
		return 403;
	}
	else if (file_exists(full_path))
	{
		_request.setPath(full_path);
		full_path.clear();
		_request.set_status_code(200);
		return 200;
	}
	std::cout << "-------- 404 Not Found: --------" << std::endl;
	_request.set_status_code(404);
	return 404;
}

// ======= find matching location =======

Location* Client::find_matching_location(Request &request, std::vector<Location>& locations) {
    Location* best_match = NULL;
    size_t best_length = 0;

    for (std::vector<Location>::iterator it = locations.begin(); it != locations.end(); ++it) {
        if (request.getpath().find(it->path) == 0 && it->path.length() > best_length) {
            best_match = &(*it);
            best_length = it->path.length();
        }
    }
    return best_match;
}

// ========== join paths ==========

std::string Client::join_paths(const std::string& a, const std::string& b) {
    if (a.empty()) return b;
    if (b.empty()) return a;

    if (a[a.size() - 1] == '/' && b[0] == '/')
        return a + b.substr(1);
    if (a[a.size() - 1] != '/' && b[0] != '/')
        return a + "/" + b;

    return a + b;
}

// ========== is directory ==========

bool Client::is_directory(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0)
        return false;
    return S_ISDIR(info.st_mode);
}

// ========= file exist ==========

bool Client::file_exists(const std::string& path) {
    struct stat info;
    return (stat(path.c_str(), &info) == 0 && S_ISREG(info.st_mode));
}

// ========== has trailing slash ==========
// bool Client::has_trailing_slash(const std::string& path) {
// 	return !path.empty() && path[path.size() - 1] == '/';
// }
// ========== end location ==========

bool Client::is_resolved()
{
	return _isResolved;
}

void Client::set_resolved(bool resolved)
{
	_isResolved = resolved;
}
