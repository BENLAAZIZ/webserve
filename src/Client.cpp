

#include "../include/web.h"

Client::Client() : _lastActive(time(NULL)), 
				endOfResponse(false),
				request_Header_Complete(false), 
							_responseSent(false), 
								_keepAlive(false),
								_header_falg(false),
								  _isopen(false),
								  _fileOffset(0) {
									full_path = "";
}


void Client::updateActivityTime() {
	_lastActive = time(NULL); // Store in seconds
}

time_t Client::getLastActivityTime() const {
	return _lastActive; // Return in seconds
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

bool Client::parse_Header_Request(Server_holder& serv_hldr) 
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
	if (_request.getContentLength() > serv_hldr.client_max_body_size)
	{	
		_request.set_status_code(413);
		this->request_Header_Complete = true;
		this->_request.endOfRequest = true;
		return false;
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
					_request.boundary += "--";
					_request.boundary += _request.getHeader("Content-Type").substr(boundary_pos + 9);
					_request.setBoundary(_request.boundary);
					_request.boundary_end = _request.boundary + "--";
					_request.setContent_type("multipart/form-data");
				}
				else
					_request.setContent_type(_request.getHeader("Content-Type"));
			}
		}
		if (_request.getHeaders().find("Connection") != _request.getHeaders().end())
		{
			if (_request.getHeader("Connection") == "keep-alive")
			{
				_keepAlive = true;

				_request.keepAlive = true;
			}
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


	if (_request.getContentLength() == 0) {
		_request.set_status_code(400);
		_request.endOfRequest = true;
		return;
	}

    
    if (contentType == "application/x-www-form-urlencoded") {
		if (isChunked) {
			_request.handleChunkedData(_request);
		} else {
			while (1)
			{	
				if (_request._requestBuffer.empty())
					break;
				_request.strCGI += _request._requestBuffer;
				_request._requestBuffer.clear();
				if (_request.strCGI.size() >= _request.getContentLength())
				{
					_request.query = _request.strCGI;
					_request.endOfRequest = true;
					return ;
				}
			}
				
		}
    }
    if (contentType == "multipart/form-data") {
        if (isChunked) {
			_request.handleChunkedBoundary(_request);
        } else {
			_request.handleBoundary(_request);
        }
    }
    else if (isChunked) {
		_request.handleChunkedData(_request);
    }
    else {
		_request.handleOtherTypes(_request);
    }
}

int Client::read_data()
{
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

// ====== location ======

std::vector<Location*> get_matching_locations(const std::string& url, std::vector<Location>& locations) {
    std::vector<Location*> matches;

    for (std::vector<Location>::iterator it = locations.begin(); it != locations.end(); ++it) {
        if (url.find(it->path) == 0) { // location path is a prefix of URL
            matches.push_back(&(*it));
        }
    }
    return matches;
}
Location* get_best_location_match(const std::string& url, std::vector<Location>& locations) {
    std::vector<Location*> matches = get_matching_locations(url, locations);

    Location* best = NULL;
    size_t max_len = 0;

    for (size_t i = 0; i < matches.size(); ++i) {
        if (matches[i]->path.length() > max_len) {
            max_len = matches[i]->path.length();
            best = matches[i];
        }
    }
    return best;
}

bool hasExtension(const std::vector<std::string>& extensions, const std::string& ext) {
    return std::find(extensions.begin(), extensions.end(), ext) != extensions.end();
}

int  Client::resolve_request_path(Server_holder & serv_hldr) {

	Location* loc = get_best_location_match(_request.getpath(), serv_hldr.locations);

	std::string root = loc ? loc->root : serv_hldr.root;
	std::string location_path = loc ? loc->path : "/";
	std::string getpath = _request.getpath();
	_request.set_fake_path(getpath);

	if (loc && loc->root.empty() && serv_hldr.root.empty()) 
		return (_request.set_status_code(500), 500);
	if (loc == NULL) {
		full_path = join_paths(_request.my_root, join_paths(serv_hldr.root, getpath));
		if (is_directory(full_path))
			full_path = join_paths(full_path, "index.html");
		if (file_exists(full_path)) 
			return (_request.setPath(full_path), full_path.clear(), _request.set_status_code(200), 200);
		else
			return (_request.set_status_code(404), 404);
	}

	std::string relative_path = getpath;
	if (loc && getpath.find(location_path) == 0)
		relative_path = getpath.substr(location_path.length());

	if (!relative_path.empty() && relative_path[0] == '/')
		relative_path = relative_path.substr(1);
	//  DUPLICATE CHECK
	std::string lastRootSegment = root.substr(root.find_last_of('/') + 1);
	if (relative_path == lastRootSegment)
		relative_path.clear();
	if (relative_path.find(lastRootSegment + "/") == 0) {
		relative_path = relative_path.substr(lastRootSegment.length() + 1);
	}
	// Now use the correct root
	if (loc && root.empty())
	{
		root = serv_hldr.root;
		full_path = join_paths(_request.my_root, join_paths(root, getpath));
	}
	else
		full_path = join_paths(_request.my_root, join_paths(root, relative_path));
	// redirect path
	if (loc && loc->redirect_code != 0) 
	{
		if (loc->redirect_url.empty()) 
			return (_request.set_status_code(400), 400);
		return (_request.setPath(loc->redirect_url), _request.set_status_code(loc->redirect_code), loc->redirect_code);
	}
	// check allowed methods
	if (loc && loc->allowed_methods.size() > 0)
	{
		std::string method = _request.getMethod();
		if (std::find(loc->allowed_methods.begin(), loc->allowed_methods.end(), method) == loc->allowed_methods.end())
			return (_request.set_status_code(405), 405);
	}

	if (_request.isCGI) {
		if (!hasExtension(loc->cgi_extension, _request.getExtension())) {
			_request.set_status_code(415);
		}
		return (_request.setPath(full_path), 200);
	}
	// Check if full_path is a directory or a file
	if (is_directory(full_path)) 
	{
		if (_request.getMethod() == "DELETE")
			return (_request.set_status_code(403), 403);
		if (loc && !loc->index.empty()) {
            for (size_t i = 0; i < loc->index.size(); ++i) {
                std::string index_path = join_paths(full_path, loc->index[i]);
                if (file_exists(index_path))
                    return (_request.setPath(index_path), full_path.clear(), _request.set_status_code(200), 200);
            }
            return (_request.set_status_code(404), 404);
        }
		else if (loc && loc->autoindex)
			return (_request.set_status_code(200), _request.setPath(full_path), full_path.clear(), 200);
		return (_request.set_status_code(403), 403);
	}
	else if (file_exists(full_path))
	{
		return (_request.setPath(full_path), full_path.clear(), _request.set_status_code(200), 200);
	}
	return (_request.set_status_code(404), 404);
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

// ========== end location ==========

bool Client::is_resolved()
{
	return _isResolved;
}

void Client::set_resolved(bool resolved)
{
	_isResolved = resolved;
}



bool Client::handleDeleteResponse()
{
	std::string targetPath = _request.getpath();

	if (_request.getStatusCode() >= 400) {
		return true;
	}	
	if (access(targetPath.c_str(), W_OK) != 0) {
		_request.set_status_code(403);
		return true;
	}

	if (remove(targetPath.c_str()) != 0) {
		_request.set_status_code(500);  // Internal Server Error
		return true;
	}
	_request.setMethod("GET");
	std::string fullPath;
	fullPath = join_paths(_request.my_root, "docs/delete/suc.html");
	_request.setPath(fullPath);

	_request.set_status_code(204);
	return false;
}