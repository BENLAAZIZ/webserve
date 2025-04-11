

#include "../include/web.h"

Client::Client() : request_Header_Complete(false), 
							_responseSent(false), 
								_keepAlive(false),
								_header_falg(false),
								  _isopen(false),
								  _fileOffset(0) {
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
	std::cout << "Header: ||" << key << "|| = ||" << value << "||" << std::endl;
	return true;
}

void Client::end_of_headers(std::string& line, int *flag)
{
	if (line.empty()) {
		if (_request.getHeaders().find("host") == _request.getHeaders().end()) {
			std::cout << "Host header not found" << std::endl;
			_request.set_status_code(400);
			*flag = 0;
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
				_keepAlive = false;
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
	_request.reset();
	_response.reset();
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