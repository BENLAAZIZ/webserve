

#include "../include/web.h"

Client::Client() : request_Header_Complete(false), _responseSent(false), _keepAlive(false) {
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
		// _response = other._response;
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
	// Check for end of headers ("\r\n\r\n")
	size_t headerEndPos = line_buf.find("\r\n\r\n");
	if (headerEndPos != std::string::npos) {
		this->request_Header_Complete = true;
	}
	// Process data line by line
	while (1) {
		size_t lineEnd = line_buf.find("\r\n");
		if (lineEnd == std::string::npos) {
			break; // Wait for more data
		}
		std::string line = line_buf.substr(0, lineEnd);
		line_buf.erase(0, lineEnd + 2); // Remove processed line
		if (_request.getMethod().empty() || _request.getpath().empty() || _request.getVersion().empty()) {
			if (!_request.parseFirstLine(line)) {
				std::cerr << "Error parsing first line" << std::endl;
				return false;
			}
		}
		else 
		{
			int flag = 0;
			end_of_headers(line, &flag);
			if (flag == 0)
				return false;
			else if (flag == 1)
				break;
			if (generate_header_map(line) == false)
				return false;
		}	
	}
	return true;
}

// ------------------------
bool Client::generate_header_map(std::string& line)
{
	size_t colonPos = line.find(":");
	if (colonPos == std::string::npos || colonPos == 0 || line[colonPos - 1] == ' ') {
		_request.statusCode.code = 400;
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
	value.erase(0, value.find_first_not_of(" ")); // Trim leading spaces
	_request.setHeader(key, value);
	std::cout << "Header: ||" << key << "|| = ||" << value << "||" << std::endl;
	return true;
}
// ------------------------

// ========================
void Client::end_of_headers(std::string& line, int *flag)
{
	if (line.empty()) {
		if (_request.getHeaders().find("host") == _request.getHeaders().end()) {
			_request.statusCode.code = 400;
			return ;
		}
		if (_request.getMethod() == "POST")
		{
			if (_request.getHeaders().find("Content-Length") == _request.getHeaders().end())
			{
				_request.statusCode.code = 411;
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
// ========================

void Client::generateResponse_GET_DELETE() {
	// Route request based on method and URI
	if (_request.getMethod() == "GET") {
		_request.initializeEncode();
		handleGetRequest();
	}
	else if (_request.getMethod() == "DELETE") {
		handleDeleteRequest();
	}
}

void Client::handleGetRequest() {
	// Remove query parameters
	std::string path = _request.getpath();
	std::cout << "path: " << path << std::endl;

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
		std::cerr << "Directory traversal attempt: " << path << std::endl;
		sendErrorResponse(403, "Forbidden");
		return;
	}
	
	// Prepend document root from config
	// std::string fullPath = _serverConfig.getRoot() + path;
	std::string fullPath = "/Users/hben-laz/Desktop/webserve/web_merge/www" + path;
	
	std::cout << "fullPath: " << fullPath << std::endl;

	// Check if file exists
	struct stat fileStat;
	if (stat(fullPath.c_str(), &fileStat) == 0 && S_ISREG(fileStat.st_mode)) {
		// File exists, serve it
		std::ifstream file(fullPath, std::ios::binary);
		if (file) {
			// Determine content type based on file extension
			std::string extension = getExtension(path);
			
			// Read file content
			std::string fileContent((std::istreambuf_iterator<char>(file)),
									 std::istreambuf_iterator<char>());
			
			// Generate HTTP response
			std::ostringstream response;
			response << "HTTP/1.1 200 OK\r\n";
			response << "Content-Type: " << extension << "\r\n";
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


void Client::handleDeleteRequest() {
	// Simple DELETE handler
	// Generate response with method and URI

	std::ostringstream response;
	response << "HTTP/1.1 200 OK\r\n";
	response << "Content-Type: text/plain\r\n";

	// sendErrorResponse(501, "Method Not Implemented");
}

std::string Client::getExtension(const std::string& path) {
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

bool Client::isDoneWithResponse() const {
	return _responseSent;
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
	// Keep the socket and address intact
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